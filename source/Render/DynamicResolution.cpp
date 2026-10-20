//
//  DynamicResolution.cpp
//  Square
//
//  See DynamicResolution.h.
//
#include "Square/Render/DynamicResolution.h"
#include <algorithm>
#include <cmath>

namespace Square
{
namespace Render
{
	namespace AuxDynamicResolution
	{
		static constexpr float s_late = 1.1f;            //late: the frames over the budget by its tolerance (10%)
		static constexpr float s_gpu_room = 0.85f;        //higher: the GPU under this share of the budget
		static constexpr float s_window = 0.5f;          //seconds of the frames it looks at
		static constexpr float s_up = 0.1f;              //higher on time: a share of the scene a second
		static constexpr float s_glide = 6.0f;           //the share shown going to its target (a share a second)
		static constexpr float s_longest_frame = 0.25f;  //seconds: a longer frame is a loading, not counted
		static constexpr float s_largest_range = 0.9f;   //the scene at least a tenth
	}

	void DynamicResolution::settings(const Settings& settings)
	{
		m_settings = settings;
		if (m_settings.m_range <= 0.0f)
		{
			reset();
			return;
		}
		//(its share within the new range)
		m_share = std::max(m_share, lowest());
		m_shown = std::max(m_shown, lowest());
	}

	const DynamicResolution::Settings& DynamicResolution::settings() const
	{
		return m_settings;
	}

	void DynamicResolution::reset()
	{
		m_share = 1.0f;
		m_shown = 1.0f;
		enter(State::SETTLE);
	}

	Vec2 DynamicResolution::update(float delta_time, float fps, float gpu_ms)
	{
		using namespace AuxDynamicResolution;
		if (m_settings.m_range <= 0.0f) return Vec2(1.0f);
		//(a loading: its frames forgotten, a new window)
		if (delta_time > s_longest_frame)
		{
			enter(State::SETTLE);
		}
		else
		{
			push(delta_time);
			const float budget_ms = 1000.0f / std::max(fps, 1.0f);
			switch (m_state)
			{
			case State::SETTLE:
				//a whole window of frames after a change: its effect seen
				if (m_window >= s_window) m_state = State::WATCH;
			break;
			case State::WATCH:
			{
				const float average_ms = m_window * 1000.0f / float(std::max<size_t>(m_frames.size(), 1));
				if (average_ms > budget_ms * s_late)
				{
					//as much lower as they are late: its time goes with its pixels (one axis: the
					//share, two: its square)
					const bool both = m_settings.m_width && m_settings.m_height;
					const float ratio = budget_ms / average_ms;
					m_share = std::max(m_share * (both ? std::sqrt(ratio) : ratio), lowest());
					enter(State::SETTLE);
				}
				else if (gpu_ms < 0.0f || gpu_ms < budget_ms * s_gpu_room)
				{
					m_share = std::min(m_share + s_up * delta_time, 1.0f);
				}
			}
			break;
			}
		}
		//the share shown gliding to it
		m_shown += (m_share - m_shown) * std::min(s_glide * delta_time, 1.0f);
		return Vec2(m_settings.m_width ? m_shown : 1.0f, m_settings.m_height ? m_shown : 1.0f);
	}

	void DynamicResolution::push(float delta_time)
	{
		m_frames.push_back(delta_time);
		m_window += delta_time;
		//(the older ones out, the newest always in)
		size_t out = 0;
		while (out + 1 < m_frames.size() && m_window - m_frames[out] >= AuxDynamicResolution::s_window)
		{
			m_window -= m_frames[out];
			++out;
		}
		if (out) m_frames.erase(m_frames.begin(), m_frames.begin() + out);
	}

	void DynamicResolution::enter(State state)
	{
		m_state = state;
		//(a new window: the frames of before not counted)
		m_frames.clear();
		m_window = 0.0f;
	}

	float DynamicResolution::lowest() const
	{
		return 1.0f - std::clamp(m_settings.m_range, 0.0f, AuxDynamicResolution::s_largest_range);
	}
}
}
