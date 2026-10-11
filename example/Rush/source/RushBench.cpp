//
//  RushBench.cpp
//  Rush
//
//  See RushBench.h.
//
#include <RushBench.h>
#include <Race.h>
#include <Arena.h>
#include <algorithm>
#include <Collision.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <random>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>

namespace AuxRushBench
{
	//frames after a change before its measure (the caches of the shadows, the levels of detail)
	static constexpr unsigned int s_window = 45;

	//the group of a node: its name less its numbers and its level of detail
	static std::string group_of(const std::string& name)
	{
		const std::regex numbers("(_lod[0-9]+)$|(_[0-9]+)+$");
		std::string group = std::regex_replace(name, numbers, "");
		group = std::regex_replace(group, numbers, "");
		return group.empty() ? name : group;
	}

	//a point of the game in the coordinates of the scene of the map (Blender: z up), as Arena::view
	static Square::Vec3 to_scene(const Square::Vec3& game, const Square::Vec3& map_position)
	{
		const Square::Vec3 p = game - map_position;
		return Square::Vec3(p.x, -p.z, p.y);
	}

	//milliseconds since a time
	static double since(const std::chrono::high_resolution_clock::time_point& start)
	{
		return std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count();
	}

	static Square::RenderSystem* render_system(Square::Context& context)
	{
		return Square::System::get<Square::RenderSystem>(context);
	}
}

RushBench::RushBench(Square::Context& context, Square::Scene::World& world, const std::string& report)
: m_context(context)
, m_world(world)
, m_report(report)
, m_static(std::getenv("RUSH_BENCH_STATIC") != nullptr)
{
}

std::string RushBench::collisions(const Race& race) const
{
	using namespace Square;
	using Clock = std::chrono::high_resolution_clock;
	const Arena& arena = race.arena();
	std::ostringstream out;
	out << std::fixed << std::setprecision(3);
	//the collider of the map made again
	CollisionMesh mesh;
	const auto build_start = Clock::now();
	mesh.add(m_context, arena.actor());
	const double build_ms = AuxRushBench::since(build_start);
	out << "collisions: " << mesh.size() << " triangles, built in " << build_ms << " ms\n";
	//moves of a sphere near the ground, rays down (the same ones every run)
	std::mt19937 random(7);
	std::uniform_real_distribution<float> unit(0.0f, 1.0f);
	const Vec3 low = arena.min(), high = arena.max();
	const int moves = 20000;
	//the moves: 1 m over the ground (a ray down, not timed), 3 m across, a little up or down
	std::vector<CollisionMesh::Line> lines;
	lines.reserve(moves);
	while (int(lines.size()) < moves)
	{
		const Vec3 origin(low.x + (high.x - low.x) * unit(random), high.y + 5.0f, low.z + (high.z - low.z) * unit(random));
		CollisionMesh::Hit hit;
		if (mesh.raycast(origin, Vec3(0.0f, -1.0f, 0.0f), high.y - low.y + 10.0f, hit))
		{
			const float angle = 6.2831853f * unit(random);
			CollisionMesh::Line line;
			line.m_origin = hit.m_point + Vec3(0.0f, 1.0f + 1.2f, 0.0f);
			line.m_direction = Vec3(std::cos(angle) * 3.0f, (unit(random) - 0.5f) * 2.0f, std::sin(angle) * 3.0f);
			lines.push_back(line);
		}
	}
	int hits = 0;
	const auto move_start = Clock::now();
	for (const auto& line : lines)
	{
		CollisionMesh::Collision collision;
		hits += mesh.collide(line, 1.2f, collision, 1.0f) ? 1 : 0;
	}
	const double move_ms = AuxRushBench::since(move_start);
	int ground = 0;
	const auto ray_start = Clock::now();
	for (int i = 0; i < moves; ++i)
	{
		const Vec3 origin(low.x + (high.x - low.x) * unit(random), high.y + 5.0f, low.z + (high.z - low.z) * unit(random));
		CollisionMesh::Hit hit;
		ground += mesh.raycast(origin, Vec3(0.0f, -1.0f, 0.0f), high.y - low.y + 10.0f, hit) ? 1 : 0;
	}
	const double ray_ms = AuxRushBench::since(ray_start);
	out << "  " << moves << " sphere moves: " << move_ms << " ms (" << move_ms * 1000.0 / moves << " us each), " << hits << " hits\n";
	out << "  " << moves << " rays down: " << ray_ms << " ms (" << ray_ms * 1000.0 / moves << " us each), " << ground << " hits\n";
	//the raster of the occlusion: the occluders of the map from the camera of the race
	std::vector< Weak<Render::Occluder> > occluders;
	arena.actor()->visit([&occluders](Shared<Scene::Actor> node) -> bool
	{
		if (node->contains<Scene::Occluder>())
		{
			occluders.push_back(StaticPointerCast<Render::Occluder>(node->component<Scene::Occluder>()));
		}
		return true;
	});
	auto camera_actor = arena.camera();
	if (camera_actor && camera_actor->contains<Scene::Camera>() && !occluders.empty())
	{
		auto camera = camera_actor->component<Scene::Camera>();
		out << "occlusion raster (" << occluders.size() << " occluders, " << Render::SoftwareOcclusion::instructions() << ")\n";
		for (int width : { 256, 1024 })
		{
			Render::SoftwareOcclusion occlusion;
			Render::SoftwareOcclusion::Settings settings;
			settings.width = width;
			occlusion.settings(settings);
			const int draws = 200;
			const auto draw_start = Clock::now();
			for (int i = 0; i < draws; ++i)
			{
				occlusion.draw(*camera, occluders);
			}
			const double draw_ms = AuxRushBench::since(draw_start);
			//its depth buffer (the same on every instruction set): the texels covered, their sum
			double sum = 0.0;
			size_t covered = 0;
			for (float texel : occlusion.depth())
			{
				sum += texel;
				covered += texel > 0.0f ? 1 : 0;
			}
			out << "  width " << width << ": " << draw_ms / draws << " ms a draw, " << occlusion.stats().m_triangles << " triangles, "
			    << covered << " texels covered, sum " << std::setprecision(6) << sum << std::setprecision(3) << "\n";
		}
	}
	return out.str();
}

void RushBench::prepare(const Race& race, const Rush::RaceMap& map)
{
	using namespace Square;
	const Arena& arena = race.arena();
	auto root = arena.actor();
	//the points of view: the view of the map, behind two starts looking at the middle
	m_collisions = collisions(race);
	m_context.logger()->info(m_collisions);
	//RUSH_BENCH_COLLISION=only: the collisions alone, no test of the frame
	const char* collision_only = std::getenv("RUSH_BENCH_COLLISION");
	if (collision_only && std::string(collision_only) == "only")
	{
		return;
	}
	m_views.push_back({ "view", map.m_view_from, map.m_view_to, map.m_view_lens });
	const Vec3 center = arena.center();
	for (size_t id : { size_t(0), size_t(2) })
	{
		const Vec3 start = arena.start(id);
		Vec3 back = start - center;
		back.y = 0.0f;
		back = length(back) > 0.01f ? normalize(back) : Vec3(0.0f, 0.0f, 1.0f);
		const Vec3 eye = start + back * 6.0f + Vec3(0.0f, 3.0f, 0.0f);
		const Vec3 at = center + Vec3(0.0f, 2.0f, 0.0f);
		m_views.push_back({ "start " + std::to_string(id), AuxRushBench::to_scene(eye, root->position()), AuxRushBench::to_scene(at, root->position()), 20.0f });
	}
	//the groups: the nodes under the root of the map by their names, their renderables visible
	std::map<std::string, size_t> by_name;
	for (auto& node : root->childs())
	{
		const std::string name = AuxRushBench::group_of(node->name());
		std::vector<Render::Renderable*> renderables;
		node->visit([&renderables](Shared<Scene::Actor> actor) -> bool
		{
			for (auto& component : actor->components())
			{
				auto renderable = dynamic_cast<Render::Renderable*>(component.second.get());
				if (renderable && renderable->visible())
				{
					renderables.push_back(renderable);
				}
			}
			return true;
		});
		if (!renderables.empty())
		{
			auto found = by_name.find(name);
			if (found == by_name.end())
			{
				by_name[name] = m_groups.size();
				m_groups.push_back({ name, {} });
				found = by_name.find(name);
			}
			auto& group = m_groups[found->second].m_renderables;
			group.insert(group.end(), renderables.begin(), renderables.end());
		}
	}
	//the tests: at each point of view, the frame as it is, then each change
	for (size_t view = 0; view < m_views.size(); ++view)
	{
		m_tests.push_back({ view, Change::NONE, 0, "as it is" });
		m_tests.push_back({ view, Change::OCCLUSION_OFF, 0, "occlusion off" });
		if (!m_static)
		{
			m_tests.push_back({ view, Change::STATIC_MAP, 0, "map static" });
		}
		for (size_t group = 0; group < m_groups.size(); ++group)
		{
			m_tests.push_back({ view, Change::HIDE_GROUP, group, "hide " + m_groups[group].m_name });
		}
	}
	//RUSH_BENCH_STATIC: the map static in all the tests (the shadows of the map cached)
	if (m_static)
	{
		root->set_static(true);
	}
	//the profiler on
	if (auto system = AuxRushBench::render_system(m_context))
	{
		if (auto profiler = system->profiler())
		{
			profiler->enable(true);
			profiler->window(AuxRushBench::s_window);
		}
	}
	m_context.logger()->info("bench: " + std::to_string(m_views.size()) + " views, " + std::to_string(m_groups.size()) + " groups, " + std::to_string(m_tests.size()) + " tests");
}

void RushBench::apply(const Race& race, const Test& test, bool on)
{
	using namespace Square;
	switch (test.m_change)
	{
	case Change::OCCLUSION_OFF:
	{
		if (auto instance = m_world.instance<RenderInstance>())
		{
			Render::SoftwareOcclusion::Settings settings = instance->occlusion();
			settings.enabled = !on;
			instance->occlusion(settings);
		}
	}
	break;
	case Change::STATIC_MAP:
	{
		race.arena().actor()->set_static(on);
	}
	break;
	case Change::HIDE_GROUP:
	{
		for (auto renderable : m_groups[test.m_group].m_renderables)
		{
			renderable->visible(!on);
		}
	}
	break;
	default:
	break;
	}
}

RushBench::Result RushBench::measure() const
{
	Result result;
	auto system = AuxRushBench::render_system(m_context);
	auto profiler = system ? system->profiler() : nullptr;
	if (profiler && !profiler->scopes().empty())
	{
		const auto& frame = profiler->scopes().front();
		result.m_gpu_ms = frame.m_gpu_ms;
		result.m_cpu_ms = frame.m_cpu_ms;
		result.m_draws = frame.m_draws;
		for (const auto& scope : profiler->scopes())
		{
			if (scope.m_depth == 1)
			{
				result.m_passes.push_back({ scope.m_name, scope.m_gpu_ms });
			}
		}
	}
	if (auto instance = m_world.instance<Square::RenderInstance>())
	{
		const auto stats = instance->occlusion_stats();
		result.m_hidden = stats.m_hidden;
		result.m_tested = stats.m_tested;
		result.m_instances_hidden = stats.m_instances_hidden;
		result.m_instances_tested = stats.m_instances_tested;
	}
	result.m_frame_ms = m_frame_count ? m_frame_sum / m_frame_count * 1000.0 : 0.0;
	return result;
}

bool RushBench::update(const Race& race, const Rush::RaceMap& map, double delta_time)
{
	if (!m_prepared)
	{
		prepare(race, map);
		m_prepared = true;
	}
	auto system = AuxRushBench::render_system(m_context);
	auto profiler = system ? system->profiler() : nullptr;
	const bool running = !m_done && profiler && m_test < m_tests.size();
	if (running)
	{
		const Test& test = m_tests[m_test];
		const View& view = m_views[test.m_view];
		race.arena().view(view.m_from, view.m_to, view.m_lens);
		if (m_warmup > 0)
		{
			--m_warmup;
			if (m_warmup == 0)
			{
				//the first test: its change, then two windows (the first one has frames before it)
				apply(race, test, true);
				m_wait_version = profiler->version() + 2;
			}
		}
		else if (profiler->version() < m_wait_version)
		{
			//the window measured (the last one before the wait): the time between its frames
			if (profiler->version() + 1 == m_wait_version)
			{
				m_frame_sum += delta_time;
				++m_frame_count;
			}
		}
		else
		{
			m_results.push_back(measure());
			apply(race, test, false);
			m_frame_sum = 0.0;
			m_frame_count = 0;
			++m_test;
			if (m_test < m_tests.size())
			{
				apply(race, m_tests[m_test], true);
				m_wait_version = profiler->version() + 2;
			}
			m_context.logger()->info("bench: " + std::to_string(m_test) + "/" + std::to_string(m_tests.size()));
		}
	}
	if (!m_done && m_prepared && (!profiler || m_test >= m_tests.size()))
	{
		write(map);
		m_done = true;
	}
	return m_done;
}

void RushBench::write(const Rush::RaceMap& map) const
{
	std::ostringstream out;
	out << std::fixed << std::setprecision(2);
	out << "bench of " << map.m_name << ": the frame as it is, then each change (its gain: less ms is faster)\n";
	out << "GPU / CPU: the profiler (the render), frame: the time between the frames\n";
	out << "\n" << m_collisions;
	size_t base = 0;
	for (size_t i = 0; i < m_results.size() && i < m_tests.size(); ++i)
	{
		const Test& test = m_tests[i];
		const Result& result = m_results[i];
		if (test.m_change == Change::NONE)
		{
			base = i;
			out << "\n== " << m_views[test.m_view].m_name
			    << ": GPU " << result.m_gpu_ms << " ms, CPU " << result.m_cpu_ms << " ms, frame " << result.m_frame_ms
			    << " ms, draws " << result.m_draws << ", occlusion hid " << result.m_hidden << "/" << result.m_tested
			    << " (instances " << result.m_instances_hidden << "/" << result.m_instances_tested << ")\n";
			for (const auto& pass : result.m_passes)
			{
				out << "   pass " << std::setw(28) << std::left << pass.first << std::right << " GPU " << pass.second << " ms\n";
			}
			out << "   " << std::setw(34) << std::left << "change" << std::right
			    << std::setw(10) << "dGPU" << std::setw(10) << "dCPU" << std::setw(10) << "dframe" << std::setw(10) << "draws" << "\n";
		}
		else
		{
			const Result& as_is = m_results[base];
			std::string name = test.m_name;
			if (test.m_change == Change::HIDE_GROUP)
			{
				name += " (" + std::to_string(m_groups[test.m_group].m_renderables.size()) + ")";
			}
			out << "   " << std::setw(34) << std::left << name << std::right
			    << std::setw(10) << (result.m_gpu_ms - as_is.m_gpu_ms)
			    << std::setw(10) << (result.m_cpu_ms - as_is.m_cpu_ms)
			    << std::setw(10) << (result.m_frame_ms - as_is.m_frame_ms)
			    << std::setw(10) << result.m_draws << "\n";
		}
	}
	Square::Filesystem::text_file_write_all(m_report, out.str());
	m_context.logger()->info("bench: written " + m_report);
}
