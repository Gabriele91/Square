//
//  CameraFollow.cpp
//  Rush
//
#include "CameraFollow.h"
#include "Collision.h"
#include <algorithm>
#include <cmath>

using namespace Square;

namespace
{
	const Vec3 AXIS_X(1.0f, 0.0f, 0.0f);
	const Vec3 AXIS_Y(0.0f, 1.0f, 0.0f);
	const Vec3 AXIS_Z(0.0f, 0.0f, 1.0f);
}

SQUARE_CLASS_OBJECT_REGISTRATION(CameraFollow);

void CameraFollow::object_registration(Context& ctx)
{
	//factory: actor->component<CameraFollow>()
	ctx.add_object<CameraFollow>();
}

CameraFollow::CameraFollow(Context& context) : Component(context)
{
}

void CameraFollow::target(Shared<Scene::Actor> target)
{
	m_target = target;
	if (target) m_target_previous = target->position(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
//snap
void CameraFollow::snap()
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	if (!camera || !target) return;
	m_target_previous = target->position(true);
	camera->position(m_target_previous + target->rotation(true) * m_settings.offset);
	//a teleport also for the camera sphere, if it has one
	if (camera->contains<SphereCollider>()) camera->component<SphereCollider>()->reset();
	look_at_target();
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame
void CameraFollow::on_update(double delta_time)
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	if (!camera || !target) return;
	const float steps = std::clamp(float(delta_time / m_settings.step), 0.0f, 4.0f);
	const Vec3  body = target->position(true);
	const Quat  rotation = target->rotation(true);
	//moving backward: the target went against its forward (+z)
	const bool backward = dot(body - m_target_previous, rotation * AXIS_Z) < 0.0f;
	m_target_previous = body;
	//towards the pivot
	if (steps > 0.0f && !(m_settings.hold_backward && backward))
	{
		const Vec3 pivot = body + rotation * m_settings.offset;
		Vec3 position = camera->position();
		position += (pivot - position) * (1.0f - std::pow(1.0f - m_settings.follow, steps));
		camera->position(position);
	}
	look_at_target();
}

void CameraFollow::look_at_target()
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	//PointEntity camera,target (no roll)
	const Vec3 direction = target->position(true) - camera->position();
	const float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	if (horizontal < 1e-5f && std::abs(direction.y) < 1e-5f) return;
	camera->rotation(angle_axis(std::atan2(direction.x, direction.z), AXIS_Y) * angle_axis(-std::atan2(direction.y, horizontal), AXIS_X));
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void CameraFollow::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void CameraFollow::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void CameraFollow::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void CameraFollow::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
