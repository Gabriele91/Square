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

class RushGame : public Square::AppInterface
{
public:

	void start()
	{
		using namespace Square;
		using namespace Square::Filesystem;
		//rs files
		context().add_resources(join(resource_dir(), "/resources.rs"));
		context().add_resources(join(resource_dir(), "common/resources.rs"));
		context().add_resources(join(resource_dir(), "example/Rush/resources.rs"));
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
		}
		//the title (its level), it starts in the menu
		m_title.load(world());
		enter_menu();
	}

	bool run(double delta_time)
	{
		m_counter.count_frame();
		//a state asked (by the UI: out of its events) at the start of the frame
		if (m_next != m_state)
		{
			if (m_next == State::RACE) enter_race();
			else                       enter_menu();
		}
		//the state
		switch (m_state)
		{
		case State::MENU: m_title.update(delta_time); break;
		case State::RACE: if (m_race) m_race->update(delta_time); break;
		default: break;
		}
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
		RACE
	};

	//the menu: no race, the level of the title active
	void enter_menu()
	{
		if (m_demo) m_demo->race_ended();
		m_race.reset();
		m_graphics.fog(RaceFog{}, Square::Vec3(0.0f, -1.0f, 0.0f));
		m_title.show(true, world());
		m_ui.title(true);
		m_state = m_next = State::MENU;
	}

	//a race: the level of the title not active, the race on its level (it starts: the camera
	//comes to the player)
	void enter_race()
	{
		m_ui.title(false);
		m_title.show(false, world());
		m_race = std::make_unique<Race>(context(), world());
		const RaceMap& map = m_ui.race_map();
		m_race->load(map);
		m_graphics.fog(map.m_fog, m_race->arena().sun_direction());
		if (m_demo) m_demo->race_started(m_race->arena());
		m_state = m_next = State::RACE;
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
			//back on the ground at the start
			if (m_race && m_race->phase() == Race::Phase::PLAY) m_race->spawn(0);
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
		collision->debug(false);
	}

	bool                       m_loop{ true };
	State                      m_state{ State::MENU };
	State                      m_next{ State::MENU }; //asked by the UI, at the next frame
	Square::Time::FPSCounter   m_counter;
	Graphics                   m_graphics{ context() };
	TitleScreen                m_title{ context() };
	RushUI                     m_ui{ context() };
	std::unique_ptr<Race>      m_race;
	std::unique_ptr<DemoTools> m_demo; //RUSH_DEMO only
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
	, new RushGame()
	);
	return 0;
}
