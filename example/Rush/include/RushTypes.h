//
//  RushTypes.h
//  Rush
//
//  The types and the constants of the game: the collision types, the racers (their number,
//  skins, names, engines), the rules of a match.
//
#pragma once
#include <cstddef>
#include <Square/Square.h>

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

//the hovercraft of a race: the player (the first) and the NPCs; the first to s_winning_score
//lights wins
inline constexpr size_t s_racers = 4;
inline constexpr int    s_winning_score = 10;

//names of the racers in the HUD
inline constexpr const char* s_racer_names[s_racers]{ "You", "Blue", "Green", "Yellow" };

//colors of the hovercraft: textures of assets/hovercraft_skins (made by origial_assets/
//hovercraft/skins.py; "": the one of the model)
inline constexpr const char* s_skins[s_racers]
{
	"hovercraft_skins/hovercraft_red",
	"hovercraft_skins/hovercraft_blue",
	"hovercraft_skins/hovercraft_green",
	"hovercraft_skins/hovercraft_yellow",
};

//the engine of each hovercraft (data_player_positions of Limit Rush: move distance and
//friction), relative to the player: acceleration, top speed, grip
struct Engine
{
	float acceleration;
	float max_speed;
	float drag;
};
inline constexpr Engine s_engines[s_racers]
{
	{ 1.00f, 1.00f, 1.000f }, // player: 0.075, 0.974
	{ 0.73f, 0.96f, 1.006f }, // npc 1:  0.055, 0.980
	{ 0.80f, 0.83f, 1.001f }, // npc 2:  0.060, 0.975
	{ 0.93f, 0.69f, 0.991f }, // npc 3:  0.070, 0.965
};

//start of the hovercraft, when the arena has no spawn point: it drops on the first surface
//under it (under the roof, over the field)
inline constexpr Square::Vec3 s_start{ 0.0f, 50.0f, 0.0f };

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
//a map of a race: its scene ("<name>/scene", assets/<name>.sqz), its name in the title, its fog,
//its falling snow (a light one), the trails of the hovercraft in its ground (a PBRSnow material).
//The wall of its camera is in its scene: its "camera_bounds..." meshes (see Arena)
struct RaceMap
{
	const char*  m_name;
	const char*  m_title;
	RaceFog      m_fog;
	bool         m_snow{ false };
	bool         m_trails{ false };
	bool         m_wakes{ false };  //the wakes of the hovercraft on its water (PBRWater)
};
inline const RaceMap s_race_maps[]
{
	//the stadium (origial_assets/arena): a light haze of desert dust, warm, dense in its pit (its
	//field from y ~-12 to 0), thin over the stands; the grooves of the hovercraft in its clay
	{ "arena",    "Arena",    { true, { 0.55f, 0.40f, 0.28f }, 0.0018f, -6.0f, 0.05f, { 0.45f, 0.30f, 0.18f } }, false, true },
	//the swamp (origial_assets/backwash): green, dense on the water (y ~3.5 with the arena
	//placed), thin over the trees
	{ "backwash", "Backwash", { true, { 0.17f, 0.21f, 0.12f }, 0.012f, 3.5f, 0.08f, { 0.35f, 0.32f, 0.17f } } },
	//the snowy canyon (origial_assets/containment): a cold light haze, thin; its snow, its trails
	{ "containment", "Containment", { true, { 0.48f, 0.53f, 0.6f }, 0.0035f, 3.5f, 0.04f, { 0.35f, 0.33f, 0.26f } }, true, true },
	//the forerunner ruins in a green crater (origial_assets/sanctuary): a light warm mist, low
	//(the field near y 4 with the arena placed), the sun through it
	{ "sanctuary", "Sanctuary", { true, { 0.8f, 0.82f, 0.76f }, 0.0006f, 4.0f, 0.05f, { 0.6f, 0.5f, 0.32f } }, false, false, true },
};
inline constexpr size_t s_race_maps_count = sizeof(s_race_maps) / sizeof(s_race_maps[0]);
//a map with trails (snow, clay): the hovercraft shown this lower (world units, its meshes only:
//its physics as on any map), in its groove
inline constexpr float s_snow_sink = 0.3f;
//the phases of a race: the start (the camera comes to the player, the hovercraft still), then
//"GO!" for a while
inline constexpr double s_start_time = 3.0;
inline constexpr double s_go_time = 1.0;
//the time of a phase goes on at most this per frame (the first frame of a race: the end of its
//loading, long, the countdown would be gone)
inline constexpr double s_max_frame_time = 0.1;

//the title screen: its scene far from the arena (out of its views and of its shadows), the
//hovercraft turning in it (degrees per second)
inline constexpr Square::Vec3 s_title_origin{ 0.0f, 0.0f, 5000.0f };
inline constexpr float        s_title_turn_speed = 20.0f;
