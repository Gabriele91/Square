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

	using Vec2     = Square::Vec2;
	using SRObject = Square::Shared < Square::ResourceObject >;
	using STexture = Square::Shared < Square::Resource::Texture >;
	
	Vec2    m_scale; //scale
	Vec2    m_pos;   //2D pos
	STexture m_texture;

	Sprite(Square::Context& context) : Component(context) { }
    
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
		context.add_attributes<Sprite>(Square::attribute_field< Sprite >("pos", Square::Vec2(0, 0), &Sprite::m_pos));
		context.add_attributes<Sprite>(Square::attribute_field< Sprite >("texture", STexture(nullptr), &Sprite::m_texture));
		//std::cout << Square::variant_traits<STexture>() << std::endl;
    }
    
    
    //serialize
    virtual void serialize(Square::Data::Archive& archivie)  override
    {
        //serialize this
        Square::Data::serialize(archivie, this);
    }
    virtual void serialize_json(Square::Data::Json& archivie) override
    {
        //none
    }
    //deserialize
    virtual void deserialize(Square::Data::Archive& archivie) override
    {
        //serialize this
        Square::Data::deserialize(archivie, this);
    }
    virtual void deserialize_json(Square::Data::Json& archivie) override
    {
        //none
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

	Body(Square::Context& context) : Component(context) { }

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
    
    //serialize
    virtual void serialize(Square::Data::Archive& archivie)  override
    {
        //serialize this
        Square::Data::serialize(archivie, this);
    }
    virtual void serialize_json(Square::Data::Json& archivie) override
    {
        //none
    }
    //deserialize
    virtual void deserialize(Square::Data::Archive& archivie) override
    {
        //serialize this
        Square::Data::deserialize(archivie, this);
    }
    virtual void deserialize_json(Square::Data::Json& archivie) override
    {
        //none
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
		case Square::Video::KEY_ESCAPE:
		case Square::Video::KEY_Q: m_loop = false; break;
		case Square::Video::KEY_W: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_F: Square::Application::instance()->fullscreen(true); break;
		default: break;
		}
	}

    void start()
    {
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//level
		m_level = context().create<Level>();

		//rs file
		context().add_resource_file("example.png");
        context().add_resource_file("effect.hlsl");
        context().add_resource_file("effect.shfx");
        context().add_resource_file("header.hlsl");

		//shader
        auto shader = context().resource<Shader>("effect");
        auto effect = context().resource<Effect>("effect");

		//test
		auto player = m_level->actor("player");
		player->translation({ 10,0,0 });
		player->component<Sprite>()->m_scale = { 2,2 };
		player->component<Sprite>()->m_pos = { 100,0 };
		player->component<Sprite>()->m_texture = context().resource<Texture>("example");
		player->component<Body>()->m_gravity = { 0,10, 0 };

		//chils 
		player->child("left")->child("a.child.");
		player->child("right")->child("the.child");

		//serialize
		serialize("level.sq");
		deserialize("level.sq");

		//find
		auto p_l_s = m_level->find("player.left.a\\.child\\.");
		auto p_r_s = m_level->find("player.right.the\\.child");

		//test
		std::cout << "child1: " << p_l_s->name() << std::endl;
		std::cout << "child2: " << p_r_s->name() << std::endl;
    }
    bool run(double dt)
    {
        return m_loop;
    }
    bool end()
    {
        return true;
    }

	//level serialize
	void serialize(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		std::ofstream ofile(path, std::ios::binary | std::ios::out);
		ArchiveBinWrite out(context(), ofile);
		m_level->serialize(out);
	}
	//level deserialize
	void deserialize(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		std::ifstream ifile(path, std::ios::binary | std::ios::in);
		ArchiveBinRead in(context(), ifile);
		m_level->deserialize(in);
	}

private:
    
    bool m_loop = true;
	Square::Shared<Square::Scene::Level> m_level;
};

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//Create square application
	Application app;
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
