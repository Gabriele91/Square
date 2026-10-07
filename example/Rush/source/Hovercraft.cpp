//
//  Hovercraft.cpp
//  Rush
//
#include <Hovercraft.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Square;

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
#if 0
	//height probes: each one comes down from half a body height over its corner to it (it
	//stops on what is under it: a ramp, a step, not the top of a wall)
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		const Vec3 corner = position + rotation * m_wheel_offsets[wheel_id];
		m_wheels[wheel_id]->component<SphereCollider>()->reset(corner + Constants::axis_y * m_body->radius_y());
		m_wheels[wheel_id]->position(Vec3(to_hovercraft * Vec4(corner, 1.0f)));
	}
#else
	//height probes: a ray for each wheel, down over its corner from the highest corner (plus
	//half a body height: a level body over a small step) to the lowest one. The lower corner of
	//a body tilted a lot is under the ground (a probe from there would not see it). A wheel
	//touches when the ground is at its corner or over it; with at least two wheels off the
	//ground (the body on a side, on one wheel) a lifted wheel comes down on the ground found
	//down to the lowest corner, and the body goes back down; otherwise it stays at its corner
	//(in the air the tilt stays)
	auto collision = world().lock()->instance<CollisionWorld>();
	std::array<Vec3, 4> corners;
	std::array<float, 4> grounds;
	std::array<bool, 4> hits;
	float top = -std::numeric_limits<float>::max();
	float bottom = std::numeric_limits<float>::max();
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		corners[wheel_id] = position + rotation * m_wheel_offsets[wheel_id];
		top = std::max(top, corners[wheel_id].y);
		bottom = std::min(bottom, corners[wheel_id].y);
	}
	top += m_body->radius_y();
	int off_ground = 0;
	//the surfaces under the wheels (the one of most of them)
	std::array<int, size_t(Surface::COUNT)> surfaces{};
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		const float radius = m_wheels[wheel_id]->component<SphereCollider>()->radius();
		const Vec3 origin(corners[wheel_id].x, top, corners[wheel_id].z);
		CollisionMesh::Hit hit;
		hits[wheel_id] = collision && collision->raycast(origin, -Constants::axis_y, top - bottom + radius, hit);
		grounds[wheel_id] = hits[wheel_id] ? hit.m_point.y + radius : corners[wheel_id].y;
		if (!hits[wheel_id] || grounds[wheel_id] < corners[wheel_id].y) ++off_ground;
		if (hits[wheel_id]) ++surfaces[size_t(hit.m_surface)];
	}
	const auto most = std::max_element(surfaces.begin(), surfaces.end());
	if (*most > 0) m_surface = Surface(most - surfaces.begin());
	const bool lifted = off_ground == 3;
	for (int wheel_id = 0; wheel_id < 4; ++wheel_id)
	{
		Vec3 target = corners[wheel_id];
		if (hits[wheel_id] && (lifted || grounds[wheel_id] >= corners[wheel_id].y))
		{
			target.y = grounds[wheel_id];
		}
		//the collider of the wheel does not move (it shows where the probe is)
		m_wheels[wheel_id]->component<SphereCollider>()->reset(target);
		m_wheels[wheel_id]->position(Vec3(to_hovercraft * Vec4(target, 1.0f)));
	}
#endif
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
	hovercraft->rotation(angle_axis(radians(yaw), Constants::axis_y));
	//over the first surface under start: the body drops from its radius over it
	Vec3 position = start;
	CollisionMesh::Hit hit;
	auto collision = world().lock()->instance<CollisionWorld>();
	if (collision && collision->raycast(start, -Constants::axis_y, 10000.0f, hit))
	{
		position.y = hit.m_point.y + m_body->radius_y() * 2.0f - m_body->offset().y;
	}
	hovercraft->position(position);
	//a teleport: the next collisions of the body start from here
	m_body->reset();
	place_wheels();
	m_speed = 0.0f;
	m_velocity = Vec3(0.0f);
	m_drive = Vec3(0.0f);
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
	hovercraft->align_to_vector((wheels_position[FRONT_RIGHT] + wheels_position[BACK_RIGHT]) - (wheels_position[FRONT_LEFT] + wheels_position[BACK_LEFT]), Constants::axis_x);
	hovercraft->align_to_vector((wheels_position[FRONT_LEFT] + wheels_position[FRONT_RIGHT]) - (wheels_position[BACK_LEFT] + wheels_position[BACK_RIGHT]), Constants::axis_z);
	//velocity of the last step (where the collisions left it), per step
	const Vec3 position = hovercraft->position();
	if (m_has_previous)
	{
		m_velocity = (position - m_previous) / steps;
	}
	m_previous = position;
	m_has_previous = true;
}

void HovercraftDriver::boost(float seconds, float speed, float acceleration)
{
	m_boost = seconds;
	m_boost_speed = speed;
	m_boost_acceleration = acceleration;
	m_speed = std::max(m_speed, m_settings.max_speed * speed * 0.9f);
}

void HovercraftDriver::update_input(float steps)
{
	auto hovercraft = actor().lock();
	//steering: faster the faster it goes forward, at least idle_turn (left handed, +x right:
	//a positive angle around +y turns to the right)
	const Settings::Grip& grip = m_settings.grips[size_t(m_surface)];
	const float rate = std::max(m_speed > 0.0f ? m_speed * m_settings.turn : 0.0f, m_settings.idle_turn) * grip.turn * steps;
	const float yaw = (m_input.right ? rate : 0.0f) - (m_input.left ? rate : 0.0f);
	if (yaw != 0.0f)
	{
		hovercraft->rotation(hovercraft->rotation() * angle_axis(radians(yaw), Constants::axis_y));
	}
	//on the ground (a contact under the body, not a wall in front or over it): throttle/brake/
	//drag, forward along the body plus a little gravity (it stays pressed on the ground); in
	//the air or against a wall: no throttle, the velocity slowed by the air drag (also the
	//speed, it lands slower) plus gravity (it falls back)
	Vec3 velocity(0.0f);
	if (m_body->collided(m_settings.scene_type, m_settings.floor_normal_y))
	{
		//the ground under it: its acceleration, its top speed, its drag
		//(a boost: faster, quicker)
		const bool  boosted = m_boost > 0.0f;
		const float acceleration = m_settings.acceleration * grip.acceleration * (boosted ? m_boost_acceleration : 1.0f);
		const float max_speed = m_settings.max_speed * grip.max_speed * (boosted ? m_boost_speed : 1.0f);
		const float drag = std::min(m_settings.drag * grip.drag, 0.999f);
		if (m_input.forward || boosted) m_speed = std::min(m_speed + acceleration * steps, std::max(max_speed, m_speed * std::pow(drag, steps)));
		else if (m_input.backward) m_speed = std::max(m_speed - acceleration * steps, m_settings.max_reverse * grip.max_speed);
		else                       m_speed *= std::pow(drag, steps);
		//its velocity toward its nose, at once with a full grip, sliding with a low one
		const Vec3 nose = hovercraft->rotation() * (Constants::axis_z * m_speed);
		const float follow = 1.0f - std::pow(1.0f - std::clamp(grip.follow, 0.0f, 1.0f), steps);
		m_drive = m_drive + (Vec3(nose.x, 0.0f, nose.z) - m_drive) * follow;
		velocity = Vec3(m_drive.x, nose.y, m_drive.z) + Constants::axis_y * m_settings.gravity;
	}
	else
	{
		const float keep = std::pow(m_settings.air_drag, steps);
		m_speed *= keep;
		velocity = Vec3(m_velocity.x * keep, m_velocity.y + m_settings.gravity * steps, m_velocity.z * keep);
		//landing: it goes on the way it flew
		m_drive = Vec3(velocity.x, 0.0f, velocity.z);
	}
	hovercraft->position(hovercraft->position() + velocity * steps);
	m_boost = std::max(m_boost - float(steps * m_settings.step), 0.0f);
	//the wheels back under the body (where the hovercraft is now)
	place_wheels();
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftDriver::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftDriver::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftDriver::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftDriver::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
