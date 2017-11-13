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
		context.add<Sprite>();
        context.add<Sprite>(Square::attribute_field< Sprite >("scale", Square::Vec3(0,0,0), &Sprite::m_scale));
        context.add<Sprite>(Square::attribute_field< Sprite >("pos", Square::Vec3(0,0,0), &Sprite::m_pos));
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
		context.add<Body>();
        context.add<Body>(Square::attribute_method< Body >("gravity", Square::Vec3(0,0,0), &Body::get_gravity, &Body::set_gravity));
        context.add<Body>(Square::attribute_field< Body >("mass", float(0), &Body::m_mass));
        context.add<Body>(Square::attribute_field< Body >("nshape", int(0), &Body::m_nshape));
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

struct Men
{
    int m_age{ 0 };
    std::string m_name;
    std::vector< std::string > m_friends;
    
    void archive(Square::Data::Archive& ar)
    {
        ar % m_age % m_name % m_friends;
    }
    
    void print() const
    {
        std::cout << "Age: " << m_age << std::endl;
        std::cout << "Name: " << m_name << std::endl;
        std::cout << "Friends: " << std::endl;
        for(auto& afriend: m_friends) std::cout << "\t" << afriend << std::endl;
    }
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
	player.component<Sprite>()->m_scale = { 2,2 };
	player.component<Sprite>()->m_pos   = { 100,0 };
    //Serialize test
    {
        Men mario
        {
            15, "Mario", { "Luigi", "Anaconda" }
        };
        //serialize
        std::ofstream ofile("test.bin", std::ios::binary | std::ios::out);
        ArchiveBinWrite out(ofile);
        mario.archive(out);
    }
    //Deserialize test
    {
        
        Men mario{};
        //derialize
        std::ifstream ifile("test.bin", std::ios::binary | std::ios::in);
        ArchiveBinRead in(ifile);
        mario.archive(in);
        mario.print();
    }
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
