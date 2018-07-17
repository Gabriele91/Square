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
		context().add_resource_file(Filesystem::resource_dir() + "/assets/example.png");
        context().add_resource_file(Filesystem::resource_dir() + "/assets/effect.hlsl");
        context().add_resource_file(Filesystem::resource_dir() + "/assets/effect.sqfx");
		context().add_resource_file(Filesystem::resource_dir() + "/assets/header.hlsl");
		context().add_resource_file(Filesystem::resource_dir() + "/assets/effect.mat");

		//shader
        //m_shader = context().resource<Shader>("effect");
		m_effect = context().resource<Effect>("effect");
		m_material = context().resource<Material>("effect");

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
		auto p_l_s = m_level->find_actor("player.left.a\\.child\\.");
		auto p_r_s = m_level->find_actor("player.right.the\\.child");

		//test
		std::cout << "child1: " << p_l_s->name() << std::endl;
		std::cout << "child2: " << p_r_s->name() << std::endl;
		
		//wrongs
		for (const std::string& wrong : Square::reverse( context().wrongs() ))
		{
			std::cout << wrong << std::endl;
		}

		//int 
		build_buffers();
    }
    
	struct Vertex
	{
		Square::Vec3 m_position;
		Square::Vec3 m_color;   
	};

	Square::Shared< Square::Render::ConstBuffer >  m_cbtransform;
	Square::Shared< Square::Render::ConstBuffer >  m_cbcamera;
	Square::Shared< Square::Render::VertexBuffer > m_model;
	Square::Shared< Square::Render::IndexBuffer >  m_index_model;
	Square::Shared< Square::Render::InputLayout >  m_input_layout;
	
	void build_buffers()
	{
		//test effect
		if(m_material && m_effect)
		{
			using namespace Square;
			using namespace Square::Data;
			using namespace Square::Scene;
			using namespace Square::Resource;
			//bind
			auto& pass = m_effect->techniques().begin()->second.operator[](0);

			//input layout
			m_input_layout = Render::input_layout(&render(), pass.m_shader->base_shader(), Render::AttributeList{
				{ Render::ATT_POSITION,  Render::AST_FLOAT3, 0                        },
				{ Render::ATT_COLOR0,    Render::AST_FLOAT3, offsetof(Vertex,m_color) }
			});
			//model
			m_model = Render::stream_vertex_buffer< Vertex >(&render(), 8);
			auto umodel = Render::map_buffer < Vertex > (&render(), m_model.get(), 8);
			umodel[0].m_position = { -1.0, -1.0,  1.0 }; umodel[0].m_color = { 1,0,0 };
			umodel[1].m_position = {  1.0, -1.0,  1.0 }; umodel[1].m_color = { 0,1,0 };
			umodel[2].m_position = {  1.0,  1.0,  1.0 }; umodel[2].m_color = { 0,0,1 };
			umodel[3].m_position = { -1.0,  1.0,  1.0 }; umodel[3].m_color = { 1,1,1 };

			umodel[4].m_position = { -1.0, -1.0, -1.0 }; umodel[4].m_color = { 1,0,0 };
			umodel[5].m_position = {  1.0, -1.0, -1.0 }; umodel[5].m_color = { 0,1,0 };
			umodel[6].m_position = {  1.0,  1.0, -1.0 }; umodel[6].m_color = { 0,0,1 };
			umodel[7].m_position = { -1.0,  1.0, -1.0 }; umodel[7].m_color = { 1,1,1 };
			Render::unmap_buffer(&render(), m_model.get());

			m_index_model = Render::stream_index_buffer(&render(), 6 * 6);
			auto uimodel = Render::map_buffer(&render(), m_index_model.get(), 6 * 6);
			unsigned int ids[]
			{
				// front
				0, 1, 2,
				2, 3, 0,
				// right
				1, 5, 6,
				6, 2, 1,
				// back
				7, 6, 5,
				5, 4, 7,
				// left
				4, 0, 3,
				3, 7, 4,
				// bottom
				4, 5, 1,
				1, 0, 4,
				// top
				3, 2, 6,
				6, 7, 3,
			};
			std::memcpy(uimodel, ids, sizeof(unsigned int)*6*6);
			Render::unmap_buffer(&render(), m_index_model.get());
			//transform
			m_cbtransform = Render::stream_constant_buffer<UniformBufferTransform>(&render());
			auto utransform = Render::map_buffer<UniformBufferTransform>(&render(), m_cbtransform.get());
			utransform->m_position = { 0,0,5.0 };
			utransform->m_scale = { 1,1,1 };
			utransform->m_rotation = Mat3(1);
			utransform->m_model = translate(Mat4(1.0f), { utransform->m_position });
			Render::unmap_buffer(&render(), m_cbtransform.get());

			//camera
			m_cbcamera = Render::stream_constant_buffer<UniformBufferCamera>(&render());
			auto ucamera = Render::map_buffer<UniformBufferCamera>(&render(), m_cbcamera.get());
			ucamera->m_viewport = { 0, 0, 1280, 768 };
			ucamera->m_position = { 0, 5.0,-10. };
			ucamera->m_projection = perspective<float>(radians(90.0f), ucamera->m_viewport.z / ucamera->m_viewport.w, 0.1f,100.0f);
			ucamera->m_view = look_at(ucamera->m_position, Vec3(0.0, 0.0, 5.0), Vec3(0.0, 1.0, 0.0));
			ucamera->m_model = inverse(ucamera->m_view);
			Render::unmap_buffer(&render(), m_cbcamera.get());
			
		}
	}

	void set_transform_actor(Square::Scene::Actor& actor)
	{
		using namespace Square;
		using namespace Square::Scene;
		auto utransform = Render::map_buffer<UniformBufferTransform>(&render(), m_cbtransform.get());
		actor.set(utransform);
		Render::unmap_buffer(&render(), m_cbtransform.get());
	}

    bool run(double dt)
    {		
		//test effect
		if(m_material && m_effect)
		{
			using namespace Square;
			using namespace Square::Data;
			using namespace Square::Scene;
			using namespace Square::Resource;
			//by actor
			Actor actor(context());
			//bind
			auto& pass = m_effect->techniques().begin()->second.operator[](0);
			//start
			render().set_viewport_state(Render::ViewportState({ 0, 0, 1280, 768 }));
			render().clear();
			//errors?
			render().print_errors();
			//bin
			pass.bind(&render(), m_cbcamera.get(), m_cbtransform.get(), m_material->parameters());
			//draw
			const float dist = 15.0;
			const float nmax = 30.0;
			render().bind_VBO(m_model.get());
			render().bind_IBO(m_index_model.get());
			render().bind_IL(m_input_layout.get());
			for (float n = 0; n < nmax; ++n)
			{
				actor.position({ 
					 std::cos(Constants::pi2<float>()*(n / nmax))*dist
					,0
					,std::sin(Constants::pi2<float>()*(n / nmax))*dist+10.0
				});
				set_transform_actor(actor);

				render().draw_elements(Render::DRAW_TRIANGLES, 6 * 6);
			}
			render().bind_IL(m_input_layout.get());
			render().unbind_IBO(m_index_model.get());
			render().unbind_VBO(m_model.get());
			//unbind
			pass.unbind();
			//errors?
			render().print_errors();
		}
		//frames
		std::cout << m_counter.count_frame() << std::endl;
		//
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
	Square::Shared<Square::Resource::Shader>   m_shader;
	Square::Shared<Square::Resource::Effect>   m_effect;
	Square::Shared<Square::Resource::Material> m_material;
	Square::Shared<Square::Scene::Level>	   m_level;
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
