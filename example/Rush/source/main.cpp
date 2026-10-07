//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
//  Rush: the menu (the title: TitleScreen, RushUI) and the race (Race on a level, Arena; its
//  phases START, PLAY, END), the Graphics of both; in Debug (RUSH_DEMO) the engine demo over it
//  (DemoTools, the full Esc menu).
//
#define SQUARE_MAIN
#include <algorithm>
#include <memory>
#include <Square/Square.h>
#include <RushTypes.h>
#include <Collision.h>
#include <Arena.h>
#include <Race.h>
#include <Graphics.h>
#include <RushUI.h>
#include <TitleScreen.h>
#include <DemoTools.h>
#include <RushConfig.h>

using namespace Rush;

namespace AuxMain
{
	//the reach of the shadows of a map as far as its camera sees it: min(the largest side of the
	//map, the far of the camera; none: the map)
	static float shadow_view(const Arena& arena)
	{
		using namespace Square;
		const Vec3  sides = arena.max() - arena.min();
		const float map = std::max({ sides.x, sides.y, sides.z });
		auto camera_actor = arena.camera();
		if (!camera_actor || !camera_actor->contains<Scene::Camera>()) return map;
		const float camera_far = camera_actor->component<Scene::Camera>()->viewport().near_and_far().y;
		if (camera_far <= 0.0f) return map;
		return std::min(map, camera_far);
	}

	//the share of a fade: its time over its length (no length: done at once)
	static float fade_share(float time, float length)
	{
		if (length <= 0.0f) return 1.0f;
		return std::min(time / length, 1.0f);
	}

	//the loading screen of a race: it comes over the title, it covers it (the race loads under it),
	//it stays some frames after the load (the first ones of a map are slow), it goes
	class LoadingFade
	{
	public:

		enum class Phase
		{
			HIDDEN,
			COMING,  //over the title
			COVERED, //all of it drawn: the race can load
			HOLDING, //the race loaded, its first frames
			GOING
		};

		Phase phase() const { return m_phase; }

		void come()
		{
			m_phase = Phase::COMING;
			m_time = 0.0f;
		}

		//the race loaded: it stays, then it goes
		void hold()
		{
			m_phase = Phase::HOLDING;
			m_frames = 0;
		}

		void hide()
		{
			m_phase = Phase::HIDDEN;
		}

		//a frame (its seconds): its opacity
		float update(float delta_time, const LoadingScreen& loading)
		{
			switch (m_phase)
			{
			case Phase::COMING:  return coming(delta_time, loading);
			case Phase::COVERED: return 1.0f;
			case Phase::HOLDING: return holding(loading);
			case Phase::GOING:   return going(delta_time, loading);
			case Phase::HIDDEN:
			default:             return 0.0f;
			}
		}

	private:

		float coming(float delta_time, const LoadingScreen& loading)
		{
			m_time += delta_time;
			const float shown = fade_share(m_time, loading.m_fade_in);
			if (shown >= 1.0f) m_phase = Phase::COVERED;
			return shown;
		}

		//(frames, not seconds: the first ones after the load are long)
		float holding(const LoadingScreen& loading)
		{
			++m_frames;
			if (m_frames >= loading.m_hold_frames)
			{
				m_phase = Phase::GOING;
				m_time = 0.0f;
			}
			return 1.0f;
		}

		float going(float delta_time, const LoadingScreen& loading)
		{
			m_time += delta_time;
			const float gone = fade_share(m_time, loading.m_fade_out);
			if (gone >= 1.0f) m_phase = Phase::HIDDEN;
			return 1.0f - gone;
		}

		Phase m_phase{ Phase::HIDDEN };
		float m_time{ 0.0f };
		int   m_frames{ 0 };
	};
}

class RushGame : public Square::AppInterface
{
public:

	//map: a race on it at once (its name, --map), else the title; view: the view of that map
	//(--view: its photo) instead of its race
	RushGame(const std::string& map, bool view)
	: m_start_map(map)
	, m_view(view && !map.empty())
	{
	}

	void start()
	{
		using namespace Square;
		using namespace Square::Filesystem;
		//rs files
		context().add_resources(join(resource_dir(), "/resources.rs"));
		context().add_resources(join(resource_dir(), "common/resources.rs"));
		context().add_resources(join(resource_dir(), "example/Rush/resources.rs"));
		//the configuration of the game (config/: its rules, its hovercraft, its maps)
		const std::string config = join(resource_dir(), "example/Rush/config");
		const std::string assets = join(resource_dir(), "example/Rush/assets");
		if (!Config::load(context(), config, assets))
		{
			context().logger()->warning("Rush: the configuration is not complete (" + config + "), its defaults kept");
		}
		//the game
		setup_controls();
		setup_collisions();
		m_graphics.setup(world());
		//the UI: the model of the game (and of the demo), then the documents
		if (m_ui.create())
		{
#if defined(RUSH_DEMO)
			m_demo = std::make_unique<DemoTools>(context(), world());
			m_demo->bind(m_ui.model());
#endif
			m_ui.load_documents();
			m_ui.on_play([this]() { m_next = State::RACE; });
			m_ui.on_quit([this]() { m_loop = false; });
			m_ui.on_exit([this]() { m_next = State::MENU; });
			if (m_demo) m_demo->setup(m_ui.menu_document(), m_ui.model());
			//the settings of the player (saved): the window, the effects
			m_ui.load_settings(m_graphics);
		}
		//the levels of the game, made once: the title and the race (one runs at a time)
		world().create_level(s_title_world_level);
		world().create_level(s_race_world_level);
		//the title in its level, it starts in the menu
		m_title.load(world().level(s_title_world_level));
		enter_menu();
		//a map asked by the command line: its race at the next frame
		if (!m_start_map.empty())
		{
			if (m_ui.race_map(m_start_map)) m_next = State::RACE;
			else context().logger()->warning("no map named " + m_start_map);
		}
	}

	bool run(double delta_time)
	{
		m_counter.count_frame();
		//a state asked (by the UI: out of its events) at the start of the frame
		if (m_next != m_state)
		{
			switch (m_next)
			{
			//(a race: its loading screen first)
			case State::RACE: enter_loading(); break;
			case State::MENU:
			default:          enter_menu(); break;
			}
		}
		//the title and the race run in their levels (the one active)
		switch (m_state)
		{
		case State::MENU:    update_menu(delta_time); break;
		case State::LOADING: update_loading(delta_time); break;
		case State::RACE:    update_race(delta_time); break;
		default: break;
		}
		//the UI
		m_ui.update(m_race.get(), m_graphics, float(m_counter.get()));
		if (m_demo) m_demo->update(m_race.get(), m_graphics);
		return m_loop;
	}

	bool end()
	{
		return true;
	}

	void key_event(Square::Video::KeyboardEvent key, short mode, Square::Video::ActionEvent action)
	{
		using namespace Square;
		if (action != Video::ActionEvent::PRESS) return;
		switch (m_state)
		{
		case State::MENU: m_ui.title_key(key); break;
		case State::RACE: race_key(key); break;
		default: break;
		}
	}

	void mouse_button_event(Square::Video::MouseButtonEvent button, Square::Video::ActionEvent action)
	{
		using namespace Square;
		//the end of a race: a click, back to the menu
		const bool press = action == Video::ActionEvent::PRESS;
		if (press && race_ended()) m_next = State::MENU;
	}

	void window_event(Square::Video::WindowEvent event)
	{
		using namespace Square;
		//a new size (the fullscreen and back): the viewport of the cameras
		switch (event)
		{
		case Video::WindowEvent::RESIZE:
		case Video::WindowEvent::MAXIMIZED:
		{
			unsigned int width = 0, height = 0;
			context().window()->get_size(width, height);
			m_title.viewport(width, height);
			if (m_race) m_race->arena().viewport(width, height);
		}
		break;
		default: break;
		}
	}

	void mouse_scroll_event(double scroll)
	{
		if (m_demo) m_demo->mouse_scroll(scroll);
	}

private:

	enum class State
	{
		MENU,
		LOADING, //the loading screen of a race (before it)
		RACE
	};

	//the loading screen of the race chosen: it comes over the title (enter_race when it covers it)
	void enter_loading()
	{
		m_ui.loading(m_ui.race_map());
		m_loading.come();
		m_state = m_next = State::LOADING;
	}

	//the shot of the title
	void update_menu(double delta_time)
	{
		pause(false);
		m_title.update(delta_time);
	}

	//the loading screen over the title; covered on the frame before (drawn so): the race loads
	//now, under it
	void update_loading(double delta_time)
	{
		m_title.update(delta_time);
		if (m_loading.phase() == AuxMain::LoadingFade::Phase::COVERED)
		{
			enter_race();
			return;
		}
		m_ui.loading_opacity(m_loading.update(float(delta_time), Config::get().loading()));
	}

	void update_race(double delta_time)
	{
		//its loading screen going
		m_ui.loading_opacity(m_loading.update(float(delta_time), Config::get().loading()));
		pause(race_paused());
		if (!m_race) return;
		//the view: its camera there (whatever moved it before the pause)
		if (m_view)
		{
			const RaceMap& map = m_ui.race_map();
			m_race->arena().view(map.m_view_from, map.m_view_to, map.m_view_lens);
		}
		//a circuit: the haze of where the player is (the biomes along the lap)
		RaceFog zone_fog;
		if (m_race->zone_fog(zone_fog)) m_graphics.fog(zone_fog, m_race->arena().sun_direction());
	}

	//a race still: nothing moves, its time stopped
	bool race_paused() const
	{
		//the view of its map (its photo)
		if (m_view) return true;
		//its menu open
		if (m_ui.menu_visible()) return true;
		//under its loading screen (its first frames)
		return m_loading.phase() == AuxMain::LoadingFade::Phase::HOLDING;
	}

	//the pause of a race: its level (the components: the race, the drivers, the camera) and the
	//collisions (the steps of the physics) stopped, still drawn
	void pause(bool paused)
	{
		if (auto level = world().level(s_race_world_level)) level->paused(paused);
		if (auto collision = world().instance<CollisionWorld>()) collision->paused(paused);
	}

	//the menu: no race, the level of the title active
	void enter_menu()
	{
		if (m_demo) m_demo->race_ended();
		end_race();
		//(a loading screen still going: gone)
		m_loading.hide();
		m_ui.loading_opacity(0.0f);
		world().active_levels({ s_title_world_level });
		//the haze of the sunset of the title (its sun glowing in it)
		m_graphics.fog(Config::get().title_fog(), m_title.sun_direction());
		m_graphics.depth_of_field(true, 15.0f);
		//the menu: always smooth edges (its shot), whatever the settings
		m_graphics.antialiasing(true);
		m_graphics.snow(false);
		m_title.show(true);
		m_ui.title(true);
		m_state = m_next = State::MENU;
	}

	//a race: the level of the title not active, the race on its level (it starts: the camera
	//comes to the player)
	void enter_race()
	{
		m_ui.title(false);
		m_title.show(false);
		world().active_levels({ s_race_world_level });
		//the race: a component of its actor in its level (it runs with the level)
		auto race_actor = world().level(s_race_world_level)->actor();
		race_actor->name("race");
		m_race = race_actor->component<Race>();
		const RaceMap& map = m_ui.race_map();
		m_race->load(map);
		m_graphics.fog(map.m_fog, m_race->arena().sun_direction());
		m_graphics.snow(map.m_snow);
		m_graphics.race_depth_of_field(true);
		m_graphics.antialiasing(m_ui.settings().m_antialiasing);
		m_ui.settings().apply_shadows(m_race->arena().sun(), AuxMain::shadow_view(m_race->arena()));
		if (m_demo) m_demo->race_started(m_race->arena());
		m_state = m_next = State::RACE;
		//its loading screen stays a little, then it goes
		m_loading.hold();
		if (m_view) enter_view(map);
	}

	//the view of a map (its photo): the race stopped (paused: nothing moves), no HUD, the camera on
	//the view of the map, the effects at their best (the settings of the player not changed)
	void enter_view(const RaceMap& map)
	{
		m_ui.hud(false);
		GameSettings best = m_ui.settings();
		best.m_reflections  = int(Config::get().levels("reflections").size());
		best.m_occlusion    = int(Config::get().levels("occlusion").size());
		best.m_shadows      = int(Config::get().levels("shadows").size());
		best.m_bloom        = int(Config::get().levels("bloom").size());
		best.m_god_rays     = int(Config::get().levels("god_rays").size());
		best.m_antialiasing = true;
		//(still: no motion to blur)
		best.m_motion_blur  = 0;
		best.apply_effects(m_graphics);
		m_graphics.antialiasing(true);
		best.apply_shadows(m_race->arena().sun(), AuxMain::shadow_view(m_race->arena()));
		//every thing at its most detailed level, at once
		Square::Scene::LodGroup::force_level(0);
		Square::Scene::LodGroup::fade_duration(0.0f);
		context().logger()->info("view of the map " + map.m_name + (map.m_view_set ? "" : " (no view in its config: the default one)"));
	}

	//the race out of its level (its map, hovercraft, its actor)
	void end_race()
	{
		if (!m_race) return;
		auto race_actor = m_race->actor().lock();
		m_race->unload();
		if (race_actor) race_actor->remove_from_level();
		m_race.reset();
	}

	//the end of a race (win or lose shown): a key, back to the menu
	bool race_ended() const
	{
		return m_state == State::RACE && m_race && m_race->phase() == Race::Phase::END;
	}

	void race_key(Square::Video::KeyboardEvent key)
	{
		using namespace Square;
		if (race_ended())
		{
			m_next = State::MENU;
			return;
		}
		switch (key)
		{
		case Video::KEY_ESCAPE:
			//the menu (pause; with the demo: its tools)
			m_ui.menu(!m_ui.menu_visible());
		break;
		case Video::KEY_SPACE:
			//back on the ground: at the start (an arena), at the last gate (a circuit)
			if (m_race && m_race->phase() == Race::Phase::PLAY) m_race->respawn(0);
		break;
		default: break;
		}
	}

	//controls: the actions of the input system (read by the HovercraftInput of the player)
	void setup_controls()
	{
		using namespace Square;
		auto input = System::get<InputSystem>(context());
		if (!input) return;
		input->bind("forward",  Video::KEY_UP);
		input->bind("forward",  Video::KEY_W);
		input->bind("backward", Video::KEY_DOWN);
		input->bind("backward", Video::KEY_S);
		input->bind("left",     Video::KEY_LEFT);
		input->bind("left",     Video::KEY_A);
		input->bind("right",    Video::KEY_RIGHT);
		input->bind("right",    Video::KEY_D);
	}

	//collisions of the world (a game system, started on demand): body and wheels slide on the
	//scene, the bodies on each other, the camera on the scene
	void setup_collisions()
	{
		context().start_system<CollisionSystem>();
		auto collision = world().instance<CollisionWorld>();
		if (!collision) return;
		collision->collisions(TYPE_BODY, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
		collision->collisions(TYPE_WHEEL, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
		collision->collisions(TYPE_BODY, TYPE_BODY, CollisionMethod::SPHERE, CollisionResponse::SLIDEXZ);
		collision->collisions(TYPE_CAMERA, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDE);
		collision->collisions(TYPE_CAMERA, TYPE_CAMERA_BOUNDS, CollisionMethod::POLYGON, CollisionResponse::SLIDE);
		collision->debug(false);
	}

	std::string                m_start_map; //--map
	bool                       m_view{ false }; //--view: the view of the map of --map (its photo)
	AuxMain::LoadingFade       m_loading;       //the loading screen of a race
	bool                       m_loop{ true };
	State                      m_state{ State::MENU };
	State                      m_next{ State::MENU }; //asked by the UI, at the next frame
	Square::Time::FPSCounter   m_counter;
	Graphics                   m_graphics{ context() };
	TitleScreen                m_title{ context() };
	RushUI                     m_ui{ context() };
	Square::Shared<Race>       m_race;    //the component of the actor of the race
	std::unique_ptr<DemoTools> m_demo; //RUSH_DEMO only
};

static Square::Shell::ParserCommands s_ShellCommands
{
	  Square::Shell::Command{ "backend","b", "select backend [ogl, d3d, mtl]" , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("ogl")) }
	, Square::Shell::Command{ "gputype","g", "select gpu type [low, high]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("high")) }
	, Square::Shell::Command{ "debug",  "d", "enable debug"               , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
	, Square::Shell::Command{ "srgb",   "c", "enable gamme correction"    , Square::Shell::ValueType::value_bool  , false, Square::Shell::Value_t(true) }
	, Square::Shell::Command{ "verbose","v", "enable verbose"             , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
	, Square::Shell::Command{ "map",    "m", "start a race on a map [arena, backwash, containment, sanctuary, valley]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("")) }
	, Square::Shell::Command{ "view",   "w", "the view of the map of --map (its photo): still, no HUD, the effects at their best", Square::Shell::ValueType::value_none, false, Square::Shell::Value_t(false) }
	, Square::Shell::Command{ "help",   "h", "show help"                  , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
};

square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
	using namespace Square;
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
	//debug, verbose
	const bool debug = std::get<bool>(args.at("debug"));
	app.logger()->verbose(std::get<bool>(args.at("verbose")));
	//GPU type, sRGB
	const GpuType gputype = std::get<std::string>(args.at("gputype")) == "low" ? GpuType::GPU_LOW : GpuType::GPU_HIGH;
	const bool srgb = std::get<bool>(args.at("srgb"));
	//driver
	const std::string& backend = std::get<std::string>(args.at("backend"));
	WindowRenderDriver render_driver{ Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, gputype, srgb, debug };
	if (backend == "d3d") render_driver = WindowRenderDriver{ Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, gputype, srgb, debug };
	if (backend == "mtl") render_driver = WindowRenderDriver{ Render::RenderDriver::DR_METAL, 3, 0, 24, 8, gputype, srgb, debug };
	//run
	app.execute
	(
	  WindowSizePixel({ 1920, 1080 })
	, WindowMode::NOT_RESIZABLE
	, render_driver
	, "Rush"
	, new RushGame(std::get<std::string>(args.at("map")), std::get<bool>(args.at("view")))
	);
	return 0;
}
