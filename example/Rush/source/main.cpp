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
#include <array>
#include <algorithm>
#include <Collision.h>
#include <Hovercraft.h>
#include <Checkpoints.h>
#include <HovercraftInput.h>
#include <HovercraftAI.h>
#include <CameraFollow.h>

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
		case Square::Video::KEY_X:
			//toggle the debug view of the collisions (mesh colliders, spheres, contacts)
			if (action == Square::Video::ActionEvent::PRESS)
			if (auto collision = world().instance<CollisionWorld>())
			{
				collision->debug(!collision->debug());
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
		case Square::Video::KEY_K:
			if (action == Square::Video::ActionEvent::RELEASE && m_ssao)
			{
				m_ssao->enabled(!m_ssao->enabled());
				context().logger()->info(std::string("SSAO: ") + (m_ssao->enabled() ? "on" : "off"));
			}
			break;
		case Square::Video::KEY_U:
			//SSAO debug view: the occlusion on the screen
			if (action == Square::Video::ActionEvent::RELEASE && m_ssao)
			{
				auto settings = m_ssao->settings();
				settings.debug = !settings.debug;
				m_ssao->settings(settings);
			}
			break;
		case Square::Video::KEY_H:
			if (action == Square::Video::ActionEvent::RELEASE && m_bloom)
			{
				m_bloom->enabled(!m_bloom->enabled());
				context().logger()->info(std::string("Bloom: ") + (m_bloom->enabled() ? "on" : "off"));
			}
			break;
		case Square::Video::KEY_I:
			//bloom debug view: only the bloom on the screen
			if (action == Square::Video::ActionEvent::RELEASE && m_bloom)
			{
				auto settings = m_bloom->settings();
				settings.debug = !settings.debug;
				m_bloom->settings(settings);
			}
			break;
		case Square::Video::KEY_F:
			//the player's hovercraft all mirror: the SSR on it
			if (action == Square::Video::ActionEvent::RELEASE)
			{
				m_mirror = !m_mirror;
				mirror(m_mirror);
				context().logger()->info(std::string("Mirror hovercraft: ") + (m_mirror ? "on" : "off"));
			}
			break;
		case Square::Video::KEY_R:
			if (action == Square::Video::ActionEvent::RELEASE && m_ssr)
			{
				m_ssr->enabled(!m_ssr->enabled());
				context().logger()->info(std::string("SSR: ") + (m_ssr->enabled() ? "on" : "off"));
			}
			break;
		case Square::Video::KEY_Y:
			//SSR debug views: off, only the reflection, projection check
			if (action == Square::Video::ActionEvent::RELEASE && m_ssr)
			{
				auto settings = m_ssr->settings();
				settings.debug = (settings.debug + 1) % 3;
				m_ssr->settings(settings);
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
				spawn(0);
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
		using namespace Square::Filesystem;
		//rs file
		context().add_resources(join(resource_dir(), "/resources.rs"));
		context().add_resources(join(resource_dir(), "common/resources.rs"));
		context().add_resources(join(resource_dir(), "example/Rush/resources.rs"));
		// level
		m_level = world().level("main");
		// the game
		setup_controls();
		setup_rendering();
		setup_collisions();
		auto arena = load_arena();
		load_light_beam(arena);
		load_hovercraft();
    }

	//controls: the actions of the input system (read by the HovercraftInput of the player)
	void setup_controls()
	{
		using namespace Square;
		if (auto input = System::get<InputSystem>(context()))
		{
			input->bind("forward",  Video::KEY_UP);
			input->bind("forward",  Video::KEY_W);
			input->bind("backward", Video::KEY_DOWN);
			input->bind("backward", Video::KEY_S);
			input->bind("left",     Video::KEY_LEFT);
			input->bind("left",     Video::KEY_A);
			input->bind("right",    Video::KEY_RIGHT);
			input->bind("right",    Video::KEY_D);
		}
	}

	//rendering pipeline of the world: SQUARE_RENDERING=forward|deferred (default: deferred), and
	//its post effects
	void setup_rendering()
	{
		using namespace Square;
		if (auto render_world = world().instance<RenderInstance>())
		{
			const char* rendering_type = std::getenv("SQUARE_RENDERING");
			const bool forward = rendering_type && Square::case_insensitive_equal(rendering_type, "forward");
			render_world->pipeline((forward ? RP_FORWARD : RP_DEFERRED) | RP_DEBUG);
			// post effects: SSAO (deferred: it darkens the ambient light), K to turn it on/off
			m_ssao = MakeShared<Render::SSAO>(context());
			// softer than the defaults: a light shade in the creases, not a dark halo
			Render::SSAO::Settings ssao_settings;
			ssao_settings.radius     = 0.85f; //smaller creases
			ssao_settings.intensity  = 0.45f; //light occlusion
			ssao_settings.contrast   = 1.1f;  //linear: no extra darkening
			ssao_settings.max_pixels = 32.0f; //near the camera: short reach, less cache misses
			m_ssao->settings(ssao_settings);
			render_world->add_post_effect(m_ssao);
			// screen space reflections (deferred): before the bloom, the reflected lights glow too;
			// R to turn it on/off, Y for its debug views
			m_ssr = MakeShared<Render::SSR>(context());
			Render::SSR::Settings ssr_setting;
			ssr_setting.half_resolution = false;
			ssr_setting.max_distance = 100.0;
			ssr_setting.steps = 128;
			m_ssr->settings(ssr_setting);
			render_world->add_post_effect(m_ssr);
			// bloom (forward and deferred): the lights and the emissive glow, H to turn it on/off
			m_bloom = MakeShared<Render::Bloom>(context());
			render_world->add_post_effect(m_bloom);
		}
	}

	//collisions of the world (a game system, started on demand): body and wheels slide on the
	//scene (Collisions BODY,SCENE,2,3 / WHEEL,SCENE,2,3: polygon, slide xz)
	void setup_collisions()
	{
		context().start_system<CollisionSystem>();
		if (auto collision = world().instance<CollisionWorld>())
		{
			collision->collisions(TYPE_BODY, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
			collision->collisions(TYPE_WHEEL, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDEXZ);
			collision->collisions(TYPE_BODY, TYPE_BODY, CollisionMethod::SPHERE, CollisionResponse::SLIDEXZ);
			collision->collisions(TYPE_CAMERA, TYPE_SCENE, CollisionMethod::POLYGON, CollisionResponse::SLIDE);
			collision->debug(false);
		}
	}

	//the map: the arena (solid), its sun, the starts of the hovercraft (spawn_point_1..4) and
	//its camera (the chase camera of the player)
	Square::Shared<Square::Scene::Actor> load_arena()
	{
		using namespace Square;
		using namespace Square::Scene;
		auto arena = m_level->load_actor("arena/scene");
		if (!arena)
		{
			context().logger()->info("Error to load arena");
			return nullptr;
		}
		arena->position({ 0.0f, 4.0f, 0.0f });
		// Sun
		m_light = arena->child("sun");
		if (!m_light)
		{
			context().logger()->info("arena has no 'sun'/'light' node");
		}
		// the arena is solid: a mesh collider of the scene type (its triangles, from where it is
		// placed)
		auto arena_collider = arena->component<MeshCollider>();
		arena_collider->type(TYPE_SCENE);
		context().logger()->info("arena collision triangles: " + std::to_string(arena_collider->mesh().size()));
		// start of the hovercraft: the spawn points of the scene (after the arena is placed),
		// spawn_point_1 the player, the others the NPCs; they start facing the middle
		m_arena_center = arena->position(true);
		arena->visit([&](Shared<Actor> node) -> bool
		{
			for (size_t id = 0; id != s_racers; ++id)
			{
				if (node->name() == "spawn_point_" + std::to_string(id + 1)) m_starts[id] = node->position(true);
			}
			return true;
		});
		// the camera of the scene: it chases the hovercraft in world space (out of the arena, at
		// the level root), with a sphere of the camera type, so it does not go through the walls
		// and the ground
		m_camera = arena->child("camera");
		if (!m_camera)
		{
			context().logger()->info("arena has no 'camera' node");
			return arena;
		}
		uint32_t window_width, window_height;
		context().window()->get_size(window_width, window_height);
		m_camera->component<Camera>()->viewport({ 0,0, window_width, window_height });
		m_level->add(m_camera);
		auto camera_collider = m_camera->component<SphereCollider>();
		camera_collider->type(TYPE_CAMERA);
		camera_collider->radius(1.0f);
		m_camera_follow = m_camera->component<CameraFollow>();
		return arena;
	}

	//the light: the beam on the checkpoints of the arena (checkpoint_1, checkpoint_2...), with
	//its point light; who touches it scores and the beam goes to another checkpoint
	void load_light_beam(Square::Shared<Square::Scene::Actor> arena)
	{
		using namespace Square;
		using namespace Square::Scene;
		auto light_beam = m_level->load_actor("light_beam/scene");
		if (!light_beam)
		{
			context().logger()->info("Error to load light_beam");
			return;
		}
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
		// the checkpoints
		m_checkpoints = light_beam->component<Checkpoints>();
		const size_t count = m_checkpoints->collect(arena);
		context().logger()->info("checkpoints: " + std::to_string(count));
		m_checkpoints->on_reached([this](size_t index, Shared<Actor> who)
		{
			reached(index, who);
		});
	}

	//the hovercraft: the player (the first) and the NPCs, each with its driver, at their
	//starts; the checkpoints know all of them (who reaches the light scores), the camera
	//follows the player
	void load_hovercraft()
	{
		using namespace Square;
		for (size_t id = 0; id != s_racers; ++id)
		{
			Racer racer;
			racer.m_name  = id == 0 ? "player" : "npc " + std::to_string(id);
			racer.m_actor = m_level->load_actor("hovercraft/scene");
			if (!racer.m_actor)
			{
				context().logger()->info("Error to load hovercraft");
				break;
			}
			racer.m_actor->name("hovercraft_" + std::to_string(id + 1));
			// its color (the player: the one of the model)
			if (s_skins[id][0]) paint(racer.m_actor, s_skins[id]);
			// the driver: a component of the hovercraft, updated every frame by the scene; who
			// drives it sets its input: the player (keys), an NPC (towards the light)
			racer.m_driver = racer.m_actor->component<HovercraftDriver>();
			racer.m_driver->settings(hovercraft_settings(id));
			if (id == 0)
			{
				m_player = racer.m_actor->component<HovercraftInput>();
			}
			else
			{
				racer.m_actor->component<HovercraftAI>()->checkpoints(m_checkpoints);
			}
			//
			if (m_checkpoints)
			{
				m_checkpoints->add_target(racer.m_actor);
			}
			m_racers.push_back(racer);
			spawn(id, false);
		}
		// the camera follows the player; at the start it is in its place of the scene: it glides
		// behind the hovercraft
		if (m_camera_follow && !m_racers.empty()) m_camera_follow->target(m_racers[0].m_actor);
	}

	//a hovercraft of its own color: its materials become its own (new objects of the same .mat,
	//not shared with the other hovercraft), with the albedo of the skin texture
	void paint(Square::Shared<Square::Scene::Actor> hovercraft, const std::string& skin)
	{
		using namespace Square;
		auto texture = context().resource<Resource::Texture>(skin);
		if (!texture)
		{
			context().logger()->info("Error to load the skin " + skin);
			return;
		}
		hovercraft->visit([&](Shared<Scene::Actor> node) -> bool
		{
			if (!node->contains<Scene::StaticMesh>()) return true;
			for (auto& material : node->component<Scene::StaticMesh>()->m_materials)
			{
				if (!material) continue;
				auto own = DynamicPointerCast<Resource::Material>(context().resource_instance(material->resource_name()));
				if (!own) continue;
				if (auto albedo = own->parameter_by_name("albedo_map")) albedo->set(texture);
				//the skin is the whole albedo (the model color, e.g. the 0.8 grey of Blender, would darken it)
				if (auto color = own->parameter_by_name("color")) color->set(Vec4(1.0f));
				material = own;
			}
			return true;
		});
	}

	//the player's hovercraft all mirror (to show the screen space reflections: it reflects the
	//arena and the other hovercraft), or back to its skin
	void mirror(bool enable)
	{
		using namespace Square;
		if (m_racers.empty() || !m_racers[0].m_actor) return;
		auto hovercraft = m_racers[0].m_actor;
		//back: its materials again from the .mat, with the skin
		if (!enable)
		{
			if (s_skins[0][0]) paint(hovercraft, s_skins[0]);
			return;
		}
		//chrome: a white metal (the albedo of a metal is its reflected color), perfectly smooth
		auto white = context().resource<Resource::Texture>("white");
		hovercraft->visit([&](Shared<Scene::Actor> node) -> bool
		{
			if (!node->contains<Scene::StaticMesh>()) return true;
			for (auto& material : node->component<Scene::StaticMesh>()->m_materials)
			{
				if (!material) continue;
				if (auto p = material->parameter_by_name("albedo_map"))    p->set(white);
				if (auto p = material->parameter_by_name("metallic_map"))  p->set(white);
				if (auto p = material->parameter_by_name("roughness_map")) p->set(white);
				if (auto p = material->parameter_by_name("color"))         p->set(Vec4(0.95f, 0.95f, 0.95f, 1.0f));
				if (auto p = material->parameter_by_name("metallic"))      p->set(1.0f);
				if (auto p = material->parameter_by_name("roughness"))     p->set(0.0f);
			}
			return true;
		});
	}

	//a hovercraft at its start, facing the middle of the arena; the player's camera straight
	//behind it (a teleport)
	void spawn(size_t id, bool snap_camera = true)
	{
		if (id >= m_racers.size()) return;
		const Square::Vec3& start = m_starts[id];
		const Square::Vec3  to_center = m_arena_center - start;
		const float yaw = (to_center.x * to_center.x + to_center.z * to_center.z) > 1e-4f
		                ? Square::degrees(std::atan2(to_center.x, to_center.z))
		                : 0.0f;
		m_racers[id].m_driver->spawn(start, yaw);
		if (id == 0 && snap_camera && m_camera_follow) m_camera_follow->snap();
	}

	//a hovercraft reached the light: it scores; at s_winning_score the match ends and a new one
	//starts
	void reached(size_t checkpoint, Square::Shared<Square::Scene::Actor> who)
	{
		for (auto& racer : m_racers)
		{
			if (racer.m_actor != who) continue;
			++racer.m_score;
			std::string board;
			for (const auto& other : m_racers) board += " " + other.m_name + ":" + std::to_string(other.m_score);
			context().logger()->info(racer.m_name + " reached checkpoint " + std::to_string(checkpoint + 1) + " |" + board);
			if (racer.m_score >= s_winning_score)
			{
				context().logger()->info(&racer == &m_racers[0] ? std::string("You win!") : "You lose! (" + racer.m_name + " wins)");
				for (auto& other : m_racers) other.m_score = 0;
			}
			break;
		}
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

	//hovercraft of the race: the player and the NPCs; the first to s_winning_score lights wins
	static constexpr size_t s_racers = 4;
	static constexpr int    s_winning_score = 10;
	//colors of the hovercraft: textures of assets/hovercraft_skins (made by origial_assets/
	//hovercraft/skins.py; "": the one of the model)
	static constexpr const char* s_skins[s_racers]
	{
		"hovercraft_skins/hovercraft_red",
		"hovercraft_skins/hovercraft_blue",
		"hovercraft_skins/hovercraft_green",
		"hovercraft_skins/hovercraft_yellow",
	};

	static HovercraftDriver::Settings hovercraft_settings(size_t id)
	{
		HovercraftDriver::Settings settings;
		// collision types of body, wheels and ground
		settings.body_type  = TYPE_BODY;
		settings.wheel_type = TYPE_WHEEL;
		settings.scene_type = TYPE_SCENE;
		// body: x/z radius at 80% of the hull (closer to its shape, between the hovercraft),
		// y radius as the hull
		settings.body_radius_scale = Square::Vec2(0.8f, 1.0f);
		// each hovercraft its own engine (data_player_positions of Limit Rush: move distance
		// and friction), relative to the player: acceleration, top speed, grip
		struct Engine { float acceleration; float max_speed; float drag; };
		static const Engine s_engines[s_racers]
		{
			{ 1.00f, 1.00f, 1.000f }, // player: 0.075, 0.974
			{ 0.73f, 0.96f, 1.006f }, // npc 1:  0.055, 0.980
			{ 0.80f, 0.83f, 1.001f }, // npc 2:  0.060, 0.975
			{ 0.93f, 0.69f, 0.991f }, // npc 3:  0.070, 0.965
		};
		const Engine& engine = s_engines[id % s_racers];
		settings.acceleration *= engine.acceleration;
		settings.max_speed    *= engine.max_speed;
		settings.max_reverse  *= engine.max_speed;
		settings.drag          = std::min(settings.drag * engine.drag, 0.999f);
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
	//a hovercraft of the race
	struct Racer
	{
		std::string                          m_name;
		Square::Shared<Square::Scene::Actor> m_actor;
		Square::Shared<HovercraftDriver>     m_driver;
		int                                  m_score{ 0 };
	};
	std::vector<Racer>                        m_racers;   //the first: the player
	Square::Shared<HovercraftInput>           m_player;
	Square::Shared<CameraFollow>              m_camera_follow;
	Square::Shared<Checkpoints>               m_checkpoints;
	Square::Shared<Square::Render::SSAO>      m_ssao;
	Square::Shared<Square::Render::Bloom>     m_bloom;
	Square::Shared<Square::Render::SSR>       m_ssr;
	bool                                      m_mirror{ false }; //F: the player's hovercraft all mirror
	//starts of the hovercraft (spawn_point_1..4 of the arena, a fallback without them), the middle
	//they face
	std::array<Square::Vec3, s_racers>        m_starts{ s_start, s_start + Square::Vec3(10, 0, 0), s_start + Square::Vec3(0, 0, 10), s_start + Square::Vec3(10, 0, 10) };
	Square::Vec3                              m_arena_center{ 0.0f };
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
