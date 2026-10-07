//
//  Arena.cpp
//  Rush
//
//  See Arena.h.
//
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <Arena.h>
#include <Collision.h>
#include <CameraFollow.h>
#include <RushConfig.h>

using namespace Rush;

Arena::Arena(Square::Context& context)
: m_context(context)
{
}

bool Arena::load(Square::Shared<Square::Scene::Level> level, const std::string& name)
{
	using namespace Square;
	m_actor = level->load_actor(name + "/scene");
	if (!m_actor)
	{
		m_context.logger()->info("Error to load the level " + name);
		return false;
	}
	m_actor->position({ 0.0f, 4.0f, 0.0f });
	// the sun
	m_sun = m_actor->child("sun");
	if (!m_sun) m_context.logger()->info("arena has no 'sun' node");
	// the arena is solid: a mesh collider of the scene type (its triangles, from where it is
	// placed)
	auto collider = m_actor->component<MeshCollider>();
	collider->type(TYPE_SCENE);
	m_context.logger()->info("arena collision triangles: " + std::to_string(collider->mesh().size()));
	hide_helpers();
	instance_props();
	find_camera_bounds();
	find_water();
	find_bounds();
	find_starts();
	find_course();
	setup_camera(level);
	return true;
}

namespace AuxArena
{
	//a node of a name that starts with prefix
	static bool named(const Square::Shared<Square::Scene::Actor>& node, const char* prefix)
	{
		return node->name().rfind(prefix, 0) == 0;
	}
}

void Arena::hide_helpers()
{
	using namespace Square;
	//"collider..." nodes (solid: in the mesh collider), "navmesh..." and "camera_bounds..." ones
	//(and their children): not drawn
	m_actor->visit([](Shared<Scene::Actor> node) -> bool
	{
		const bool helper = AuxArena::named(node, "collider") || AuxArena::named(node, "navmesh") || AuxArena::named(node, "camera_bounds");
		if (!helper) return true;
		node->visit([](Shared<Scene::Actor> part) -> bool
		{
			//(StaticMesh overrides only the getter: the setter of the renderable)
			if (part->contains<Scene::StaticMesh>()) part->component<Scene::StaticMesh>()->Render::Renderable::visible(false);
			return true;
		});
		return true;
	});
}

void Arena::find_camera_bounds()
{
	using namespace Square;
	//the walls of the camera: "camera_bounds..." meshes (alpha tested: not in the solid mesh
	//collider of the scene), their own collider of the camera bounds type, one sided (facing in:
	//the camera comes in from out of them, in them it stays)
	m_camera_bounds = 0;
	m_actor->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (!AuxArena::named(node, "camera_bounds")) return true;
		auto collider = node->component<MeshCollider>();
		collider->type(TYPE_CAMERA_BOUNDS);
		collider->solid_only(false);
		collider->one_sided(true);
		m_camera_bounds += collider->mesh().size();
		return false;
	});
	if (!m_camera_bounds) m_context.logger()->info("arena has no 'camera_bounds' mesh: the camera goes anywhere");
	else m_context.logger()->info("arena camera bounds triangles: " + std::to_string(m_camera_bounds));
}

void Arena::find_water()
{
	using namespace Square;
	//the materials with a water_time (PBRWater), each once
	m_water.clear();
	m_actor->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		for (const auto& material : node->component<Scene::StaticMesh>()->m_materials)
		{
			if (!material || !material->parameter_by_name("water_time")) continue;
			if (std::find(m_water.begin(), m_water.end(), material) == m_water.end()) m_water.push_back(material);
		}
		return true;
	});
}

void Arena::animate(double time)
{
	using namespace Square;
	const Vec4 seconds(float(std::fmod(time, 3600.0)), 0.0f, 0.0f, 0.0f);
	for (const auto& material : m_water)
	{
		if (auto parameter = material->parameter_by_name("water_time")) parameter->set(seconds);
	}
}

void Arena::find_bounds()
{
	using namespace Square;
	//x/z: the map of the lights in the menu covers them
	std::vector<Vec3> triangles;
	m_actor->component<MeshCollider>()->mesh().triangles(triangles);
	m_min = Vec3(std::numeric_limits<float>::max());
	m_max = Vec3(std::numeric_limits<float>::lowest());
	for (const Vec3& vertex : triangles)
	{
		m_min = glm::min(m_min, vertex);
		m_max = glm::max(m_max, vertex);
	}
}

void Arena::find_starts()
{
	using namespace Square;
	//the spawn points of the scene (after the arena is placed): spawn_point_1 the player, the
	//others the NPCs; they start facing the middle
	m_center = m_actor->position(true);
	//(none: around the fallback of the config, it drops on what is under it)
	const Vec3 fallback = Config::get().spawn_fallback();
	m_starts = 
	{ 
		fallback, 
		fallback + Vec3(10, 0, 0), 
		fallback + Vec3(0, 0, 10), 
		fallback + Vec3(10, 0, 10) 
	};
	m_actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		for (size_t id = 0; id != s_racers; ++id)
		{
			if (node->name() == "spawn_point_" + std::to_string(id + 1)) m_starts[id] = node->position(true);
		}
		return true;
	});
}

void Arena::find_course()
{
	using namespace Square;
	//its boost pads
	m_boosts.clear();
	m_actor->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (AuxArena::named(node, "boost_")) m_boosts.push_back(node->position(true));
		return true;
	});
	//a circuit: its guide, its checkpoints, its other ways (none: an arena)
	if (!m_course.collect(m_actor)) return;
	m_context.logger()->info("course: " + std::to_string(m_course.checkpoints().size()) + " checkpoints, " + std::to_string(m_course.routes().size())
	                         + " other ways, " + std::to_string(int(m_course.length())) + (m_course.closed() ? " a lap" : " start to finish"));
}

void Arena::instance_props()
{
	using namespace Square;
	//the chunks of the props of a library ("props_<i>_<j>_lod<n>": instances of its meshes)
	std::vector< Shared<Scene::Actor> > chunks;
	m_actor->visit([&chunks](Shared<Scene::Actor> node) -> bool
	{
		//(the group of its levels of detail, "props_<i>_<j>": a chunk is a level of it)
		if (node->contains<Scene::LodGroup>()) return true;
		if (!AuxArena::named(node, "props_")) return true;
		chunks.push_back(node);
		return false;
	});
	//a mesh of a chunk: its first static mesh (its materials), where each one is in the chunk,
	//their nodes
	struct Group
	{
		Shared<Scene::StaticMesh>          m_first;
		std::vector<Mat4>                  m_models;
		std::vector< Shared<Scene::Actor> > m_nodes;
	};
	size_t meshes = 0, instances = 0;
	for (const auto& chunk : chunks)
	{
		const Mat4 to_chunk = inverse(chunk->global_model_matrix());
		std::map<const void*, Group> groups;
		const Scene::ActorList children = chunk->childs();
		for (const auto& child : children)
		{
			//(its mesh: on it, or on a node of the exporter under it)
			Shared<Scene::Actor> holder;
			child->visit([&holder](Shared<Scene::Actor> part) -> bool
			{
				if (!part->contains<Scene::StaticMesh>()) return true;
				holder = part;
				return false;
			});
			if (!holder) continue;
			auto mesh = holder->component<Scene::StaticMesh>();
			if (!mesh->m_mesh) continue;
			Group& group = groups[mesh->m_mesh.get()];
			if (!group.m_first) group.m_first = mesh;
			group.m_models.push_back(to_chunk * holder->global_model_matrix());
			group.m_nodes.push_back(child);
		}
		for (auto& entry : groups)
		{
			Group& group = entry.second;
			auto node = MakeShared<Scene::Actor>(m_context, "instances");
			auto instanced = node->component<Scene::InstancedMesh>();
			instanced->mesh(group.m_first->m_mesh, group.m_first->m_materials);
			instanced->instances(group.m_models, group.m_first->local_bounding_box());
			for (const auto& old : group.m_nodes) chunk->remove(old);
			chunk->add(node);
			++meshes;
			instances += group.m_models.size();
		}
		//its group: its meshes changed
		auto parent = chunk->parent().lock();
		if (parent && parent->contains<Scene::LodGroup>())
		{
			parent->component<Scene::LodGroup>()->refresh();
		}
	}
	if (meshes) m_context.logger()->info("props instanced: " + std::to_string(instances) + " in " + std::to_string(meshes) + " draws");
}

void Arena::setup_camera(Square::Shared<Square::Scene::Level> level)
{
	using namespace Square;
	//the camera of the scene: it chases the hovercraft in world space (out of the arena, at the
	//level root), with a sphere of the camera type, so it does not go through the walls and
	//the ground
	m_camera = m_actor->child("camera");
	if (!m_camera)
	{
		m_context.logger()->info("arena has no 'camera' node");
		return;
	}
	unsigned int width = 0, height = 0;
	m_context.window()->get_size(width, height);
	viewport(width, height);
	level->add(m_camera);
	auto collider = m_camera->component<SphereCollider>();
	collider->type(TYPE_CAMERA);
	collider->radius(1.0f);
	m_camera_follow = m_camera->component<CameraFollow>();
}

void Arena::unload(Square::Shared<Square::Scene::Level> level)
{
	if (m_camera) level->remove(m_camera);
	if (m_actor) level->remove(m_actor);
	m_camera.reset();
	m_camera_follow.reset();
	m_sun.reset();
	m_actor.reset();
}

void Arena::viewport(unsigned int width, unsigned int height) const
{
	using namespace Square;
	if (!m_camera || !m_camera->contains<Scene::Camera>()) return;
	if (!width || !height) return;
	m_camera->component<Scene::Camera>()->viewport({ 0, 0, width, height });
}

void Arena::camera_clip(float clip_near, float clip_far) const
{
	using namespace Square;
	if (!m_camera || !m_camera->contains<Scene::Camera>()) return;
	auto camera = m_camera->component<Scene::Camera>();
	const Render::Viewport& viewport = camera->viewport();
	//(its perspective as it is: its field of view, its aspect)
	const Vec2  planes = viewport.near_and_far();
	const float new_near = clip_near > 0.0f ? clip_near : planes.x;
	const float new_far = clip_far > 0.0f ? clip_far : planes.y;
	camera->perspective(viewport.fov(), viewport.aspect(), new_near, new_far);
}

Square::Shared<Square::Scene::Actor> Arena::navmesh() const
{
	using namespace Square;
	Shared<Scene::Actor> found;
	if (!m_actor) return found;
	m_actor->visit([&found](Shared<Scene::Actor> node) -> bool
	{
		if (!AuxArena::named(node, "navmesh")) return true;
		found = node;
		return false;
	});
	return found;
}

Square::Shared<Square::Scene::Actor> Arena::actor() const
{
	return m_actor;
}

Square::Shared<Square::Scene::DirectionLight> Arena::sun() const
{
	using namespace Square;
	//component<T>() would add it: only a sun with its light
	if (!m_sun || !m_sun->contains<Scene::DirectionLight>()) return nullptr;
	return m_sun->component<Scene::DirectionLight>();
}

Square::Vec3 Arena::sun_direction() const
{
	using namespace Square;
	//a direction light lights along its z axis
	if (!m_sun) return Vec3(0.0f, -1.0f, 0.0f);
	return m_sun->rotation(true) * Vec3(0.0f, 0.0f, 1.0f);
}

void Arena::sun_direction(const Square::Vec3& direction)
{
	using namespace Square;
	if (!m_sun) return;
	//a direction light lights along its z axis: z turned to the direction
	const Vec3  from(0.0f, 0.0f, 1.0f);
	const Vec3  to = normalize(direction);
	const float cosine = std::clamp(dot(from, to), -1.0f, 1.0f);
	const Vec3  axis = cross(from, to);
	Quat turn = angle_axis(Constants::pi<float>(), Vec3(1.0f, 0.0f, 0.0f));
	if (length(axis) > 0.0001f)
	{
		turn = angle_axis(std::acos(cosine), normalize(axis));
	}
	else if (cosine > 0.0f)
	{
		turn = Quat(0.0f, 0.0f, 0.0f, 1.0f);
	}
	m_sun->rotation(turn);
}

Square::Vec3 Arena::sun_direction(float azimuth, float elevation)
{
	using namespace Square;
	const float a = radians(azimuth);
	const float e = radians(elevation);
	//toward the sun, its light the other way
	const Vec3 toward(std::cos(e) * std::cos(a), std::sin(e), std::cos(e) * std::sin(a));
	return -toward;
}

Square::Shared<Square::Scene::Actor> Arena::camera() const
{
	return m_camera;
}

Square::Shared<CameraFollow> Arena::camera_follow() const
{
	return m_camera_follow;
}

const Square::Vec3& Arena::min() const
{
	return m_min;
}

const Square::Vec3& Arena::max() const
{
	return m_max;
}

const Square::Vec3& Arena::center() const
{
	return m_center;
}

const Square::Vec3& Arena::start(size_t id) const
{
	return m_starts[id % s_racers];
}
