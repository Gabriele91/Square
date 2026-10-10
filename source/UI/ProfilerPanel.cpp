//
//  ProfilerPanel.cpp
//  Square
//
//  See ProfilerPanel.h for the high level description.
//
#include <cstdio>
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Core/Logger.h"
#include "Square/System/RenderSystem.h"
#include "Square/Render/Profiler.h"
#include "ProfilerPanel.h"

namespace Square
{
namespace UI
{
	//shaders on the panel, at most
	static constexpr size_t s_max_shaders = 12;

	ProfilerPanel::ProfilerPanel(Square::Context& context)
	: m_context(context)
	{
	}

	ProfilerPanel::~ProfilerPanel()
	{
	}

	std::string ProfilerPanel::rml()
	{
		return
			"<div class=\"controls\">"
				"<input type=\"checkbox\" id=\"profile\"/>profile "
				"<input type=\"checkbox\" id=\"shaders\"/>cost per shader"
				"<button id=\"log\">report to log</button>"
			"</div>"
			"<div><span id=\"note\"></span></div>"
			"<div id=\"rows\"></div>";
	}

	Render::Profiler* ProfilerPanel::profiler() const
	{
		auto* render_system = System::get<RenderSystem>(m_context);
		return render_system ? render_system->profiler() : nullptr;
	}

	void ProfilerPanel::attach(Document& document)
	{
		//its elements (the old ones gone with the panel made again)
		m_pool.clear();
		m_profile = document.find("profile");
		m_shaders = document.find("shaders");
		m_rows = document.find("rows");
		m_note = document.find("note");
		m_attached = m_profile.valid() && m_shaders.valid() && m_rows.valid() && m_note.valid();
		m_version = ~0ull;
		if (m_attached)
		{
			Element profile = m_profile;
			m_profile.on(EventType::CHANGE, [this, profile](Event&)
			{
				if (!m_showing)
				{
					enable(profile.has_attribute("checked"));
				}
			});
			Element shaders = m_shaders;
			m_shaders.on(EventType::CHANGE, [this, shaders](Event&)
			{
				auto* render_profiler = profiler();
				if (!m_showing && render_profiler)
				{
					render_profiler->shader_detail(shaders.has_attribute("checked"));
				}
			});
			//the report in the log
			document.find("log").on(EventType::CLICK, [this](Event&)
			{
				if (auto* render_profiler = profiler()) m_context.logger()->info("Render profiler\n" + render_profiler->report());
			});
			if (!profiler())
			{
				m_note.set_text("(no render profiler: the engine is built without RENDER_PROFILER)");
			}
			show_state();
		}
	}

	void ProfilerPanel::detach()
	{
		m_attached = false;
		m_pool.clear();
		m_profile = Element();
		m_shaders = Element();
		m_rows = Element();
		m_note = Element();
	}

	void ProfilerPanel::enable(bool enable)
	{
		Render::Profiler* render_profiler = profiler();
		m_requested = enable && render_profiler;
		if (render_profiler)
		{
			render_profiler->enable(m_requested);
			m_version = ~0ull;
		}
		if (m_attached)
		{
			show_state();
			if (!enable)
			{
				for (Row& row : m_pool) row.m_row.set_property("display", "none");
			}
		}
	}

	bool ProfilerPanel::enabled() const
	{
		return m_requested;
	}

	void ProfilerPanel::show_state()
	{
		//(their change events: from here, not from the user)
		m_showing = true;
		Render::Profiler* render_profiler = profiler();
		if (m_requested) m_profile.set_attribute("checked", "");
		else m_profile.remove_attribute("checked");
		if (render_profiler && render_profiler->shader_detail()) m_shaders.set_attribute("checked", "");
		else m_shaders.remove_attribute("checked");
		m_showing = false;
	}

	ProfilerPanel::Row& ProfilerPanel::row(size_t index)
	{
		while (m_pool.size() <= index)
		{
			Row row;
			row.m_row = m_rows.create_child("div");
			row.m_row.set_class("row");
			row.m_name = row.m_row.create_child("span");
			row.m_name.set_class("name");
			row.m_gpu = row.m_row.create_child("span");
			row.m_gpu.set_class("gpu");
			row.m_cpu = row.m_row.create_child("span");
			row.m_cpu.set_class("cpu");
			row.m_draws = row.m_row.create_child("span");
			row.m_draws.set_class("draws");
			Element bar = row.m_row.create_child("span");
			bar.set_class("bar");
			row.m_fill = bar.create_child("span");
			row.m_fill.set_class("fill");
			m_pool.push_back(row);
		}
		return m_pool[index];
	}

	void ProfilerPanel::set_row(size_t index, const std::string& name, int depth, const std::string& gpu, const std::string& cpu, const std::string& draws, double fraction, bool head)
	{
		Row& entry = row(index);
		entry.m_row.set_property("display", "block");
		entry.m_row.set_class("head", head);
		entry.m_name.set_text(name);
		//the indentation inside the column: the others stay aligned
		entry.m_name.set_property("padding-left", std::to_string(depth * 12) + "dp");
		entry.m_name.set_property("width", std::to_string(230 - depth * 12) + "dp");
		entry.m_gpu.set_text(gpu);
		entry.m_cpu.set_text(cpu);
		entry.m_draws.set_text(draws);
		entry.m_fill.set_property("width", std::to_string(int(std::clamp(fraction, 0.0, 1.0) * 100.0)) + "%");
	}

	void ProfilerPanel::update()
	{
		Render::Profiler* render_profiler = profiler();
		if (!m_attached || !render_profiler || !render_profiler->enabled() || render_profiler->version() == m_version) return;
		m_version = render_profiler->version();
		//the note
		m_note.set_text(render_profiler->gpu()
					   ? "(average / maximum of " + std::to_string(render_profiler->window()) + " frames)"
					   : "(no GPU timer: CPU only)");
		auto ms = [](double value, double max) -> std::string
		{
			char text[64];
			std::snprintf(text, sizeof(text), "%.2f / %.2f", value, max);
			return text;
		};
		auto number = [](double value, int decimals) -> std::string
		{
			char text[32];
			std::snprintf(text, sizeof(text), "%.*f", decimals, value);
			return text;
		};
		//the scopes
		size_t index = 0;
		const auto& scopes = render_profiler->scopes();
		const bool gpu = render_profiler->gpu();
		const double frame_gpu = scopes.empty() ? 1.0 : std::max(scopes.front().m_gpu_ms, 1e-6);
		const double frame_cpu = scopes.empty() ? 1.0 : std::max(scopes.front().m_cpu_ms, 1e-6);
		set_row(index++, "scope", 0, "GPU ms / max", "CPU ms", "draws", 0.0, true);
		for (const auto& stat : scopes)
		{
			set_row(index++, stat.m_name, stat.m_depth
				   , gpu ? ms(stat.m_gpu_ms, stat.m_gpu_max_ms) : "-"
				   , number(stat.m_cpu_ms, 2)
				   , number(stat.m_draws, 0)
				   , gpu ? stat.m_gpu_ms / frame_gpu : stat.m_cpu_ms / frame_cpu
				   , false);
		}
		//the most expensive shaders
		const auto& shaders = render_profiler->shaders();
		if (!shaders.empty())
		{
			set_row(index++, "shader", 0, "GPU ms / max", "", "draws", 0.0, true);
			for (size_t i = 0; i < shaders.size() && i < s_max_shaders; ++i)
			{
				const auto& stat = shaders[i];
				set_row(index++, stat.m_name, 0, ms(stat.m_gpu_ms, stat.m_gpu_max_ms), "", number(stat.m_calls, 0), stat.m_gpu_ms / frame_gpu, false);
			}
		}
		//the rows left
		for (; index < m_pool.size(); ++index) m_pool[index].m_row.set_property("display", "none");
	}
}
}
