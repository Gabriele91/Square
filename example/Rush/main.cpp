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
#include <memory>
#include "Collision.h"
#include "Hovercraft.h"

class RushGame : public Square::AppInterface
{
public:

	void key_event(Square::Video::KeyboardEvent key, short mode, Square::Video::ActionEvent action)
	{
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//move vel
		const auto  level_path = Square::Filesystem::join(Square::Filesystem::resource_dir(), "level.sq");
		const auto  level_path_json = Square::Filesystem::join(Square::Filesystem::resource_dir(), "level.jsq");
		//hovercraft controls: held state, applied every frame in run()
		if (action != Square::Video::ActionEvent::REPEAT)
		{
			const bool down = action == Square::Video::ActionEvent::PRESS;
			switch (key)
			{
			case Square::Video::KEY_UP:    m_hovercraft_input.forward  = down; return;
			case Square::Video::KEY_DOWN:  m_hovercraft_input.backward = down; return;
			case Square::Video::KEY_LEFT:  m_hovercraft_input.left     = down; return;
			case Square::Video::KEY_RIGHT: m_hovercraft_input.right    = down; return;
			default: break;
			}
		}
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
		case Square::Video::KEY_O:
			if (action == Square::Video::ActionEvent::PRESS)
			if (render_debug())
			{
				render_debug()->draw_flags(render_debug()->draw_flags() ^ Render::DF_DRAW_OBB);
			}
		break;
		case Square::Video::KEY_L:
			//toggle the light volumes (spot cone, point cube faces, directional CSM cascades)
			if (action == Square::Video::ActionEvent::PRESS)
			if (render_debug())
			{
				render_debug()->draw_flags(render_debug()->draw_flags() ^ (Render::DF_DRAW_SPOT_LIGHT | Render::DF_DRAW_POINT_LIGHT | Render::DF_DRAW_DIRECTIONAL_LIGHT));
			}
		break;
		case Square::Video::KEY_T:
			//toggle the texture panel (images/TBO/RBO)
			if (action == Square::Video::ActionEvent::PRESS)
			if (render_debug())
			{
				render_debug()->draw_flags(render_debug()->draw_flags() ^ Render::DB_DRAW_TEXTURES);
			}
		break;
		case Square::Video::KEY_C:
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				context().logger()->info("FPS avg: " + std::to_string(m_counter.get()));
			}
			break;
		case Square::Video::KEY_P:
			if (action == Square::Video::ActionEvent::RELEASE)
			if (m_light && m_light->contains<DirectionLight>())
			{
				auto light = m_light->component<DirectionLight>();
				light->visible(!light->visible());
			}
			break;
		case Square::Video::KEY_SPACE:
			//back on the ground at the start
			if (action == Square::Video::ActionEvent::PRESS && m_hovercraft_drive)
			{
				m_hovercraft_drive->spawn(0.0f, 0.0f);
			}
		break;
		default: break;
		}
	}

	void mouse_scroll_event(double scroll)
	{
		//scroll the debug texture panel
		if (render_debug()) render_debug()->panel_scroll((float)scroll * 20.0f);
	}

    void start()
    {
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//rs file
		context().add_resources(Filesystem::join(Filesystem::resource_dir(), "/resources.rs"));
		context().add_resources(Filesystem::join(Filesystem::resource_dir(), "common/resources.rs"));
		context().add_resources(Filesystem::join(Filesystem::resource_dir(), "example/Rush/resources.rs"));
		// window size
		uint32_t window_width, window_height;
		context().window()->get_size(window_width, window_height);
		// level
		m_level = world().level("main");
		// rendering pipeline of the world: SQUARE_RENDERING=forward|deferred (default: deferred)
		if (auto render_world = world().instance<RenderInstance>())
		{
			const char* rendering_type = std::getenv("SQUARE_RENDERING");
			render_world->pipeline(rendering_type && Square::case_insensitive_equal(rendering_type, "forward") ? "forward" : "deferred");
		}
		// arena
		if (auto arena = m_level->load_actor("arena/scene"))
		{
			arena->position({ 0.0f, 4.0f, 0.0f });
			m_camera = arena->child("camera");
			if (m_camera) m_camera->component<Camera>()->viewport({ 0,0, window_width, window_height });
			else context().logger()->info("arena has no 'camera' node");
			m_light = arena->child("light") ? arena->child("light") : arena->child("sun");
			if (!m_light) context().logger()->info("arena has no 'sun'/'light' node");
			// the solid triangles of the arena (after it is placed)
			m_collision.add(context(), arena);
			context().logger()->info("arena collision triangles: " + std::to_string(m_collision.size()));
		}
		else
		{
			context().logger()->info("Error to load arena");
		}
		// the camera chases the hovercraft in world space: out of the arena, at the level root
		if (m_camera) m_level->add(m_camera);
		// hovercraft
		m_hovercraft = m_level->load_actor("hovercraft/scene");
		if (m_hovercraft)
		{
			// the model is about twice the size that fits the arena
			m_hovercraft->scale({ 0.5f, 0.5f, 0.5f });
			m_hovercraft_drive = std::make_unique<Hovercraft>(context(), m_hovercraft, m_camera, m_collision, hovercraft_settings());
			m_hovercraft_drive->spawn(0.0f, 0.0f);
		}
		else
		{
			context().logger()->info("Error to load hovercraft");
		}
    }

    bool run(double dt)
    {
		using namespace Square;
		m_acc += dt;
		//fps counter
		m_counter.count_frame();
		// hovercraft: arrows drive/steer, the camera follows
		if (m_hovercraft_drive)
		{
			m_hovercraft_drive->update(dt, m_hovercraft_input);
		}
		//loop event
        return m_loop;
    }

	static Hovercraft::Settings hovercraft_settings()
	{
		Hovercraft::Settings settings;
		// the model floats a little: a quarter of its height lower
		settings.model_offset_y = -0.25f;
		return settings;
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

	//the debug pass of the world (OBB, lights, textures): the RenderSystem draws the world
	//every frame, the drawer of the world exists after start()
	Square::Shared<Square::Render::DrawerPassDebug> render_debug()
	{
		auto render_world = world().instance<Square::RenderInstance>();
		return render_world ? render_world->debug_pass() : nullptr;
	}

private:

    bool m_loop = true;
	double m_acc = 0;
	Square::Time::FPSCounter				   m_counter;
	Square::Shared<Square::Scene::Level>	   m_level;
	Square::Shared<Square::Scene::Actor>      m_camera;
	Square::Shared<Square::Scene::Actor>      m_light;
	Square::Shared<Square::Scene::Actor>      m_hovercraft;
	CollisionMesh                             m_collision;
	std::unique_ptr<Hovercraft>               m_hovercraft_drive;
	Hovercraft::Input                         m_hovercraft_input;
};

static Square::Shell::ParserCommands s_ShellCommands
{
	  Square::Shell::Command{ "backend","b", "select backend [ogl, d3d, mtl]" , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("ogl")) }
    , Square::Shell::Command{ "gputype","g", "select gpu type [low, high]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("high")) }
	, Square::Shell::Command{ "debug",  "d", "enable debug"               , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
	, Square::Shell::Command{ "srgb",   "c", "enable gamme correction"    , Square::Shell::ValueType::value_bool  , false, Square::Shell::Value_t(true) }
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
	//Enable SRGB
	const bool srgb = std::get<bool>(args.at("srgb"));
	//driver?
	WindowRenderDriver render_driver = std::get<std::string>(args.at("backend")) == "d3d"
		? (WindowRenderDriver{ Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, gputype, srgb, debug })
		: std::get<std::string>(args.at("backend")) == "mtl"
		? (WindowRenderDriver{ Render::RenderDriver::DR_METAL, 3, 0, 24, 8, gputype, srgb, debug })
		: (WindowRenderDriver{ Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, gputype, srgb, debug });

	//test
    app.execute
	(
      WindowSizePixel({ 1280, 720 })
    , WindowMode::NOT_RESIZABLE
	, render_driver
    , "Rush"
    , new RushGame()
    );
    //End
    return 0;
}
