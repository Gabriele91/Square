//
//  Arena.h
//  Rush
//
//  A level of the race (the arena, by now the only one): its scene (solid, a mesh collider of
//  the scene type), its sun, its bounds, the starts of the hovercraft (spawn_point_1..4) and its
//  camera (the chase camera of the player, at the level root: it follows in world space).
//  The "collider..." meshes of a scene are solid but not drawn (invisible walls); its "navmesh"
//  (made in Blender: where the AI drives, the holes of the obstacles) is neither solid (an alpha
//  tested material) nor drawn.
//
#pragma once
#include <array>
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>
#include <Course.h>

class CameraFollow;

class Arena
{
public:

	Arena(Square::Context& context);

	//the scene of a level of the race ("<name>/scene") in a level of the world (false: not loaded)
	bool load(Square::Shared<Square::Scene::Level> level, const std::string& name);
	//its scene out of the level of the world
	void unload(Square::Shared<Square::Scene::Level> level);

	//the camera of the window size
	void viewport(unsigned int width, unsigned int height) const;

	Square::Shared<Square::Scene::Actor>        actor() const;
	Square::Shared<Square::Scene::DirectionLight> sun() const;
	//where the light of the sun goes (no sun: down)
	Square::Vec3                                sun_direction() const;
	Square::Shared<Square::Scene::Actor>        camera() const;
	Square::Shared<CameraFollow>                camera_follow() const;

	//bounds (world), the middle the hovercraft face at the start, the start of a racer
	const Square::Vec3& min() const;
	const Square::Vec3& max() const;
	const Square::Vec3& center() const;
	const Square::Vec3& start(size_t id) const;

	//the walls of its camera ("camera_bounds..." meshes: one sided, facing in), how many
	//triangles (0: none, the camera goes anywhere)
	size_t camera_bounds() const { return m_camera_bounds; }

	//its navmesh (made in Blender: where the AI drives), nullptr: the map has none
	Square::Shared<Square::Scene::Actor> navmesh() const;

	//its water (PBRWater materials: their "water_time") moves: the seconds of the race
	void animate(double time);

	//the course of a circuit (its guide, its checkpoints, its other ways: see Course); not
	//valid: an arena
	const Course& course() const { return m_course; }

	//its boost pads ("boost_<n>" nodes, world)
	const std::vector<Square::Vec3>& boosts() const { return m_boosts; }

private:

	//the colliders and the navmesh not drawn
	void hide_helpers();
	void find_course();
	//the props of a library (the same mesh many times in a chunk, "props_..."): one instanced
	//mesh a mesh a chunk (a draw call for all of them), their nodes out (after the collision)
	void instance_props();
	void find_camera_bounds();
	void find_water();
	void find_bounds();
	void find_starts();
	void setup_camera(Square::Shared<Square::Scene::Level> level);

	Square::Context&                      m_context;
	Square::Shared<Square::Scene::Actor>  m_actor;
	Square::Shared<Square::Scene::Actor>  m_sun;
	Square::Shared<Square::Scene::Actor>  m_camera;
	Square::Shared<CameraFollow>          m_camera_follow;
	Square::Vec3                          m_min{ 0.0f };
	Square::Vec3                          m_max{ 0.0f };
	Square::Vec3                          m_center{ 0.0f };
	size_t m_camera_bounds{ 0 };
	//the materials of its water (their water_time)
	std::vector< Square::Shared<Square::Resource::Material> > m_water;
	Course                                m_course;
	std::vector<Square::Vec3>             m_boosts;
	//spawn_point_1..4 of the arena, a fallback without them
	std::array<Square::Vec3, Rush::s_racers> m_starts; //(no spawn point: around the fallback of the config)
};
