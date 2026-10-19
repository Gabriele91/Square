//
//  Square
//
//  The device resources of a window: what a platform may not have (the frames a second paced by
//  the window: by default the application waits)
//
#include "Square/Driver/Window.h"

namespace Square
{
namespace Video
{
	bool DeviceResources::set_frame_rate(unsigned int fps)
	{
		return false;
	}

	unsigned int DeviceResources::get_frame_rate()
	{
		return 0;
	}
}
}
