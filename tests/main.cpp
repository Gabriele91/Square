//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <Square/Square.h>
#include <Square/Driver/Window.h>
#include <Square/Driver/Input.h>
#include <iostream>
#include <sstream>

class Sprite : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(Sprite)

	Square::Vec2 m_scale; //scale
	Square::Vec2 m_pos;   //2D pos

    Sprite(){ }
    
    virtual void on_attach(Square::Scene::Actor& entity) override
    {
        
    }
    virtual void on_deattch() override
    {
        
    }
    virtual void on_message(const Square::Scene::Message& msg) override
    {
        
    }
};
SQUARE_COMPONENT(Sprite)


class Square::Data::Serializable
{

};

class Body : Square::Data::Serializable
{
public:

	Square::Vec3 m_gravity;
	float	     m_mass;
	int 	     m_nshape;

	void  set_gravity(const Square::Vec3& g){ m_gravity = g; }
	const Square::Vec3& get_gravity()const  { return m_gravity; }

	void  set_mass(float m)       { m_mass = m; }
	float get_mass()		const { return m_mass; }
	
	static void attributes(Body* body)
	{
		auto a_gravity = Square::Data::attribute_method< Body >("gravity", Square::Vec3(0,0,0), &Body::get_gravity, &Body::set_gravity);
		auto a_mass = Square::Data::attribute_method< Body >("mass", float(0), &Body::get_mass, &Body::set_mass);
		auto a_nshape = Square::Data::attribute_field< Body >("nshape", int(0), &Body::m_nshape);
	}
};

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//define
	Json jin (Filesystem::text_file_read_all("scene.json"));
    //serialize + parsing/deserialize
	Json jout(((std::stringstream&)(std::stringstream() << jin)).str());
    //serialize again 
    std::cout << (jout.errors().size() ? jout.errors() : jout.document()) << std::endl;
    //test
    Scene::Actor player;
	player.component<Sprite>()->m_scale = { 2,2 };
	player.component<Sprite>()->m_pos   = { 100,0 };
	//test attributes
	Body b;
	Body::attributes(&b);
	//test
	Video::init();
	{
		Video::Screen screen = Video::Screen::by_index(0);

		Video::WindowInfo winfo(&screen);
		winfo.m_context.m_color = 24;
		winfo.m_size[0] = 1280;
		winfo.m_size[1] = 720;
		winfo.m_fullscreen = false;

		Video::Window window(winfo);
		window.acquire_context();

		Video::Input input(&window);
		//events
		bool exit = false;
		input.subscrive_window_listener([&](Video::WindowEvent e)
		{
			if (e == Video::WindowEvent::CLOSE) exit = true;
		});
		input.subscrive_keyboard_listener([&](Video::KeyboardEvent e, int mode, Video::ActionEvent action)
		{
			if (e == Video::KeyboardEvent::KEY_Q) exit = true;
			if (e == Video::KeyboardEvent::KEY_F) window.enable_fullscreen(true);
			if (e == Video::KeyboardEvent::KEY_W) window.enable_fullscreen(false);
		});
		//
		while (!exit)
		{
			Video::Input::pull_events();
			window.swap();
		}
	}
	Video::close();
    //End
    return 0;
}
