//
//  RushBench.h
//  Rush
//
//  A benchmark of a map (debug tools only): RUSH_BENCH=<report file> with --map <name>. The race
//  paused (nothing moves), the camera at some points of view (the view of the map, behind some
//  starts looking at the middle), the settings of the player. At each point the cost of a frame
//  (the profiler: its GPU and CPU times, its draws; the time between the frames) as it is, then
//  with a change: the occlusion off, the map static (the shadows of the lights cached), each group
//  of the map hidden (the nodes under the root of the map by their name less its numbers and its
//  level of detail: "props_4_5_lod0" in "props"). Each measure after some frames (the caches
//  made again), over a window of the profiler. At the end the report (a table: each change, its
//  gain against the frame as it is) and the game quits.
//
#pragma once
#include <string>
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>

class Race;

class RushBench
{
public:

	RushBench(Square::Context& context, Square::Scene::World& world, const std::string& report);

	//a frame of a race (its loading over, paused): its step; true when it is done (the report
	//written)
	bool update(const Race& race, const Rush::RaceMap& map, double delta_time);

private:

	//a point of view (in the coordinates of the scene of the map, Blender: as the view of a map)
	struct View
	{
		std::string  m_name;
		Square::Vec3 m_from;
		Square::Vec3 m_to;
		float        m_lens{ 22.0f };
	};
	//a group of the map: its renderables
	struct Group
	{
		std::string                                   m_name;
		std::vector< Square::Render::Renderable* >     m_renderables;
	};
	//a change measured: the frame as it is (none), the occlusion off, the map static, a group hidden
	enum class Change
	{
		NONE,
		OCCLUSION_OFF,
		STATIC_MAP,
		HIDE_GROUP
	};
	struct Test
	{
		size_t      m_view{ 0 };
		Change      m_change{ Change::NONE };
		size_t      m_group{ 0 };
		std::string m_name;
	};
	//what a test measured
	struct Result
	{
		double m_gpu_ms{ 0.0 };
		double m_cpu_ms{ 0.0 };
		double m_frame_ms{ 0.0 }; //between the frames
		double m_draws{ 0.0 };
		size_t m_hidden{ 0 };     //the renderables the occlusion hid
		size_t m_tested{ 0 };
		size_t m_instances_hidden{ 0 };
		size_t m_instances_tested{ 0 };
		std::vector< std::pair<std::string, double> > m_passes; //the GPU time of the passes (depth 1)
	};

	//the collider of the map made again, moves and rays against it: their times (a report)
	std::string collisions(const Race& race) const;
	//the points of view, the groups, the tests (at the first frame)
	void prepare(const Race& race, const Rush::RaceMap& map);
	//a change on, off
	void apply(const Race& race, const Test& test, bool on);
	//the measure of the window just published
	Result measure() const;
	//the report written
	void write(const Rush::RaceMap& map) const;

	Square::Context&        m_context;
	Square::Scene::World&   m_world;
	std::string             m_report;
	std::vector<View>       m_views;
	std::vector<Group>      m_groups;
	std::vector<Test>       m_tests;
	std::vector<Result>     m_results;
	std::string             m_collisions;             //the report of the collisions
	size_t                  m_test{ 0 };
	bool                    m_prepared{ false };
	bool                    m_static{ false };        //RUSH_BENCH_STATIC: the map static in all the tests
	int                     m_warmup{ 400 };          //frames before the first test
	unsigned long long      m_wait_version{ 0 };      //the window of the profiler of the measure
	double                  m_frame_sum{ 0.0 };
	int                     m_frame_count{ 0 };
	bool                    m_done{ false };
};
