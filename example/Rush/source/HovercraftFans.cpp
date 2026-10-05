//
//  HovercraftFans.cpp
//  Rush
//
//  See HovercraftFans.h.
//
#include <algorithm>
#include <cmath>
#include <HovercraftFans.h>
#include <Hovercraft.h>

using namespace Square;

SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftFans);

void HovercraftFans::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftFans>()
	ctx.add_object<HovercraftFans>();
}

HovercraftFans::HovercraftFans(Context& context) : Component(context)
{
}

void HovercraftFans::find_fans()
{
	m_found = true;
	m_fans.clear();
	auto hovercraft = actor().lock();
	if (!hovercraft) return;
	hovercraft->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (node->name().rfind("fan_blades", 0) == 0) m_fans.push_back({ node, node->rotation() });
		return true;
	});
}

void HovercraftFans::on_update(double delta_time)
{
	if (!m_found) find_fans();
	if (m_fans.empty()) return;
	//faster with the speed of the driver (none: idle)
	auto hovercraft = actor().lock();
	float share = 0.0f;
	if (hovercraft && hovercraft->contains<HovercraftDriver>())
	{
		auto driver = hovercraft->component<HovercraftDriver>();
		const float top = std::max(driver->settings().max_speed, 0.0001f);
		share = std::min(std::abs(driver->speed()) / top, 1.0f);
	}
	const float speed = m_settings.idle + (m_settings.full - m_settings.idle) * share;
	m_angle = std::fmod(m_angle + float(delta_time) * speed, 2.0f * Constants::pi<float>());
	//around the axis of the hull (in the space of the hovercraft), from their rotation of the model
	const Quat spin = angle_axis(m_angle, Constants::axis_z);
	for (const Fan& fan : m_fans)
	{
		if (auto node = fan.m_node.lock()) node->rotation(spin * fan.m_rest);
	}
}

void HovercraftFans::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftFans::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftFans::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftFans::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
