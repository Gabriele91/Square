//
//  ProfilerPanel.h
//  Square
//
//  The panel of the render profiler (Render/Profiler.h) over the frame, by the UISystem
//  (UISystem::profiler(true)): a document of its own (common/ui/profiler.rml), on the right.
//  It shows the scopes (GPU ms average / maximum, CPU ms, draws, a bar of the GPU cost) and,
//  with its check box "per shader", the most expensive shaders; its button writes the report
//  to the log. Its rows are made once and only their text changes, before the update of
//  RmlUi (the layout of a new element would be missing for a frame).
//
#pragma once
#include <vector>
#include <string>
#include "Square/Config.h"
#include "Square/UI/Context.h"

namespace Square
{
	class Context;
namespace Render
{
	class Profiler;
}
namespace UI
{
	class ProfilerPanel
	{
	public:
		ProfilerPanel(Square::Context& context, UI::Context& ui);
		~ProfilerPanel();

		//shown: the profiler on; hidden: off
		void show(bool visible);
		bool visible() const;

		//its rows from the statistics of the profiler (when they change)
		void update();

	private:
		struct Row
		{
			Element m_row;
			Element m_name;
			Element m_gpu;
			Element m_cpu;
			Element m_draws;
			Element m_fill;
		};

		Square::Context&   m_context;
		UI::Context&       m_ui;
		Document           m_document;
		Element            m_rows;
		Element            m_note;
		std::vector<Row>   m_pool;
		bool               m_visible{ false };
		unsigned long long m_version{ ~0ull };

		Render::Profiler* profiler() const;
		bool create();
		Row& row(size_t index);
		void set_row(size_t index, const std::string& name, int depth, const std::string& gpu, const std::string& cpu, const std::string& draws, double fraction, bool head);
	};
}
}
