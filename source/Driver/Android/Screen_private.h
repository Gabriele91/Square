//
//  Square
//
//  Android: the display of the activity (one), as big as its window.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Android
{
	struct ScreenAndroid
	{
		unsigned int m_screen_id{ 0 };
		////////////////////////////////////////
		ScreenAndroid(unsigned int screen_id);
		//its size: the one of the window of the activity (full screen)
		void get_size(unsigned int size[2]) const;
	};
}
}
}
