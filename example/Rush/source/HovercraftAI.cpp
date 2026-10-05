//
//  HovercraftAI.cpp
//  Rush
//
#include <HovercraftAI.h>
#include <Hovercraft.h>
#include <Checkpoints.h>
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
	//a new path: another checkpoint, its time, none
	m_replan -= float(delta_time);
	const bool other_checkpoint = checkpoint != m_path_checkpoint;
	if (other_checkpoint || m_replan <= 0.0f || m_path.empty())
	{
		m_path_checkpoint = checkpoint;
		m_replan = m_settings.replan;
		m_next = 1;
		if (!navigation->find_path(position, goal, m_path)) m_path.clear();
	}
	if (m_path.size() < 2) return goal;
	//the next point of the path not reached yet (the last one: the checkpoint)
	while (m_next + 1 < m_path.size() && flat_distance(position, m_path[m_next]) < m_settings.reach) ++m_next;
	return m_next + 1 < m_path.size() ? m_path[m_next] : goal;
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
	//no race (no checkpoints): still
	if (race && race->size() && race->current() != Checkpoints::NONE)
	{
		//angle between the forward of the hovercraft and where it goes, on x/z (degrees,
		//positive: on the right, +x)
		const Vec3  position = hovercraft->position(true);
		const Vec3  forward  = hovercraft->rotation(true) * Constants::axis_z;
		const Vec3  to_target = target(position, race->current(), race->point(race->current()), delta_time) - position;
		float angle = degrees(std::atan2(to_target.x, to_target.z) - std::atan2(forward.x, forward.z));
		while (angle >  180.0f) angle -= 360.0f;
		while (angle < -180.0f) angle += 360.0f;
		//stuck: driving, still for stuck_time: back up for reverse_time
		const float top = hovercraft_driver->settings().max_speed;
		const bool  still = std::abs(hovercraft_driver->speed()) < top * m_settings.stuck_speed;
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
		controls.forward  = !reversing;
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
