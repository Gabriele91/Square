//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#define SQUARE_MAIN
#include <Square/Square.h>
#include <iostream>
#include <sstream>
#include <fstream>
// #define MODEL_LOAD // "adamHead" // "drone" // "box" // "crate"


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
	bool										   m_global_obb_dirty;
    
    Cube(Square::Context& context) : Component(context)
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Scene;
        using namespace Square::Resource;
        using namespace Square::Render;
        using namespace Square::Render::Layout;
		//layout
		m_mesh = MakeShared<Render::Mesh>(context);
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
		//
		m_global_obb_dirty = true;
    }
	virtual ~Cube()
	{
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
		m_global_obb_dirty = true;
    }

    
    virtual const Square::Geometry::OBoundingBox& bounding_box() override
    {
		if (m_global_obb_dirty)
		{
			if (auto transform = m_transform.lock())
			{
				m_obb_global = m_obb_local * transform->global_model_matrix();
			}
			else
			{
				m_obb_global = m_obb_local;
			}
			m_global_obb_dirty = false;
		}
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
		m_global_obb_dirty = true;
    }
    virtual void on_deattch() override
    {
		m_transform.reset();
		m_global_obb_dirty = true;
    }
    virtual void on_message(const Square::Scene::Message& msg) override  
	{
		if (auto str = msg.m_value.ptr<std::string>().value_or(nullptr))
		{
			context().logger()->info(*str);
		}
	}
    
    //regs
    static void object_registration(Square::Context& ctx)
	{ 
		//using
		using namespace Square;
		using namespace Square::Resource;
		//add cude
		ctx.add_object<Cube>(); 
		//material
		ctx.add_attribute_function<Cube, std::string>
		("material"
		, std::string()
		, [](const Cube* cube) -> std::string     
		{ if(cube->m_material) return cube->m_material->resource_untyped_name(); else return ""; }
		, [](Cube* cube, const std::string& name) 
		{ if(name.size()) cube->m_material = cube->context().resource<Material>(name); });
	}
    
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
		const float velocity = 20.0f  * application().last_delta_time();
		const auto  level_path = Square::Filesystem::join(Square::Filesystem::resource_dir(), "level.sq");
		//
		switch (key)
		{
		case Square::Video::KEY_ESCAPE:
		case Square::Video::KEY_Q: m_loop = false; break;
		case Square::Video::KEY_W: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_F: Square::Application::instance()->fullscreen(true); break;
		case Square::Video::KEY_N: serialize(level_path); break;
		case Square::Video::KEY_M: 
			if (Square::Filesystem::exists(level_path))
				deserialize(level_path);
		break;

		case Square::Video::KEY_C:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				context().logger()->info("FPS avg: " + std::to_string(m_counter.get()));
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

		case Square::Video::KEY_0:  
			m_level->actor("light0")->turn(rotate_euler( 
				0.0f, 
				Constants::pi2<float>() / 4.f * float(application().last_delta_time()),
				0.0f 
			));
			m_level->actor("box_center")->turn(rotate_euler(
				0.0f,
				Constants::pi2<float>() / 4.f * float(application().last_delta_time()),
				0.0f
			));
		break;
		case Square::Video::KEY_U:
			m_level->actor("camera")->move({ 0,0,-10 * application().last_delta_time() });
		break;
		case Square::Video::KEY_J:
			m_level->actor("camera")->move({ 0,0, 10 * application().last_delta_time() });
		break;
		case Square::Video::KEY_H:
			m_level->actor("camera")->move({ -10 * application().last_delta_time(),0,0 });
		break;
		case Square::Video::KEY_K:
			m_level->actor("camera")->move({ 10 * application().last_delta_time(),0,0 });
		break;
		case Square::Video::KEY_O:
			m_level->actor("camera")->move({ 0,-10 * application().last_delta_time(),0 });
		break;
		case Square::Video::KEY_L:
			m_level->actor("camera")->move({ 0, 10 * application().last_delta_time(),0 });
		break;
		case Square::Video::KEY_I:
			m_level->actor("camera")->turn(rotate_euler<float>(0,-1. * application().last_delta_time(),0 ));
		break;
		case Square::Video::KEY_Y:
			m_level->actor("camera")->turn(rotate_euler<float>(0, 1. * application().last_delta_time(), 0));
		break;
		break;
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
		context().add_resources(Filesystem::join(Filesystem::resource_dir(), "/resources.rs"));
		context().add_resources(Filesystem::join(Filesystem::resource_dir(), "../Model/resources.rs"));
		//level
		m_level = world().level();
        m_drawer = Square::MakeShared<Render::Drawer>(context());
		m_drawer->add(MakeShared<Render::DrawerPassForward>(context()));
		m_drawer->add(MakeShared<Render::DrawerPassShadow>(context()));
		#ifdef MODEL_LOAD
		{
			auto model = m_level->load_actor(MODEL_LOAD);
			model->translation({ 0.0,-2.0, 10.0 });
		}
		#endif
        //camera
        auto camera = m_level->actor("camera");
        camera->translation({ 0,0,-5.0 });
        camera->rotation(rotate_euler(0.0f,Square::radians(180.0f),0.0f));
        camera->component<Camera>()->viewport({0,0, 1280, 768});
        camera->component<Camera>()->perspective(radians(90.0f), 1280.0f / 768.0f, 0.1f, 30.0f);		
		//test
		auto node = m_level->actor("main_node");
		node->translation({ 0,0, 10.0 });
		//bottom box
		auto box_bottom = m_level->actor("box_bottom");
		//model + material + position + scale
		box_bottom->component<Cube>()->m_material = context().resource<Material>("box");
		//box_bottom->component<StaticMesh>()->m_material = context().resource<Material>("box");
		//box_bottom->component<StaticMesh>()->m_mesh = context().resource<Mesh>("cube");
		//box_bottom->component<StaticMesh>()->m_obb_local.set(Constants::identity<Mat3>(), { 0,0,0 }, { 0.5,0.5,0.5 });
		box_bottom->position({ 0.0, -6.0, 10.0 });
		box_bottom->scale({ 20.0, 1.0, 20.0 });
		//right box
		auto box_right = m_level->actor("box_right");
		//model + material + position + scale
		box_right->component<Cube>()->m_material = context().resource<Material>("box");
		box_right->position({ -6.0, -2.5f, 10.0 });
		box_right->scale({ 0.5f, 5.0f, 20.0f });
		//left box
		auto box_left = m_level->actor("box_left");
		//model + material + position + scale
		box_left->component<Cube>()->m_material = context().resource<Material>("box");
		box_left->position({ 6.0f, 0.0f, 10.0f });
		box_left->scale({ 0.5f, 20.0f, 20.0f });
		//light
		auto light0 = m_level->actor("light0");
		light0->component<DirectionLight>()->diffuse({ 1.0,0.0,0.0 });
		light0->component<DirectionLight>()->visible(false);
		light0->component<DirectionLight>()->shadow({ 1024,1024 });
		light0->rotation(rotate_euler(Square::radians(-45.0f), Square::radians(-45.0f), 0.0f));

		auto light1 = m_level->actor("light1");
		light1->component<PointLight>()->radius(20.0f, 15.0f);
		light1->component<PointLight>()->diffuse({ 0.0f,1.0f,0.0f });
		light1->component<PointLight>()->specular({ 0.0f,1.0f,0.0f });
		light1->component<PointLight>()->visible(false);
		light1->component<PointLight>()->shadow({ 512,512 });
		light1->position({ 0,0,10 });

		auto light2 = m_level->actor("light2");
		light2->component<SpotLight>()->radius(20.0f, 6.0f);
		light2->component<SpotLight>()->diffuse({ 1.0f,1.0f,1.0f });
		light2->component<SpotLight>()->specular({ 1.0f,1.0f,1.0f });
		light2->component<SpotLight>()->shadow({ 512,512 });
		light2->component<SpotLight>()->visible(false);
		light2->rotation(rotate_euler(Square::radians(-45.0f), Square::radians(-45.0f), 0.0f));
		light2->position({ 0,0,13 });
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
				0
			});
		}
    }
    
    bool run(double dt)
    {
		using namespace Square;
		m_acc += dt;
		if(m_turn)
			m_level->actor("main_node")->turn(
				rotate_euler<float>(0, 0, Square::Constants::pi2<float>() * 0.1f * float(dt))
			);
		m_level->actor("main_node")->position({ 0, 0, sin(m_acc * Square::Constants::pi<float>() * 0.25) * 10 + 5 });
        for(auto child : m_level->actor("main_node")->childs())
        {
            child->turn(rotate_euler<float>(0.0f,radians(90.0f*float(dt)),0.0f));
        }
        m_drawer->draw( 
			  Vec4(0.25,0.5,1.0,1.0)
			, Vec4(0.1,0.1,0.1,1.0)
			, m_level->randerable_collection()
		);
		// Adam
#ifdef MODEL_LOAD
		if (auto model = m_level->actor(MODEL_LOAD))
		{
			model->turn(
				rotate_euler(0.0f, Square::radians(90.0f) * float(dt), 0.0f)
			);
		}
#endif
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
		using namespace Square::Filesystem::Stream;
		GZOStream ofile(path);
		ArchiveBinWrite out(context(), ofile);
		m_level->serialize(out);
	}
	//level deserialize
	void deserialize(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Filesystem::Stream;
		GZIStream ifile(path);
		ArchiveBinRead in(context(), ifile);
		m_level->deserialize(in);
	}

private:
    
    bool m_loop = true;
	double m_acc = 0;
	Square::Time::FPSCounter				   m_counter;
	Square::Shared<Square::Scene::Level>	   m_level;
    Square::Shared<Square::Render::Drawer>     m_drawer;
};

static Square::Shell::ParserCommands s_ShellCommands
{
	  Square::Shell::Command{ "backend","b", "select backend [ogl, d3d]"  , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("ogl")) }
    , Square::Shell::Command{ "gputype","g", "select gpu type [low, high]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("high")) }
	, Square::Shell::Command{ "debug",  "d", "enable debug"               , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
	, Square::Shell::Command{ "help",   "h", "show help"                  , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
};

square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	// Show help:
	if (args.find("help") != args.end() && std::get<bool>(args["help"]))
	{
		app.context()->logger()->info((std::get<std::string>(args[Shell::__filename__]) + ":\n"));
		app.context()->logger()->info(Shell::help(s_ShellCommands));
		return 0;
	}
	// Test error
	if (errors.type != Shell::ErrorType::none)
	{
		app.context()->logger()->error("Error to parse input [" + std::to_string(errors.id_argument) + "]: " + errors.what);
		return -1;
	}
	// Build cube
	if (0)
	{
		//init
		std::vector<Render::Layout::Position3DNormalTangetBinomialUV> vertexs
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
		//build context
		Parser::StaticMesh::Context context;
		context.m_vertex = std::move(vertexs);
		context.m_index = std::move(ids);
		std::vector<unsigned char> buffer;
		//Serialize
		Parser::StaticMesh().serialize(context, buffer);
		Square::Filesystem::binary_compress_file_write_all("assets/cube.sm3dgz", buffer);
	}
	//debug?
	bool debug = std::get<bool>(args.at("debug"));
	//gpy type
	GpuType gputype = std::get<std::string>(args.at("gputype")) == "low" ? GpuType::GPU_LOW : GpuType::GPU_HIGH;
	//driver?
	WindowRenderDriver render_driver = std::get<std::string>(args.at("backend")) == "d3d"
		? (WindowRenderDriver{ Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, gputype, debug })
		: (WindowRenderDriver{ Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, gputype, debug });
	//test
    app.execute
	(
      WindowSizePixel({ 1280, 720 })
    , WindowMode::NOT_RESIZABLE
	, render_driver
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
