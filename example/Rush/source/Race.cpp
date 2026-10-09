//
//  Race.cpp
//  Rush
//
//  See Race.h.
//
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <limits>
#include <Race.h>
#include <SnowTrails.h>
#include <Arena.h>
#include <Collision.h>
#include <Checkpoints.h>
#include <HovercraftInput.h>
#include <HovercraftAI.h>
#include <HovercraftFans.h>
#include <CameraFollow.h>
#include <RushConfig.h>

using namespace Rush;

namespace AuxRace
{
	//the motion blur of a hovercraft: its meshes (the body, the fans) blurred by their own motion
	void motion_blur(Square::Shared<Square::Scene::Actor> hovercraft)
	{
		using namespace Square;
		hovercraft->visit([](Shared<Scene::Actor> part) -> bool
		{
			if (part->contains<Scene::StaticMesh>()) part->component<Scene::StaticMesh>()->motion_blur(true);
			return true;
		});
	}

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
				//the parts of their own (the skirt, the fans: "hovercraft_..." materials) not painted
				const std::string& name = material->resource_name();
				if (name.compare(name.find_last_of('/') + 1, 11, "hovercraft_") == 0) continue;
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

SQUARE_CLASS_OBJECT_REGISTRATION(Race);

void Race::object_registration(Square::Context& ctx)
{
	//factory: actor->component<Race>()
	ctx.add_object<Race>();
}

Race::Race(Square::Context& context)
: Component(context)
{
}

void Race::serialize(Square::Data::Archive& archive)           { Square::Data::serialize(archive, this); }
void Race::serialize_json(Square::Data::JsonValue& archive)    { Square::Data::serialize_json(archive, this); }
void Race::deserialize(Square::Data::Archive& archive)         { Square::Data::deserialize(archive, this); }
void Race::deserialize_json(Square::Data::JsonValue& archive)  { Square::Data::deserialize_json(archive, this); }

Race::~Race()
{
	unload();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//load
bool Race::load(const RaceMap& map)
{
	//the level of its actor
	auto owner = actor().lock();
	m_level = owner ? owner->level().lock() : nullptr;
	if (!m_level) return false;
	m_map = &map;
	m_arena = std::make_unique<Arena>(context());
	if (!m_arena->load(m_level, map.m_name)) return false;
	m_arena->camera_clip(map.m_camera_near, map.m_camera_far);
	if (map.m_sun_set) m_arena->sun_direction(Arena::sun_direction(map.m_sun_azimuth, map.m_sun_elevation));
	update_sun();
	m_sink = map.m_trails ? Config::get().trails_sink() : 0.0f;
	//an arena: its navigation, the beam of light; a circuit: its course (the line of the AI)
	if (!circuit())
	{
		load_navigation();
		load_light_beam();
	}
	if (map.m_trails) load_trails();
	if (map.m_wakes) load_wakes();
	load_hovercraft();
	m_standings.clear();
	for (size_t id = 0; id != m_racers.size(); ++id) m_standings.push_back(id);
	phase(Phase::START);
	return true;
}

void Race::load_wakes()
{
	//the water of the field: a small map (the waves: small, soft), gone in less than a second
	m_wakes = std::make_unique<SnowTrails>(context());
	SnowTrails::Settings settings;
	const Square::Vec3 center = m_arena->center();
	settings.center = Square::Vec2(center.x, center.z);
	settings.size = 220.0f;
	settings.resolution = 512;
	settings.radius = 1.4f;
	settings.refill = 0.8f;
	settings.map = "wake_map";
	settings.area = "wake_area";
	if (!m_wakes->create(settings))
	{
		context().logger()->warning("water wakes: no texture");
		m_wakes.reset();
		return;
	}
	m_wakes->attach(m_arena->actor());
}

void Race::load_trails()
{
	//the field (radius ~85): a map a little larger, centered on the arena; a circuit: a window
	//around the player (moved with it)
	m_trails = std::make_unique<SnowTrails>(context());
	SnowTrails::Settings settings;
	const Square::Vec3 center = m_arena->center();
	settings.center = Square::Vec2(center.x, center.z);
	if (circuit()) settings.size = Config::get().circuit().m_trails_window;
	if (!m_trails->create(settings))
	{
		context().logger()->warning("snow trails: no texture");
		m_trails.reset();
		return;
	}
	m_trails->attach(m_arena->actor());
}

void Race::load_navigation()
{
	using namespace Square;
	//an agent as wide as a hovercraft (and a little more): the edges kept that far
	Navigation::NavGrid::Settings settings;
	settings.cell_size    = 1.0f;
	settings.agent_radius = 2.2f;
	settings.agent_height = 2.5f;
	settings.max_slope    = 30.0f;
	settings.max_step     = 0.7f;
	m_navigation = std::make_shared<Navigation::NavGrid>();
	//the navmesh of the map (where the AI drives: its edges and holes the obstacles), else the
	//triangles the hovercraft collide with (the invisible walls too: the obstacles found there)
	auto navmesh = m_arena->navmesh();
	const bool authored = navmesh != nullptr;
	bool built = false;
	if (authored)
	{
		built = m_navigation->build(navmesh, settings);
	}
	else
	{
		std::vector<Vec3> triangles;
		m_arena->actor()->component<MeshCollider>()->mesh().triangles(triangles);
		built = m_navigation->build(triangles, m_arena->min(), m_arena->max(), settings);
	}
	if (!built)
	{
		context().logger()->warning("navigation: nothing walkable");
		m_navigation.reset();
		return;
	}
	context().logger()->info(std::string(authored ? "navigation (navmesh): " : "navigation (collisions): ") + std::to_string(m_navigation->width()) + "x" + std::to_string(m_navigation->height()) + " cells");
}

void Race::sink(Square::Shared<Square::Scene::Actor> hovercraft) const
{
	using namespace Square;
	//its meshes (the children: the actor is the body of the physics), down along its up
	for (const auto& child : hovercraft->childs())
	{
		if (!child->contains<Scene::StaticMesh>()) continue;
		child->position(child->position() - Vec3(0.0f, m_sink, 0.0f));
	}
}

void Race::load_light_beam()
{
	using namespace Square;
	m_light_beam = m_level->load_actor("light_beam/scene");
	if (!m_light_beam)
	{
		context().logger()->info("Error to load light_beam");
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
	context().logger()->info("checkpoints: " + std::to_string(count));
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
			context().logger()->info("Error to load hovercraft");
			break;
		}
		racer.m_actor->name("hovercraft_" + std::to_string(id + 1));
		// the driver: a component of the hovercraft, updated every frame by the scene; who drives
		// it sets its input (from PLAY: the player its keys, an NPC the light)
		racer.m_driver = racer.m_actor->component<HovercraftDriver>();
		racer.m_driver->settings(Config::get().driver(id, circuit()));
		//its fans spin with its speed
		racer.m_actor->component<HovercraftFans>();
		// who reaches the light scores
		if (m_checkpoints) m_checkpoints->add_target(racer.m_actor);
		m_racers.push_back(racer);
		paint(id);
		AuxRace::motion_blur(racer.m_actor);
		if (m_sink > 0.0f) sink(racer.m_actor);
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
	m_level.reset();
	m_map = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//phases
void Race::on_update(double delta_time)
{
	const double max_frame_time = Config::get().rules().m_max_frame_time;
	m_phase_time += std::min(delta_time, max_frame_time);
	//the water moves, the sun goes on its way
	m_water_time += delta_time;
	update_sun();
	if (m_arena) m_arena->animate(m_water_time);
	//a circuit: its laps (while playing, and after: the others finish)
	if (circuit() && m_phase != Phase::START)
	{
		if (m_phase == Phase::PLAY) m_race_time += std::min(delta_time, max_frame_time);
		update_circuit(delta_time);
	}
	//the grooves of the hovercraft in the snow; a circuit: their map a window around the player,
	//moved with it (the grooves kept where they are)
	if (m_trails && circuit() && !m_racers.empty())
	{
		const Square::Vec3 player = m_racers[0].m_actor->position(true);
		const Square::Vec2 at(player.x, player.z);
		if (glm::length(at - m_trails->center()) > m_trails->size() * 0.25f) m_trails->recenter(at);
	}
	if (m_trails)
	{
		for (size_t id = 0; id != m_racers.size(); ++id)
		{
			const Racer& racer = m_racers[id];
			m_trails->press(id, racer.m_actor->position(true), racer.m_driver && racer.m_driver->on_ground());
		}
		m_trails->update(delta_time);
	}
	//the wakes on the water (where there is no water, nothing reads them); a large map: their
	//map a window around the player, moved with it
	if (m_wakes && !m_racers.empty())
	{
		const Square::Vec3 player = m_racers[0].m_actor->position(true);
		const Square::Vec2 at(player.x, player.z);
		if (glm::length(at - m_wakes->center()) > m_wakes->size() * 0.3f) m_wakes->recenter(at);
	}
	if (m_wakes)
	{
		for (size_t id = 0; id != m_racers.size(); ++id)
		{
			const Racer& racer = m_racers[id];
			m_wakes->press(id, racer.m_actor->position(true), racer.m_driver && racer.m_driver->on_ground());
		}
		m_wakes->update(delta_time);
	}
	switch (m_phase)
	{
	case Phase::START:
		if (m_phase_time >= Config::get().rules().m_start_time) phase(Phase::PLAY);
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
		//an NPC: the light of an arena, the line of a circuit (each in its own lane)
		auto drive = [&]()
		{
			auto ai = actor->component<HovercraftAI>();
			//(each its lane, its courage: who takes the other ways)
			if (circuit())
			{
				const float lane = (float(id % 3) - 1.0f) * 4.0f;
				ai->follow(m_arena->course(), lane, Config::get().racer(id).m_courage);
			}
			else           ai->race(m_checkpoints, m_navigation);
		};
		switch (phase)
		{
		case Phase::PLAY:
			//the player its keys (RUSH_AUTOPILOT set: the AI, to watch a whole race), the NPCs the light
			if (player && !std::getenv("RUSH_AUTOPILOT")) actor->component<HovercraftInput>();
			else                                          drive();
		break;
		case Phase::END:
			//everyone the light: the player an NPC too
			drive();
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
	context().logger()->info(it->m_name + " reached checkpoint " + std::to_string(checkpoint + 1) + " |" + board);
	if (it->m_score < Config::get().rules().m_winning_score) return;
	//the end of the race
	m_winner = size_t(it - m_racers.begin());
	context().logger()->info(m_winner == 0 ? std::string("You win!") : "You lose! (" + it->m_name + " wins)");
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
	const std::string& skin = Config::get().racer(id).m_skin;
	if (id >= m_racers.size() || skin.empty()) return;
	paint(context(), m_racers[id].m_actor, skin);
}

void Race::paint(Square::Context& context, Square::Shared<Square::Scene::Actor> hovercraft, const std::string& skin)
{
	AuxRace::paint(context, hovercraft, skin);
}

void Race::spawn(size_t id, bool snap_camera)
{
	if (id >= m_racers.size() || !m_arena) return;
	Square::Vec3 start = m_arena->start(id);
	const Square::Vec3  to_center = m_arena->center() - start;
	const bool  away_from_center = (to_center.x * to_center.x + to_center.z * to_center.z) > 1e-4f;
	float yaw = away_from_center ? Square::degrees(std::atan2(to_center.x, to_center.z)) : 0.0f;
	//a circuit: along its guide where it starts, the progress there
	if (circuit())
	{
		const Course& course = m_arena->course();
		Racer& racer = m_racers[id];
		racer.m_segment = 0;
		racer.m_progress = course.project(start, racer.m_segment, true);
		//a loop: before its line (the end of the guide) is before the start of the lap
		if (course.closed() && racer.m_progress > course.length() * 0.5f) racer.m_progress -= course.length();
		//a loop starts past its line (cp_1), a start to a finish goes to its first checkpoint
		racer.m_next_checkpoint = course.closed() ? 1 % course.checkpoints().size() : 0;
		const Square::Vec3 way = course.direction(racer.m_progress);
		yaw = Square::degrees(std::atan2(way.x, way.z));
	}
	m_racers[id].m_driver->spawn(start, yaw);
	const bool player = id == 0;
	auto follow = m_arena->camera_follow();
	if (player && snap_camera && follow) follow->snap();
}

const Arena& Race::arena() const
{
	return *m_arena;
}

const std::vector<Race::Racer>& Race::racers() const
{
	return m_racers;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//circuit
bool Race::circuit() const
{
	return m_map && m_map->m_mode == RaceMode::CIRCUIT && m_arena && m_arena->course().valid();
}

int Race::laps() const
{
	//from a start to a finish: one
	if (circuit() && !m_arena->course().closed()) return 1;
	return m_map ? std::max(m_map->m_laps, 1) : 1;
}

size_t Race::place(size_t id) const
{
	const auto it = std::find(m_standings.begin(), m_standings.end(), id);
	return it == m_standings.end() ? id + 1 : size_t(it - m_standings.begin()) + 1;
}

const std::vector<size_t>& Race::standings() const
{
	return m_standings;
}

bool Race::wrong_way() const
{
	return circuit() && !m_racers.empty() && m_racers[0].m_wrong_way > 0.8f;
}

double Race::race_time() const
{
	return m_race_time;
}

float Race::checkpoint_progress(size_t checkpoint, int lap) const
{
	const Course& course = m_arena->course();
	return float(lap - 1) * course.length() + course.checkpoints()[checkpoint].m_along;
}

float Race::progress_now(size_t id)
{
	const Course& course = m_arena->course();
	Racer& racer = m_racers[id];
	const float along = course.project(racer.m_actor->position(true), racer.m_segment);
	float progress = along;
	if (course.closed())
	{
		//on its lap: the one of the laps near where it was (around the line of the start)
		const float lap = course.length();
		const float base = std::floor(racer.m_progress / lap) * lap;
		float best = std::numeric_limits<float>::max();
		for (float candidate : { base + along - lap, base + along, base + along + lap })
		{
			if (std::abs(candidate - racer.m_progress) < best)
			{
				best = std::abs(candidate - racer.m_progress);
				progress = candidate;
			}
		}
	}
	//never past the checkpoint it did not pass (a shortcut through nowhere does not pay)
	if (!racer.m_finished)
	{
		const size_t next = racer.m_next_checkpoint;
		//(a loop: its line of the start is at the next lap)
		const int lap = course.closed() && next == 0 ? racer.m_lap + 1 : racer.m_lap;
		progress = std::min(progress, checkpoint_progress(next, lap) + 25.0f);
	}
	return progress;
}

void Race::update_circuit(double delta_time)
{
	using namespace Square;
	const Course& course = m_arena->course();
	for (size_t id = 0; id != m_racers.size(); ++id)
	{
		Racer& racer = m_racers[id];
		const Vec3 position = racer.m_actor->position(true);
		//the checkpoints passed: past the line of the next one, within its radius
		for (size_t step = 0; step != 2 && !racer.m_finished; ++step)
		{
			if (!course.crossed(racer.m_next_checkpoint, position)) break;
			passed(id);
		}
		racer.m_progress = progress_now(id);
		//a boost pad under it (not again while it is boosted)
		if (racer.m_driver && !racer.m_driver->boosting())
		{
			for (const Vec3& pad : m_arena->boosts())
			{
				const BoostRules& boost = Config::get().boost();
				const float across = length(Vec2(pad.x - position.x, pad.z - position.z));
				const float above = std::abs(pad.y - position.y);
				if (across > boost.m_radius || above > 6.0f) continue;
				racer.m_driver->boost(boost.m_time, boost.m_speed, boost.m_acceleration);
				break;
			}
		}
		//fallen (off a jump, into the gorge): back on the guide
		const Vec3 on_guide = course.point(racer.m_progress);
		if (position.y < on_guide.y - Config::get().circuit().m_fall_depth)
		{
			respawn(id);
			continue;
		}
		//the wrong way: driving forward against the race
		const Vec3  forward = racer.m_actor->rotation(true) * Constants::axis_z;
		const Vec3  way = course.direction(racer.m_progress);
		const bool  moving = racer.m_driver && racer.m_driver->speed() > racer.m_driver->settings().max_speed * 0.15f;
		const bool  against = forward.x * way.x + forward.z * way.z < -0.3f;
		racer.m_wrong_way = moving && against ? racer.m_wrong_way + float(delta_time) : 0.0f;
	}
	rank();
}

void Race::passed(size_t id)
{
	Racer& racer = m_racers[id];
	const Course& course = m_arena->course();
	const size_t count = course.checkpoints().size();
	bool finish = false;
	if (course.closed())
	{
		//the line of the start (cp_1): a lap; the last lap: the finish
		const bool line = racer.m_next_checkpoint == 0;
		racer.m_next_checkpoint = (racer.m_next_checkpoint + 1) % count;
		if (line)
		{
			++racer.m_lap;
			finish = racer.m_lap > laps();
			if (!finish) context().logger()->info(racer.m_name + " lap " + std::to_string(racer.m_lap) + "/" + std::to_string(laps()));
		}
	}
	else
	{
		//from a start to a finish: the last checkpoint is the finish
		finish = racer.m_next_checkpoint + 1 >= count;
		if (!finish) ++racer.m_next_checkpoint;
	}
	if (!finish || racer.m_finished) return;
	racer.m_finished = true;
	racer.m_finish_time = m_race_time;
	racer.m_lap = laps();
	if (++m_finished == 1) m_winner = id;
	context().logger()->info(racer.m_name + " finished " + std::to_string(m_finished) + " in " + std::to_string(m_race_time) + "s");
	//the player: the end of the race (in the update of the race, as an arena)
	if (id == 0 && m_phase == Phase::PLAY) m_end = true;
}

void Race::rank()
{
	//the finished ones by their time, then the others by how far they are
	std::vector<float> ahead(m_racers.size());
	for (size_t id = 0; id != m_racers.size(); ++id)
	{
		const Racer& racer = m_racers[id];
		ahead[id] = racer.m_finished ? 1e9f - float(racer.m_finish_time) : racer.m_progress;
	}
	m_standings.resize(m_racers.size());
	for (size_t id = 0; id != m_racers.size(); ++id) m_standings[id] = id;
	std::stable_sort(m_standings.begin(), m_standings.end(), [&ahead](size_t a, size_t b) { return ahead[a] > ahead[b]; });
}

void Race::respawn(size_t id)
{
	using namespace Square;
	if (id >= m_racers.size() || !m_arena) return;
	if (!circuit())
	{
		spawn(id);
		return;
	}
	//on the guide a little behind where it is, facing the race, each a little aside
	const Course& course = m_arena->course();
	Racer& racer = m_racers[id];
	const float  along = racer.m_progress - 10.0f;
	const Vec3   way = course.direction(along);
	const Vec3   right(-way.z, 0.0f, way.x);
	const Vec3   start = course.point(along) + right * ((float(id) - 1.5f) * 3.5f) + Vec3(0.0f, 3.0f, 0.0f);
	racer.m_driver->spawn(start, degrees(std::atan2(way.x, way.z)));
	auto follow = m_arena->camera_follow();
	if (id == 0 && follow) follow->snap();
}

namespace AuxRace
{
	//where the sun is at a time along its steps: between two of them on its way, before the
	//first one and after the last one there
	static SunStep sun_at(const std::vector<SunStep>& steps, float time)
	{
		if (time <= steps.front().m_time) return steps.front();
		for (size_t i = 1; i < steps.size(); ++i)
		{
			const SunStep& to = steps[i];
			if (time > to.m_time) continue;
			const SunStep& from = steps[i - 1];
			const float span = std::max(to.m_time - from.m_time, 0.0001f);
			const float t = (time - from.m_time) / span;
			SunStep at;
			at.m_time      = time;
			at.m_azimuth   = from.m_azimuth + (to.m_azimuth - from.m_azimuth) * t;
			at.m_elevation = from.m_elevation + (to.m_elevation - from.m_elevation) * t;
			return at;
		}
		return steps.back();
	}
}

bool Race::sun_moving() const
{
	bool moving = false;
	if (m_map && !m_map->m_sun_steps.empty())
	{
		//between its first step and its last one
		const float time = float(m_race_time);
		moving = time >= m_map->m_sun_steps.front().m_time && time < m_map->m_sun_steps.back().m_time;
	}
	return moving;
}

void Race::update_sun()
{
	if (m_map && m_arena)
	{
		if (!m_map->m_sun_steps.empty())
		{
			const SunStep at = AuxRace::sun_at(m_map->m_sun_steps, float(m_race_time));
			m_arena->sun_direction(Arena::sun_direction(at.m_azimuth, at.m_elevation));
		}
		//the fit of its cascades: while it moves, once it stays (as the map chooses)
		auto sun = m_arena->sun();
		if (sun && m_map->m_shadow_fit_set)
		{
			sun->cascade_fit(m_map->shadow_fit(sun_moving()));
		}
	}
}

bool Race::zone_fog(RaceFog& fog) const
{
	using namespace Square;
	if (!circuit() || m_map->m_zones.empty() || m_racers.empty()) return false;
	//where the player is on its lap, the zones from their checkpoints
	const Course& course = m_arena->course();
	const float  lap = course.length();
	const float  along = course.closed() ? std::fmod(std::max(m_racers[0].m_progress, 0.0f), lap) : m_racers[0].m_progress;
	const std::vector<CircuitZone>& zones = m_map->m_zones;
	const size_t zones_count = zones.size();
	const auto& checkpoints = course.checkpoints();
	auto start_of = [&](size_t zone) { return checkpoints[std::min(zones[zone].m_gate, checkpoints.size() - 1)].m_along; };
	size_t zone = 0;
	for (size_t k = 0; k != zones_count; ++k)
	{
		if (start_of(k) <= along) zone = k;
	}
	//the next zone (after the last one: the first one, at the end of the lap) blending in
	const size_t next = (zone + 1) % zones_count;
	const float  start = next == 0 ? lap : start_of(next);
	const float  blend = Config::get().circuit().m_zone_blend;
	const float  t = std::clamp((along - (start - blend)) / blend, 0.0f, 1.0f);
	const RaceFog& a = zones[zone].m_fog;
	const RaceFog& b = zones[next].m_fog;
	fog.m_on      = a.m_on || b.m_on;
	fog.m_color   = a.m_color + (b.m_color - a.m_color) * t;
	fog.m_density = a.m_density + (b.m_density - a.m_density) * t;
	fog.m_height  = a.m_height + (b.m_height - a.m_height) * t;
	fog.m_falloff = a.m_falloff + (b.m_falloff - a.m_falloff) * t;
	fog.m_sun     = a.m_sun + (b.m_sun - a.m_sun) * t;
	return true;
}

int Race::player_speed() const
{
	if (m_racers.empty() || !m_racers[0].m_driver) return 0;
	const auto& driver = m_racers[0].m_driver;
	//(out of the top speed of an arena: a circuit's faster, up to 130)
	const float faster = circuit() ? Config::get().circuit().m_speed : 1.0f;
	const float top = std::max(driver->settings().max_speed / faster, 0.0001f);
	const float share = std::clamp(std::abs(driver->speed()) / top, 0.0f, faster);
	return int(std::round(share * 100.0f));
}
