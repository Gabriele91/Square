//
//  Square
//
//  Android: the display of the activity (one), as big as its window.
//
#include "Screen_private.h"

namespace Square
{
namespace Video
{
namespace Android
{
	ScreenAndroid::ScreenAndroid(unsigned int screen_id)
	{
		m_screen_id = screen_id;
	}

	void ScreenAndroid::get_size(unsigned int size[2]) const
	{
		native_window_size(size);
	}
}
}
}
