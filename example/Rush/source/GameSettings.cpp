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
	//the resolution of an effect of a level (1 low: a quarter of the frame, 2 high: half)
	static Square::Render::PostEffectResolution resolution(int level)
	{
		return level >= 2 ? Square::Render::PER_HALF : Square::Render::PER_QUARTER;
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
	m_reflections  = std::clamp(field(root, "reflections", m_reflections), 0, 2);
	m_occlusion    = std::clamp(field(root, "occlusion", m_occlusion), 0, 2);
	m_shadows      = std::clamp(field(root, "shadows", m_shadows), 0, 2);
	m_bloom        = field(root, "bloom", m_bloom);
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
	text << "\t\"fullscreen\": " << (m_fullscreen ? "true" : "false") << ",\n";
	text << "\t\"resolution\": " << m_resolution << ",\n";
	text << "\t\"show_fps\": " << (m_show_fps ? "true" : "false") << ",\n";
	text << "\t\"reflections\": " << m_reflections << ",\n";
	text << "\t\"occlusion\": " << m_occlusion << ",\n";
	text << "\t\"shadows\": " << m_shadows << ",\n";
	text << "\t\"bloom\": " << (m_bloom ? "true" : "false") << ",\n";
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
		ssr->enabled(m_reflections > 0);
		auto settings = ssr->settings();
		settings.resolution = AuxGameSettings::resolution(m_reflections);
		ssr->settings(settings);
	}
	if (auto ssao = graphics.ssao())
	{
		ssao->enabled(m_occlusion > 0);
		auto settings = ssao->settings();
		settings.resolution = AuxGameSettings::resolution(m_occlusion);
		ssao->settings(settings);
	}
	if (auto bloom = graphics.bloom()) bloom->enabled(m_bloom);
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
