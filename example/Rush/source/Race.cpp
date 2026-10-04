//
//  Race.cpp
//  Rush
//
//  See Race.h.
//
#include <algorithm>
#include <cmath>
#include <Race.h>
#include <Arena.h>
#include <Checkpoints.h>
#include <HovercraftInput.h>
#include <HovercraftAI.h>
#include <CameraFollow.h>

namespace AuxRace
{
	//the skin of a hovercraft: its materials its own, the albedo of the skin
	void paint(Square::Context& square_context, Square::Shared<Square::Scene::Actor> hovercraft, const std::string& skin)
	{
		using namespace Square;
		if (!hovercraft) return;
		auto skin_texture = square_context.resource<Resource::Texture>(skin);
		if (!skin_texture)
		{
			square_context.logger()->info("Error to load the skin " + skin);
			return;
		}
		hovercraft->visit([&square_context, &skin_texture](Shared<Scene::Actor> node) -> bool
		{
			if (!node->contains<Scene::StaticMesh>()) return true;
			for (auto& material : node->component<Scene::StaticMesh>()->m_materials)
			{
				if (!material) continue;
				auto own = DynamicPointerCast<Resource::Material>(square_context.resource_instance(material->resource_name()));
				if (!own) continue;
				if (auto albedo = own->parameter_by_name("albedo_map")) albedo->set(skin_texture);
				//the skin is the whole albedo (the model color, e.g. the 0.8 grey of Blender, would darken it)
				if (auto color = own->parameter_by_name("color")) color->set(Square::Vec4(1.0f));
				material = own;
			}
			return true;
		});
	}

	//a component of an actor out of it (if it has one)
	template < typename T >
	void remove_component(Square::Shared<Square::Scene::Actor> actor)
	{
		if (actor->contains<T>()) actor->remove(actor->component<T>());
	}
}

Race::Race(Square::Context& context, Square::Scene::World& world)
: m_context(context)
, m_world(world)
{
}

Race::~Race()
{
	unload();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//load
bool Race::load(const RaceMap& map)
{
	m_level = m_world.level(s_race_world_level);
	m_level->active(true);
	m_arena = std::make_unique<Arena>(m_context);
	if (!m_arena->load(m_level, map.m_name)) return false;
	if (auto follow = m_arena->camera_follow()) follow->bounds(map.m_camera_bounds);
	load_light_beam();
	load_hovercraft();
	phase(Phase::START);
	return true;
}

void Race::load_light_beam()
{
	using namespace Square;
	m_light_beam = m_level->load_actor("light_beam/scene");
	if (!m_light_beam)
	{
		m_context.logger()->info("Error to load light_beam");
		return;
	}
	// its light: a point light in the middle, a little over the ground, with shadow and a large
	// radius (a child of the beam: it goes with it from checkpoint to checkpoint)
	auto beam_light = m_light_beam->child();
	beam_light->name("light_beam_light");
	beam_light->position({ 0.0f, 1.25f, 0.0f });
	auto point_light = beam_light->component<Scene::PointLight>();
	point_light->diffuse({ 0.1f, 0.7f, 1.0f });
	point_light->specular({ 0.1f, 0.7f, 1.0f });
	point_light->constant(1.0f);
	point_light->radius(80.0f);
	point_light->inside_radius(15.0f);
	point_light->shadow({ 2048, 2048 });
	// the checkpoints of the level
	m_checkpoints = m_light_beam->component<Checkpoints>();
	const size_t count = m_checkpoints->collect(m_arena->actor());
	m_context.logger()->info("checkpoints: " + std::to_string(count));
	m_checkpoints->on_reached([this](size_t index, Shared<Scene::Actor> who)
	{
		reached(index, who);
	});
}

void Race::load_hovercraft()
{
	using namespace Square;
	m_racers.reserve(s_racers);
	for (size_t id = 0; id != s_racers; ++id)
	{
		Racer racer;
		racer.m_name  = id == 0 ? "player" : "npc " + std::to_string(id);
		racer.m_actor = m_level->load_actor("hovercraft/scene");
		if (!racer.m_actor)
		{
			m_context.logger()->info("Error to load hovercraft");
			break;
		}
		racer.m_actor->name("hovercraft_" + std::to_string(id + 1));
		// the driver: a component of the hovercraft, updated every frame by the scene; who drives
		// it sets its input (from PLAY: the player its keys, an NPC the light)
		racer.m_driver = racer.m_actor->component<HovercraftDriver>();
		racer.m_driver->settings(settings(id));
		// who reaches the light scores
		if (m_checkpoints) m_checkpoints->add_target(racer.m_actor);
		m_racers.push_back(racer);
		paint(id);
		spawn(id, false);
	}
	// the camera follows the player; at the start it is in its place of the scene: it glides
	// behind the hovercraft (the START of the race)
	auto follow = m_arena->camera_follow();
	if (follow && !m_racers.empty()) follow->target(m_racers[0].m_actor);
}

void Race::unload()
{
	if (!m_level) return;
	for (const auto& racer : m_racers)
	{
		if (racer.m_actor) m_level->remove(racer.m_actor);
	}
	m_racers.clear();
	if (m_light_beam) m_level->remove(m_light_beam);
	m_light_beam.reset();
	m_checkpoints.reset();
	if (m_arena) m_arena->unload(m_level);
	m_arena.reset();
	m_level->active(false);
	m_level.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//phases
void Race::update(double delta_time)
{
	m_phase_time += std::min(delta_time, s_max_frame_time);
	switch (m_phase)
	{
	case Phase::START:
		if (m_phase_time >= s_start_time) phase(Phase::PLAY);
	break;
	case Phase::PLAY:
		if (m_end) phase(Phase::END);
	break;
	case Phase::END:
	default: break;
	}
}

void Race::phase(Phase phase)
{
	m_phase = phase;
	m_phase_time = 0.0;
	controls(phase);
}

void Race::controls(Phase phase)
{
	for (size_t id = 0; id != m_racers.size(); ++id)
	{
		auto actor = m_racers[id].m_actor;
		const bool player = id == 0;
		//START: no one drives
		AuxRace::remove_component<HovercraftInput>(actor);
		AuxRace::remove_component<HovercraftAI>(actor);
		switch (phase)
		{
		case Phase::PLAY:
			//the player its keys, the NPCs the light
			if (player) actor->component<HovercraftInput>();
			else        actor->component<HovercraftAI>()->checkpoints(m_checkpoints);
		break;
		case Phase::END:
			//everyone the light: the player an NPC too
			actor->component<HovercraftAI>()->checkpoints(m_checkpoints);
		break;
		case Phase::START:
		default: break;
		}
	}
}

void Race::reached(size_t checkpoint, Square::Shared<Square::Scene::Actor> who)
{
	//the scores: only while playing
	if (m_phase != Phase::PLAY || m_end) return;
	auto it = std::find_if(m_racers.begin(), m_racers.end(), [&](const Racer& racer) { return racer.m_actor == who; });
	if (it == m_racers.end()) return;
	++it->m_score;
	std::string board;
	for (const auto& other : m_racers) board += " " + other.m_name + ":" + std::to_string(other.m_score);
	m_context.logger()->info(it->m_name + " reached checkpoint " + std::to_string(checkpoint + 1) + " |" + board);
	if (it->m_score < s_winning_score) return;
	//the end of the race
	m_winner = size_t(it - m_racers.begin());
	m_context.logger()->info(m_winner == 0 ? std::string("You win!") : "You lose! (" + it->m_name + " wins)");
	//in the update of the race: here the scene is updating its components (the controls change)
	m_end = true;
}

Race::Phase Race::phase() const
{
	return m_phase;
}

double Race::phase_time() const
{
	return m_phase_time;
}

size_t Race::winner() const
{
	return m_winner;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//hovercraft
void Race::paint(size_t id)
{
	if (id >= m_racers.size() || !s_skins[id][0]) return;
	paint(m_context, m_racers[id].m_actor, s_skins[id]);
}

void Race::paint(Square::Context& context, Square::Shared<Square::Scene::Actor> hovercraft, const std::string& skin)
{
	AuxRace::paint(context, hovercraft, skin);
}

void Race::spawn(size_t id, bool snap_camera)
{
	if (id >= m_racers.size() || !m_arena) return;
	const Square::Vec3& start = m_arena->start(id);
	const Square::Vec3  to_center = m_arena->center() - start;
	const bool  away_from_center = (to_center.x * to_center.x + to_center.z * to_center.z) > 1e-4f;
	const float yaw = away_from_center ? Square::degrees(std::atan2(to_center.x, to_center.z)) : 0.0f;
	m_racers[id].m_driver->spawn(start, yaw);
	const bool player = id == 0;
	auto follow = m_arena->camera_follow();
	if (player && snap_camera && follow) follow->snap();
}

HovercraftDriver::Settings Race::settings(size_t id)
{
	HovercraftDriver::Settings settings;
	// collision types of body, wheels and ground
	settings.body_type  = TYPE_BODY;
	settings.wheel_type = TYPE_WHEEL;
	settings.scene_type = TYPE_SCENE;
	// body: x/z radius at 80% of the hull (closer to its shape, between the hovercraft), y radius
	// as the hull
	settings.body_radius_scale = Square::Vec2(0.8f, 1.0f);
	// its engine
	const Engine& engine = s_engines[id % s_racers];
	settings.acceleration *= engine.acceleration;
	settings.max_speed    *= engine.max_speed;
	settings.max_reverse  *= engine.max_speed;
	settings.drag          = std::min(settings.drag * engine.drag, 0.999f);
	return settings;
}

const Arena& Race::arena() const
{
	return *m_arena;
}

const std::vector<Race::Racer>& Race::racers() const
{
	return m_racers;
}

int Race::player_speed() const
{
	if (m_racers.empty() || !m_racers[0].m_driver) return 0;
	const auto& driver = m_racers[0].m_driver;
	const float top = std::max(driver->settings().max_speed, 0.0001f);
	const float share = std::clamp(std::abs(driver->speed()) / top, 0.0f, 1.0f);
	return int(std::round(share * 100.0f));
}
