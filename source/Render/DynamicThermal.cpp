//
//  DynamicThermal.cpp
//  Square
//
//  See DynamicThermal.h.
//
#include "Square/Render/DynamicThermal.h"
#include <algorithm>
#include <iterator>

namespace Square
{
namespace Render
{
	namespace AuxDynamicThermal
	{
		static constexpr float s_period = 1.0f;  //seconds between two reads of the heat (the system: at most once a second)
		static constexpr float s_up = 0.5f;      //the level going up (a share a second: 2 seconds the whole range)
		static constexpr float s_down = 0.02f;   //going down (50 seconds the whole range)
		static constexpr float s_glide = 2.0f;   //the level shown going to its level (a share a second)

		//the level of a status (a device without its headroom), by Video::ThermalStatus
		static constexpr float s_status_level[] =
		{
			0.0f, //UNKNOWN
			0.0f, //NONE
			0.25f,//LIGHT
			0.6f, //MODERATE
			1.0f, //SEVERE
			1.0f, //CRITICAL
			1.0f, //EMERGENCY
			1.0f  //SHUTDOWN
		};
	}

	void DynamicThermal::settings(const Settings& settings)
	{
		m_settings = settings;
		if (m_settings.m_range <= 0.0f)
		{
			reset();
		}
	}

	const DynamicThermal::Settings& DynamicThermal::settings() const
	{
		return m_settings;
	}

	float DynamicThermal::level() const
	{
		return m_level;
	}

	const Video::ThermalState& DynamicThermal::state() const
	{
		return m_state;
	}

	void DynamicThermal::reset()
	{
		m_target = 0.0f;
		m_level = 0.0f;
		m_shown = 0.0f;
		//(the heat read at the next frame)
		m_time = AuxDynamicThermal::s_period;
	}

	Vec2 DynamicThermal::update(float delta_time, Video::DeviceResources* device)
	{
		using namespace AuxDynamicThermal;
		if (m_settings.m_range <= 0.0f || !device)
		{
			return Vec2(1.0f);
		}
		//the heat, once a period
		m_time += delta_time;
		if (m_time >= s_period)
		{
			m_time = 0.0f;
			Video::ThermalState state;
			if (device->get_thermal(m_settings.m_forecast, state))
			{
				m_state = state;
				m_target = level_of(state);
			}
		}
		//its level up quickly, down slowly; the level shown gliding to it
		if (m_target > m_level)
		{
			m_level = std::min(m_level + s_up * delta_time, m_target);
		}
		else
		{
			m_level = std::max(m_level - s_down * delta_time, m_target);
		}
		m_shown += (m_level - m_shown) * std::min(s_glide * delta_time, 1.0f);
		//the share of the axes it lowers
		const float share = 1.0f - m_settings.m_range * m_shown;
		Vec2 scale(1.0f);
		if (m_settings.m_width)
		{
			scale.x = share;
		}
		if (m_settings.m_height)
		{
			scale.y = share;
		}
		return scale;
	}

	float DynamicThermal::level_of(const Video::ThermalState& state) const
	{
		using namespace AuxDynamicThermal;
		//its headroom between from and to
		if (state.m_headroom >= 0.0f)
		{
			const float span = std::max(m_settings.m_to - m_settings.m_from, 0.01f);
			return std::clamp((state.m_headroom - m_settings.m_from) / span, 0.0f, 1.0f);
		}
		//else its status
		const size_t status = size_t(state.m_status);
		if (status >= std::size(s_status_level))
		{
			return 0.0f;
		}
		return s_status_level[status];
	}
}
}
