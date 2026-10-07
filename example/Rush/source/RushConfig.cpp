//
//  RushConfig.cpp
//  Rush
//
//  See RushConfig.h.
//
#include <algorithm>
#include <Square/Data/Json.h>
#include <RushConfig.h>

namespace Rush
{
	namespace AuxConfig
	{
		using Square::Data::JsonValue;

		//the configuration of the game (Config::get)
		static Config s_config;

		//a field of an object: its value if it has it (of its type), else the default
		static double number(const JsonValue& json, const std::string& name, double value)
		{
			if (!json.is_object() || !json.contains(name)) return value;
			const JsonValue& field = json[name];
			if (!field.is_number()) return value;
			return field.number();
		}

		static float number(const JsonValue& json, const std::string& name, float value)
		{
			return float(number(json, name, double(value)));
		}

		static int number(const JsonValue& json, const std::string& name, int value)
		{
			return int(number(json, name, double(value)));
		}

		static bool boolean(const JsonValue& json, const std::string& name, bool value)
		{
			if (!json.is_object() || !json.contains(name)) return value;
			const JsonValue& field = json[name];
			if (!field.is_boolean()) return value;
			return field.boolean();
		}

		static std::string string(const JsonValue& json, const std::string& name, const std::string& value)
		{
			if (!json.is_object() || !json.contains(name)) return value;
			const JsonValue& field = json[name];
			if (!field.is_string()) return value;
			return field.string();
		}

		//a vector: an array of numbers ([x, y] or [x, y, z])
		static Square::Vec3 vec3(const JsonValue& json, const std::string& name, const Square::Vec3& value)
		{
			if (!json.is_object() || !json.contains(name)) return value;
			const JsonValue& field = json[name];
			if (!field.is_array() || field.size() < 3) return value;
			Square::Vec3 result = value;
			for (size_t i = 0; i != 3; ++i)
			{
				if (field[i].is_number()) result[int(i)] = float(field[i].number());
			}
			return result;
		}

		static Square::Vec2 vec2(const JsonValue& json, const std::string& name, const Square::Vec2& value)
		{
			if (!json.is_object() || !json.contains(name)) return value;
			const JsonValue& field = json[name];
			if (!field.is_array() || field.size() < 2) return value;
			Square::Vec2 result = value;
			for (size_t i = 0; i != 2; ++i)
			{
				if (field[i].is_number()) result[int(i)] = float(field[i].number());
			}
			return result;
		}

		//a fog: on when it is there
		static RaceFog fog(const JsonValue& json)
		{
			RaceFog fog;
			if (!json.is_object()) return fog;
			fog.m_on      = true;
			fog.m_color   = vec3(json, "color", fog.m_color);
			fog.m_density = number(json, "density", fog.m_density);
			fog.m_height  = number(json, "height", fog.m_height);
			fog.m_falloff = number(json, "falloff", fog.m_falloff);
			fog.m_sun     = vec3(json, "sun", fog.m_sun);
			return fog;
		}

		//an object of a field (nullptr: none)
		static const JsonValue* object(const JsonValue& json, const std::string& name)
		{
			if (!json.is_object() || !json.contains(name)) return nullptr;
			const JsonValue& field = json[name];
			if (!field.is_object()) return nullptr;
			return &field;
		}

		//the names of an array of strings
		static std::vector<std::string> names(const JsonValue& json, const std::string& name)
		{
			std::vector<std::string> result;
			if (!json.is_object() || !json.contains(name)) return result;
			const JsonValue& field = json[name];
			if (!field.is_array()) return result;
			result.reserve(field.size());
			for (const JsonValue& value : field.array())
			{
				if (value.is_string()) result.push_back(value.string());
			}
			return result;
		}

		//a file of the folder parsed (false: missing, not an object)
		static bool read(Square::Context& context, const std::string& path, Square::Data::Json& json)
		{
			using namespace Square;
			if (!Filesystem::exists(path))
			{
				context.logger()->warning("Rush config: missing " + path);
				return false;
			}
			const bool parsed = json.parser(Filesystem::text_file_read_all(path));
			if (!parsed || !json.document().is_object())
			{
				context.logger()->warning("Rush config: not valid " + path);
				return false;
			}
			return true;
		}
	}

	bool Config::load(Square::Context& context, const std::string& folder, const std::string& assets)
	{
		using namespace Square;
		Config& config = AuxConfig::s_config;
		config = Config();
		bool loaded = true;
		//the game, the hovercraft, the grounds
		Data::Json game, hovercraft, surfaces;
		if (AuxConfig::read(context, Filesystem::join(folder, "game.json"), game))
		{
			loaded &= config.load_game(game.document());
		}
		else
		{
			loaded = false;
		}
		if (AuxConfig::read(context, Filesystem::join(folder, "hovercraft.json"), hovercraft))
		{
			loaded &= config.load_hovercraft(hovercraft.document());
		}
		else
		{
			loaded = false;
		}
		if (AuxConfig::read(context, Filesystem::join(folder, "surfaces.json"), surfaces))
		{
			loaded &= config.load_surfaces(surfaces.document());
		}
		else
		{
			loaded = false;
		}
		//the maps of the title: the ones with their scene
		const std::string maps = Filesystem::join(folder, "maps");
		auto read_maps = [&](const std::vector<std::string>& names, std::vector<RaceMap>& out)
		{
			out.reserve(names.size());
			for (const std::string& name : names)
			{
				const std::string scene = Filesystem::join(assets, name + ".sqz");
				if (!Filesystem::exists(scene))
				{
					context.logger()->info("Rush config: the map " + name + " has no scene (" + scene + "), not shown");
					continue;
				}
				Data::Json json;
				if (!AuxConfig::read(context, Filesystem::join(maps, name + ".json"), json))
				{
					loaded = false;
					continue;
				}
				RaceMap map;
				loaded &= config.load_map(json.document(), name, map);
				out.push_back(map);
			}
		};
		read_maps(config.m_arena_names, config.m_arenas);
		read_maps(config.m_circuit_names, config.m_circuits);
		return loaded;
	}

	const Config& Config::get()
	{
		return AuxConfig::s_config;
	}

	bool Config::load_game(const Square::Data::JsonValue& root)
	{
		using AuxConfig::number;
		using AuxConfig::object;
		if (const auto* rules = object(root, "rules"))
		{
			m_rules.m_winning_score  = number(*rules, "winning_score", m_rules.m_winning_score);
			m_rules.m_start_time     = number(*rules, "start_time", m_rules.m_start_time);
			m_rules.m_go_time        = number(*rules, "go_time", m_rules.m_go_time);
			m_rules.m_max_frame_time = number(*rules, "max_frame_time", m_rules.m_max_frame_time);
		}
		if (const auto* spawn = object(root, "spawn"))
		{
			m_spawn_fallback = AuxConfig::vec3(*spawn, "fallback", m_spawn_fallback);
		}
		if (const auto* circuit = object(root, "circuit"))
		{
			m_circuit.m_speed         = number(*circuit, "speed", m_circuit.m_speed);
			m_circuit.m_trails_window = number(*circuit, "trails_window", m_circuit.m_trails_window);
			m_circuit.m_fall_depth    = number(*circuit, "fall_depth", m_circuit.m_fall_depth);
			m_circuit.m_zone_blend    = number(*circuit, "zone_blend", m_circuit.m_zone_blend);
		}
		if (const auto* boost = object(root, "boost"))
		{
			m_boost.m_radius       = number(*boost, "radius", m_boost.m_radius);
			m_boost.m_time         = number(*boost, "time", m_boost.m_time);
			m_boost.m_speed        = number(*boost, "speed", m_boost.m_speed);
			m_boost.m_acceleration = number(*boost, "acceleration", m_boost.m_acceleration);
		}
		if (const auto* trails = object(root, "trails"))
		{
			m_trails_sink = number(*trails, "sink", m_trails_sink);
		}
		if (const auto* title = object(root, "title"))
		{
			m_title_origin = AuxConfig::vec3(*title, "origin", m_title_origin);
			if (const auto* fog = object(*title, "fog")) m_title_fog = AuxConfig::fog(*fog);
		}
		const auto* maps = object(root, "maps");
		if (!maps) return false;
		m_arena_names = AuxConfig::names(*maps, "arenas");
		m_circuit_names = AuxConfig::names(*maps, "circuits");
		return !m_arena_names.empty();
	}

	bool Config::load_hovercraft(const Square::Data::JsonValue& root)
	{
		using AuxConfig::number;
		using AuxConfig::object;
		//the racers, in order (the first: the player)
		if (root.contains("racers") && root["racers"].is_array())
		{
			const auto& racers = root["racers"].array();
			const size_t count = std::min(racers.size(), s_racers);
			for (size_t id = 0; id != count; ++id)
			{
				const auto& json = racers[id];
				Racer& racer = m_racers[id];
				racer.m_name    = AuxConfig::string(json, "name", racer.m_name);
				racer.m_color   = AuxConfig::string(json, "color", racer.m_color);
				racer.m_skin    = AuxConfig::string(json, "skin", racer.m_skin);
				racer.m_courage = number(json, "courage", racer.m_courage);
				if (const auto* engine = object(json, "engine"))
				{
					racer.m_engine.m_acceleration = number(*engine, "acceleration", racer.m_engine.m_acceleration);
					racer.m_engine.m_max_speed    = number(*engine, "max_speed", racer.m_engine.m_max_speed);
					racer.m_engine.m_drag         = number(*engine, "drag", racer.m_engine.m_drag);
				}
			}
		}
		//the driver: its base values
		const auto* driver = object(root, "driver");
		if (!driver) return false;
		m_driver.gravity           = number(*driver, "gravity", m_driver.gravity);
		m_driver.acceleration      = number(*driver, "acceleration", m_driver.acceleration);
		m_driver.drag              = number(*driver, "drag", m_driver.drag);
		m_driver.air_drag          = number(*driver, "air_drag", m_driver.air_drag);
		m_driver.max_speed         = number(*driver, "max_speed", m_driver.max_speed);
		m_driver.max_reverse       = number(*driver, "max_reverse", m_driver.max_reverse);
		m_driver.turn              = number(*driver, "turn", m_driver.turn);
		m_driver.idle_turn         = number(*driver, "idle_turn", m_driver.idle_turn);
		m_driver.floor_normal_y    = number(*driver, "floor_normal_y", m_driver.floor_normal_y);
		m_driver.body_radius_scale = AuxConfig::vec2(*driver, "body_radius_scale", m_driver.body_radius_scale);
		return true;
	}

	bool Config::load_surfaces(const Square::Data::JsonValue& root)
	{
		using AuxConfig::number;
		//each ground by its name (as the names of the meshes of a map: surface_of)
		for (const auto& entry : root.object())
		{
			if (!entry.second.is_object()) continue;
			const size_t surface = size_t(surface_of(entry.first));
			if (surface >= m_driver.grips.size()) continue;
			auto& grip = m_driver.grips[surface];
			grip.acceleration = number(entry.second, "acceleration", grip.acceleration);
			grip.max_speed    = number(entry.second, "max_speed", grip.max_speed);
			grip.drag         = number(entry.second, "drag", grip.drag);
			grip.turn         = number(entry.second, "turn", grip.turn);
			grip.follow       = number(entry.second, "follow", grip.follow);
		}
		return true;
	}

	bool Config::load_map(const Square::Data::JsonValue& root, const std::string& name, RaceMap& map)
	{
		using AuxConfig::boolean;
		map.m_name   = name;
		map.m_title  = AuxConfig::string(root, "title", name);
		map.m_snow   = boolean(root, "snow", map.m_snow);
		map.m_trails = boolean(root, "trails", map.m_trails);
		map.m_wakes  = boolean(root, "wakes", map.m_wakes);
		map.m_laps   = AuxConfig::number(root, "laps", map.m_laps);
		const bool circuit = AuxConfig::string(root, "mode", "arena") == "circuit";
		map.m_mode = circuit ? RaceMode::CIRCUIT : RaceMode::ARENA;
		if (const auto* fog = AuxConfig::object(root, "fog")) map.m_fog = AuxConfig::fog(*fog);
		//the zones of a circuit (its fog: the first one's)
		if (root.contains("zones") && root["zones"].is_array())
		{
			const auto& zones = root["zones"].array();
			map.m_zones.reserve(zones.size());
			for (const auto& json : zones)
			{
				CircuitZone zone;
				zone.m_gate = size_t(AuxConfig::number(json, "gate", 0));
				if (const auto* fog = AuxConfig::object(json, "fog")) zone.m_fog = AuxConfig::fog(*fog);
				map.m_zones.push_back(zone);
			}
		}
		if (!map.m_zones.empty() && !AuxConfig::object(root, "fog")) map.m_fog = map.m_zones.front().m_fog;
		return true;
	}

	HovercraftDriver::Settings Config::driver(size_t id, bool circuit) const
	{
		HovercraftDriver::Settings settings = m_driver;
		//collision types of body, wheels and ground
		settings.body_type  = TYPE_BODY;
		settings.wheel_type = TYPE_WHEEL;
		settings.scene_type = TYPE_SCENE;
		//its engine
		const Engine& engine = racer(id).m_engine;
		settings.acceleration *= engine.m_acceleration;
		settings.max_speed    *= engine.m_max_speed;
		settings.max_reverse  *= engine.m_max_speed;
		settings.drag          = std::min(settings.drag * engine.m_drag, 0.999f);
		//a circuit: faster (its reverse as in an arena)
		if (circuit)
		{
			settings.acceleration *= m_circuit.m_speed;
			settings.max_speed    *= m_circuit.m_speed;
		}
		return settings;
	}

	const RaceMap* Config::map(const std::string& name) const
	{
		for (const RaceMap& map : m_arenas)
		{
			if (map.m_name == name) return &map;
		}
		for (const RaceMap& map : m_circuits)
		{
			if (map.m_name == name) return &map;
		}
		return nullptr;
	}
}
