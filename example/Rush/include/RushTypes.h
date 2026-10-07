//
//  RushTypes.h
//  Rush
//
//  The types of the game that are code: the collision types, the number of the racers, the
//  levels of the world, a map of a race (its fog, its zones). Their values are in the
//  configuration (config/: RushConfig.h).
//
#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <Square/Square.h>

namespace Rush
{
	//collision types (Const BODY=1,WHEEL=2,SCENE=3), the camera, the walls of the camera (the
	//"camera_bounds..." meshes of a map: only the camera hits them)
	enum CollisionType : int
	{
		TYPE_BODY          = 1,
		TYPE_WHEEL         = 2,
		TYPE_SCENE         = 3,
		TYPE_CAMERA        = 4,
		TYPE_CAMERA_BOUNDS = 5
	};

	//the hovercraft of a race: the player (the first) and the NPCs (each one its settings:
	//config/hovercraft.json)
	inline constexpr size_t s_racers = 4;

	//the levels of the world: the title, the race (one active at a time)
	inline constexpr const char* s_title_world_level = "title";
	inline constexpr const char* s_race_world_level  = "race";

	//the fog of a map (off: none), world units; see Square::Render::Fog
	struct RaceFog
	{
		bool         m_on{ false };
		Square::Vec3 m_color{ 0.0f };   //linear, as the light of the scene
		float        m_density{ 0.0f }; //per world unit at the height
		float        m_height{ 0.0f };  //world height (y) of the density, thinner over it
		float        m_falloff{ 0.0f };
		Square::Vec3 m_sun{ 0.0f };     //the glow looking toward the sun of the map
	};

	//the kind of a map: an arena (the beam of light: who reaches it scores), a circuit (laps along
	//its gates, the first over the line wins)
	enum class RaceMode
	{
		ARENA,
		CIRCUIT
	};

	//a stretch of a circuit: from one of its checkpoints on, its haze (the environment changes
	//along the way: the player's one blends into the next zone's)
	struct CircuitZone
	{
		size_t  m_gate{ 0 };   //its first checkpoint (cp_<n>: n - 1)
		RaceFog m_fog;
	};

	//a map of a race (config/maps/<name>.json): its scene ("<name>/scene", assets/<name>.sqz), its
	//name in the title, its fog, its falling snow (a light one), the trails of the hovercraft in its
	//ground (a PBRSnow material), the wakes on its water. The wall of its camera is in its scene:
	//its "camera_bounds..." meshes (see Arena). A circuit: its laps, its zones (their hazes, by its
	//gates; its fog the one of the first)
	//where the sun of a map is at a time of the race (seconds, degrees)
	struct SunStep
	{
		float m_time{ 0.0f };
		float m_azimuth{ 0.0f };
		float m_elevation{ 45.0f };
	};

	struct RaceMap
	{
		std::string              m_name;
		std::string              m_title;
		RaceFog                  m_fog;
		bool                     m_snow{ false };
		bool                     m_trails{ false };
		bool                     m_wakes{ false };
		RaceMode                 m_mode{ RaceMode::ARENA };
		int                      m_laps{ 3 };
		std::vector<CircuitZone> m_zones;
		//the clip planes of its camera (world units; 0: as in its scene)
		float                    m_camera_near{ 0.0f };
		float                    m_camera_far{ 0.0f };
		//where its sun is (degrees; not set: as in its scene): azimuth from +x toward +z of the
		//world (+y of Blender, seen from above), elevation over the horizon; its steps: where it
		//is at a time of the race (seconds from the start), between them on its way, after the
		//last one there (none: still)
		bool                     m_sun_set{ false };
		float                    m_sun_azimuth{ 0.0f };
		float                    m_sun_elevation{ 45.0f };
		std::vector<SunStep>     m_sun_steps;
	};
}
