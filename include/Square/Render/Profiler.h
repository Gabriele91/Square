//
//  Profiler.h
//  Square
//
//  The costs of the render, frame by frame (RENDER_PROFILER, a CMake option: SQUARE_RENDER_PROFILER).
//
//  The passes open named scopes (SQUARE_RENDER_SCOPE), nested as they are called: every scope
//  has its CPU time and, by two GPU timestamps (Render::Context::gpu_timer_*), its GPU time,
//  read back some frames later (the GPU is never waited for). The draws of the meshes are
//  counted in the scopes; with the shader detail (shader_detail(true)) every draw also has
//  its own two timestamps and its GPU time is summed to the shader bound (by its name).
//
//  The statistics are averages (and maximums) over a window of frames (window()), published at
//  its end: scopes() in the tree order (the first is "Frame"), shaders() by GPU cost.
//
//  The RenderSystem owns it (RenderSystem::profiler(), and Render::Context::profiler()) only
//  with RENDER_PROFILER: without it the macros are empty and the profiler is nullptr.
//
#pragma once
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <chrono>
#include "Square/Config.h"

namespace Square
{
namespace Render
{
	class Context;

	class SQUARE_API Profiler
	{
	public:
		//the statistic of a scope (or of a shader)
		struct Stat
		{
			std::string m_name;
			int    m_depth{ 0 };       //in the tree (0: Frame), 0 for the shaders
			double m_gpu_ms{ 0 };      //average per frame
			double m_gpu_max_ms{ 0 };  //maximum in a frame
			double m_cpu_ms{ 0 };      //average per frame (the scopes only)
			double m_cpu_max_ms{ 0 };  //maximum in a frame (the scopes only)
			double m_calls{ 0 };       //average per frame: the scope opened, the draws of the shader
			double m_draws{ 0 };       //average draws per frame, its children included (the scopes only)
		};

		Profiler(Context& render);
		~Profiler();

		//on / off (from the next frame): off it costs nothing and its statistics are cleared
		void enable(bool enable);
		bool enabled() const;

		//the draws timed one by one and summed by shader (from the next frame)
		void shader_detail(bool enable);
		bool shader_detail() const;

		//frames of a statistics window (default 60)
		void window(unsigned int frames);
		unsigned int window() const;

		//the GPU times are there (device support)
		bool gpu() const;

		//a frame: by the RenderSystem, around the draws (a "Frame" scope)
		void begin_frame();
		void end_frame();

		//a scope (nested): false when it is not recorded (off or out of a frame), then no end
		bool begin_scope(const char* name);
		void end_scope();

		//the shader bound (Resource::Shader::bind), its draws are its cost (nullptr: unbound)
		void shader(const char* name);

		//a draw (Mesh::draw): false when it is not recorded, then no end
		bool begin_draw();
		void end_draw();

		//the statistics of the last window
		const std::vector<Stat>& scopes() const;
		const std::vector<Stat>& shaders() const;
		//a counter of the windows published (the statistics have changed when it changes)
		unsigned long long version() const;
		//the statistics as text (a table)
		std::string report() const;

	protected:
		using Clock = std::chrono::high_resolution_clock;

		//the scopes, by their parent and their name (a tree)
		struct Node
		{
			std::string           m_name;
			int                   m_parent{ -1 };
			int                   m_depth{ 0 };
			std::vector<int>      m_children;
			//the frame (a scope can be opened more times), the accumulation of the window
			double m_gpu_frame{ 0 }, m_gpu_sum{ 0 }, m_gpu_max{ 0 };
			double m_cpu_frame{ 0 }, m_cpu_sum{ 0 }, m_cpu_max{ 0 };
			double m_calls{ 0 }, m_draws{ 0 };
		};
		struct ShaderNode
		{
			std::string m_name;
			double m_gpu_frame{ 0 }, m_gpu_sum{ 0 }, m_gpu_max{ 0 };
			double m_calls{ 0 };
		};
		//what a frame has recorded, until its GPU timestamps are read
		struct ScopeRecord
		{
			int m_node{ -1 };
			int m_gpu_begin{ -1 };
			int m_gpu_end{ -1 };
		};
		struct DrawRecord
		{
			int m_shader{ -1 };
			int m_gpu_begin{ -1 };
			int m_gpu_end{ -1 };
		};
		struct FrameRecord
		{
			unsigned long long       m_id{ 0 };
			std::vector<ScopeRecord> m_scopes;
			std::vector<DrawRecord>  m_draws;
		};
		//the scope open
		struct OpenScope
		{
			size_t            m_record{ 0 };
			Clock::time_point m_start;
		};

		Context& m_render;
		//settings
		bool         m_enabled{ false };
		bool         m_enable_request{ false };
		bool         m_shader_detail{ false };
		bool         m_shader_detail_request{ false };
		unsigned int m_window{ 60 };
		bool         m_gpu{ false };
		//the frame
		bool                   m_in_frame{ false };
		unsigned long long     m_frame_id{ 0 };
		FrameRecord            m_frame;
		std::vector<OpenScope> m_open;
		int                    m_current_shader{ -1 };
		bool                   m_draw_open{ false };
		//the frames waiting for their timestamps
		std::deque<FrameRecord> m_pending;
		std::vector<double>     m_timestamps;
		//the tree, the shaders
		std::vector<Node>                    m_nodes;
		std::map<std::pair<int, std::string>, int> m_node_map;
		std::vector<ShaderNode>              m_shader_nodes;
		std::map<std::string, int>           m_shader_map;
		//the window
		unsigned int m_cpu_frames{ 0 };
		unsigned int m_gpu_frames{ 0 };
		//published
		std::vector<Stat>  m_scopes;
		std::vector<Stat>  m_shaders;
		unsigned long long m_version{ 0 };

		int  node(int parent, const char* name);
		void read_frames();
		void read_frame(const FrameRecord& frame);
		void publish();
		void clear();
	};

	//a scope as a C++ scope
	class SQUARE_API ProfilerScope
	{
	public:
		ProfilerScope(Profiler* profiler, const char* name)
		: m_profiler(profiler && profiler->begin_scope(name) ? profiler : nullptr) {}
		~ProfilerScope() { if (m_profiler) m_profiler->end_scope(); }
		ProfilerScope(const ProfilerScope&) = delete;
		ProfilerScope& operator=(const ProfilerScope&) = delete;
	private:
		Profiler* m_profiler;
	};

	//a draw as a C++ scope
	class SQUARE_API ProfilerDraw
	{
	public:
		ProfilerDraw(Profiler* profiler)
		: m_profiler(profiler && profiler->begin_draw() ? profiler : nullptr) {}
		~ProfilerDraw() { if (m_profiler) m_profiler->end_draw(); }
		ProfilerDraw(const ProfilerDraw&) = delete;
		ProfilerDraw& operator=(const ProfilerDraw&) = delete;
	private:
		Profiler* m_profiler;
	};
}
}

//the macros of the passes: empty without RENDER_PROFILER (render: a Render::Context&)
#if defined(RENDER_PROFILER)
	#define SQUARE_RENDER_PROFILER_JOIN_(a, b) a##b
	#define SQUARE_RENDER_PROFILER_JOIN(a, b) SQUARE_RENDER_PROFILER_JOIN_(a, b)
	#define SQUARE_RENDER_SCOPE(render, name)\
		::Square::Render::ProfilerScope SQUARE_RENDER_PROFILER_JOIN(sq_render_scope_, __LINE__)((render).profiler(), name)
	#define SQUARE_RENDER_DRAW(render)\
		::Square::Render::ProfilerDraw SQUARE_RENDER_PROFILER_JOIN(sq_render_draw_, __LINE__)((render).profiler())
	#define SQUARE_RENDER_SHADER(render, name)\
		do { if (auto* sq_render_profiler = (render).profiler()) sq_render_profiler->shader(name); } while (0)
#else
	#define SQUARE_RENDER_SCOPE(render, name) ((void)0)
	#define SQUARE_RENDER_DRAW(render) ((void)0)
	#define SQUARE_RENDER_SHADER(render, name) ((void)0)
#endif
