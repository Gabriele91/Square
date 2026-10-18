//
//  Android.h
//  Square
//
//  The NativeActivity of a game on Android: android_main (SquareMain.h) gives the engine the
//  application of the glue of the NDK, its window and its input, before the game starts.
//
#pragma once
#include "Square/Config.h"
#if defined(__ANDROID__)

struct android_app;

namespace Square
{
namespace Video
{
namespace Android
{
	//the application of the glue: its events, its window (waited: the game starts with it), its
	//folders (the working one: the resources of the game, the files of the application), the
	//environment of the file env.txt of the resources (a line NAME=VALUE)
	SQUARE_API void attach(android_app* app);
	//the command line of the game: its name, the backend of Android (-b vk), then the arguments in
	//the file args.txt of the resources (one line: "-m arena -v")
	SQUARE_API void arguments(int& argc, const char**& argv);
	//the game ended: the activity finished, then the process (no state of the engine kept for
	//the next activity)
	SQUARE_API void detach();
	//the application of the glue (its activity: JNI, the assets)
	SQUARE_API android_app* application();
}
}
}

#endif // __ANDROID__
