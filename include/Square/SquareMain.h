#pragma once
#include "Square/Config.h"

#if defined( SQUARE_MAIN )

// OpenGL Optimus selection variables
#if defined( _WIN32 ) && defined( DLL_EXPORT )
extern "C"
{
	DLL_EXPORT uint32_t NvOptimusEnablement = 1;
	DLL_EXPORT int AmdPowerXpressRequestHighPerformance = 1;
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

// Define main
#define square_main(shell_commands)\
	int square_main(::Square::Application& app, ::Square::Shell::ParserValue& args, ::Square::Shell::Error& errors);\
	int main(int argc, const char** argv)\
	{\
		::Square::Application app;\
		auto [errors, args] = ::Square::Shell::parser(argc, argv, shell_commands);\
		return square_main(app, args, errors);\
	}\
	int square_main

#endif // SQUARE_MAIN