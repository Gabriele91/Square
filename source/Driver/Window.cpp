//
//  Square
//
//  The device resources of a window: what a platform may not have (its swap at a frame rate)
//
#include "Square/Driver/Window.h"
#include <cmath>

namespace Square
{
namespace Video
{
	bool DeviceResources::set_frame_rate(unsigned int fps)
	{
		return false;
	}

	unsigned int DeviceResources::get_refresh()
	{
		return 0;
	}

	bool DeviceResources::get_thermal(int forecast, ThermalState& state)
	{
		return false;
	}

	unsigned int DeviceResources::swap_interval(unsigned int refresh, unsigned int fps)
	{
		//at most 4 vsyncs (DXGI), a refresh of 59 Hz as 60
		static constexpr unsigned int s_max_interval = 4;
		static constexpr double       s_tolerance = 1.0;
		if (!refresh || !fps)
		{
			return 0;
		}
		if (fps >= refresh)
		{
			return 1;
		}
		const unsigned int interval = (unsigned int)std::lround(double(refresh) / double(fps));
		if (interval > s_max_interval)
		{
			return 0;
		}
		const double rate = double(refresh) / double(interval);
		return std::abs(rate - double(fps)) <= s_tolerance ? interval : 0;
	}
}
}
