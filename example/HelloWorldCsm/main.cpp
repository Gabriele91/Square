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
		const auto  level_path_json = Square::Filesystem::join(Square::Filesystem::resource_dir(), "level.jsq");
		context().logger()->info("value: " + std::to_string(key));
		context().logger()->info("delta: " + std::to_string(application().last_delta_time()));
		//
		switch (key)
		{
		case Square::Video::KEY_ESCAPE:
		case Square::Video::KEY_DELETE: m_loop = false; break;
		case Square::Video::KEY_V: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_G: Square::Application::instance()->fullscreen(true); break;
		case Square::Video::KEY_J: 
			serialize_json(level_path_json); 
		break;
		case Square::Video::KEY_M:			
			if (Square::Filesystem::exists(level_path_json))
				deserialize_json(level_path_json); 
		break;
		case Square::Video::KEY_B: 
			serialize(level_path); 
		break;
		case Square::Video::KEY_N:
			if (Square::Filesystem::exists(level_path))
				deserialize(level_path);
		break;

		case Square::Video::KEY_C:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				context().logger()->info("FPS avg: " + std::to_string(m_counter.get()));
			}
			break;
		case Square::Video::KEY_Y:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				m_turn = !m_turn;
			}
		break;
		case Square::Video::KEY_L:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				auto plight = m_level->find_actor("csm_scene.Light")->component<DirectionLight>();
				plight->visible(!plight->visible());
			}
			break;
		case Square::Video::KEY_W:
			m_level->find_actor("csm_scene.camera")->move({ 0,0, 10 * application().last_delta_time() });
		break;
		case Square::Video::KEY_S:
			m_level->find_actor("csm_scene.camera")->move({ 0,0, -10 * application().last_delta_time() });
		break;
		case Square::Video::KEY_D:
			m_level->find_actor("csm_scene.camera")->move({ 10 * application().last_delta_time(),0,0 });
		break;
		case Square::Video::KEY_A:
			m_level->find_actor("csm_scene.camera")->move({ -10 * application().last_delta_time(),0,0 });
		break;
		case Square::Video::KEY_R:
			m_level->find_actor("csm_scene.camera")->move({ 0, 10 * application().last_delta_time(),0 });
		break;
		case Square::Video::KEY_F:
			m_level->find_actor("csm_scene.camera")->move({ 0,-10 * application().last_delta_time(),0 });
		break;
		case Square::Video::KEY_E:
			m_level->find_actor("csm_scene.camera")->turn(rotate_euler<float>(0,1. * application().last_delta_time(),0 ));
		break;
		case Square::Video::KEY_Q:
			m_level->find_actor("csm_scene.camera")->turn(rotate_euler<float>(0, -1. * application().last_delta_time(), 0));
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
		// window size
		uint32_t window_width, window_height;
		context().window()->get_size(window_width, window_height);
		// level
		m_level = world().level("main");
		// load
		if (m_level->load_actor("csm_scene"))
		{
			auto camera = m_level->find_actor("csm_scene.camera");
			camera->component<Camera>()->viewport({ 0,0, window_width, window_height });
		}
		else
		{
			context().logger()->info("Error to load base_scene");
		}
		//
        m_drawer = Square::MakeShared<Render::Drawer>(context());
		m_drawer->create<Render::DrawerPassForward>();
		m_drawer->create<Render::DrawerPassDebug>()->draw_flags(0);
		m_drawer->create<Render::DrawerPassShadow>();
    }
    
    bool run(double dt)
    {
		using namespace Square;
		m_acc += dt;
		//fps counter
		m_counter.count_frame();
		// draw
		m_drawer->draw(
			  Vec4(0.25, 0.5, 1.0, 1.0)
			, Vec4(0.1, 0.1, 0.1, 1.0)
			, m_level->randerable_collection()
		);
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
		world().serialize(out);
	}
	void serialize_json(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		Json jout = Json(JsonObject());
		world().serialize_json(jout);
		std::ofstream(path) << jout;
	}
	//level deserialize
	void deserialize(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Filesystem::Stream;
		GZIStream ifile(path);
		ArchiveBinRead in(context(), ifile);
		m_level.reset();
		world().deserialize(in);
		m_level = world().level("main");
	}
	void deserialize_json(const std::string& path)
	{
		using namespace Square;
		using namespace Square::Data;
		Json jin;
		if (jin.parser(Square::Filesystem::text_file_read_all(path)))
		{
			m_level.reset();
			world().deserialize_json(jin);
			m_level = world().level("main");
		}
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
	, Square::Shell::Command{ "verbose","v", "enable verbose"             , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
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
		app.context()->logger()->info(Shell::filename(args) + ":\n");
		app.context()->logger()->info(Shell::help(s_ShellCommands));
		return 0;
	}
	// Test error
	if (errors.type != Shell::ErrorType::none)
	{
		app.context()->logger()->error("Error to parse input [" + std::to_string(errors.id_argument) + "]: " + errors.what);
		return -1;
	}
	//debug?
	bool debug = std::get<bool>(args.at("debug"));
	//verbose?
	app.logger()->verbose(std::get<bool>(args.at("verbose")));
	//GPU type
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
