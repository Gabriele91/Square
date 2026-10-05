//
//  Turntable.cpp
//  Rush
//
//  See Turntable.h.
//
#include <cmath>
#include <Turntable.h>

using namespace Square;

SQUARE_CLASS_OBJECT_REGISTRATION(Turntable);

void Turntable::object_registration(Context& ctx)
{
	//factory: actor->component<Turntable>()
	ctx.add_object<Turntable>();
	//attributes
	ctx.add_attribute_function<Turntable, float>
	("speed"
	, 20.0f
	, [](const Turntable* turntable) -> float { return turntable->speed(); }
	, [](Turntable* turntable, const float& speed) { turntable->speed(speed); });
}

Turntable::Turntable(Context& context) : Component(context)
{
}

void Turntable::yaw(float yaw)
{
	m_yaw = std::fmod(yaw, 360.0f);
	if (auto owner = actor().lock()) owner->rotation(angle_axis(radians(m_yaw), Constants::axis_y));
}

void Turntable::on_update(double delta_time)
{
	yaw(m_yaw + float(delta_time) * m_speed);
}

void Turntable::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void Turntable::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void Turntable::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void Turntable::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
