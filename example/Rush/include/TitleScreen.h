//
//  TitleScreen.h
//  Rush
//
//  The scene of the title screen: a cinematic shot behind the menu (RushUI over it), the scene
//  "title/scene" (origial_assets/title: the water at sunset, the city, the tower, the arena; the
//  far things sprites) with the hovercraft of the player on the right, a racer behind it
//  ("racer_1" of the scene), still on the painted water, their fans turning. A
//  scene of its own in the level it is given (s_title_world_level: the game runs it only in the
//  menu, out of the rendering and of the shadows of a race), far from the race (its origin in the config),
//  with its camera; its water moved (PBRWater: water_time).
//
#pragma once
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>

class TitleScreen
{
public:

	TitleScreen(Square::Context& context);

	//its scene in a level
	void load(Square::Shared<Square::Scene::Level> level);

	//the background of the title (its sky) shown; off: the background of before back
	void show(bool show);

	//the camera of the window size
	void viewport(unsigned int width, unsigned int height);

	//a frame of the title: the water, the hovercraft on the waves
	void update(double delta_time);

	//where the light of its sun goes (the haze of the title)
	Square::Vec3 sun_direction() const;

private:

	//a hovercraft of the title: its skin, where, its heading (degrees, around y)
	Square::Shared<Square::Scene::Actor> hovercraft(const std::string& skin, const Square::Vec3& position, float yaw);
	void setup_camera();
	void find_water();

	struct Rider
	{
		Square::Shared<Square::Scene::Actor> m_actor;
		Square::Vec3                         m_position{ 0.0f };
		float                                m_yaw{ 0.0f };
		float                                m_phase{ 0.0f };
	};

	Square::Context&                                          m_context;
	Square::Shared<Square::Scene::Level>                      m_level;
	Square::Shared<Square::Scene::Actor>                      m_root;
	Square::Shared<Square::Scene::Actor>                      m_scene;
	Square::Shared<Square::Scene::Actor>                      m_camera;
	std::vector<Rider>                                        m_riders;
	std::vector< Square::Shared<Square::Resource::Material> > m_water;
	double                                                    m_time{ 0.0 };
	Square::Vec4                                              m_clear_color{ 0.0f };
};
