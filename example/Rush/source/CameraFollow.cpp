//
//  CameraFollow.cpp
//  Rush
//
#include <CameraFollow.h>
#include "Collision.h"
#include <algorithm>
#include <cmath>

using namespace Square;

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
	//from where it is now, it glides behind the target
	m_intro = target ? 0.0f : -1.0f;
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
	camera->rotation(look_at_target());
	m_intro = -1.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame
void CameraFollow::on_update(double delta_time)
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	if (!camera || !target) return;
	//steps of this frame (at the start a long frame, the end of the loading, counts at most 1)
	const float steps = std::clamp(float(delta_time / m_settings.step), 0.0f, m_intro >= 0.0f ? 1.0f : 4.0f);
	const Vec3  body = target->position(true);
	const Quat  rotation = target->rotation(true);
	//moving backward: the target went against its forward (+z)
	const bool backward = dot(body - m_target_previous, rotation * Constants::axis_z) < 0.0f;
	m_target_previous = body;
	if (steps <= 0.0f) return;
	//share of the way per step: at the start from intro_follow up to follow (smoothstep)
	float follow = m_settings.follow;
	if (m_intro >= 0.0f)
	{
		m_intro += float(std::min(delta_time, m_settings.step));
		const float t = m_settings.intro_time > 0.0f ? std::min(m_intro / m_settings.intro_time, 1.0f) : 1.0f;
		follow = m_settings.intro_follow + (m_settings.follow - m_settings.intro_follow) * (t * t * (3.0f - 2.0f * t));
		if (t >= 1.0f) m_intro = -1.0f;
	}
	//towards the pivot
	if (!(m_settings.hold_backward && backward))
	{
		const Vec3 pivot = body + rotation * m_settings.offset;
		Vec3 position = camera->position();
		position += (pivot - position) * (1.0f - std::pow(1.0f - follow, steps));
		camera->position(position);
	}
	//looks at the target
	camera->rotation(look_at_target());
}

Quat CameraFollow::look_at_target() const
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	//PointEntity camera,target (no roll)
	const Vec3 direction = target->position(true) - camera->position();
	const float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	if (horizontal < 1e-5f && std::abs(direction.y) < 1e-5f) return camera->rotation();
	return angle_axis(std::atan2(direction.x, direction.z), Constants::axis_y) * angle_axis(-std::atan2(direction.y, horizontal), Constants::axis_x);
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void CameraFollow::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void CameraFollow::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void CameraFollow::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void CameraFollow::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
