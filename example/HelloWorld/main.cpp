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

class Cube : public Square::Scene::Component
           , public Square::Render::Renderable
{
public:
    SQUARE_OBJECT(Cube)
    
    Square::Geometry::OBoundingBox m_obb_local;
    Square::Geometry::OBoundingBox m_obb_global;
	Square::Shared< Square::Render::Mesh >         m_mesh;
    Square::Shared< Square::Resource::Material >   m_material;
    Square::Weak<Square::Render::Transform>        m_transform;
    
    Cube(Square::Context& context) : Component(context)
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Scene;
        using namespace Square::Resource;
        using namespace Square::Render;
        using namespace Square::Render::Layout;
		//layout
		m_mesh = MakeShared<Mesh>(context);
        //init
		std::vector<Position3DNormalTangetBinomialUV> vertexs
        {
            
            { Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.0f, 0.0f) }, // +Y (top face)
            { Vec3(0.5f, 0.5f, -0.5f), Vec3(0.0f, 1.0f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3(0.5f, 0.5f,  0.5f), Vec3(0.0f, 1.0f, 0.0f), Vec2(1.0f, 1.0f) },
            { Vec3(-0.5f, 0.5f,  0.5f), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.0f, 1.0f) },
            
            { Vec3(-0.5f, -0.5f,  0.5f), Vec3(0.0f, -1.0f, 0.0f), Vec2(0.0f, 0.0f) }, // -Y (bottom face)
            { Vec3(0.5f, -0.5f,  0.5f), Vec3(0.0f, -1.0f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3(0.5f, -0.5f, -0.5f), Vec3(0.0f, -1.0f, 0.0f), Vec2(1.0f, 1.0f) },
            { Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.0f, -1.0f, 0.0f), Vec2(0.0f, 1.0f) },
            
            { Vec3(0.5f,  0.5f,  0.5f), Vec3(1.0f, 0.0f, 0.0f), Vec2(0.0f, 0.0f) }, // +X (right face)
            { Vec3(0.5f,  0.5f, -0.5f), Vec3(1.0f, 0.0f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3(0.5f, -0.5f, -0.5f), Vec3(1.0f, 0.0f, 0.0f), Vec2(1.0f, 1.0f) },
            { Vec3(0.5f, -0.5f,  0.5f), Vec3(1.0f, 0.0f, 0.0f), Vec2(0.0f, 1.0f) },
            
            { Vec3(-0.5f,  0.5f, -0.5f), Vec3(-1.0f, 0.0f, 0.0f), Vec2(0.0f, 0.0f) }, // -X (left face)
            { Vec3(-0.5f,  0.5f,  0.5f), Vec3(-1.0f, 0.0f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3(-0.5f, -0.5f,  0.5f), Vec3(-1.0f, 0.0f, 0.0f), Vec2(1.0f, 1.0f) },
            { Vec3(-0.5f, -0.5f, -0.5f), Vec3(-1.0f, 0.0f, 0.0f), Vec2(0.0f, 1.0f) },
            
            { Vec3(-0.5f,  0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.0f, 0.0f) }, // +Z (front face)
            { Vec3(0.5f,  0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(1.0f, 0.0f) },
            { Vec3(0.5f, -0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(1.0f, 1.0f) },
            { Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.0f, 1.0f) },
            
            { Vec3(0.5f,  0.5f, -0.5f), Vec3(0.0f, 0.0f, -1.0f), Vec2(0.0f, 0.0f) }, // -Z (back face)
            { Vec3(-0.5f,  0.5f, -0.5f), Vec3(0.0f, 0.0f, -1.0f), Vec2(1.0f, 0.0f) },
            { Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.0f, 0.0f, -1.0f), Vec2(1.0f, 1.0f) },
            { Vec3(0.5f, -0.5f, -0.5f), Vec3(0.0f, 0.0f, -1.0f), Vec2(0.0f, 1.0f) },
        };
		std::vector<unsigned int> ids
		{
			0, 2, 1,
			0, 3, 2,

			4, 6, 5,
			4, 7, 6,

			8, 10, 9,
			8, 11, 10,

			12, 14, 13,
			12, 15, 14,

			16, 18, 17,
			16, 19, 18,

			20, 22, 21,
			20, 23, 22
		};
		//compute tangent and binomial
		Square::tangent_model_fast(ids, vertexs);
		//build gpu buffers
		m_mesh->build(vertexs, ids);
		//cube as obb
        m_obb_global =
        m_obb_local = Geometry::OBoundingBox
        (
            Constants::identity<Mat3>()
           ,Vec3(0,0,0)
           ,Vec3(0.5,0.5,0.5)
        );
    }
    

	virtual size_t materials_count() const { return 1; }

	virtual Square::Weak<Square::Render::Material> material(size_t i = 0) const override
	{
		return Square::DynamicPointerCast<Square::Render::Material>(m_material);
	}
    
    virtual void draw
	(
		  Square::Render::Context&          render
		, size_t							material_id
		, Square::Render::EffectPassInputs& input
		, Square::Render::EffectPass&       pass
	) override
    {

		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		using namespace Square::Render;
		using namespace Square::Render::Layout;
		//bind
		pass.bind(render, input, m_material->parameters());
		//draw
		m_mesh->draw(render);
		//unbind
		pass.unbind();
    }
    
    virtual bool support_culling() const override
    {
        return true;
    }
    
    virtual void on_transform() override
    {
        if(auto transform = m_transform.lock())
        {
            using namespace Square;
            m_obb_global = m_obb_local * transform->global_model_matrix();
        }
    }

    
    virtual const Square::Geometry::OBoundingBox& bounding_box() override
    {
        return m_obb_global;
    }
	    
    virtual Square::Weak<Square::Render::Transform> transform() const override
    {
        return m_transform;
    }
    
    //events
    virtual void on_attach(Square::Scene::Actor& entity) override
    {
        m_transform = Square::DynamicPointerCast<Square::Render::Transform>(entity.shared_from_this());
    }
    virtual void on_deattch() override
    {
    //m_transform = {nullptr};
    }
    virtual void on_message(const Square::Scene::Message& msg) override  {  }
    
    //regs
    static void object_registration(Square::Context& context) { context.add_object<Cube>(); }
    
    //serialize
    virtual void serialize(Square::Data::Archive& archivie)  override   {  Square::Data::serialize(archivie, this); }
    virtual void serialize_json(Square::Data::Json& archivie) override  {  /* none */ }
    //deserialize
    virtual void deserialize(Square::Data::Archive& archivie) override { Square::Data::deserialize(archivie, this); }
    virtual void deserialize_json(Square::Data::Json& archivie) override {  /* none */ }
};
SQUARE_CLASS_OBJECT_REGISTRATION(Cube);

class Game01 : public Square::AppInterface
{
public:

	bool m_turn{ true };
	
	void key_event(Square::Video::KeyboardEvent key, short mode, Square::Video::ActionEvent action)
	{
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//move vel
		const float velocity = 100.0  * application().last_delta_time();
		//
		switch (key)
		{
		case Square::Video::KEY_ESCAPE:
		case Square::Video::KEY_Q: m_loop = false; break;
		case Square::Video::KEY_W: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_F: Square::Application::instance()->fullscreen(true); break;

		case Square::Video::KEY_C:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				std::cout << "FPS avg: " << m_counter.get() << std::endl;
			}
			break;
		case Square::Video::KEY_T:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				m_turn = !m_turn;
			}
		break;
		case Square::Video::KEY_D:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				auto dlight = m_level->actor("light0")->component<DirectionLight>();
				dlight->visible(!dlight->visible());
			}
		break;
		case Square::Video::KEY_P:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				auto plight = m_level->actor("light1")->component<PointLight>();
				plight->visible(!plight->visible());
			}
		break;
		case Square::Video::KEY_S:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				auto slight = m_level->actor("light2")->component<SpotLight>();
				slight->visible(!slight->visible());
			}
		break;
		case Square::Video::KEY_LEFT:  m_level->actor("light1")->translation({ velocity,0,0 });   break;
		case Square::Video::KEY_RIGHT: m_level->actor("light1")->translation({ -velocity,0,0 });  break;
		case Square::Video::KEY_UP:    m_level->actor("light1")->translation({ 0,0,velocity });   break;
		case Square::Video::KEY_DOWN:  m_level->actor("light1")->translation({ 0,0,-velocity });  break;
		case Square::Video::KEY_PAGE_UP:    m_level->actor("light1")->translation({ 0,velocity,0 });   break;
		case Square::Video::KEY_PAGE_DOWN:  m_level->actor("light1")->translation({ 0,-velocity,0 });  break;
		default: break;
		}
	}

    void start()
    {
		using namespace Square;		
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//rs file
		context().add_resources(Filesystem::resource_dir() + "/resources.rs");
		//level
		m_level = context().create<Level>();
        m_drawer = Square::MakeShared<Render::Drawer>(context());
		m_drawer->add(MakeShared<Render::DrawerPassForward>(context()));
		m_drawer->add(MakeShared<Render::DrawerPassShadow>(context()));
        //camera
        auto camera = m_level->actor("camera");
        camera->translation({ 0,0,0 });
        camera->rotation(rotate_euler(0.0f,Square::radians(180.0f),0.0f));
        camera->component<Camera>()->viewport({0,0, 1280, 768});
        camera->component<Camera>()->perspective(radians(90.0), 1280. / 768., 0.1, 1000.);		
		//test
		auto node = m_level->actor("main_node");
		node->translation({ 0,0, 10.0 });
		//background
		auto background = m_level->actor("background");
		//model + material + position + scale
		background->component<Cube>()->m_material = context().resource<Material>("box");
		background->position({ 0.0, -6.0, 10.0 });
		background->scale({ 20.0, 1.0, 20.0 });
		//light
		auto light0 = m_level->actor("light0");
		light0->component<DirectionLight>()->diffuse({ 1.0,0.0,0.0 });
		light0->component<DirectionLight>()->visible(false);
		light0->rotation(rotate_euler(0.0f, Square::radians(-90.0f), 0.0f));

		auto light1 = m_level->actor("light1");
		light1->component<PointLight>()->radius(20.0, 15.0);
		light1->component<PointLight>()->diffuse({ 0.0,1.0,0.0 });
		light1->component<PointLight>()->specular({ 0.0,1.0,0.0 });
		light1->component<PointLight>()->visible(false);
		light1->component<PointLight>()->shadow({ 512,512 });
		light1->position({ 0,0,10 });

		auto light2 = m_level->actor("light2");
		light2->component<SpotLight>()->radius(15.0, 5.0);
		light2->component<SpotLight>()->diffuse({ 1.0,1.0,1.0 });
		light2->component<SpotLight>()->specular({ 1.0,1.0,1.0 });
		light2->component<SpotLight>()->shadow({ 512,512 });
		light2->component<SpotLight>()->visible(false);
		light2->rotation(rotate_euler(Square::radians(-90.0f), 0.0f, 0.0f));
		light2->position({ 0,0,10 });
		//n cubes
		int n_cubes = 2;
		float radius_dist = 5.0f;
		//get
		for (int i = 0; i != n_cubes; ++i)
		{
			auto player = m_level->actor("player"+std::to_string(i)); 
			//add to main node
			node->add(player);
			//model + material
			player->component<Cube>()->m_material = context().resource<Material>("box");
			//angle
			float angle = float(i) / (n_cubes) * Constants::pi2<float>();
			//local pos
			player->position({ 
				std::cos(angle) * radius_dist, 
				std::sin(angle) * radius_dist,
				0.0
			});
		}
		//wrongs
		for (const std::string& wrong : Square::reverse( context().wrongs() ))
		{
			std::cout << wrong << std::endl;
		}
    }
    
    bool run(double dt)
    {
		using namespace Square;
		if(m_turn)
			m_level->actor("main_node")->turn(
				rotate_euler<float>(0, 0, Square::Constants::pi2<float>() * 0.1 * dt)
			);
        for(auto child : m_level->actor("main_node")->childs())
        {
            child->turn(rotate_euler<float>(0.0f,radians(90.0f*dt),0.0f));
        }
        m_drawer->draw( 
			  Vec4(0.25,0.5,1.0,1.0)
			, Vec4(0.1,0.1,0.1,1.0)
			, m_level->randerable_collection()
		);
		//fps counter
		m_counter.count_frame();
		//loop event
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
	Square::Time::FPSCounter				   m_counter;
	Square::Shared<Square::Scene::Level>	   m_level;
    Square::Shared<Square::Render::Drawer>     m_drawer;
};

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//Create square application
	Application app;
#ifdef _DEBUG & 1
	bool debug = true;
#else
	bool debug = false;
#endif
	//test
    app.execute(
      WindowSizePixel({ 1280, 768 })
    , WindowMode::NOT_RESIZABLE
	, 
#if defined(_WIN32) & 1
	  WindowRenderDriver
	  {
		 Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, debug
	  }
#else
	  WindowRenderDriver
	  {
		 Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, debug
	  }
#endif
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
