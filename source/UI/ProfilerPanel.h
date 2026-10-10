//
//  ProfilerPanel.h
//  Square
//
//  The render profiler (Render/Profiler.h) in the tab Profiler of the debug panel (DebugPanel):
//  its check box "profile" turns the profiler on, it shows the scopes (GPU ms average / maximum,
//  CPU ms, draws, a bar of the GPU cost) and, with its check box "per shader", the most expensive
//  shaders; its button writes the report to the log. Its rows are made once (each time the panel
//  is made) and only their text changes, before the update of RmlUi (the layout of a new element
//  would be missing for a frame).
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
		ProfilerPanel(Square::Context& context);
		~ProfilerPanel();

		//the markup of its tab (the elements it finds by their ids)
		static std::string rml();

		//its elements in a document (made again with it), none
		void attach(Document& document);
		void detach();

		//the profiler on (its rows filled) or off
		void enable(bool enable);
		bool enabled() const;

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
		Element            m_profile;
		Element            m_shaders;
		Element            m_rows;
		Element            m_note;
		std::vector<Row>   m_pool;
		bool               m_attached{ false };
		bool               m_requested{ false }; //the profiler asked on (it turns on at the next frame)
		bool               m_showing{ false };   //the check boxes changed by it (their events ignored)
		unsigned long long m_version{ ~0ull };

		Render::Profiler* profiler() const;
		Row& row(size_t index);
		void set_row(size_t index, const std::string& name, int depth, const std::string& gpu, const std::string& cpu, const std::string& draws, double fraction, bool head);
		//the check boxes as the profiler is
		void show_state();
	};
}
}
