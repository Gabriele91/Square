//
//  TitleScreen.h
//  Rush
//
//  The scene of the title screen (as the one of Limit Rush): the hovercraft turning on black,
//  framed at the bottom left (the menu of the title, RushUI, over it). A scene of its own in the
//  level it is given (s_title_world_level: the game runs it only in the menu, out of the
//  rendering and of the shadows of a race), far from the race (s_title_origin), with its camera
//  and a fill light; the hovercraft on a Turntable (it turns while the level runs).
//
#pragma once
#include <Square/Square.h>
#include <RushTypes.h>

class TitleScreen
{
public:

	TitleScreen(Square::Context& context);

	//its scene in a level
	void load(Square::Shared<Square::Scene::Level> level);

	//the background black (the title shown); off: the background of before back
	void show(bool show);

	//the camera of the window size
	void viewport(unsigned int width, unsigned int height);

private:

	void setup_camera();
	void setup_light();

	Square::Context&                     m_context;
	Square::Shared<Square::Scene::Level> m_level;
	Square::Shared<Square::Scene::Actor> m_root;
	Square::Shared<Square::Scene::Actor> m_hovercraft;
	Square::Shared<Square::Scene::Actor> m_camera;
	Square::Vec4                         m_clear_color{ 0.0f };
};
