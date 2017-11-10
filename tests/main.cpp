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
    
    static void attributes(Square::SerializeContext& context)
    {
        context.add<Sprite>(Square::attribute_field< Sprite >("scale", Square::Vec3(0,0,0), &Sprite::m_scale));
        context.add<Sprite>(Square::attribute_field< Sprite >("pos", Square::Vec3(0,0,0), &Sprite::m_pos));
    }
};
SQUARE_COMPONENT_REGISTRATION(Sprite)


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
	
    static void attributes(Square::SerializeContext& context)
	{
        context.add<Body>(Square::attribute_method< Body >("gravity", Square::Vec3(0,0,0), &Body::get_gravity, &Body::set_gravity));
        context.add<Body>(Square::attribute_field< Body >("mass", float(0), &Body::m_mass));
        context.add<Body>(Square::attribute_field< Body >("nshape", int(0), &Body::m_nshape));
	}
};
SQUARE_COMPONENT_REGISTRATION(Body)

class Game01 : public Square::AppInterface
{
public:
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
    SerializeContext ctx;
    ComponentFactory::attributes_registration(ctx);
	//test
    Application app;
    app.execute(
      Square::WindowSizePercentage({ 50.0, 50.0 })
    , Square::WindowMode::NOT_RESIZABLE
    , 4, 1
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
