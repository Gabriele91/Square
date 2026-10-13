//
//  SplashScreen.h
//  Square
//
//  The splash screen of the engine: the icon of Square in the middle of the window, a frame
//  shown by Application::execute while the application loads (before AppInterface::start).
//  The icon (common/texture/square_icon.png) is inside the library, put there at build time:
//  a resource of Square.dll on Windows (Square.rc), a section of the library elsewhere (.incbin).
//
#pragma once
#include <vector>
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Square
{
	class Context;
}

namespace Square
{
namespace Render
{
namespace SplashScreen
{
	//the PNG of the icon inside the library (empty: none)
	SQUARE_API std::vector<unsigned char> image();

	//draw a frame (the background, the icon in the middle) and present it; false when it cannot
	//(no render, no window, the image or the shader not there)
	SQUARE_API bool show(Square::Context& context, const Vec4& background = Vec4(0.06f, 0.07f, 0.09f, 1.0f));
}
}
}
