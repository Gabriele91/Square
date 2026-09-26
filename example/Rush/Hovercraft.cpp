//
//  Hovercraft.cpp
//  Rush
//
#include "Hovercraft.h"
#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Square;

Hovercraft::Hovercraft(Context& context, Shared<Scene::Actor> actor, Shared<Scene::Actor> camera, const CollisionMesh& scene, const Settings& settings)
: m_actor(actor)
, m_camera(camera)
, m_scene(scene)
, m_settings(settings)
{
	//hull bounds in hull space, from the vertices of its meshes (the bounding box of a rotated
	//mesh, turned into hull space, is a lot bigger than the mesh)
	//hull space: position and rotation of the actor, not its scale (the simulation moves and
	//turns the hull, so its corners must already be scaled)
	CollisionMesh hull;
	hull.add(context, m_actor);
	const Mat4 hull_frame = glm::translate(Mat4(1.0f), m_actor->position(true)) * to_mat4(m_actor->rotation(true));
	Vec3 bounds_min, bounds_max;
	if (!hull.bounds(inverse(hull_frame), bounds_min, bounds_max))
	{
		//no mesh: a small box
		bounds_min = Vec3(-1.0f, 0.0f, -1.5f);
		bounds_max = Vec3(1.0f, 1.0f, 1.5f);
	}
	//the four bottom corners (forward is +z, right is +x), the centers of the wheels; the model
	//offset moves the height kept above the ground: a negative offset raises it on the hull,
	//so the model sits lower
	const float bottom = bounds_min.y - m_settings.model_offset_y * (bounds_max.y - bounds_min.y);
	m_corners[FRONT_LEFT]  = Vec3(bounds_min.x, bottom, bounds_max.z);
	m_corners[FRONT_RIGHT] = Vec3(bounds_max.x, bottom, bounds_max.z);
	m_corners[BACK_LEFT]   = Vec3(bounds_min.x, bottom, bounds_min.z);
	m_corners[BACK_RIGHT]  = Vec3(bounds_max.x, bottom, bounds_min.z);
	reset_wheels();
}

void Hovercraft::reset_wheels()
{
	const Vec3 position = m_actor->position();
	const Quat rotation = m_actor->rotation();
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		Wheel& wheel = m_wheels[corner_id];
		wheel.m_position = position + rotation * m_corners[corner_id];
		wheel.m_fall = 0.0f;
		wheel.m_ground = false;
	}
	m_velocity = Vec3(0.0f);
	m_speed = 0.0f;
}

void Hovercraft::spawn(const Vec3& start)
{
	//identity (the Quat constructor order depends on GLM_FORCE_QUAT_DATA_XYZW)
	m_actor->rotation(angle_axis(0.0f, Vec3(0.0f, 1.0f, 0.0f)));
	Vec3 position = start;
	//on the first surface under start: the wheels (their centers at the bottom corners)
	//`hover` over it; nothing under it, at start
	CollisionMesh::Hit hit;
	if (m_scene.raycast(start, Vec3(0.0f, -1.0f, 0.0f), 10000.0f, hit))
	{
		position.y = hit.m_point.y + (m_settings.hover - m_corners[FRONT_LEFT].y) * 2.0f;
	}
	m_actor->position(position);
	reset_wheels();
	//camera straight at the chase point
	if (m_camera) m_camera->position(position + m_settings.camera_offset);
	follow_camera();
}

void Hovercraft::update(double dt, const Input& input)
{
	m_time += dt;
	//at most a few steps per frame, a long frame (loading, debugger) does not explode
	int steps = 0;
	while (m_time >= m_settings.step && steps < 8)
	{
		step(input);
		m_time -= m_settings.step;
		++steps;
	}
	if (steps == 8) m_time = 0.0;
}

void Hovercraft::step(const Input& input)
{
	const Vec3 world_up(0.0f, 1.0f, 0.0f);
	Vec3 position = m_actor->position();
	Quat rotation = m_actor->rotation();

	//1) steering: faster the faster it goes, and at least idle_turn (it also turns in place)
	{
		const float rate = std::max(std::abs(m_speed) * m_settings.turn, m_settings.idle_turn);
		//left handed (+x right): a positive angle around +y turns +z towards +x, to the right
		float yaw = 0.0f;
		if (input.left)  yaw -= rate;
		if (input.right) yaw += rate;
		if (yaw != 0.0f) rotation = rotation * angle_axis(radians(yaw), world_up);
	}

	//2) drive, shared by the wheels: with a wheel on the ground throttle, drag and grip,
	//   along the hull (up and down its slope too); in the air it keeps going
	bool ground = false;
	for (const Wheel& wheel : m_wheels) ground |= wheel.m_ground;
	if (ground)
	{
		if (input.forward)
		{
			m_speed = std::min(m_speed + m_settings.acceleration, m_settings.max_speed);
		}
		else if (input.backward)
		{
			m_speed = std::max(m_speed - m_settings.acceleration, m_settings.max_reverse);
		}
		else
		{
			m_speed *= m_settings.drag;
		}
		m_velocity = rotation * Vec3(0.0f, 0.0f, m_speed);
	}

	//3) the wheels: each one moves with the collisions from where it is to its corner of the
	//   moved hull, plus its own fall
	const Vec3 forward = rotation * Vec3(0.0f, 0.0f, 1.0f);
	Vec3 center_before(0.0f);
	for (const Wheel& wheel : m_wheels) center_before += wheel.m_position * 0.25f;
	const Vec3 moved = position + m_velocity;
	std::vector<CollisionMesh::Contact> contacts;
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		Wheel& wheel = m_wheels[corner_id];
		wheel.m_fall += m_settings.gravity;
		const Vec3 corner = moved + rotation * m_corners[corner_id];
		const Vec3 target = corner + Vec3(0.0f, wheel.m_fall, 0.0f);
		contacts.clear();
		const Vec3 resolved = m_scene.move(wheel.m_position, target, m_settings.hover, &contacts);
		//ground (facing up) or wall
		wheel.m_ground = false;
		for (const CollisionMesh::Contact& contact : contacts)
		{
			if (contact.m_normal.y >= m_settings.floor_normal_y)
			{
				wheel.m_ground = true;
			}
			else
			{
				//no drive into a wall (in the air too)
				const float into = dot(m_velocity, contact.m_normal);
				if (into < 0.0f) m_velocity -= contact.m_normal * into;
			}
		}
		//its own fall: what the collisions left of it (the drive and the hull are not its)
		const float fall = resolved.y - corner.y;
		if (wheel.m_ground)
		{
			//landing: falling faster than one step of gravity is a hit, it takes away its
			//share of the speed
			const float impact = std::max(0.0f, -wheel.m_fall + m_settings.gravity);
			m_speed *= std::max(0.0f, 1.0f - m_settings.wheel_impact * impact * 0.25f);
			//the ground holds it: it does not throw it up
			wheel.m_fall = std::min(fall, 0.0f);
		}
		else
		{
			wheel.m_fall = fall;
		}
		wheel.m_position = resolved;
	}

	//4) the hull follows the wheels: aligned to them, left/right then front/rear, and
	//   centered on them
	const std::array<Vec3, 4> wheels
	{
		m_wheels[FRONT_LEFT].m_position, m_wheels[FRONT_RIGHT].m_position,
		m_wheels[BACK_LEFT].m_position,  m_wheels[BACK_RIGHT].m_position
	};
	const Vec3 left_to_right = (wheels[FRONT_RIGHT] + wheels[BACK_RIGHT]) * 0.5f
	                         - (wheels[FRONT_LEFT]  + wheels[BACK_LEFT])  * 0.5f;
	const Vec3 back_to_front = (wheels[FRONT_LEFT] + wheels[FRONT_RIGHT]) * 0.5f
	                         - (wheels[BACK_LEFT]  + wheels[BACK_RIGHT])  * 0.5f;
	if (length(left_to_right) > 1e-4f && length(back_to_front) > 1e-4f)
	{
		const Vec3 hull_forward = normalize(back_to_front);
		const Vec3 hull_up      = normalize(cross(hull_forward, left_to_right));
		const Vec3 right        = cross(hull_up, hull_forward);
		rotation = normalize(glm::quat_cast(Mat3(right, hull_up, hull_forward)));
	}
	Vec3 center_after(0.0f), corners_center(0.0f);
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		center_after += wheels[corner_id] * 0.25f;
		corners_center += m_corners[corner_id] * 0.25f;
	}
	position = center_after - rotation * corners_center;

	//5) the speed is what the hull covered along its forward: a wall stops it, sliding
	//   along it keeps a part
	if (ground)
	{
		const float covered = dot(center_after - center_before, forward);
		if (m_speed > 0.0f)      m_speed = std::clamp(covered, 0.0f, m_speed);
		else if (m_speed < 0.0f) m_speed = std::clamp(covered, m_speed, 0.0f);
	}

	m_actor->position(position);
	m_actor->rotation(rotation);

	//6) camera
	follow_camera();
}

void Hovercraft::follow_camera()
{
	if (!m_camera) return;
	const Vec3 hull = m_actor->position();
	Vec3 camera = m_camera->position();
	//towards the chase point (not while reversing, like the original)
	if (m_speed >= 0.0f)
	{
		const Vec3 target = hull + m_actor->rotation() * m_settings.camera_offset;
		camera += (target - camera) * m_settings.camera_follow;
		m_camera->position(camera);
	}
	//look at the hull, no roll
	const Vec3 direction = hull - camera;
	const float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	if (horizontal < 1e-5f && std::abs(direction.y) < 1e-5f) return;
	const float yaw   = std::atan2(direction.x, direction.z);
	const float pitch = -std::atan2(direction.y, horizontal);
	m_camera->rotation(angle_axis(yaw, Vec3(0.0f, 1.0f, 0.0f)) * angle_axis(pitch, Vec3(1.0f, 0.0f, 0.0f)));
}


//////////////////////////////////////////////////////////////////////////////////////////
//HovercraftDriver
SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftDriver);

void HovercraftDriver::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftDriver>()
	ctx.add_object<HovercraftDriver>();
}

HovercraftDriver::HovercraftDriver(Context& context) : Component(context)
{
}

bool HovercraftDriver::ready()
{
	if (m_hovercraft) return true;
	auto actor = this->actor().lock();
	auto current_world = world().lock();
	if (!actor || !current_world) return false;
	auto collision = current_world->instance<CollisionWorld>();
	if (!collision) return false;
	m_hovercraft = std::make_unique<Hovercraft>(context(), actor, m_camera, collision->mesh(), m_settings);
	return true;
}

void HovercraftDriver::spawn(const Vec3& start)
{
	if (ready()) m_hovercraft->spawn(start);
}

void HovercraftDriver::on_deattch()
{
	m_hovercraft.reset();
}

void HovercraftDriver::on_update(double delta_time)
{
	if (ready()) m_hovercraft->update(delta_time, m_input);
}

void HovercraftDriver::serialize(Data::Archive& archive)
{
	Data::serialize(archive, this);
}

void HovercraftDriver::serialize_json(Data::JsonValue& archive)
{
	Data::serialize_json(archive, this);
}

void HovercraftDriver::deserialize(Data::Archive& archive)
{
	Data::deserialize(archive, this);
}

void HovercraftDriver::deserialize_json(Data::JsonValue& archive)
{
	Data::deserialize_json(archive, this);
}
