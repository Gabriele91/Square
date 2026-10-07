//
//  RushConfig.h
//  Rush
//
//  The configuration of the game, read at the start from its folder (config/):
//   - game.json: the rules (the score, the times of the start), the circuits (their speed, the
//     window of the trails, the falls, the blend of the zones), the boost pads, the trails, the
//     title (its place, its haze), the maps in the title (arenas and circuits, in order);
//   - hovercraft.json: the racers (name, color, skin, engine, courage), the driver of a
//     hovercraft (its base values: gravity, acceleration, drag, turn...);
//   - surfaces.json: the grips of the grounds (by their names: ground, dirt, sand, mud, shallow,
//     water, ice);
//   - maps/<name>.json: a map (RaceMap: its title, mode, laps, snow, trails, wakes, fog, zones);
//     one without its scene (assets/<name>.sqz) is not in the title.
//  A file missing or wrong: its values as the defaults here (a warning).
//
#pragma once
#include <array>
#include <string>
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>
#include <Hovercraft.h>

namespace Rush
{
	//the engine of a hovercraft, shares of the driver's: acceleration, top speed, drag
	struct Engine
	{
		float m_acceleration{ 1.0f };
		float m_max_speed{ 1.0f };
		float m_drag{ 1.0f };
	};

	//a racer: its name in the HUD, its color (the class of its row), its skin (a texture of
	//assets/hovercraft_skins, "": the one of the model), its engine, its courage (a share: how
	//often it takes the other ways of a circuit)
	struct Racer
	{
		std::string m_name;
		std::string m_color;
		std::string m_skin;
		Engine      m_engine;
		float       m_courage{ 0.0f };
	};

	//the rules of a race: the first to the winning score (an arena) wins; the start (the camera
	//comes to the player), then "GO!" for a while; the time of a phase goes on at most this per
	//frame (the first frame of a race: the end of its loading)
	struct Rules
	{
		int    m_winning_score{ 10 };
		double m_start_time{ 3.0 };
		double m_go_time{ 1.0 };
		double m_max_frame_time{ 0.1 };
	};

	//a circuit: the hovercraft faster than in an arena (a share of its top speed and of its
	//acceleration), the window of its trails around the player (world units), a racer this lower
	//than the guide fell, the haze blended into the next zone's over this many world units
	struct CircuitRules
	{
		float m_speed{ 1.3f };
		float m_trails_window{ 300.0f };
		float m_fall_depth{ 18.0f };
		float m_zone_blend{ 60.0f };
	};

	//the boost pads ("boost_<n>"): touched this near (x/z), for this long (seconds) faster (shares
	//of the top speed and of the acceleration)
	struct BoostRules
	{
		float m_radius{ 6.0f };
		float m_time{ 1.6f };
		float m_speed{ 1.45f };
		float m_acceleration{ 2.5f };
	};

	class Config
	{
	public:
		//the configuration of the folder (config/), the maps with their scene in assets: false, a
		//file missing or wrong (its defaults)
		static bool load(Square::Context& context, const std::string& folder, const std::string& assets);
		static const Config& get();

		const Rules&        rules() const { return m_rules; }
		const CircuitRules& circuit() const { return m_circuit; }
		const BoostRules&   boost() const { return m_boost; }
		//a map with trails: the hovercraft shown this lower (world units, its meshes only)
		float               trails_sink() const { return m_trails_sink; }
		//start of a hovercraft when the arena has no spawn point (it drops on what is under it)
		const Square::Vec3& spawn_fallback() const { return m_spawn_fallback; }
		//the title: its scene far from the race, its haze
		const Square::Vec3& title_origin() const { return m_title_origin; }
		const RaceFog&      title_fog() const { return m_title_fog; }

		const std::array<Racer, s_racers>& racers() const { return m_racers; }
		const Racer& racer(size_t id) const { return m_racers[id % s_racers]; }
		//the settings of the driver of a racer: the base ones, its engine, the grips of the
		//grounds; a circuit: faster
		HovercraftDriver::Settings driver(size_t id, bool circuit) const;

		//the maps of the title, in order (an arena: the cards of the title, by index)
		const std::vector<RaceMap>& arenas() const { return m_arenas; }
		const std::vector<RaceMap>& circuits() const { return m_circuits; }
		//a map by its name (nullptr: none)
		const RaceMap* map(const std::string& name) const;

	private:
		bool load_game(const Square::Data::JsonValue& root);
		bool load_hovercraft(const Square::Data::JsonValue& root);
		bool load_surfaces(const Square::Data::JsonValue& root);
		bool load_map(const Square::Data::JsonValue& root, const std::string& name, RaceMap& map);

		Rules        m_rules;
		CircuitRules m_circuit;
		BoostRules   m_boost;
		float        m_trails_sink{ 0.3f };
		Square::Vec3 m_spawn_fallback{ 0.0f, 50.0f, 0.0f };
		Square::Vec3 m_title_origin{ 0.0f, 0.0f, 5000.0f };
		RaceFog      m_title_fog;

		std::array<Racer, s_racers> m_racers;
		HovercraftDriver::Settings  m_driver;

		std::vector<std::string> m_arena_names;
		std::vector<std::string> m_circuit_names;
		std::vector<RaceMap>     m_arenas;
		std::vector<RaceMap>     m_circuits;
	};
}
