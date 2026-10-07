//
//  GameSettings.h
//  Rush
//
//  The settings of the game chosen in the title (Settings): the display (fullscreen, the size
//  of the window, the frames per second shown) and the effects (reflections, ambient occlusion,
//  shadows, bloom, motion blur, the falling snow). Saved in the data of the user (square/rush/setting.json in
//  Filesystem::app_data_dir: %APPDATA% on Windows)
//  at every change, loaded at the start.
//
#pragma once
#include <string>
#include <Square/Square.h>

class Graphics;

struct GameSettings
{
	//the sizes of the window (resolution: one of them)
	static constexpr int s_resolutions_count = 5;
	static const Square::IVec2 s_resolutions[s_resolutions_count];

	bool m_fullscreen{ false };
	int  m_resolution{ 2 };   //1920 x 1080
	bool m_show_fps{ true };
	//the levels of the effects: 0 off, n the n-th level of the effect in config/graphics.json
	//(Rush::Config::levels, in order: the options of the menu); in the file by their names
	//(version 5; before: numbers of fixed levels)
	static constexpr int s_version = 5;

	int  m_reflections{ 0 };
	int  m_occlusion{ 0 };
	int  m_shadows{ 0 };
	int  m_bloom{ 0 };
	int  m_motion_blur{ 0 };
	int  m_god_rays{ 0 };
	bool m_weather{ true };   //the falling snow of the maps with snow
	bool m_antialiasing{ true }; //FXAA (the menu always: its shot)

	bool operator == (const GameSettings& other) const;
	bool operator != (const GameSettings& other) const { return !(*this == other); }

	//the file of the settings, read (the defaults first: high reflections, low occlusion, high
	//shadows, medium bloom, low motion blur, medium god rays; false: no file, the defaults), written
	static std::string path();
	bool load();
	bool save() const;

	//a level of an effect ("reflections", "occlusion", "shadows", "bloom", "motion_blur") by its
	//name ("off": 0; a name it does not have: its nearest higher level), its name
	static int level(const std::string& effect, const std::string& name);
	static std::string level_name(const std::string& effect, int level);

	//the window, the post effects; the shadows of a sun (of a map: once, at its load: its
	//distance from the one of the map, at most view: the map as far as the camera sees it,
	//min(the largest side of the map, the far of the camera); a level with distance 0: view)
	void apply_window() const;
	void apply_effects(Graphics& graphics) const;
	void apply_shadows(const Square::Shared<Square::Scene::DirectionLight>& sun, float view) const;
};
