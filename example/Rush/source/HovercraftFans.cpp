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
	m_flames.clear();
	auto hovercraft = actor().lock();
	if (!hovercraft) return;
	hovercraft->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (node->name().rfind("fan_blades", 0) == 0) m_fans.push_back({ node, node->rotation() });
		if (node->name().rfind("fan_flame", 0) == 0) m_flames.push_back({ node, node->scale() });
		return true;
	});
}

void HovercraftFans::update_flames(float delta_time, bool boosting)
{
	//out fast, in slower
	const float rate = boosting ? delta_time / std::max(m_settings.flame_in, 0.001f) : -delta_time / std::max(m_settings.flame_out, 0.001f);
	m_flame = std::clamp(m_flame + rate, 0.0f, 1.0f);
	m_time += delta_time;
	for (size_t i = 0; i != m_flames.size(); ++i)
	{
		auto node = m_flames[i].m_node.lock();
		if (!node) continue;
		//(hidden: scaled to nothing)
		if (m_flame <= 0.0f) { node->scale(Vec3(0.0001f)); continue; }
		//the flicker: fast, each flame its own; longer than wide
		const float phase = float(i) * 2.3f;
		const float flicker = m_settings.flicker * (std::sin(m_time * 47.0f + phase) * 0.6f + std::sin(m_time * 29.0f + phase * 1.7f) * 0.4f);
		const float out = m_flame * m_flame * (3.0f - 2.0f * m_flame);
		const Vec3& rest = m_flames[i].m_rest;
		//(the axis of the hull: y of the model, z of its node)
		node->scale(Vec3(rest.x * out * (1.0f + flicker * 0.4f), rest.y * out * (1.0f + flicker * 0.4f), rest.z * out * (1.0f + flicker)));
	}
}

void HovercraftFans::on_update(double delta_time)
{
	if (!m_found) find_fans();
	//faster with the speed of the driver (none: idle); its flames while it boosts
	auto hovercraft = actor().lock();
	float share = 0.0f;
	bool  boosting = false;
	if (hovercraft && hovercraft->contains<HovercraftDriver>())
	{
		auto driver = hovercraft->component<HovercraftDriver>();
		const float top = std::max(driver->settings().max_speed, 0.0001f);
		share = std::min(std::abs(driver->speed()) / top, 1.0f);
		boosting = driver->boosting();
	}
	update_flames(float(delta_time), boosting);
	if (m_fans.empty()) return;
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
