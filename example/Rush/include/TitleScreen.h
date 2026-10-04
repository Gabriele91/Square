//
//  TitleScreen.h
//  Rush
//
//  The scene of the title screen (as the one of Limit Rush): the hovercraft turning on black,
//  framed at the bottom left (the menu of the title, RushUI, over it). A scene of its own in its
//  level of the world (s_title_world_level), far from the race (s_title_origin), with its
//  camera and a fill light, the background black; the level active only in the menu (out of
//  the rendering and of the shadows of a race).
//
#pragma once
#include <Square/Square.h>
#include <RushTypes.h>

class TitleScreen
{
public:

	TitleScreen(Square::Context& context);

	//its scene in its level of the world
	void load(Square::Scene::World& world);

	//its level active, the background black; off: not active, the background of before back
	void show(bool show, Square::Scene::World& world);

	//the camera of the window size
	void viewport(unsigned int width, unsigned int height);

	//the hovercraft turning
	void update(double delta_time);

private:

	void setup_camera();
	void setup_light();

	Square::Context&                     m_context;
	Square::Shared<Square::Scene::Level> m_level;
	Square::Shared<Square::Scene::Actor> m_root;
	Square::Shared<Square::Scene::Actor> m_hovercraft;
	Square::Shared<Square::Scene::Actor> m_camera;
	float                                m_yaw{ 30.0f };
	Square::Vec4                         m_clear_color{ 0.0f };
};
