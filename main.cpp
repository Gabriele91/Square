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
    
    Sprite(const std::string& name, size_t id) : Component(name, id)
    {
        
    }
    
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
REGISTERED_COMPONENT(Sprite, "Sprite")
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
    player.add(ComponentFactory::create("Sprite")); 
	player.component<Sprite>();
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
