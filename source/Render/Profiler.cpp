//
//  Profiler.cpp
//  Square
//
//  See Profiler.h for the high level description.
//
#include <cstdio>
#include <algorithm>
#include "Square/Driver/Render.h"
#include "Square/Render/Profiler.h"

namespace Square
{
namespace Render
{
	//frames waiting for their timestamps, at most (the older ones are dropped)
	static constexpr size_t s_max_pending_frames = 16;

	Profiler::Profiler(Context& render)
	: m_render(render)
	{
	}

	Profiler::~Profiler()
	{
	}

	void Profiler::enable(bool enable)
	{
		m_enable_request = enable;
	}

	bool Profiler::enabled() const
	{
		return m_enable_request;
	}

	void Profiler::shader_detail(bool enable)
	{
		m_shader_detail_request = enable;
	}

	bool Profiler::shader_detail() const
	{
		return m_shader_detail_request;
	}

	void Profiler::window(unsigned int frames)
	{
		m_window = std::max(frames, 1u);
	}

	unsigned int Profiler::window() const
	{
		return m_window;
	}

	bool Profiler::gpu() const
	{
		return m_render.gpu_timer_supported();
	}

	////////////////////////////////////////////////////////////////////
	// Frame
	void Profiler::begin_frame()
	{
		//the settings, between two frames
		if (m_enable_request != m_enabled)
		{
			m_enabled = m_enable_request;
			m_gpu = m_enabled && m_render.gpu_timer_supported();
			clear();
		}
		if (m_shader_detail_request != m_shader_detail)
		{
			m_shader_detail = m_shader_detail_request;
			m_shader_nodes.clear();
			m_shader_map.clear();
			m_shaders.clear();
		}
		if (!m_enabled) return;
		//a new frame (its id: the count of the frames of the driver)
		m_frame = FrameRecord();
		m_frame.m_id = m_frame_id;
		if (m_gpu)
		{
			m_render.gpu_timer_begin_frame();
			++m_frame_id;
		}
		m_in_frame = true;
		m_current_shader = -1;
		m_draw_open = false;
		begin_scope("Frame");
	}

	void Profiler::end_frame()
	{
		if (!m_in_frame) return;
		//the scopes left open (Frame at least)
		if (m_draw_open) end_draw();
		while (!m_open.empty()) end_scope();
		m_in_frame = false;
		//the CPU times of the frame
		for (Node& node : m_nodes)
		{
			node.m_cpu_sum += node.m_cpu_frame;
			node.m_cpu_max = std::max(node.m_cpu_max, node.m_cpu_frame);
			node.m_cpu_frame = 0;
		}
		++m_cpu_frames;
		//the GPU times: some frames later
		if (m_gpu)
		{
			m_render.gpu_timer_end_frame();
			m_pending.push_back(std::move(m_frame));
			while (m_pending.size() > s_max_pending_frames) m_pending.pop_front();
			read_frames();
		}
		m_frame = FrameRecord();
		//the end of the window
		if (m_cpu_frames >= m_window) publish();
	}

	////////////////////////////////////////////////////////////////////
	// Scopes
	bool Profiler::begin_scope(const char* name)
	{
		if (!m_in_frame || !name) return false;
		const int parent = m_open.empty() ? -1 : m_frame.m_scopes[m_open.back().m_record].m_node;
		ScopeRecord record;
		record.m_node = node(parent, name);
		record.m_gpu_begin = m_gpu ? m_render.gpu_timer_timestamp() : -1;
		m_nodes[record.m_node].m_calls += 1;
		m_frame.m_scopes.push_back(record);
		m_open.push_back({ m_frame.m_scopes.size() - 1, Clock::now() });
		return true;
	}

	void Profiler::end_scope()
	{
		if (m_open.empty()) return;
		const OpenScope open = m_open.back();
		m_open.pop_back();
		ScopeRecord& record = m_frame.m_scopes[open.m_record];
		record.m_gpu_end = m_gpu ? m_render.gpu_timer_timestamp() : -1;
		m_nodes[record.m_node].m_cpu_frame += std::chrono::duration<double, std::milli>(Clock::now() - open.m_start).count();
	}

	int Profiler::node(int parent, const char* name)
	{
		auto key = std::make_pair(parent, std::string(name));
		auto it = m_node_map.find(key);
		if (it != m_node_map.end()) return it->second;
		Node node;
		node.m_name = key.second;
		node.m_parent = parent;
		node.m_depth = parent >= 0 ? m_nodes[parent].m_depth + 1 : 0;
		const int id = int(m_nodes.size());
		m_nodes.push_back(std::move(node));
		if (parent >= 0) m_nodes[parent].m_children.push_back(id);
		m_node_map.emplace(std::move(key), id);
		return id;
	}

	////////////////////////////////////////////////////////////////////
	// Draws
	void Profiler::shader(const char* name)
	{
		if (!m_in_frame || !m_shader_detail) return;
		//unbound: the draws that follow are of no shader
		if (!name)
		{
			m_current_shader = -1;
			return;
		}
		std::string key(name);
		auto it = m_shader_map.find(key);
		if (it != m_shader_map.end())
		{
			m_current_shader = it->second;
			return;
		}
		ShaderNode shader;
		shader.m_name = key;
		m_current_shader = int(m_shader_nodes.size());
		m_shader_nodes.push_back(std::move(shader));
		m_shader_map.emplace(std::move(key), m_current_shader);
	}

	bool Profiler::begin_draw()
	{
		if (!m_in_frame) return false;
		//counted in all the open scopes
		for (const OpenScope& open : m_open) m_nodes[m_frame.m_scopes[open.m_record].m_node].m_draws += 1;
		//timed by shader
		if (!m_shader_detail || !m_gpu || m_current_shader < 0 || m_draw_open) return false;
		DrawRecord record;
		record.m_shader = m_current_shader;
		record.m_gpu_begin = m_render.gpu_timer_timestamp();
		m_frame.m_draws.push_back(record);
		m_draw_open = true;
		return true;
	}

	void Profiler::end_draw()
	{
		if (!m_draw_open) return;
		m_frame.m_draws.back().m_gpu_end = m_render.gpu_timer_timestamp();
		m_draw_open = false;
	}

	////////////////////////////////////////////////////////////////////
	// GPU
	void Profiler::read_frames()
	{
		unsigned long long id = 0;
		while (m_render.gpu_timer_read_frame(id, m_timestamps))
		{
			//the frames before it are lost (dropped by the driver)
			while (!m_pending.empty() && m_pending.front().m_id < id) m_pending.pop_front();
			if (!m_pending.empty() && m_pending.front().m_id == id)
			{
				read_frame(m_pending.front());
				m_pending.pop_front();
			}
		}
	}

	void Profiler::read_frame(const FrameRecord& frame)
	{
		auto elapsed = [&](int begin, int end, double& ms) -> bool
		{
			if (begin < 0 || end < 0 || size_t(begin) >= m_timestamps.size() || size_t(end) >= m_timestamps.size()) return false;
			//not sampled by the driver: negative
			if (m_timestamps[begin] < 0.0 || m_timestamps[end] < 0.0) return false;
			ms = std::max(m_timestamps[end] - m_timestamps[begin], 0.0);
			return true;
		};
		double ms = 0;
		for (const ScopeRecord& record : frame.m_scopes)
		{
			if (elapsed(record.m_gpu_begin, record.m_gpu_end, ms)) m_nodes[record.m_node].m_gpu_frame += ms;
		}
		for (const DrawRecord& record : frame.m_draws)
		{
			if (record.m_shader < 0 || size_t(record.m_shader) >= m_shader_nodes.size()) continue;
			if (!elapsed(record.m_gpu_begin, record.m_gpu_end, ms)) continue;
			m_shader_nodes[record.m_shader].m_gpu_frame += ms;
			m_shader_nodes[record.m_shader].m_calls += 1;
		}
		for (Node& node : m_nodes)
		{
			node.m_gpu_sum += node.m_gpu_frame;
			node.m_gpu_max = std::max(node.m_gpu_max, node.m_gpu_frame);
			node.m_gpu_frame = 0;
		}
		for (ShaderNode& shader : m_shader_nodes)
		{
			shader.m_gpu_sum += shader.m_gpu_frame;
			shader.m_gpu_max = std::max(shader.m_gpu_max, shader.m_gpu_frame);
			shader.m_gpu_frame = 0;
		}
		++m_gpu_frames;
	}

	////////////////////////////////////////////////////////////////////
	// Statistics
	void Profiler::publish()
	{
		const double cpu_frames = double(std::max(m_cpu_frames, 1u));
		const double gpu_frames = double(std::max(m_gpu_frames, 1u));
		//the scopes of the window, in the tree order
		m_scopes.clear();
		std::vector<int> stack;
		for (int id = int(m_nodes.size()) - 1; id >= 0; --id)
		{
			if (m_nodes[id].m_parent < 0) stack.push_back(id);
		}
		while (!stack.empty())
		{
			const Node& node = m_nodes[stack.back()];
			stack.pop_back();
			if (node.m_calls <= 0) continue;
			Stat stat;
			stat.m_name = node.m_name;
			stat.m_depth = node.m_depth;
			stat.m_gpu_ms = node.m_gpu_sum / gpu_frames;
			stat.m_gpu_max_ms = node.m_gpu_max;
			stat.m_cpu_ms = node.m_cpu_sum / cpu_frames;
			stat.m_cpu_max_ms = node.m_cpu_max;
			stat.m_calls = node.m_calls / cpu_frames;
			stat.m_draws = node.m_draws / cpu_frames;
			m_scopes.push_back(std::move(stat));
			for (auto child = node.m_children.rbegin(); child != node.m_children.rend(); ++child) stack.push_back(*child);
		}
		//the shaders, by GPU cost
		m_shaders.clear();
		for (const ShaderNode& shader : m_shader_nodes)
		{
			if (shader.m_calls <= 0) continue;
			Stat stat;
			stat.m_name = shader.m_name;
			stat.m_gpu_ms = shader.m_gpu_sum / gpu_frames;
			stat.m_gpu_max_ms = shader.m_gpu_max;
			stat.m_calls = shader.m_calls / gpu_frames;
			m_shaders.push_back(std::move(stat));
		}
		std::stable_sort(m_shaders.begin(), m_shaders.end(), [](const Stat& left, const Stat& right) { return left.m_gpu_ms > right.m_gpu_ms; });
		//a new window
		for (Node& node : m_nodes)
		{
			node.m_gpu_sum = node.m_gpu_max = 0;
			node.m_cpu_sum = node.m_cpu_max = 0;
			node.m_calls = node.m_draws = 0;
		}
		for (ShaderNode& shader : m_shader_nodes)
		{
			shader.m_gpu_sum = shader.m_gpu_max = 0;
			shader.m_calls = 0;
		}
		m_cpu_frames = 0;
		m_gpu_frames = 0;
		++m_version;
	}

	void Profiler::clear()
	{
		m_in_frame = false;
		m_frame = FrameRecord();
		m_open.clear();
		m_current_shader = -1;
		m_draw_open = false;
		m_pending.clear();
		m_nodes.clear();
		m_node_map.clear();
		m_shader_nodes.clear();
		m_shader_map.clear();
		m_cpu_frames = 0;
		m_gpu_frames = 0;
		m_scopes.clear();
		m_shaders.clear();
		++m_version;
	}

	const std::vector<Profiler::Stat>& Profiler::scopes() const
	{
		return m_scopes;
	}

	const std::vector<Profiler::Stat>& Profiler::shaders() const
	{
		return m_shaders;
	}

	unsigned long long Profiler::version() const
	{
		return m_version;
	}

	std::string Profiler::report() const
	{
		std::string text;
		char line[256];
		std::snprintf(line, sizeof(line), "%-36s %16s %16s %7s %7s\n", "scope", "GPU ms (max)", "CPU ms (max)", "calls", "draws");
		text += line;
		for (const Stat& stat : m_scopes)
		{
			const std::string name = std::string(size_t(stat.m_depth) * 2, ' ') + stat.m_name;
			std::snprintf(line, sizeof(line), "%-36.36s %7.3f (%6.3f) %7.3f (%6.3f) %7.1f %7.1f\n"
						 , name.c_str(), stat.m_gpu_ms, stat.m_gpu_max_ms, stat.m_cpu_ms, stat.m_cpu_max_ms, stat.m_calls, stat.m_draws);
			text += line;
		}
		if (!m_shaders.empty())
		{
			std::snprintf(line, sizeof(line), "\n%-36s %16s %7s\n", "shader", "GPU ms (max)", "draws");
			text += line;
			for (const Stat& stat : m_shaders)
			{
				std::snprintf(line, sizeof(line), "%-36.36s %7.3f (%6.3f) %7.1f\n", stat.m_name.c_str(), stat.m_gpu_ms, stat.m_gpu_max_ms, stat.m_calls);
				text += line;
			}
		}
		return text;
	}
}
}
