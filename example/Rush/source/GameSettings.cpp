//
//  GameSettings.cpp
//  Rush
//
//  See GameSettings.h.
//
#include <algorithm>
#include <sstream>
#include <Square/Data/Json.h>
#include <GameSettings.h>
#include <Graphics.h>

const Square::IVec2 GameSettings::s_resolutions[GameSettings::s_resolutions_count]
{
	{ 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 }
};

namespace AuxGameSettings
{
	//the reflections of a level: the size of the trace, its rays, its blur
	static void reflections(Square::Render::SSR::Settings& settings, int level)
	{
		using namespace Square::Render;
		switch (level)
		{
		case GameSettings::EFFECT_SUPER_LOW:
			settings.resolution   = PER_QUARTER;
			settings.steps        = 20;
			settings.max_distance = 40.0f;
			settings.blur         = SSR::Settings::BLUR_LOW;
			settings.denoise      = false;
		break;
		case GameSettings::EFFECT_LOW:
			settings.resolution   = PER_QUARTER;
			settings.steps        = 40;
			settings.max_distance = 60.0f;
			settings.blur         = SSR::Settings::BLUR_MEDIUM;
			settings.denoise      = false;
		break;
		case GameSettings::EFFECT_MEDIUM:
			settings.resolution   = PER_HALF;
			settings.steps        = 28;
			settings.max_distance = 50.0f;
			settings.blur         = SSR::Settings::BLUR_LOW;
			settings.denoise      = false;
		break;
		case GameSettings::EFFECT_HIGH:
			settings.resolution   = PER_HALF;
			settings.steps        = 40;
			settings.max_distance = 60.0f;
			settings.blur         = SSR::Settings::BLUR_MEDIUM;
			settings.denoise      = false;
		break;
		case GameSettings::EFFECT_ULTRA:
			settings.resolution   = PER_FULL;
			settings.steps        = 64;
			settings.max_distance = 90.0f;
			settings.blur         = SSR::Settings::BLUR_HIGH;
			settings.denoise      = true;
		break;
		default: break;
		}
	}

	//the occlusion of a level: its size, its blur, its reach on the screen (no super low: its
	//blur too light, the pattern of the samples shows; low instead)
	static void occlusion(Square::Render::SSAO::Settings& settings, int level)
	{
		using namespace Square::Render;
		switch (level)
		{
		case GameSettings::EFFECT_SUPER_LOW:
		case GameSettings::EFFECT_LOW:
			settings.resolution = PER_QUARTER;
			settings.blur       = SSAO::Settings::BLUR_LOW;
			settings.max_pixels = 32.0f;
		break;
		case GameSettings::EFFECT_MEDIUM:
			settings.resolution = PER_HALF;
			settings.blur       = SSAO::Settings::BLUR_LOW;
			settings.max_pixels = 32.0f;
		break;
		case GameSettings::EFFECT_HIGH:
			settings.resolution = PER_HALF;
			settings.blur       = SSAO::Settings::BLUR_MEDIUM;
			settings.max_pixels = 48.0f;
		break;
		case GameSettings::EFFECT_ULTRA:
			settings.resolution = PER_FULL;
			settings.blur       = SSAO::Settings::BLUR_HIGH;
			settings.max_pixels = 64.0f;
		break;
		default: break;
		}
	}

	//the bloom of a level: the levels of its chain (its width), how much of it
	static void bloom(Square::Render::Bloom::Settings& settings, int level)
	{
		switch (level)
		{
		case GameSettings::BLOOM_LOW:
			settings.levels    = 3;
			settings.intensity = 0.4f;
			settings.scatter   = 0.6f;
		break;
		case GameSettings::BLOOM_MEDIUM:
			settings.levels    = 5;
			settings.intensity = 0.5f;
			settings.scatter   = 0.7f;
		break;
		case GameSettings::BLOOM_HIGH:
			settings.levels    = 6;
			settings.intensity = 0.55f;
			settings.scatter   = 0.75f;
		break;
		default: break;
		}
	}

	//the motion blur of a level: how much of the motion, its samples
	static void motion_blur(Square::Render::MotionBlur::Settings& settings, int level)
	{
		switch (level)
		{
		case GameSettings::MOTION_BLUR_LOW:
			settings.shutter    = 0.35f;
			settings.max_pixels = 28.0f;
			settings.samples    = 8;
		break;
		case GameSettings::MOTION_BLUR_HIGH:
			settings.shutter    = 0.6f;
			settings.max_pixels = 48.0f;
			settings.samples    = 16;
		break;
		default: break;
		}
	}

	//a level of an effect of an older file as a level of now (1: 0 off, 1 low, 2 high; 2: no
	//medium, high and ultra one lower)
	static int effect_from_version(int level, int version)
	{
		if (version <= 1)
		{
			switch (level)
			{
			case 1:  return GameSettings::EFFECT_LOW;
			case 2:  return GameSettings::EFFECT_HIGH;
			default: return GameSettings::EFFECT_OFF;
			}
		}
		if (version == 2 && level >= GameSettings::EFFECT_MEDIUM) return level + 1;
		return level;
	}
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

bool GameSettings::load()
{
	using namespace Square;
	using AuxGameSettings::field;
	const std::string file = path();
	if (!Filesystem::exists(file)) return false;
	Data::Json json;
	if (!json.parser(Filesystem::text_file_read_all(file)) || !json.document().is_object()) return false;
	const Data::JsonValue& root = json.document();
	m_fullscreen   = field(root, "fullscreen", m_fullscreen);
	m_resolution   = std::clamp(field(root, "resolution", m_resolution), 0, s_resolutions_count - 1);
	m_show_fps     = field(root, "show_fps", m_show_fps);
	//(an older file: its levels as now; version 1, its bloom on: medium)
	const int version = field(root, "version", 1);
	m_reflections  = AuxGameSettings::effect_from_version(field(root, "reflections", m_reflections), version);
	m_reflections  = std::clamp(m_reflections, 0, int(EFFECT_LEVELS) - 1);
	m_occlusion    = AuxGameSettings::effect_from_version(field(root, "occlusion", m_occlusion), version);
	m_occlusion    = std::clamp(m_occlusion, 0, int(EFFECT_LEVELS) - 1);
	if (m_occlusion == EFFECT_SUPER_LOW) m_occlusion = EFFECT_LOW;
	m_shadows      = std::clamp(field(root, "shadows", m_shadows), 0, 2);
	m_bloom        = std::clamp(field(root, "bloom", m_bloom), 0, int(BLOOM_LEVELS) - 1);
	if (version <= 1) m_bloom = m_bloom ? BLOOM_MEDIUM : BLOOM_OFF;
	m_motion_blur  = std::clamp(field(root, "motion_blur", m_motion_blur), 0, int(MOTION_BLUR_LEVELS) - 1);
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
	text << "\t\"reflections\": " << m_reflections << ",\n";
	text << "\t\"occlusion\": " << m_occlusion << ",\n";
	text << "\t\"shadows\": " << m_shadows << ",\n";
	text << "\t\"bloom\": " << m_bloom << ",\n";
	text << "\t\"motion_blur\": " << m_motion_blur << ",\n";
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
		ssr->enabled(m_reflections != EFFECT_OFF);
		auto settings = ssr->settings();
		AuxGameSettings::reflections(settings, m_reflections);
		ssr->settings(settings);
	}
	if (auto ssao = graphics.ssao())
	{
		ssao->enabled(m_occlusion != EFFECT_OFF);
		auto settings = ssao->settings();
		AuxGameSettings::occlusion(settings, m_occlusion);
		ssao->settings(settings);
	}
	if (auto bloom = graphics.bloom())
	{
		bloom->enabled(m_bloom != BLOOM_OFF);
		auto settings = bloom->settings();
		AuxGameSettings::bloom(settings, m_bloom);
		bloom->settings(settings);
	}
	if (auto motion_blur = graphics.motion_blur())
	{
		motion_blur->enabled(m_motion_blur != MOTION_BLUR_OFF);
		auto settings = motion_blur->settings();
		AuxGameSettings::motion_blur(settings, m_motion_blur);
		motion_blur->settings(settings);
	}
	graphics.weather(m_weather);
	graphics.antialiasing(m_antialiasing);
}

void GameSettings::apply_shadows(const Square::Shared<Square::Scene::DirectionLight>& sun) const
{
	using namespace Square;
	if (!sun) return;
	switch (m_shadows)
	{
	case 0:
		sun->shadow_filter(Render::ShadowFilter::NONE);
		sun->cascades(2);
	break;
	case 1:
		sun->shadow_filter(Render::ShadowFilter::PCF);
		sun->cascades(DIRECTION_SHADOW_CSM_DEFAULT_FACES);
	break;
	default:
		sun->shadow_filter(Render::ShadowFilter::PCSS);
		sun->cascades(DIRECTION_SHADOW_CSM_DEFAULT_FACES);
	break;
	}
}
