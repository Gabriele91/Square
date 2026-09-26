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
	//the four bottom corners (forward is +z, right is +x); the model offset moves the height
	//kept above the ground: a negative offset raises it on the hull, so the model sits lower
	const float bottom = bounds_min.y - m_settings.model_offset_y * (bounds_max.y - bounds_min.y);
	m_corners[FRONT_LEFT]  = Vec3(bounds_min.x, bottom, bounds_max.z);
	m_corners[FRONT_RIGHT] = Vec3(bounds_max.x, bottom, bounds_max.z);
	m_corners[BACK_LEFT]   = Vec3(bounds_min.x, bottom, bounds_min.z);
	m_corners[BACK_RIGHT]  = Vec3(bounds_max.x, bottom, bounds_min.z);
	//against the walls: a row of spheres as wide as the hull, from the back to the front
	const Vec3  center = (bounds_min + bounds_max) * 0.5f;
	const float width  = bounds_max.x - bounds_min.x;
	const float length = bounds_max.z - bounds_min.z;
	m_radius = std::max(0.1f, std::min(width, length) * 0.5f);
	const float span  = std::max(0.0f, std::max(width, length) - 2.0f * m_radius); //between the first and last center
	const int   count = 1 + int(std::ceil(span / m_radius));                         //spheres overlap by half a radius at least
	for (int i = 0; i < count; ++i)
	{
		const float offset = count > 1 ? -span * 0.5f + span * float(i) / float(count - 1) : 0.0f;
		m_wall_spheres.push_back(width < length ? Vec3(center.x, center.y, center.z + offset)
		                                        : Vec3(center.x + offset, center.y, center.z));
	}
	reset_wheels();
}

//////////////////////////////////////////////////////////////////////////////////////////
//Wheel
bool Hovercraft::Wheel::update(float ground_now, bool ground_found, const Settings& settings, float& impact)
{
	impact = 0.0f;
	//how much the ground under the wheel rose since the last step (a ramp, a step)
	const float rise = (m_has_ground && ground_found) ? ground_now - m_ground : 0.0f;
	m_ground = ground_now;
	m_has_ground = ground_found;
	//gravity, always
	m_velocity += settings.gravity;
	m_y += m_velocity;
	if (!ground_found) return (m_touching = false);
	if (m_y < ground_now)
	{
		//landing: falling faster than one step of gravity is a hit
		impact += std::max(0.0f, -m_velocity + settings.gravity);
		//the ground pushes it up at the speed it rises (up a ramp: then off it, it jumps),
		//up to max_climb, more than that (a step) is a hit
		const float climb = std::max(0.0f, rise);
		impact += std::max(0.0f, climb - settings.max_climb);
		m_velocity = std::min(climb, settings.max_climb);
		m_y = ground_now;
	}
	//on the ground or close to it: it drives
	return (m_touching = m_y <= ground_now + settings.contact_tolerance);
}

//////////////////////////////////////////////////////////////////////////////////////////
//Hovercraft
bool Hovercraft::ground(const Vec3& point, float& height) const
{
	//from a bit above the point down: not the ceilings over it
	CollisionMesh::Hit hit;
	const Vec3 origin = point + Vec3(0.0f, m_settings.probe_reach, 0.0f);
	if (!m_scene.raycast(origin, Vec3(0.0f, -1.0f, 0.0f), m_settings.probe_reach + m_settings.probe_depth, hit)) return false;
	height = hit.m_point.y;
	return true;
}

void Hovercraft::reset_wheels()
{
	const Vec3 position = m_actor->position();
	const Quat rotation = m_actor->rotation();
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		Wheel& wheel = m_wheels[corner_id];
		const Vec3 corner = position + rotation * m_corners[corner_id];
		wheel.m_y = corner.y - m_settings.hover;
		wheel.m_velocity = 0.0f;
		wheel.m_has_ground = ground(Vec3(corner.x, wheel.m_y, corner.z), wheel.m_ground);
		wheel.m_touching = false;
	}
	m_velocity = Vec3(0.0f);
	m_speed = 0.0f;
}

void Hovercraft::spawn(float x, float z)
{
	//identity (the Quat constructor order depends on GLM_FORCE_QUAT_DATA_XYZW)
	m_actor->rotation(angle_axis(0.0f, Vec3(0.0f, 1.0f, 0.0f)));
	Vec3 position(x, m_actor->position().y, z);
	//drop it from high above
	CollisionMesh::Hit hit;
	if (m_scene.raycast(Vec3(x, 10000.0f, z), Vec3(0.0f, -1.0f, 0.0f), 20000.0f, hit))
	{
		position.y = hit.m_point.y + m_settings.hover - m_corners[FRONT_LEFT].y;
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

	//2) x/z, shared by the four wheels: with a wheel on the ground throttle, drag and grip
	//   (along the hull only), in the air it keeps going
	int touching = 0;
	for (const Wheel& wheel : m_wheels) touching += wheel.m_touching ? 1 : 0;
	if (touching)
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
		const Vec3 drive = rotation * Vec3(0.0f, 0.0f, m_speed);
		m_velocity = Vec3(drive.x, 0.0f, drive.z);
	}
	position += m_velocity;

	//3) y, each wheel on its own: gravity, the ground under it, the hits
	std::array<Vec3, 4> contacts;
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		Wheel& wheel = m_wheels[corner_id];
		const Vec3 corner = position + rotation * m_corners[corner_id];
		float ground_height = 0.0f;
		const bool found = ground(Vec3(corner.x, wheel.m_y, corner.z), ground_height);
		float impact = 0.0f;
		wheel.update(ground_height, found, m_settings, impact);
		//every wheel hitting takes away its share of the speed
		m_speed *= std::max(0.0f, 1.0f - m_settings.wheel_impact * impact * 0.25f);
		contacts[corner_id] = Vec3(corner.x, wheel.m_y, corner.z);
	}

	//4) the hull follows the wheels: aligned to them, left/right then front/rear, at the
	//   height that keeps its corners `hover` above them
	const Vec3 left_to_right = (contacts[FRONT_RIGHT] + contacts[BACK_RIGHT]) * 0.5f
	                         - (contacts[FRONT_LEFT]  + contacts[BACK_LEFT])  * 0.5f;
	const Vec3 back_to_front = (contacts[FRONT_LEFT] + contacts[FRONT_RIGHT]) * 0.5f
	                         - (contacts[BACK_LEFT]  + contacts[BACK_RIGHT])  * 0.5f;
	if (length(left_to_right) > 1e-4f && length(back_to_front) > 1e-4f)
	{
		const Vec3 forward  = normalize(back_to_front);
		const Vec3 hull_up  = normalize(cross(forward, left_to_right));
		const Vec3 right    = cross(hull_up, forward);
		rotation = normalize(glm::quat_cast(Mat3(right, hull_up, forward)));
	}
	float height = 0.0f;
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		height += m_wheels[corner_id].m_y + m_settings.hover - (rotation * m_corners[corner_id]).y;
	}
	position.y = height * 0.25f;
	//the hull is rigid: the wheels back under its corners (their speed is kept)
	for (int corner_id = 0; corner_id < 4; ++corner_id)
	{
		m_wheels[corner_id].m_y = (position + rotation * m_corners[corner_id]).y - m_settings.hover;
	}

	//5) walls: every sphere along the hull pushes the whole hull out, and the speed drops
	//   with how head-on the hit is (head-on: stop, grazing: a little)
	Vec3 push(0.0f);
	for (const Vec3& sphere : m_wall_spheres)
	{
		const Vec3 center = position + rotation * sphere;
		Vec3 pushed = center;
		if (m_scene.push_out(pushed, m_radius, m_settings.wall_normal_y))
		{
			position += pushed - center;
			push += pushed - center;
		}
	}
	const Vec3 push_horizontal(push.x, 0.0f, push.z);
	Vec3 forward_horizontal = rotation * Vec3(0.0f, 0.0f, 1.0f);
	forward_horizontal.y = 0.0f;
	if (length(push_horizontal) > 1e-5f && length(forward_horizontal) > 1e-5f)
	{
		const Vec3  wall_normal = normalize(push_horizontal);
		const float head_on = std::abs(dot(normalize(forward_horizontal), wall_normal));
		m_speed *= std::max(0.0f, 1.0f - m_settings.wall_stop * head_on);
		//no velocity into the wall (in the air too)
		const float into_wall = dot(m_velocity, wall_normal);
		if (into_wall < 0.0f) m_velocity -= wall_normal * into_wall;
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
