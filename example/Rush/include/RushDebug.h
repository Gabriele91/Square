//
//  RushDebug.h
//  Rush
//
//  What the game adds to the debug panel of the engine (F1, UISystem::debug_sections): its tab
//  Rush, the collisions drawn (the meshes of the collision world), the player's hovercraft all
//  mirror (a look at the reflections), the level saved and loaded (binary .sq, json .jsq).
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Square/Square.h>

class Race;

class RushDebug
{
public:

	//its sections in the debug panel; race: the race now (none: nullptr)
	RushDebug(Square::Context& context, Square::Scene::World& world, std::function<Race*()> race);
	~RushDebug();

	//a race ended: its hovercraft are gone (the mirror with them)
	void race_ended();

private:

	//the player's hovercraft all mirror (each material a copy of its own, the ones it had kept),
	//or back to the materials it had
	void mirror(bool enable);
	//the level, binary (.sq) and json (.jsq)
	static std::string level_path(bool json);
	void save_level(bool json);
	void load_level(bool json);

	Square::Context&       m_context;
	Square::Scene::World&  m_world;
	std::function<Race*()> m_race;
	//a material of a part of the mirror hovercraft: where, the one it had
	struct Kept
	{
		Square::Weak<Square::Scene::StaticMesh>  m_mesh;
		size_t                                   m_index{ 0 };
		Square::Shared<Square::Resource::Material> m_material;
	};
	std::vector<Kept>      m_kept;
	bool                   m_mirror{ false };
};
