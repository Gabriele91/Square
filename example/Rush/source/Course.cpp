//
//  Course.cpp
//  Rush
//
//  See Course.h.
//
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <Course.h>

using namespace Square;

namespace AuxCourse
{
	//distance on x/z
	static float flat_distance(const Vec3& a, const Vec3& b)
	{
		return length(Vec2(a.x - b.x, a.z - b.z));
	}

	//the share along the segment a-b of the nearest point to p (x/z)
	static float project(const Vec3& a, const Vec3& b, const Vec3& p)
	{
		const Vec2 ab(b.x - a.x, b.z - a.z);
		const float length2 = dot(ab, ab);
		if (length2 < 1e-6f) return 0.0f;
		return std::clamp(dot(Vec2(p.x - a.x, p.z - a.z), ab) / length2, 0.0f, 1.0f);
	}

	//the numbers of a name after its prefix: "cp_3_40" -> 3, 40
	static std::vector<std::string> fields(const std::string& name, size_t prefix)
	{
		std::vector<std::string> out;
		std::string field;
		for (size_t i = prefix; i <= name.size(); ++i)
		{
			if (i == name.size() || name[i] == '_' || name[i] == '.')
			{
				if (!field.empty()) out.push_back(field);
				field.clear();
				//"cp_3_40.001": a copy of Blender, its suffix not a field
				if (i < name.size() && name[i] == '.') break;
			}
			else
			{
				field += name[i];
			}
		}
		return out;
	}
}

bool Course::collect(Shared<Scene::Actor> root)
{
	using namespace AuxCourse;
	m_guide.clear();
	m_along.clear();
	m_checkpoints.clear();
	m_routes.clear();
	if (!root) return false;
	//the nodes by their numbers
	std::map<int, Vec3> guide;
	std::map<int, std::pair<Vec3, float>> checkpoints;
	std::map<std::string, std::map<int, Vec3>> routes;
	root->visit([&](Shared<Scene::Actor> node) -> bool
	{
		const std::string& name = node->name();
		if (name.rfind("guide_", 0) == 0)
		{
			const auto f = fields(name, 6);
			if (!f.empty()) guide[std::atoi(f[0].c_str())] = node->position(true);
		}
		else if (name.rfind("cp_", 0) == 0)
		{
			const auto f = fields(name, 3);
			if (f.size() >= 2) checkpoints[std::atoi(f[0].c_str())] = { node->position(true), float(std::atof(f[1].c_str())) };
		}
		else if (name.rfind("route_", 0) == 0)
		{
			const auto f = fields(name, 6);
			if (f.size() >= 2) routes[f[0]][std::atoi(f[1].c_str())] = node->position(true);
		}
		return true;
	});
	m_guide.reserve(guide.size());
	for (const auto& point : guide) m_guide.push_back(point.second);
	if (m_guide.size() < 2) return false;
	//a loop: its ends meet (nearer than a few of its steps)
	const float step = flat_distance(m_guide[0], m_guide[1]);
	m_closed = flat_distance(m_guide.front(), m_guide.back()) < step * 3.0f;
	m_along.resize(m_guide.size());
	m_along[0] = 0.0f;
	for (size_t i = 1; i != m_guide.size(); ++i) m_along[i] = m_along[i - 1] + flat_distance(m_guide[i - 1], m_guide[i]);
	m_length = m_along.back() + (m_closed ? flat_distance(m_guide.back(), m_guide.front()) : 0.0f);
	//the checkpoints along the guide
	for (const auto& checkpoint : checkpoints)
	{
		Checkpoint added;
		added.m_position = checkpoint.second.first;
		added.m_radius = checkpoint.second.second;
		size_t segment = 0;
		added.m_along = project(added.m_position, segment, true);
		added.m_direction = direction(added.m_along);
		m_checkpoints.push_back(added);
	}
	for (const auto& route : routes)
	{
		std::vector<Vec3> line;
		line.reserve(route.second.size());
		for (const auto& point : route.second) line.push_back(point.second);
		if (line.size() >= 2) m_routes.push_back(line);
	}
	return valid();
}

float Course::wrap(float along) const
{
	if (!m_closed) return std::clamp(along, 0.0f, m_length);
	along = std::fmod(along, m_length);
	return along < 0.0f ? along + m_length : along;
}

size_t Course::segment_at(float along) const
{
	along = wrap(along);
	const auto it = std::upper_bound(m_along.begin(), m_along.end(), along);
	const size_t i = it == m_along.begin() ? 0 : size_t(it - m_along.begin()) - 1;
	//the last segment of a line: its last two points (a loop: back to the first one)
	return m_closed ? i : std::min(i, m_guide.size() - 2);
}

float Course::project(const Vec3& position, size_t& segment, bool anywhere) const
{
	using namespace AuxCourse;
	const size_t count = m_guide.size();
	const size_t segments = m_closed ? count : count - 1;
	//around the segment: a few behind, more ahead (where it goes)
	size_t first = 0;
	if (!anywhere) first = m_closed ? (segment + segments - std::min<size_t>(4, segments)) % segments : (segment > 4 ? segment - 4 : 0);
	const size_t steps = anywhere ? segments : std::min<size_t>(16, segments);
	float best = std::numeric_limits<float>::max();
	float along = 0.0f;
	for (size_t step = 0; step != steps; ++step)
	{
		const size_t i = m_closed ? (first + step) % segments : std::min(first + step, segments - 1);
		const Vec3& a = m_guide[i];
		const Vec3& b = m_guide[(i + 1) % count];
		const float t = AuxCourse::project(a, b, position);
		const float d = flat_distance(position, a + (b - a) * t);
		if (d < best)
		{
			best = d;
			segment = i;
			along = m_along[i] + flat_distance(a, b) * t;
		}
	}
	return along;
}

Vec3 Course::point(float along) const
{
	along = wrap(along);
	const size_t i = segment_at(along);
	const Vec3& a = m_guide[i];
	const Vec3& b = m_guide[(i + 1) % m_guide.size()];
	const float span = std::max(AuxCourse::flat_distance(a, b), 1e-4f);
	return a + (b - a) * std::clamp((along - m_along[i]) / span, 0.0f, 1.0f);
}

Vec3 Course::direction(float along) const
{
	const size_t i = segment_at(along);
	const Vec3 across = m_guide[(i + 1) % m_guide.size()] - m_guide[i];
	const Vec2 flat(across.x, across.z);
	const Vec2 unit = glm::length(flat) > 1e-4f ? glm::normalize(flat) : Vec2(0.0f, 1.0f);
	return Vec3(unit.x, 0.0f, unit.y);
}

bool Course::crossed(size_t checkpoint, const Vec3& position) const
{
	const Checkpoint& at = m_checkpoints[checkpoint];
	const Vec3  offset = position - at.m_position;
	const float along = offset.x * at.m_direction.x + offset.z * at.m_direction.z;
	const float side = std::abs(offset.x * at.m_direction.z - offset.z * at.m_direction.x);
	//past its line, not far past it (the other side of a loop), within its radius
	return along >= 0.0f && along < 80.0f && side <= at.m_radius && std::abs(offset.y) < 60.0f;
}
