//
//  GameSettings.h
//  Rush
//
//  The settings of the game chosen in the title (Settings): the display (fullscreen, the size
//  of the window, the frames per second shown) and the effects (reflections, ambient occlusion,
//  shadows, bloom, the falling snow). Saved in the data of the user (square/rush/setting.json in
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
	int  m_reflections{ 2 };  //0 off, 1 low (a quarter of the frame), 2 high (half)
	int  m_occlusion{ 1 };    //0 off, 1 low (a quarter), 2 high (half)
	int  m_shadows{ 2 };      //0 low (hard, two cascades), 1 medium (PCF), 2 high (PCSS)
	bool m_bloom{ true };
	bool m_weather{ true };   //the falling snow of the maps with snow
	bool m_antialiasing{ true }; //FXAA (the menu always: its shot)

	bool operator == (const GameSettings& other) const;
	bool operator != (const GameSettings& other) const { return !(*this == other); }

	//the file of the settings, read (false: none, the defaults kept), written
	static std::string path();
	bool load();
	bool save() const;

	//the window, the post effects; the shadows of a sun (of a map: at its load)
	void apply_window() const;
	void apply_effects(Graphics& graphics) const;
	void apply_shadows(const Square::Shared<Square::Scene::DirectionLight>& sun) const;
};
