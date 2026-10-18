//
//  Square
//
//  Android: the bars of the system (status, navigation) hidden over the game (immersive sticky:
//  a swipe from an edge shows them a little, over the frames), each time the activity gets its
//  focus back.
//
#pragma once
#include "Square/Config.h"
#include <android/native_activity.h>

namespace Square
{
namespace Video
{
namespace Android
{
	//the bars hidden now (JNI: in the thread of the activity, its UI one)
	void hide_system_bars(ANativeActivity* activity);
	//the bars hidden at each focus of the activity (its callback of the focus, in its UI thread,
	//wrapped: the one of the glue first)
	void keep_system_bars_hidden(ANativeActivity* activity);
}
}
}
