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
	//the document of the panel (its style: profiler.rcss, next to it)
	static const char* s_profiler_rml = "common/ui/profiler.rml";

	//shaders on the panel, at most
	static constexpr size_t s_max_shaders = 12;

	ProfilerPanel::ProfilerPanel(Square::Context& context, UI::Context& ui)
	: m_context(context)
	, m_ui(ui)
	{
	}

	ProfilerPanel::~ProfilerPanel()
	{
		if (m_document.valid()) m_document.close();
	}

	Render::Profiler* ProfilerPanel::profiler() const
	{
		auto* render_system = System::get<RenderSystem>(m_context);
		return render_system ? render_system->profiler() : nullptr;
	}

	bool ProfilerPanel::create()
	{
		if (m_document.valid()) return true;
		if (!m_ui.valid()) return false;
		m_document = m_ui.load(s_profiler_rml);
		if (!m_document.valid())
		{
			m_context.logger()->warning("UI: unable to load the profiler panel " + std::string(s_profiler_rml));
			return false;
		}
		m_rows = m_document.find("rows");
		m_note = m_document.find("note");
		//per shader
		Element shaders = m_document.find("shaders");
		shaders.on(EventType::CHANGE, [this, shaders](Event&)
		{
			if (auto* render_profiler = profiler()) render_profiler->shader_detail(shaders.has_attribute("checked"));
		});
		//the report in the log
		m_document.find("log").on(EventType::CLICK, [this](Event&)
		{
			if (auto* render_profiler = profiler()) m_context.logger()->info("Render profiler\n" + render_profiler->report());
		});
		return true;
	}

	void ProfilerPanel::show(bool visible)
	{
		Render::Profiler* render_profiler = profiler();
		if (visible && (!render_profiler || !create())) return;
		m_visible = visible;
		if (render_profiler) render_profiler->enable(visible);
		if (!m_document.valid()) return;
		if (visible)
		{
			m_version = ~0ull;
			Element shaders = m_document.find("shaders");
			if (render_profiler && render_profiler->shader_detail()) shaders.set_attribute("checked", "");
			else shaders.remove_attribute("checked");
			m_document.show();
		}
		else
		{
			m_document.hide();
		}
	}

	bool ProfilerPanel::visible() const
	{
		return m_visible;
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
		if (!m_visible || !m_document.valid()) return;
		Render::Profiler* render_profiler = profiler();
		if (!render_profiler || render_profiler->version() == m_version) return;
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
