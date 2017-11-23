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
#include <fstream>

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
    
    static void object_registration(Square::Context& context)
    {
		context.add_object<Sprite>();
        context.add_attributes<Sprite>(Square::attribute_field< Sprite >("scale", Square::Vec2(0,0), &Sprite::m_scale));
        context.add_attributes<Sprite>(Square::attribute_field< Sprite >("pos", Square::Vec2(0,0), &Sprite::m_pos));
    }
};
SQUARE_CLASS_OBJECT_REGISTRATION(Sprite);


class Body : public Square::Scene::Component
{
public:
    SQUARE_OBJECT(Body)

	Square::Vec3 m_gravity;
	float	     m_mass;
	int 	     m_nshape;

	void  set_gravity(const Square::Vec3& g){ m_gravity = g; }
	const Square::Vec3& get_gravity() const { return m_gravity; }

	void  set_mass(float m)       { m_mass = m; }
	float get_mass()		const { return m_mass; }

	static void object_registration(Square::Context& context)
	{
		context.add_object<Body>();
        context.add_attributes<Body>(Square::attribute_method< Body >("gravity", Square::Vec3(0,0,0), &Body::get_gravity, &Body::set_gravity));
        context.add_attributes<Body>(Square::attribute_field< Body >("mass", float(0), &Body::m_mass));
        context.add_attributes<Body>(Square::attribute_field< Body >("nshape", int(0), &Body::m_nshape));
	}
};
SQUARE_CLASS_OBJECT_REGISTRATION(Body);

class Game01 : public Square::AppInterface
{
public:

	void key_event(Square::Video::KeyboardEvent key, short mode, Square::Video::ActionEvent action)
	{
		switch (key)
		{
		case Square::Video::KEY_Q: m_loop = false; break;
		case Square::Video::KEY_W: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_F: Square::Application::instance()->fullscreen(true); break;
		default: break;
		}
	}

    void start()
    {
        using namespace Square;
        Application::context()->add_resource_file("example.png");
		Application::context()->resource<Resource::Texture>("example");
    }
    bool run(double dt)
    {
        return m_loop;
    }
    bool end()
    {
        return true;
    }
private:
    
    bool m_loop = true;
    
};

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//Create square application
	Application app;
	//define
	Json jin (Filesystem::text_file_read_all("scene.json"));
    //serialize + parsing/deserialize
	Json jout(((std::stringstream&)(std::stringstream() << jin)).str());
    //serialize again 
    std::cout << (jout.errors().size() ? jout.errors() : jout.document()) << std::endl;
    //test
    Scene::Actor player;
    player.translation({10,0,0});
	player.component<Sprite>()->m_scale = { 2,2 };
    player.component<Sprite>()->m_pos   = { 100,0 };
    player.component<Body>()->m_gravity = { 0,10, 0 };
    {
        std::ofstream ofile("agent.bin", std::ios::binary | std::ios::out);
        ArchiveBinWrite out(ofile);
        player.serialize(out);
    }
    {
        std::ifstream ifile("agent.bin", std::ios::binary | std::ios::in);
        ArchiveBinRead in(ifile);
        player.deserialize(in);
    }

    std::cout << "pos "
              << player.component<Sprite>()->m_pos.x
              << ", "
              << player.component<Sprite>()->m_pos.y;
    std::cout << std::endl;
    std::cout << "scale "
              << player.component<Sprite>()->m_scale.x
              << ", "
              << player.component<Sprite>()->m_scale.y;
    std::cout << std::endl;
	//test
    app.execute(
      WindowSizePixel({ 1280, 768 })
    , WindowMode::NOT_RESIZABLE
    , 4, 1
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
