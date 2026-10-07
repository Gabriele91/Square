//
//  HovercraftAI.cpp
//  Rush
//
#include <HovercraftAI.h>
#include <Course.h>
#include <Hovercraft.h>
#include <Checkpoints.h>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace Square;

SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftAI);

namespace AuxHovercraftAI
{
	//distance on x/z
	static float flat_distance(const Vec3& a, const Vec3& b)
	{
		return length(Vec2(a.x - b.x, a.z - b.z));
	}

	//the nearest point of the segment a-b to p (x/z), its share along it
	static float project(const Vec3& a, const Vec3& b, const Vec3& p)
	{
		const Vec2 ab(b.x - a.x, b.z - a.z);
		const float length2 = dot(ab, ab);
		if (length2 < 1e-6f) return 0.0f;
		return std::clamp(dot(Vec2(p.x - a.x, p.z - a.z), ab) / length2, 0.0f, 1.0f);
	}
}

void HovercraftAI::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftAI>()
	ctx.add_object<HovercraftAI>();
}

HovercraftAI::HovercraftAI(Context& context) : Component(context)
{
}

void HovercraftAI::race(Shared<Checkpoints> checkpoints, Shared<Navigation::NavGrid> navigation)
{
	m_checkpoints = checkpoints;
	m_navigation = navigation;
	m_path.clear();
	m_path_checkpoint = size_t(-1);
}

void HovercraftAI::follow(const Course& course, float lane, float courage)
{
	m_lines.clear();
	m_lines.push_back({ course.guide(), course.closed() });
	for (const auto& route : course.routes()) m_lines.push_back({ route, false });
	m_decided.assign(m_lines.size(), false);
	m_lane = lane;
	m_courage = courage;
	m_line = 0;
	m_line_segment = 0;
	//its segment: the nearest one to where it is now
	if (auto hovercraft = actor().lock()) m_line_segment = nearest_segment(0, hovercraft->position(true));
}

size_t HovercraftAI::nearest_segment(size_t line, const Vec3& position) const
{
	const auto& points = m_lines[line].m_points;
	const size_t segments = m_lines[line].m_closed ? points.size() : points.size() - 1;
	float best = std::numeric_limits<float>::max();
	size_t nearest = 0;
	for (size_t i = 0; i != segments; ++i)
	{
		const Vec3& a = points[i];
		const Vec3& b = points[(i + 1) % points.size()];
		const float t = AuxHovercraftAI::project(a, b, position);
		const float d = AuxHovercraftAI::flat_distance(position, a + (b - a) * t);
		if (d < best) { best = d; nearest = i; }
	}
	return nearest;
}

void HovercraftAI::choose_way(const Vec3& position, const Vec3& forward)
{
	using namespace AuxHovercraftAI;
	if (m_line == 0)
	{
		//the other ways: the one starting near it, ahead of it, once (again when far from it)
		for (size_t way = 1; way != m_lines.size(); ++way)
		{
			const auto& points = m_lines[way].m_points;
			const float distance = flat_distance(position, points[0]);
			if (distance > 60.0f)
			{
				m_decided[way] = false;
				continue;
			}
			const Vec3 along = points[1] - points[0];
			if (m_decided[way] || distance > 25.0f || forward.x * along.x + forward.z * along.z <= 0.0f) continue;
			m_decided[way] = true;
			if (std::uniform_real_distribution<float>(0.0f, 1.0f)(m_random) >= m_courage) continue;
			m_line = way;
			m_line_segment = 0;
			return;
		}
		return;
	}
	//the end of the way: back on the guide
	const auto& points = m_lines[m_line].m_points;
	if (m_line_segment + 2 >= points.size() && flat_distance(position, points.back()) < 15.0f)
	{
		m_line = 0;
		m_line_segment = nearest_segment(0, position);
	}
}

Shared<HovercraftDriver> HovercraftAI::driver() const
{
	auto hovercraft = actor().lock();
	return hovercraft && hovercraft->contains<HovercraftDriver>() ? hovercraft->component<HovercraftDriver>() : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
//where it goes
Vec3 HovercraftAI::target(const Vec3& position, size_t checkpoint, const Vec3& goal, double delta_time)
{
	using namespace AuxHovercraftAI;
	auto navigation = m_navigation.lock();
	if (!navigation) return goal;
	//where it is on its path: the nearest point of its segment or of the ones after it
	float along = 0.0f;
	float off = 0.0f;
	if (m_path.size() >= 2)
	{
		float best = std::numeric_limits<float>::max();
		for (size_t i = m_segment; i + 1 < m_path.size(); ++i)
		{
			const float t = project(m_path[i], m_path[i + 1], position);
			const float d = flat_distance(position, m_path[i] + (m_path[i + 1] - m_path[i]) * t);
			if (d < best) { best = d; m_segment = i; along = t; }
		}
		off = best;
	}
	//a new path: another checkpoint, its time, none, off it
	m_replan -= float(delta_time);
	const bool other_checkpoint = checkpoint != m_path_checkpoint;
	if (other_checkpoint || m_replan <= 0.0f || m_path.size() < 2 || off > m_settings.off_path)
	{
		m_path_checkpoint = checkpoint;
		m_replan = m_settings.replan;
		m_segment = 0;
		along = 0.0f;
		if (!navigation->find_path(position, goal, m_path)) m_path.clear();
	}
	if (m_path.size() < 2) return goal;
	//pure pursuit: lookahead further along the path from its nearest point
	float left = m_settings.lookahead;
	size_t i = m_segment;
	Vec3 point = m_path[i] + (m_path[i + 1] - m_path[i]) * along;
	while (i + 1 < m_path.size())
	{
		const float rest = flat_distance(point, m_path[i + 1]);
		if (rest >= left) return point + (m_path[i + 1] - point) * (left / std::max(rest, 1e-4f));
		left -= rest;
		point = m_path[++i];
	}
	return goal;
}

Vec3 HovercraftAI::line_target(const Vec3& position, float ahead)
{
	using namespace AuxHovercraftAI;
	const Line& line = m_lines[m_line];
	const auto& points = line.m_points;
	const size_t count = points.size();
	const size_t segments = line.m_closed ? count : count - 1;
	//its segment: the nearest one, from a little behind to a few ahead of the one it was on
	float along = 0.0f;
	float best = std::numeric_limits<float>::max();
	size_t nearest = m_line_segment;
	for (size_t step = 0; step != 8; ++step)
	{
		const size_t i = line.m_closed ? (m_line_segment + segments - 2 + step) % segments
		                               : std::min(m_line_segment - std::min<size_t>(m_line_segment, 2) + step, segments - 1);
		const Vec3& a = points[i];
		const Vec3& b = points[(i + 1) % count];
		const float t = project(a, b, position);
		const float d = flat_distance(position, a + (b - a) * t);
		if (d < best) { best = d; nearest = i; along = t; }
	}
	m_line_segment = nearest;
	//ahead along the line (past the end of an open one: on along its last segment), then in its
	//lane (on the right of the segment there)
	size_t i = nearest;
	Vec3 point = points[i] + (points[(i + 1) % count] - points[i]) * along;
	float left = ahead;
	for (size_t guard = 0; guard != count; ++guard)
	{
		const bool last = !line.m_closed && i + 1 >= segments;
		const Vec3& next = points[(i + 1) % count];
		const float rest = flat_distance(point, next);
		if (rest >= left || last)
		{
			const Vec3 direction = next - points[i];
			const float span = std::max(length(Vec2(direction.x, direction.z)), 1e-4f);
			point = rest >= left ? point + (next - point) * (left / std::max(rest, 1e-4f)) : next + direction * ((left - rest) / span);
			break;
		}
		left -= rest;
		point = next;
		i = (i + 1) % count;
	}
	const Vec3 direction = points[(i + 1) % count] - points[i];
	const Vec2 flat = length(Vec2(direction.x, direction.z)) > 1e-4f ? normalize(Vec2(direction.x, direction.z)) : Vec2(0.0f, 1.0f);
	return point + Vec3(-flat.y, 0.0f, flat.x) * m_lane;
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame: towards the checkpoint
void HovercraftAI::on_update(double delta_time)
{
	auto hovercraft = actor().lock();
	auto hovercraft_driver = driver();
	auto race = m_checkpoints.lock();
	if (!hovercraft || !hovercraft_driver) return;
	HovercraftDriver::Input controls;
	//where it goes: along the line of the circuit, else to the checkpoint (none: still)
	const Vec3 position = hovercraft->position(true);
	const bool circuit = !m_lines.empty() && m_lines[0].m_points.size() >= 2;
	const bool arena = race && race->size() && race->current() != Checkpoints::NONE;
	if (circuit || arena)
	{
		//the circuit: further ahead the faster (its speed per step, 60 steps a second)
		const float ahead = std::max(m_settings.lookahead, std::abs(hovercraft_driver->speed()) * 60.0f * 0.45f);
		//angle between the forward of the hovercraft and where it goes, on x/z (degrees,
		//positive: on the right, +x)
		const Vec3  forward  = hovercraft->rotation(true) * Constants::axis_z;
		if (circuit) choose_way(position, forward);
		const Vec3 goal = circuit ? line_target(position, ahead) : target(position, race->current(), race->point(race->current()), delta_time);
		const Vec3  to_target = goal - position;
		float angle = degrees(std::atan2(to_target.x, to_target.z) - std::atan2(forward.x, forward.z));
		while (angle >  180.0f) angle -= 360.0f;
		while (angle < -180.0f) angle += 360.0f;
		//stuck: driving, still for stuck_time: back up for reverse_time
		const float top = hovercraft_driver->settings().max_speed;
		//(how it really moves: against a wall its throttle is on, it does not move)
		const Vec3  moved = hovercraft_driver->velocity();
		const bool  still = length(Vec2(moved.x, moved.z)) < top * m_settings.stuck_speed;
		m_stuck = still && m_reverse <= 0.0f ? m_stuck + float(delta_time) : 0.0f;
		if (m_stuck > m_settings.stuck_time)
		{
			m_reverse = m_settings.reverse_time;
			m_stuck = 0.0f;
		}
		//forward, or backing up a while (the yaw turns the hull either way: the nose toward
		//where it goes); atanfull(...) - yAng: left or right, straight within the dead zone
		const bool reversing = m_reverse > 0.0f;
		if (reversing) m_reverse -= float(delta_time);
		//a sharp turn: the throttle let go (it turns on the spot, not wide into a wall)
		controls.forward  = !reversing && std::abs(angle) < m_settings.sharp_turn;
		controls.backward = reversing;
		controls.right    = angle >  m_settings.dead_zone;
		controls.left     = angle < -m_settings.dead_zone;
	}
	hovercraft_driver->input(controls);
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftAI::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftAI::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftAI::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftAI::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
