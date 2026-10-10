//
//  Build.h
//  Square
//
//  The build of the library: its kind and the version of the interface of its drivers
//  (SQUARE_BUILD_ID of Config.h when it was compiled). A game (square_main) and the drivers
//  (Render::create_render_driver) are loaded only with a library of their same build: the
//  runtime of the compiler and the debug tools change the classes between Debug, Release and
//  Retail.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
	//"<kind>/abi<version>" of the library ("release/abi1")
	SQUARE_API const char* build_id();
}
