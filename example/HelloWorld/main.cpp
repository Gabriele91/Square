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
    Square::Shared< Square::Resource::Material >   m_material;
    Square::Weak<Square::Render::Transform>        m_transform;
    Square::Shared< Square::Render::VertexBuffer > m_model;
    Square::Shared< Square::Render::IndexBuffer >  m_index_model;
    
    Cube(Square::Context& context) : Component(context)
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Scene;
        using namespace Square::Resource;
        using namespace Square::Render;
        using namespace Square::Render::Layout;
        layout_id_from_object_name("Vertex3DNormalUV");
        //init
        Vertex3DNormalUV model[]
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
        m_model  = Render::vertex_buffer< Vertex3DNormalUV >(context.render(), (unsigned char*)model, 24);
        //indexs
        unsigned int ids[]
        {
            0, 1, 2,
            0, 2, 3,
            
            4, 5, 6,
            4, 6, 7,
            
            8, 9, 10,
            8, 10, 11,
            
            12, 13, 14,
            12, 14, 15,
            
            16, 17, 18,
            16, 18, 19,
            
            20, 21, 22,
            20, 22, 23
        };
        m_index_model = Render::index_buffer(context.render(), ids, 6*6);
        //cube
        m_obb_global =
        m_obb_local = Geometry::OBoundingBox
        (
            Constants::identity<Mat3>()
           ,Vec3(0,0,0)
           ,Vec3(0.5,0.5,0.5)
        );
    }
    
    
    virtual void draw(Square::Render::Context& render) override
    {
        render.bind_VBO(m_model.get());
        render.bind_IBO(m_index_model.get());
        render.draw_elements(Square::Render::DRAW_TRIANGLES, 6*6);
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
    
    virtual Square::Weak<Square::Render::Material> material() const override
    {
        return Square::DynamicPointerCast<Square::Render::Material>(m_material);
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
        m_drawer = Square::MakeShared<Render::Drawer>(context());
        m_drawer->add(MakeShared<Render::DrawerPassForward>(context()));
		//rs file
        context().add_resource_path(Filesystem::resource_dir() + "/common");
		context().add_resource_file(Filesystem::resource_dir() + "/assets/example.png");
        context().add_resource_file(Filesystem::resource_dir() + "/assets/effect.sqfx");
		context().add_resource_file(Filesystem::resource_dir() + "/assets/effect.mat");
        //camera
        auto camera = m_level->actor("camera");
        camera->translation({ 0,0,0 });
        camera->rotation(to_quad(0.0f,Square::radians(180.0f),0.0f));
        camera->component<Camera>()->viewport({0,0, 1280, 768});
        camera->component<Camera>()->perspective(radians(90.0), 1280. / 768., 0.1, 1000.);
		//test
		auto player = m_level->actor("player");
		player->translation({ 0,0, 10.0 });
        player->component<Cube>()->m_material = context().resource<Material>("effect");
		//wrongs
		for (const std::string& wrong : Square::reverse( context().wrongs() ))
		{
			std::cout << wrong << std::endl;
		}
    }
    
    bool run(double dt)
    {
        m_drawer->draw( 
			  Square::Vec4(0.25,0.5,1.0,1.0)
			, Square::Vec4(1.0)
			, m_level->randerable_collection()
		);
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
	//test
    app.execute(
      WindowSizePixel({ 1280, 768 })
    , WindowMode::NOT_RESIZABLE
	, 
#if defined(_WIN32) & 1
	  WindowRenderDriver
	  {
		 Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, false
	  }
#else
	  WindowRenderDriver
	  {
		 Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, true
	  }
#endif
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
