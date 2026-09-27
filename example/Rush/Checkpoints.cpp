//
//  Checkpoints.cpp
//  Rush
//
#include "Checkpoints.h"
#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace Square;

namespace
{
	const Vec3 AXIS_Y(0.0f, 1.0f, 0.0f);

	//first node named name under root (root included)
	Shared<Scene::Actor> find(Shared<Scene::Actor> root, const std::string& name)
	{
		Shared<Scene::Actor> found;
		root->visit([&](Shared<Scene::Actor> node) -> bool
		{
			if (node->name() != name) return true;
			found = node;
			return false;
		});
		return found;
	}
}

SQUARE_CLASS_OBJECT_REGISTRATION(Checkpoints);

void Checkpoints::object_registration(Context& ctx)
{
	//factory: actor->component<Checkpoints>()
	ctx.add_object<Checkpoints>();
}

Checkpoints::Checkpoints(Context& context) : Component(context)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
//checkpoints
size_t Checkpoints::collect(Shared<Scene::Actor> root)
{
	m_points.clear();
	if (!root) return 0;
	//"<prefix><n>": sorted by n
	std::vector<std::pair<int, Vec3>> numbered;
	root->visit([&](Shared<Scene::Actor> node) -> bool
	{
		const std::string& name = node->name();
		if (name.size() > m_settings.prefix.size() && name.compare(0, m_settings.prefix.size(), m_settings.prefix) == 0)
		{
			numbered.push_back({ std::atoi(name.c_str() + m_settings.prefix.size()), node->position(true) });
		}
		return true;
	});
	std::sort(numbered.begin(), numbered.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
	//on the ground under them (they can be a little over it)
	auto collision = world().lock() ? world().lock()->instance<CollisionWorld>() : nullptr;
	for (auto& point : numbered)
	{
		Vec3 position = point.second;
		CollisionMesh::Hit hit;
		if (collision && collision->raycast(position, -AXIS_Y, 1000.0f, hit)) position = hit.m_point;
		m_points.push_back(position);
	}
	m_current = NONE;
	go_to(random_next());
	return m_points.size();
}

size_t Checkpoints::random_next() const
{
	const size_t count = m_points.size();
	if (count == 0) return NONE;
	//no current one: any of them
	if (m_current == NONE || m_current >= count) return std::uniform_int_distribution<size_t>(0, count - 1)(m_random);
	//only one: it is the current one
	if (count == 1) return 0;
	//one of the others: skip the current one
	const size_t index = std::uniform_int_distribution<size_t>(0, count - 2)(m_random);
	return index >= m_current ? index + 1 : index;
}

void Checkpoints::go_to(size_t index)
{
	if (m_points.empty() || index == NONE) return;
	m_current = index % m_points.size();
	if (auto beam = actor().lock()) beam->position(m_points[m_current]);
}

bool Checkpoints::touched() const
{
	auto target = m_target.lock();
	auto beam = actor().lock();
	if (!target || !beam || m_points.empty()) return false;
	const Vec3 base = beam->position(true);
	const Vec3 delta = target->position(true) - base;
	return delta.x * delta.x + delta.z * delta.z <= m_settings.radius * m_settings.radius
		&& delta.y >= -m_settings.radius && delta.y <= m_settings.height;
}

//////////////////////////////////////////////////////////////////////////////////////////
//animation
bool Checkpoints::set_parts()
{
	if (m_inner || m_outer) return true;
	auto beam = actor().lock();
	if (!beam) return false;
	m_inner = find(beam, m_settings.inner);
	m_outer = find(beam, m_settings.outer);
	if (m_inner) m_inner_rest = m_inner->rotation();
	if (m_outer) m_outer_rest = m_outer->rotation();
	return m_inner || m_outer;
}

void Checkpoints::spin(float seconds)
{
	if (!set_parts()) return;
	m_time = std::fmod(m_time + seconds, 3600.0f); //seconds of spinning, kept small
	if (m_inner) m_inner->rotation(m_inner_rest * angle_axis(radians(std::fmod(m_time * m_settings.inner_speed, 360.0f)), AXIS_Y));
	if (m_outer) m_outer->rotation(m_outer_rest * angle_axis(radians(std::fmod(m_time * m_settings.outer_speed, 360.0f)), AXIS_Y));
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame
void Checkpoints::on_update(double delta_time)
{
	spin(float(delta_time));
	if (touched())
	{
		if (m_on_reached) m_on_reached(m_current);
		go_to(random_next());
	}
}

void Checkpoints::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void Checkpoints::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void Checkpoints::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void Checkpoints::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
