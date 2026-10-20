//
//  DynamicResolution.cpp
//  Square
//
//  See DynamicResolution.h.
//
#include "Square/Render/DynamicResolution.h"
#include <algorithm>

namespace Square
{
namespace Render
{
	namespace AuxDynamicResolution
	{
		static constexpr float s_late = 1.08f;           //late: the frames over the budget by this share
		static constexpr float s_gpu_late = 0.85f;       //late too: the GPU over this share of the budget
		static constexpr float s_gpu_room = 0.7f;        //a step higher: the GPU under this share of the budget
		static constexpr float s_average = 8.0f;         //the average of the frames: its speed (a share a second)
		static constexpr float s_down = 0.15f;           //a step lower (of the level)
		static constexpr float s_up = 0.05f;             //a step higher
		static constexpr float s_cooldown = 0.5f;        //seconds after a change, before it looks again
		static constexpr float s_probe = 1.5f;           //seconds of a try
		static constexpr float s_first_wait = 2.0f;      //seconds on time before a try
		static constexpr float s_longest_wait = 30.0f;
		static constexpr float s_glide = 6.0f;           //the level shown going to its level (a share a second)
		static constexpr float s_longest_frame = 0.25f;  //seconds: a longer frame is a loading, not counted
	}

	void DynamicResolution::settings(const Settings& settings)
	{
		m_settings = settings;
		if (m_settings.m_range <= 0.0f) reset();
	}

	const DynamicResolution::Settings& DynamicResolution::settings() const
	{
		return m_settings;
	}

	void DynamicResolution::reset()
	{
		m_level = 0.0f;
		m_shown = 0.0f;
		m_wait = AuxDynamicResolution::s_first_wait;
		enter(State::COOLDOWN);
	}

	Vec2 DynamicResolution::update(float delta_time, float fps, float gpu_ms)
	{
		using namespace AuxDynamicResolution;
		if (m_settings.m_range <= 0.0f) return Vec2(1.0f);
		//(a loading: not a frame to count)
		if (delta_time > s_longest_frame)
		{
			enter(State::COOLDOWN);
		}
		else
		{
			const float frame_ms = delta_time * 1000.0f;
			m_frame_ms = m_frame_ms > 0.0f ? m_frame_ms + (frame_ms - m_frame_ms) * std::min(s_average * delta_time, 1.0f) : frame_ms;
			//their GPU time, an average too (unknown: none)
			if (gpu_ms < 0.0f)
			{
				m_gpu_ms = -1.0f;
			}
			else if (m_gpu_ms < 0.0f)
			{
				m_gpu_ms = gpu_ms;
			}
			else
			{
				m_gpu_ms += (gpu_ms - m_gpu_ms) * std::min(s_average * delta_time, 1.0f);
			}
			m_time += delta_time;
			switch (m_state)
			{
			case State::STEADY:
				//late: lower at once; on time long enough: a step higher, on trial
				if (late(fps))
				{
					m_level = std::min(m_level + s_down, 1.0f);
					enter(State::COOLDOWN);
				}
				else if (m_level > 0.0f && m_time >= m_wait && room(fps))
				{
					m_level = std::max(m_level - s_up, 0.0f);
					enter(State::PROBE);
				}
			break;
			case State::PROBE:
				//late again: back, a longer wait; on time: kept, the first wait again
				if (late(fps))
				{
					m_level = std::min(m_level + s_up, 1.0f);
					m_wait = std::min(m_wait * 2.0f, s_longest_wait);
					enter(State::COOLDOWN);
				}
				else if (m_time >= s_probe)
				{
					m_wait = s_first_wait;
					enter(State::STEADY);
				}
			break;
			case State::COOLDOWN:
				if (m_time >= s_cooldown) enter(State::STEADY);
			break;
			}
			//the level shown gliding to it
			m_shown += (m_level - m_shown) * std::min(s_glide * delta_time, 1.0f);
		}
		const float share = 1.0f - m_settings.m_range * m_shown;
		return Vec2(m_settings.m_width ? share : 1.0f, m_settings.m_height ? share : 1.0f);
	}

	bool DynamicResolution::late(float fps) const
	{
		using namespace AuxDynamicResolution;
		const float budget_ms = 1000.0f / std::max(fps, 1.0f);
		if (m_frame_ms > budget_ms * s_late)
		{
			return true;
		}
		//(on the vsync: the frames as long as the budget, their GPU says how near it they are)
		if (m_gpu_ms >= 0.0f)
		{
			return m_gpu_ms > budget_ms * s_gpu_late;
		}
		return false;
	}

	bool DynamicResolution::room(float fps) const
	{
		const float budget_ms = 1000.0f / std::max(fps, 1.0f);
		if (m_gpu_ms < 0.0f)
		{
			return true;
		}
		return m_gpu_ms <= budget_ms * AuxDynamicResolution::s_gpu_room;
	}

	void DynamicResolution::enter(State state)
	{
		m_state = state;
		m_time = 0.0f;
		//(the averages again from the next frame: the change seen)
		if (state == State::COOLDOWN)
		{
			m_frame_ms = 0.0f;
			m_gpu_ms = -1.0f;
		}
	}
}
}
