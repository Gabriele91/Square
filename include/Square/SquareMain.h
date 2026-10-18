#pragma once
#include "Square/Config.h"
#include "Square/Core/Build.h"

#if defined( SQUARE_MAIN )

// OpenGL Optimus selection variables
#if defined( _WIN32 ) && defined( SQUARE_DLL_EXPORT )
extern "C"
{
	SQUARE_DLL_EXPORT uint32_t NvOptimusEnablement = 1;
	SQUARE_DLL_EXPORT int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // _WIN32 && SQUARE_LIBRARY_IMPORT

#if !defined( _CONSOLE ) && defined( _WIN32 )
#include <Windows.h>
#include <stdlib.h>
// Define main
int main(int argc, const char** argv);
// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int result = main(__argc, (const char**)__argv);
    return result;
}
#endif

#include <cstdio>
#include <cstring>
#if defined( _WIN32 )
#include <Windows.h>
#endif

// The game and the library of the same build (Core/Build.h), else a message and out
inline bool square_same_build()
{
	const bool same = std::strcmp(::Square::build_id(), SQUARE_BUILD_ID) == 0;
	if (!same)
	{
		char message[256];
		std::snprintf(message, sizeof(message), "Square: the game is built %s, the library (Square) %s: not the same configuration\n", SQUARE_BUILD_ID, ::Square::build_id());
		std::fputs(message, stderr);
#if defined( _WIN32 )
		OutputDebugStringA(message);
#endif
	}
	return same;
}

// Define main
#if defined( __ANDROID__ )
#include "Square/Driver/Android.h"
// Android: the NativeActivity calls android_main (in the thread of the glue of the NDK), with its
// command line made by the engine (the backend of Android, the arguments of args.txt)
#define square_main(shell_commands)\
	int square_main(::Square::Application& app, ::Square::Shell::ParserValue& args, ::Square::Shell::Error& errors);\
	extern "C" void android_main(struct android_app* android)\
	{\
		::Square::Video::Android::attach(android);\
		int argc = 0;\
		const char** argv = nullptr;\
		::Square::Video::Android::arguments(argc, argv);\
		if (square_same_build())\
		{\
			::Square::Application app;\
			auto [errors, args] = ::Square::Shell::parser(argc, argv, shell_commands);\
			square_main(app, args, errors);\
		}\
		::Square::Video::Android::detach();\
	}\
	int square_main
#else
#define square_main(shell_commands)\
	int square_main(::Square::Application& app, ::Square::Shell::ParserValue& args, ::Square::Shell::Error& errors);\
	int main(int argc, const char** argv)\
	{\
		if (!square_same_build()) return 1;\
		::Square::Application app;\
		auto [errors, args] = ::Square::Shell::parser(argc, argv, shell_commands);\
		return square_main(app, args, errors);\
	}\
	int square_main
#endif // __ANDROID__

#endif // SQUARE_MAIN