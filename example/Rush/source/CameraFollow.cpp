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
	m_arm = -1.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////
//snap
void CameraFollow::snap()
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	if (!camera || !target) return;
	m_target_previous = target->position(true);
	const Vec3 head = m_target_previous + Constants::axis_y * m_settings.head;
	m_arm = -1.0f;
	camera->position(arm_pivot(head, m_target_previous + target->rotation(true) * m_settings.offset, 1000.0f));
	//a teleport also for the camera sphere, if it has one (behind the walls of the map: it comes
	//in through them, one sided, at its next moves)
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
	//towards the pivot, on its arm (in front of the walls)
	if (!(m_settings.hold_backward && backward))
	{
		const Vec3 pivot = arm_pivot(body + Constants::axis_y * m_settings.head, body + rotation * m_settings.offset, steps);
		Vec3 position = camera->position();
		position += (pivot - position) * (1.0f - std::pow(1.0f - follow, steps));
		camera->position(position);
	}
	//turns toward the target, smoothly
	const float turn = 1.0f - std::pow(1.0f - m_settings.turn, steps);
	camera->rotation(slerp(camera->rotation(), look_at_target(), m_intro >= 0.0f ? 1.0f : turn));
}

Vec3 CameraFollow::arm_pivot(const Vec3& head, const Vec3& pivot, float steps)
{
	//the arm: from the head to the pivot; a mesh collider between, it ends in front of it
	const Vec3  arm = pivot - head;
	const float full = length(arm);
	if (full < 1e-4f) return pivot;
	const Vec3  direction = arm / full;
	float length_now = full;
	auto world = this->world().lock();
	auto collision = world ? world->instance<CollisionWorld>() : nullptr;
	if (collision)
	{
		//four rays parallel to the arm: the middle, its sides, over it; the longest free one
		const Vec3 side = length(cross(direction, Constants::axis_y)) > 1e-4f ? normalize(cross(direction, Constants::axis_y)) : Constants::axis_x;
		const Vec3 offsets[] = { Vec3(0.0f), side * m_settings.arm_spread, -side * m_settings.arm_spread, Constants::axis_y * m_settings.arm_spread };
		float longest = 0.0f;
		for (const Vec3& offset : offsets)
		{
			CollisionMesh::Hit hit;
			const float free = collision->raycast(head + offset, direction, full + m_settings.arm_margin, hit) ? hit.m_distance - m_settings.arm_margin : full;
			longest = std::max(longest, free);
		}
		length_now = std::clamp(longest, std::min(m_settings.arm_min, full), full);
	}
	//quickly in, slowly back out
	if (m_arm < 0.0f) m_arm = length_now;
	const float share = length_now < m_arm ? m_settings.arm_in : m_settings.arm_out;
	m_arm += (length_now - m_arm) * (1.0f - std::pow(1.0f - share, std::max(steps, 0.0f)));
	m_arm = std::min(m_arm, full);
	return head + direction * m_arm;
}

Quat CameraFollow::look_at_target() const
{
	auto camera = actor().lock();
	auto target = m_target.lock();
	//PointEntity camera,target (no roll); right over the target its yaw kept (no flips)
	const Vec3 direction = target->position(true) - camera->position();
	const float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	if (horizontal < 1e-5f && std::abs(direction.y) < 1e-5f) return camera->rotation();
	float yaw = std::atan2(direction.x, direction.z);
	if (horizontal < 1.5f) yaw = m_yaw;
	else const_cast<CameraFollow*>(this)->m_yaw = yaw;
	return angle_axis(yaw, Constants::axis_y) * angle_axis(-std::atan2(direction.y, std::max(horizontal, 0.5f)), Constants::axis_x);
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void CameraFollow::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void CameraFollow::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void CameraFollow::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void CameraFollow::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
