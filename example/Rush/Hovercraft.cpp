//
//  Hovercraft.cpp
//  Rush
//
#include "Hovercraft.h"
#include <algorithm>
#include <cmath>

using namespace Square;

namespace
{
	const Vec3 AXIS_X(1.0f, 0.0f, 0.0f);
	const Vec3 AXIS_Y(0.0f, 1.0f, 0.0f);
	const Vec3 AXIS_Z(0.0f, 0.0f, 1.0f);
}

SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftDriver);

void HovercraftDriver::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftDriver>()
	ctx.add_object<HovercraftDriver>();
}

HovercraftDriver::HovercraftDriver(Context& context) : Component(context)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
//set wheels
bool HovercraftDriver::set_wheels()
{
	if (m_body) return true;
	auto hovercraft = actor().lock();
	if (!hovercraft || !world().lock()) return false;
	//hull bounds in body space (position and rotation of the actor, not its scale: the world
	//units of the colliders), from the vertices of its meshes
	CollisionMesh hull;
	hull.add(context(), hovercraft);
	const Mat4 body_space = inverse(glm::translate(Mat4(1.0f), hovercraft->position(true)) * to_mat4(hovercraft->rotation(true)));
	Vec3 bounds_min(-1.5f, -1.0f, -3.0f), bounds_max(1.5f, 1.0f, 3.0f); //no mesh: a hull of 3 x 2 x 6
	hull.bounds(body_space, bounds_min, bounds_max);
	const Vec3  size   = bounds_max - bounds_min;
	const Vec3  center = (bounds_min + bounds_max) * 0.5f;
	const float bottom = bounds_min.y;

	//the body: an ellipsoid on the ground, around the whole base (x/z: half the larger of width
	//and length) and half the height of the hull, each one scaled by body_radius_scale
	const float body_radius   = std::max(0.05f, std::max(size.x, size.z) * 0.5f * m_settings.body_radius_scale.x);
	const float body_radius_y = std::max(0.05f, size.y * 0.5f * m_settings.body_radius_scale.y);
	m_body = hovercraft->component<SphereCollider>();
	m_body->type(m_settings.body_type);
	m_body->radius(body_radius, body_radius_y);
	m_body->offset(Vec3(center.x, bottom + body_radius_y, center.z));

	//the wheels: four spheres of a quarter of the hull height, at the bottom of the body, in
	//from the sides by their radius, half way to the front and to the back
	const float wheel_radius = std::max(0.05f, size.y * 0.25f);
	const Vec3  wheel_spacing(std::max(0.0f, size.x * 0.5f - wheel_radius), 0.0f, size.z * 0.25f);
	int wheel_id = 0;
	for (float z : { 1.0f, -1.0f })
	for (float x : { -1.0f, 1.0f })
	{
		m_wheel_offsets[wheel_id] = Vec3(center.x + x * wheel_spacing.x, bottom, center.z + z * wheel_spacing.z);
		m_wheels[wheel_id] = hovercraft->child();
		m_wheels[wheel_id]->name(hovercraft->name() + "_wheel_" + std::to_string(wheel_id + 1));
		auto wheel = m_wheels[wheel_id]->component<SphereCollider>();
		wheel->type(m_settings.wheel_type);
		wheel->radius(wheel_radius);
		++wheel_id;
	}
	place_wheels();
	return true;
}

void HovercraftDriver::place_wheels()
{
	auto hovercraft = actor().lock();
	const Vec3 position = hovercraft->position(true);
	const Quat rotation = hovercraft->rotation(true);
	const Mat4 to_hovercraft = inverse(hovercraft->global_model_matrix());
	//height probes: each one comes down from half a body height over its corner to it (it
	//stops on what is under it: a ramp, a step, not the top of a wall)
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		const Vec3 corner = position + rotation * m_wheel_offsets[wheel_id];
		m_wheels[wheel_id]->component<SphereCollider>()->reset(corner + AXIS_Y * m_body->radius_y());
		m_wheels[wheel_id]->position(Vec3(to_hovercraft * Vec4(corner, 1.0f)));
	}
}

void HovercraftDriver::on_deattch()
{
	if (auto hovercraft = actor().lock())
	{
		for (auto& wheel : m_wheels)
		{
			if (wheel) hovercraft->remove(wheel);
		}
		if (m_body) hovercraft->remove(m_body);
	}
	m_body.reset();
	m_wheels = {};
}

//////////////////////////////////////////////////////////////////////////////////////////
//pose
HovercraftDriver::Pose HovercraftDriver::pose() const
{
	Pose current;
	if (auto hovercraft = actor().lock())
	{
		current.m_position = hovercraft->position();
		current.m_rotation = hovercraft->rotation();
	}
	return current;
}

void HovercraftDriver::pose(const Pose& pose)
{
	if (auto hovercraft = actor().lock())
	{
		hovercraft->position(pose.m_position);
		hovercraft->rotation(pose.m_rotation);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//spawn
void HovercraftDriver::spawn(const Vec3& start, float yaw)
{
	if (!set_wheels()) return;
	auto hovercraft = actor().lock();
	hovercraft->rotation(angle_axis(radians(yaw), AXIS_Y));
	//over the first surface under start: the body drops from its radius over it
	Vec3 position = start;
	CollisionMesh::Hit hit;
	auto collision = world().lock()->instance<CollisionWorld>();
	if (collision && collision->raycast(start, -AXIS_Y, 10000.0f, hit))
	{
		position.y = hit.m_point.y + m_body->radius_y() * 2.0f - m_body->offset().y;
	}
	hovercraft->position(position);
	//a teleport: the next collisions of the body start from here
	m_body->reset();
	place_wheels();
	m_speed = 0.0f;
	m_velocity = Vec3(0.0f);
	m_has_previous = false;
	//no pose to interpolate from
	m_pose_previous = m_pose_current = pose();
	m_has_pose = true;
	m_stepped = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
//frame and steps
void HovercraftDriver::on_update(double delta_time)
{
	//body and wheels as soon as it is in a world (the steps move it)
	set_wheels();
}

void HovercraftDriver::on_fixed_update(double step)
{
	if (!set_wheels()) return;
	//the pose of the simulation: after the collisions of the last step, or back from the
	//interpolated pose the frame showed
	if (m_stepped)        m_pose_current = pose();
	else if (m_has_pose)  pose(m_pose_current);
	else                  m_pose_current = pose();
	m_has_pose = true;
	m_pose_previous = m_pose_current;
	//steps of the values (1 when the fixed step is the one of the settings)
	const float steps = float(step / m_settings.step);
	update(steps);
	update_input(steps);
	m_stepped = true;
}

void HovercraftDriver::on_fixed_interpolate(double alpha)
{
	if (!m_has_pose) return;
	//the last step, after its collisions
	if (m_stepped)
	{
		m_pose_current = pose();
		m_stepped = false;
	}
	Pose shown;
	shown.m_position = glm::mix(m_pose_previous.m_position, m_pose_current.m_position, float(alpha));
	shown.m_rotation = glm::slerp(m_pose_previous.m_rotation, m_pose_current.m_rotation, float(alpha));
	pose(shown);
}

void HovercraftDriver::update(float steps)
{
	auto hovercraft = actor().lock();
	//align the hovercraft to the height difference of the four wheels: first left/right, then
	//front/rear
	std::array<Vec3, 4> wheels_position;
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		wheels_position[wheel_id] = m_wheels[wheel_id]->position(true);
	}
	hovercraft->align_to_vector((wheels_position[FRONT_RIGHT] + wheels_position[BACK_RIGHT]) - (wheels_position[FRONT_LEFT] + wheels_position[BACK_LEFT]), AXIS_X);
	hovercraft->align_to_vector((wheels_position[FRONT_LEFT] + wheels_position[FRONT_RIGHT]) - (wheels_position[BACK_LEFT] + wheels_position[BACK_RIGHT]), AXIS_Z);
	//velocity of the last step (where the collisions left it), per step
	const Vec3 position = hovercraft->position();
	if (m_has_previous)
	{
		m_velocity = (position - m_previous) / steps;
	}
	m_previous = position;
	m_has_previous = true;
}

void HovercraftDriver::update_input(float steps)
{
	auto hovercraft = actor().lock();
	//steering: faster the faster it goes forward, at least idle_turn (left handed, +x right:
	//a positive angle around +y turns to the right)
	const float rate = std::max(m_speed > 0.0f ? m_speed * m_settings.turn : 0.0f, m_settings.idle_turn) * steps;
	const float yaw = (m_input.right ? rate : 0.0f) - (m_input.left ? rate : 0.0f);
	if (yaw != 0.0f)
	{
		hovercraft->rotation(hovercraft->rotation() * angle_axis(radians(yaw), AXIS_Y));
	}
	//on the ground (a contact under the body, not a wall in front or over it): throttle/brake/
	//drag, forward along the body plus a little gravity (it stays pressed on the ground); in
	//the air or against a wall: the velocity plus gravity (it falls back)
	Vec3 velocity(0.0f);
	if (m_body->collided(m_settings.scene_type, m_settings.floor_normal_y))
	{
		if (m_input.forward)       m_speed = std::min(m_speed + m_settings.acceleration * steps, m_settings.max_speed);
		else if (m_input.backward) m_speed = std::max(m_speed - m_settings.acceleration * steps, m_settings.max_reverse);
		else                       m_speed *= std::pow(m_settings.drag, steps);
		velocity = hovercraft->rotation() * (AXIS_Z * m_speed) + AXIS_Y * m_settings.gravity;
	}
	else
	{
		velocity = m_velocity + AXIS_Y * (m_settings.gravity * steps);
	}
	hovercraft->position(hovercraft->position() + velocity * steps);
	//the wheels back under the body (where the hovercraft is now)
	place_wheels();
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftDriver::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftDriver::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftDriver::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftDriver::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
