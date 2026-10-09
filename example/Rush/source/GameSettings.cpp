//
//  GameSettings.cpp
//  Rush
//
//  See GameSettings.h.
//
#include <algorithm>
#include <functional>
#include <sstream>
#include <Square/Data/Json.h>
#include <GameSettings.h>
#include <Graphics.h>
#include <RushConfig.h>

const Square::IVec2 GameSettings::s_resolutions[GameSettings::s_resolutions_count]
{
	{ 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 }
};

namespace AuxGameSettings
{
	//the order of the levels (Rush::Config::levels: the same), a name not among them after
	static size_t rank(const std::string& name)
	{
		static const char* ranks[]{ "off", "super_low", "very_low", "low", "medium", "high", "ultra", "best" };
		for (size_t i = 0; i != sizeof(ranks) / sizeof(ranks[0]); ++i)
		{
			if (name == ranks[i]) return i;
		}
		return sizeof(ranks) / sizeof(ranks[0]);
	}

	//the levels of the files before version 5 (numbers): their names
	static std::string old_effect(int level, int version)
	{
		//(1: 0 off, 1 low, 2 high; 2: no medium, high and ultra one lower)
		if (version <= 1)
		{
			switch (level)
			{
			case 1:  return "low";
			case 2:  return "high";
			default: return "off";
			}
		}
		if (version == 2 && level >= 3) ++level;
		static const char* names[]{ "off", "super_low", "low", "medium", "high", "ultra" };
		return names[std::clamp(level, 0, 5)];
	}

	static std::string old_bloom(int level, int version)
	{
		//(1: on or off)
		if (version <= 1) return level ? "medium" : "off";
		static const char* names[]{ "off", "low", "medium", "high" };
		return names[std::clamp(level, 0, 3)];
	}

	static std::string old_motion_blur(int level)
	{
		static const char* names[]{ "off", "low", "high" };
		return names[std::clamp(level, 0, 2)];
	}

	static std::string old_shadows(int level, int version)
	{
		//(3 and before: 0 low, 1 medium, 2 high)
		if (version <= 3)
		{
			switch (level)
			{
			case 0:  return "very_low";
			case 1:  return "low";
			default: return "high";
			}
		}
		static const char* names[]{ "off", "very_low", "low", "medium", "high", "ultra", "best" };
		return names[std::clamp(level, 0, 6)];
	}
}

int GameSettings::level(const std::string& effect, const std::string& name)
{
	const auto& levels = Rush::Config::get().levels(effect);
	if (name == "off" || levels.empty()) return 0;
	for (size_t i = 0; i != levels.size(); ++i)
	{
		if (levels[i].m_name == name) return int(i) + 1;
	}
	//(not in the configuration: the nearest higher one, else the highest)
	const size_t wanted = AuxGameSettings::rank(name);
	for (size_t i = 0; i != levels.size(); ++i)
	{
		if (AuxGameSettings::rank(levels[i].m_name) >= wanted) return int(i) + 1;
	}
	return int(levels.size());
}

std::string GameSettings::level_name(const std::string& effect, int level)
{
	const auto& levels = Rush::Config::get().levels(effect);
	if (level <= 0 || levels.empty()) return "off";
	const size_t index = std::min(size_t(level), levels.size()) - 1;
	return levels[index].m_name;
}

bool GameSettings::operator == (const GameSettings& other) const
{
	return m_fullscreen == other.m_fullscreen
	    && m_resolution == other.m_resolution
	    && m_show_fps == other.m_show_fps
	    && m_reflections == other.m_reflections
	    && m_occlusion == other.m_occlusion
	    && m_shadows == other.m_shadows
	    && m_bloom == other.m_bloom
	    && m_motion_blur == other.m_motion_blur
	    && m_god_rays == other.m_god_rays
	    && m_weather == other.m_weather
	    && m_antialiasing == other.m_antialiasing;
}

std::string GameSettings::path()
{
	using namespace Square::Filesystem;
	return join(join(join(app_data_dir(), "square"), "rush"), "setting.json");
}

namespace AuxGameSettings
{
	//a field of the file: its number if it has it, else as it was
	static int field(const Square::Data::JsonValue& json, const std::string& name, int value)
	{
		if (!json.contains(name)) return value;
		const auto& field = json[name];
		if (field.is_number())  return int(field.number());
		if (field.is_boolean()) return field.boolean() ? 1 : 0;
		return value;
	}

	static bool field(const Square::Data::JsonValue& json, const std::string& name, bool value)
	{
		return field(json, name, int(value)) != 0;
	}
}

namespace AuxGameSettings
{
	//a level of an effect of the file: its name, or a number of an older file (by convert)
	static int level(const Square::Data::JsonValue& json, const std::string& effect, int value, const std::function<std::string(int)>& convert)
	{
		if (!json.contains(effect)) return value;
		const auto& field = json[effect];
		if (field.is_string()) return GameSettings::level(effect, field.string());
		if (field.is_number()) return GameSettings::level(effect, convert(int(field.number())));
		if (field.is_boolean()) return GameSettings::level(effect, convert(field.boolean() ? 1 : 0));
		return value;
	}
}

bool GameSettings::load()
{
	using namespace Square;
	using AuxGameSettings::field;
	//the defaults (the levels of the configuration)
	m_reflections = level("reflections", "high");
	m_occlusion   = level("occlusion", "low");
	m_shadows     = level("shadows", "high");
	m_bloom       = level("bloom", "medium");
	m_motion_blur = level("motion_blur", "low");
	m_god_rays    = level("god_rays", "medium");
	const std::string file = path();
	if (!Filesystem::exists(file)) return false;
	Data::Json json;
	if (!json.parser(Filesystem::text_file_read_all(file)) || !json.document().is_object()) return false;
	const Data::JsonValue& root = json.document();
	m_fullscreen   = field(root, "fullscreen", m_fullscreen);
	m_resolution   = std::clamp(field(root, "resolution", m_resolution), 0, s_resolutions_count - 1);
	m_show_fps     = field(root, "show_fps", m_show_fps);
	//the levels (an older file: numbers, as now)
	const int version = field(root, "version", 1);
	using AuxGameSettings::level;
	m_reflections  = level(root, "reflections", m_reflections, [version](int n) { return AuxGameSettings::old_effect(n, version); });
	m_occlusion    = level(root, "occlusion", m_occlusion, [version](int n) { return AuxGameSettings::old_effect(n, version); });
	m_shadows      = level(root, "shadows", m_shadows, [version](int n) { return AuxGameSettings::old_shadows(n, version); });
	m_bloom        = level(root, "bloom", m_bloom, [version](int n) { return AuxGameSettings::old_bloom(n, version); });
	m_motion_blur  = level(root, "motion_blur", m_motion_blur, [](int n) { return AuxGameSettings::old_motion_blur(n); });
	m_god_rays     = level(root, "god_rays", m_god_rays, [](int n) { return n ? std::string("medium") : std::string("off"); });
	m_weather      = field(root, "weather", m_weather);
	m_antialiasing = field(root, "antialiasing", m_antialiasing);
	return true;
}

bool GameSettings::save() const
{
	using namespace Square;
	//its folders: <home>/square, <home>/square/rush (one at a time)
	const std::string rush = Filesystem::get_directory(path());
	const std::string square = Filesystem::get_directory(rush);
	if (!Filesystem::exists(square)) Filesystem::makedir(square);
	if (!Filesystem::exists(rush)) Filesystem::makedir(rush);
	std::ostringstream text;
	text << "{\n";
	text << "\t\"version\": " << s_version << ",\n";
	text << "\t\"fullscreen\": " << (m_fullscreen ? "true" : "false") << ",\n";
	text << "\t\"resolution\": " << m_resolution << ",\n";
	text << "\t\"show_fps\": " << (m_show_fps ? "true" : "false") << ",\n";
	text << "\t\"reflections\": \"" << level_name("reflections", m_reflections) << "\",\n";
	text << "\t\"occlusion\": \"" << level_name("occlusion", m_occlusion) << "\",\n";
	text << "\t\"shadows\": \"" << level_name("shadows", m_shadows) << "\",\n";
	text << "\t\"bloom\": \"" << level_name("bloom", m_bloom) << "\",\n";
	text << "\t\"motion_blur\": \"" << level_name("motion_blur", m_motion_blur) << "\",\n";
	text << "\t\"god_rays\": \"" << level_name("god_rays", m_god_rays) << "\",\n";
	text << "\t\"weather\": " << (m_weather ? "true" : "false") << ",\n";
	text << "\t\"antialiasing\": " << (m_antialiasing ? "true" : "false") << "\n";
	text << "}\n";
	return Filesystem::text_file_write_all(path(), text.str());
}

void GameSettings::apply_window() const
{
	using namespace Square;
	auto* app = Application::instance();
	if (!app) return;
	const IVec2 size = s_resolutions[std::clamp(m_resolution, 0, s_resolutions_count - 1)];
	if (m_fullscreen)
	{
		//the mode of the screen: the size of the window as it goes fullscreen (another size:
		//out of the fullscreen, its size, in again)
		if (app->fullscreen() && app->window_size() != size) app->fullscreen(false);
		if (!app->fullscreen())
		{
			if (app->window_size() != size) app->window_size(size);
			app->fullscreen(true);
		}
	}
	else
	{
		if (app->fullscreen()) app->fullscreen(false);
		if (app->window_size() != size) app->window_size(size);
	}
}

void GameSettings::apply_effects(Graphics& graphics) const
{
	using namespace Square;
	if (auto ssr = graphics.ssr())
	{
		ssr->enabled(m_reflections != 0);
		auto settings = ssr->settings();
		Rush::Config::get().reflections(level_name("reflections", m_reflections), settings);
		ssr->settings(settings);
	}
	if (auto ssao = graphics.ssao())
	{
		ssao->enabled(m_occlusion != 0);
		auto settings = ssao->settings();
		Rush::Config::get().occlusion(level_name("occlusion", m_occlusion), settings);
		ssao->settings(settings);
	}
	if (auto bloom = graphics.bloom())
	{
		bloom->enabled(m_bloom != 0);
		auto settings = bloom->settings();
		Rush::Config::get().bloom(level_name("bloom", m_bloom), settings);
		bloom->settings(settings);
	}
	if (auto motion_blur = graphics.motion_blur())
	{
		motion_blur->enabled(m_motion_blur != 0);
		auto settings = motion_blur->settings();
		Rush::Config::get().motion_blur(level_name("motion_blur", m_motion_blur), settings);
		motion_blur->settings(settings);
	}
	if (auto god_rays = graphics.god_rays())
	{
		god_rays->enabled(m_god_rays != 0);
		auto settings = god_rays->settings();
		Rush::Config::get().god_rays(level_name("god_rays", m_god_rays), settings);
		god_rays->settings(settings);
	}
	graphics.weather(m_weather);
	graphics.antialiasing(m_antialiasing);
}

void GameSettings::apply_shadows(const Square::Shared<Square::Scene::DirectionLight>& sun, float view) const
{
	using namespace Square;
	if (!sun) return;
	//none: no shadow map (the map loaded again: its own back)
	if (m_shadows == 0)
	{
		sun->shadow(IVec2(0));
		return;
	}
	//its level (config/graphics.json; none: the sun of the map as it is)
	Rush::ShadowSettings shadows;
	if (!Rush::Config::get().shadows(level_name("shadows", m_shadows), shadows)) return;
	sun->shadow_filter(shadows.m_filter);
	sun->cascades(shadows.m_cascades);
	sun->cascade_fit(shadows.m_fit);
	sun->cascade_margin(shadows.m_margin);
	//its shadow map: the map's scaled
	const IVec2 size = sun->shadow_size();
	if (shadows.m_map_scale != 1.0f && size.x > 0 && size.y > 0)
	{
		const Vec2 scaled_size = Vec2(size) * shadows.m_map_scale;
		const IVec2 new_size = glm::clamp(IVec2(scaled_size), IVec2(128), IVec2(8192));
		sun->shadow(new_size);
	}
	//its distance, never past the map as far as the camera sees it (view); 0, or a map without
	//a distance of its own: that view
	const float scaled = sun->shadow_distance() * shadows.m_distance;
	const bool  whole = scaled <= 0.0f;
	if (whole)
	{
		sun->shadow_distance(view);
	}
	else if (view > 0.0f)
	{
		sun->shadow_distance(std::min(scaled, view));
	}
	else
	{
		sun->shadow_distance(scaled);
	}
}
