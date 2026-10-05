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

	//the triangles of its navmesh (world space); false: the map has none
	bool navmesh(std::vector<Square::Vec3>& triangles) const;

private:

	//the colliders and the navmesh not drawn
	void hide_helpers();
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
	//spawn_point_1..4 of the arena, a fallback without them
	std::array<Square::Vec3, s_racers>    m_starts{ s_start, s_start + Square::Vec3(10, 0, 0), s_start + Square::Vec3(0, 0, 10), s_start + Square::Vec3(10, 0, 10) };
};
