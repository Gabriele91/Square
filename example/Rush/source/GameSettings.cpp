//
//  GameSettings.cpp
//  Rush
//
//  See GameSettings.h.
//
#include <algorithm>
#include <sstream>
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
	    && m_weather == other.m_weather;
}

std::string GameSettings::path()
{
	using namespace Square::Filesystem;
	return join(home_dir(), ".rush", "settings.cfg");
}

bool GameSettings::load()
{
	using namespace Square;
	const std::string file = path();
	if (!Filesystem::exists(file)) return false;
	std::istringstream lines(Filesystem::text_file_read_all(file));
	std::string name;
	int value = 0;
	while (lines >> name >> value)
	{
		if      (name == "fullscreen")  m_fullscreen = value != 0;
		else if (name == "resolution")  m_resolution = std::clamp(value, 0, s_resolutions_count - 1);
		else if (name == "show_fps")    m_show_fps = value != 0;
		else if (name == "reflections") m_reflections = std::clamp(value, 0, 2);
		else if (name == "occlusion")   m_occlusion = std::clamp(value, 0, 2);
		else if (name == "shadows")     m_shadows = std::clamp(value, 0, 2);
		else if (name == "bloom")       m_bloom = value != 0;
		else if (name == "weather")     m_weather = value != 0;
	}
	return true;
}

bool GameSettings::save() const
{
	using namespace Square;
	Filesystem::makedir(Filesystem::get_directory(path()));
	std::ostringstream lines;
	lines << "fullscreen "  << int(m_fullscreen) << "\n"
	      << "resolution "  << m_resolution << "\n"
	      << "show_fps "    << int(m_show_fps) << "\n"
	      << "reflections " << m_reflections << "\n"
	      << "occlusion "   << m_occlusion << "\n"
	      << "shadows "     << m_shadows << "\n"
	      << "bloom "       << int(m_bloom) << "\n"
	      << "weather "     << int(m_weather) << "\n";
	return Filesystem::text_file_write_all(path(), lines.str());
}

void GameSettings::apply_window() const
{
	using namespace Square;
	auto* app = Application::instance();
	if (!app) return;
	if (app->fullscreen() != m_fullscreen) app->fullscreen(m_fullscreen);
	//the size of the window (in the fullscreen: the one of the screen)
	if (m_fullscreen) return;
	const IVec2 size = s_resolutions[std::clamp(m_resolution, 0, s_resolutions_count - 1)];
	if (app->window_size() != size) app->window_size(size);
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
