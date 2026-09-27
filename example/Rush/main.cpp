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
#include "Checkpoints.h"
#include "HovercraftInput.h"
#include "CameraFollow.h"

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
		//controls of the player's hovercraft
		if (m_player && m_player->key(key, action)) return;
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
			if (action == Square::Video::ActionEvent::PRESS)
			{
				spawn();
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
			const bool forward = rendering_type && Square::case_insensitive_equal(rendering_type, "forward");
			render_world->pipeline((forward ? RP_FORWARD : RP_DEFERRED) | RP_DEBUG);
		}
		// collisions of the world (a game system, started on demand): body and wheels slide
		// on the scene (Collisions BODY,SCENE,2,3 / WHEEL,SCENE,2,3: polygon, slide xz)
		context().start_system<CollisionSystem>();
		if (auto collision = world().instance<CollisionWorld>())
		{
			collision->collisions(TYPE_BODY, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
			collision->collisions(TYPE_WHEEL, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
			// the camera does not go into the map: it slides on it
			collision->collisions(TYPE_CAMERA, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDE);
		}
		// arena
		auto arena = m_level->load_actor("arena/scene");
		if (arena)
		{
			arena->position({ 0.0f, 4.0f, 0.0f });
			m_camera = arena->child("camera");
			if (m_camera)
			{
				m_camera->component<Camera>()->viewport({ 0,0, window_width, window_height });
			}
			else
			{
				context().logger()->info("arena has no 'camera' node");
			}
			// Sun
			m_light = arena->child("sun");
			if (m_light)
			{
				m_light->component<DirectionLight>()->shadow({2048,2048});
			}
			else
			{
				context().logger()->info("arena has no 'sun'/'light' node");
			}
			// the arena is solid: a mesh collider of the scene type (its triangles, from where
			// it is placed)
			auto arena_collider = arena->component<MeshCollider>();
			arena_collider->type(TYPE_SCENE);
			context().logger()->info("arena collision triangles: " + std::to_string(arena_collider->mesh().size()));
			// start of the hovercraft: the spawn point of the scene (after the arena is placed)
			arena->visit([&](Shared<Actor> node) -> bool
			{
				if (node->name() != "spawn_point_1") return true;
				m_start = node->position(true);
				return false;
			});
		}
		else
		{
			context().logger()->info("Error to load arena");
		}
		// light beam: on the checkpoints of the arena (checkpoint_1, checkpoint_2...), to the
		// following one when the hovercraft touches it
		if (auto light_beam = m_level->load_actor("light_beam/scene"))
		{
			// its light: a point light in the middle, a little over the ground, with shadow and a
			// large radius (a child of the beam: it goes with it from checkpoint to checkpoint)
			auto beam_light = light_beam->child();
			beam_light->name("light_beam_light");
			beam_light->position({ 0.0f, 1.25f, 0.0f });
			auto point_light = beam_light->component<PointLight>();
			point_light->diffuse({ 0.1f, 0.7f, 1.0f });
			point_light->specular({ 0.1f, 0.7f, 1.0f });
			point_light->constant(1.0f);
			point_light->radius(80.0f);
			point_light->inside_radius(15.0f);
			point_light->shadow({ 2048, 2048 });
			m_checkpoints = light_beam->component<Checkpoints>();
			const size_t count = m_checkpoints->collect(arena);
			context().logger()->info("checkpoints: " + std::to_string(count));
			m_checkpoints->on_reached([this](size_t index)
			{
				context().logger()->info("checkpoint " + std::to_string(index + 1) + " reached");
			});
		}
		else
		{
			context().logger()->info("Error to load light_beam");
		}
		// the camera chases the hovercraft in world space: out of the arena, at the level root;
		// a sphere of the camera type, so it does not go through the walls and the ground
		if (m_camera)
		{
			m_level->add(m_camera);
			auto camera_collider = m_camera->component<SphereCollider>();
			camera_collider->type(TYPE_CAMERA);
			camera_collider->radius(1.0f);
			m_camera_follow = m_camera->component<CameraFollow>();
		}
		// hovercraft
		m_hovercraft = m_level->load_actor("hovercraft/scene");
		if (m_hovercraft)
		{
			// the model is about twice the size that fits the arena
			m_hovercraft->scale({ 0.5f, 0.5f, 0.5f });
			// the driver: a component of the hovercraft, updated every frame by the scene; the
			// player drives it (its keys become the input of the driver)
			m_driver = m_hovercraft->component<HovercraftDriver>();
			m_driver->settings() = hovercraft_settings();
			m_player = m_hovercraft->component<HovercraftInput>();
			// the camera follows it
			if (m_camera_follow) m_camera_follow->target(m_hovercraft);
			if (m_checkpoints) m_checkpoints->target(m_hovercraft);
			spawn();
		}
		else
		{
			context().logger()->info("Error to load hovercraft");
		}
    }

	//the hovercraft at the start, the camera straight behind it
	void spawn()
	{
		if (!m_driver) return;
		m_driver->spawn(m_start);
		if (m_camera_follow) m_camera_follow->snap();
	}

    bool run(double dt)
    {
		using namespace Square;
		m_acc += dt;
		//fps counter
		m_counter.count_frame();
		//loop event
        return m_loop;
    }

	//start of the hovercraft, when the arena has no spawn point: it drops on the first surface
	//under it (under the roof, over the field)
	static constexpr Square::Vec3 s_start{ 0.0f, 50.0f, 0.0f };

	//collision types (Const BODY=1,WHEEL=2,SCENE=3), and the camera
	enum CollisionType : int
	{
		TYPE_BODY  = 1,
		TYPE_WHEEL = 2,
		TYPE_SCENE = 3,
		TYPE_CAMERA = 4
	};

	static HovercraftDriver::Settings hovercraft_settings()
	{
		HovercraftDriver::Settings settings;
		// collision types of body, wheels and ground
		settings.body_type  = TYPE_BODY;
		settings.wheel_type = TYPE_WHEEL;
		settings.scene_type = TYPE_SCENE;
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
	Square::Time::FPSCounter				  m_counter;
	Square::Shared<Square::Scene::Level>	  m_level;
	Square::Shared<Square::Scene::Actor>      m_camera;
	Square::Shared<Square::Scene::Actor>      m_light;
	Square::Shared<Square::Scene::Actor>      m_hovercraft;
	Square::Shared<HovercraftDriver>          m_driver;
	Square::Shared<HovercraftInput>           m_player;
	Square::Shared<CameraFollow>              m_camera_follow;
	Square::Shared<Checkpoints>               m_checkpoints;
	Square::Vec3                              m_start{ s_start }; //spawn_point_1 of the arena
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
