//
//  RushConfig.cpp
//  Rush
//
//  See RushConfig.h.
//
#include <algorithm>
#include <cctype>
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

		//a name of a field as one of its values (by their names); value: none of them
		template < typename T >
		static T named(const JsonValue& json, const std::string& name, const std::vector< std::pair<std::string, T> >& values, T value)
		{
			const std::string text = string(json, name, std::string());
			for (const auto& entry : values)
			{
				if (entry.first == text) return entry.second;
			}
			return value;
		}

		static Square::Render::PostEffectResolution resolution(const JsonValue& json, Square::Render::PostEffectResolution value)
		{
			using namespace Square::Render;
			return named<PostEffectResolution>(json, "resolution", { { "full", PER_FULL }, { "half", PER_HALF }, { "quarter", PER_QUARTER } }, value);
		}

		//the order of the levels of the graphics (the others after them)
		static size_t level_rank(const std::string& name)
		{
			static const char* ranks[]{ "super_low", "very_low", "low", "medium", "high", "ultra", "best" };
			for (size_t rank = 0; rank != sizeof(ranks) / sizeof(ranks[0]); ++rank)
			{
				if (name == ranks[rank]) return rank;
			}
			return sizeof(ranks) / sizeof(ranks[0]);
		}

		//the title of a level from its name: "super_low", "Super low"
		static std::string level_title(const std::string& name)
		{
			std::string title = name;
			for (char& c : title)
			{
				if (c == '_') c = ' ';
			}
			if (!title.empty()) title[0] = char(std::toupper((unsigned char)title[0]));
			return title;
		}

		//the levels of an effect of graphics.json, in order
		static std::vector<GraphicsLevel> levels(const JsonValue& graphics, const std::string& effect)
		{
			std::vector<GraphicsLevel> result;
			const JsonValue* levels = object(graphics, effect);
			if (!levels) return result;
			for (const auto& entry : levels->object())
			{
				if (!entry.second.is_object()) continue;
				result.push_back({ entry.first, string(entry.second, "title", level_title(entry.first)) });
			}
			std::sort(result.begin(), result.end(), [](const GraphicsLevel& a, const GraphicsLevel& b)
			{
				const size_t rank_a = level_rank(a.m_name);
				const size_t rank_b = level_rank(b.m_name);
				if (rank_a != rank_b) return rank_a < rank_b;
				return a.m_name < b.m_name;
			});
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
		//the levels of the graphics
		Data::Json graphics;
		if (AuxConfig::read(context, Filesystem::join(folder, "graphics.json"), graphics))
		{
			config.m_graphics = graphics.document();
		}
		else
		{
			loaded = false;
		}
		for (const char* effect : s_graphics_effects)
		{
			config.m_levels[effect] = AuxConfig::levels(config.m_graphics, effect);
		}
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
		if (const auto* loading = object(root, "loading"))
		{
			m_loading.m_fade_in     = number(*loading, "fade_in", m_loading.m_fade_in);
			m_loading.m_fade_out    = number(*loading, "fade_out", m_loading.m_fade_out);
			m_loading.m_hold_frames = number(*loading, "hold_frames", m_loading.m_hold_frames);
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
		//its sun: where it is
		if (const auto* sun = AuxConfig::object(root, "sun_position"))
		{
			map.m_sun_set       = true;
			map.m_sun_azimuth   = AuxConfig::number(*sun, "azimuth", map.m_sun_azimuth);
			map.m_sun_elevation = AuxConfig::number(*sun, "elevation", map.m_sun_elevation);
			//its steps (an angle not written: the one of the sun)
			if (sun->contains("steps") && (*sun)["steps"].is_array())
			{
				const auto& steps = (*sun)["steps"].array();
				map.m_sun_steps.reserve(steps.size());
				for (const auto& json : steps)
				{
					SunStep step;
					step.m_time      = AuxConfig::number(json, "time", 0.0f);
					step.m_azimuth   = AuxConfig::number(json, "azimuth", map.m_sun_azimuth);
					step.m_elevation = AuxConfig::number(json, "elevation", map.m_sun_elevation);
					map.m_sun_steps.push_back(step);
				}
				std::sort(map.m_sun_steps.begin(), map.m_sun_steps.end(), [](const SunStep& a, const SunStep& b) { return a.m_time < b.m_time; });
			}
		}
		//its camera: its clip planes
		if (const auto* camera = AuxConfig::object(root, "camera"))
		{
			map.m_camera_near = AuxConfig::number(*camera, "near", map.m_camera_near);
			map.m_camera_far  = AuxConfig::number(*camera, "far", map.m_camera_far);
		}
		//its view (the photo of the map)
		if (const auto* view = AuxConfig::object(root, "view"))
		{
			map.m_view_set  = true;
			map.m_view_from = AuxConfig::vec3(*view, "from", map.m_view_from);
			map.m_view_to   = AuxConfig::vec3(*view, "to", map.m_view_to);
			map.m_view_lens = AuxConfig::number(*view, "lens", map.m_view_lens);
		}
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

	const std::vector<GraphicsLevel>& Config::levels(const std::string& effect) const
	{
		static const std::vector<GraphicsLevel> none;
		auto it = m_levels.find(effect);
		if (it == m_levels.end()) return none;
		return it->second;
	}

	const Square::Data::JsonValue* Config::graphics(const std::string& effect, const std::string& level) const
	{
		const Square::Data::JsonValue* levels = AuxConfig::object(m_graphics, effect);
		if (!levels) return nullptr;
		return AuxConfig::object(*levels, level);
	}

	bool Config::reflections(const std::string& level, Square::Render::SSR::Settings& settings) const
	{
		using namespace Square::Render;
		using AuxConfig::number;
		using AuxConfig::boolean;
		const Square::Data::JsonValue* json = graphics("reflections", level);
		if (!json) return false;
		settings.intensity      = number(*json, "intensity", settings.intensity);
		settings.max_distance   = number(*json, "max_distance", settings.max_distance);
		settings.steps          = number(*json, "steps", settings.steps);
		settings.thickness      = number(*json, "thickness", settings.thickness);
		settings.max_roughness  = number(*json, "max_roughness", settings.max_roughness);
		settings.edge_fade      = number(*json, "edge_fade", settings.edge_fade);
		settings.resolution     = AuxConfig::resolution(*json, settings.resolution);
		settings.screen_march   = boolean(*json, "screen_march", settings.screen_march);
		settings.blur           = AuxConfig::named<SSR::Settings::BlurQuality>(*json, "blur",
		{
			  { "off", SSR::Settings::BLUR_OFF }, { "low", SSR::Settings::BLUR_LOW }
			, { "medium", SSR::Settings::BLUR_MEDIUM }, { "high", SSR::Settings::BLUR_HIGH }
		}, settings.blur);
		settings.denoise        = boolean(*json, "denoise", settings.denoise);
		settings.denoise_radius = number(*json, "denoise_radius", settings.denoise_radius);
		return true;
	}

	bool Config::occlusion(const std::string& level, Square::Render::SSAO::Settings& settings) const
	{
		using namespace Square::Render;
		using AuxConfig::number;
		const Square::Data::JsonValue* json = graphics("occlusion", level);
		if (!json) return false;
		settings.radius         = number(*json, "radius", settings.radius);
		settings.intensity      = number(*json, "intensity", settings.intensity);
		settings.bias           = number(*json, "bias", settings.bias);
		settings.contrast       = number(*json, "contrast", settings.contrast);
		settings.max_pixels     = number(*json, "max_pixels", settings.max_pixels);
		settings.resolution     = AuxConfig::resolution(*json, settings.resolution);
		settings.blur           = AuxConfig::named<SSAO::Settings::BlurQuality>(*json, "blur",
		{
			  { "off", SSAO::Settings::BLUR_OFF }, { "very_low", SSAO::Settings::BLUR_VERY_LOW }
			, { "low", SSAO::Settings::BLUR_LOW }, { "medium", SSAO::Settings::BLUR_MEDIUM }
			, { "high", SSAO::Settings::BLUR_HIGH }
		}, settings.blur);
		settings.blur_sharpness = number(*json, "blur_sharpness", settings.blur_sharpness);
		return true;
	}

	bool Config::bloom(const std::string& level, Square::Render::Bloom::Settings& settings) const
	{
		using AuxConfig::number;
		const Square::Data::JsonValue* json = graphics("bloom", level);
		if (!json) return false;
		settings.threshold = number(*json, "threshold", settings.threshold);
		settings.knee      = number(*json, "knee", settings.knee);
		settings.intensity = number(*json, "intensity", settings.intensity);
		settings.scatter   = number(*json, "scatter", settings.scatter);
		settings.levels    = number(*json, "levels", settings.levels);
		settings.clamp     = number(*json, "clamp", settings.clamp);
		return true;
	}

	bool Config::motion_blur(const std::string& level, Square::Render::MotionBlur::Settings& settings) const
	{
		using AuxConfig::number;
		const Square::Data::JsonValue* json = graphics("motion_blur", level);
		if (!json) return false;
		settings.shutter    = number(*json, "shutter", settings.shutter);
		settings.max_pixels = number(*json, "max_pixels", settings.max_pixels);
		settings.min_pixels = number(*json, "min_pixels", settings.min_pixels);
		settings.samples    = number(*json, "samples", settings.samples);
		return true;
	}

	bool Config::god_rays(const std::string& level, Square::Render::GodRays::Settings& settings) const
	{
		using AuxConfig::number;
		const Square::Data::JsonValue* json = graphics("god_rays", level);
		if (!json) return false;
		settings.color      = AuxConfig::vec3(*json, "color", settings.color);
		settings.intensity  = number(*json, "intensity", settings.intensity);
		settings.threshold  = number(*json, "threshold", settings.threshold);
		settings.sun_radius = number(*json, "sun_radius", settings.sun_radius);
		settings.density    = number(*json, "density", settings.density);
		settings.decay      = number(*json, "decay", settings.decay);
		settings.weight     = number(*json, "weight", settings.weight);
		settings.samples    = number(*json, "samples", settings.samples);
		settings.resolution = AuxConfig::resolution(*json, settings.resolution);
		settings.fade       = number(*json, "fade", settings.fade);
		settings.volumetric   = AuxConfig::boolean(*json, "volumetric", settings.volumetric);
		settings.max_distance = number(*json, "max_distance", settings.max_distance);
		settings.steps        = number(*json, "steps", settings.steps);
		settings.anisotropy   = number(*json, "anisotropy", settings.anisotropy);
		settings.air          = number(*json, "air", settings.air);
		return true;
	}

	bool Config::shadows(const std::string& level, ShadowSettings& settings) const
	{
		using namespace Square::Render;
		using AuxConfig::number;
		const Square::Data::JsonValue* json = graphics("shadows", level);
		if (!json) return false;
		settings.m_filter   = AuxConfig::named<ShadowFilter>(*json, "filter",
		{
			{ "none", ShadowFilter::NONE }, { "pcf", ShadowFilter::PCF }, { "pcss", ShadowFilter::PCSS }
		}, settings.m_filter);
		settings.m_cascades = number(*json, "cascades", settings.m_cascades);
		settings.m_distance = number(*json, "distance", settings.m_distance);
		settings.m_map_scale = number(*json, "map_scale", settings.m_map_scale);
		return true;
	}
}
