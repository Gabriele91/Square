//
//  CodeShaders.cpp
//  Square
//
//  See CodeShaders.h.
//
#include "Square/Render/CodeShaders.h"

namespace Square
{
namespace Render
{
namespace CodeShaders
{
	static const char* s_splash_screen =
	#include "CodeShaders/SplashScreen.hlsl"
	;

	static const char* s_ui =
	#include "CodeShaders/UI.hlsl"
	;

	const char* splash_screen()
	{
		return s_splash_screen;
	}

	const char* ui()
	{
		return s_ui;
	}

	Vector<String> all()
	{
		return { s_splash_screen, s_ui };
	}
}
}
}
