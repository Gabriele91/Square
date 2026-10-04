//
//  Arena.cpp
//  Rush
//
//  See Arena.h.
//
#include <limits>
#include <Arena.h>
#include <Collision.h>
#include <CameraFollow.h>

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
	hide_colliders();
	find_bounds();
	find_starts();
	setup_camera(level);
	return true;
}

void Arena::hide_colliders()
{
	using namespace Square;
	//"collider..." nodes (and their children): solid (in the mesh collider), not drawn
	m_actor->visit([](Shared<Scene::Actor> node) -> bool
	{
		if (node->name().rfind("collider", 0) != 0) return true;
		node->visit([](Shared<Scene::Actor> part) -> bool
		{
			//(StaticMesh overrides only the getter: the setter of the renderable)
			if (part->contains<Scene::StaticMesh>()) part->component<Scene::StaticMesh>()->Render::Renderable::visible(false);
			return true;
		});
		return true;
	});
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
	m_actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		for (size_t id = 0; id != s_racers; ++id)
		{
			if (node->name() == "spawn_point_" + std::to_string(id + 1)) m_starts[id] = node->position(true);
		}
		return true;
	});
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
