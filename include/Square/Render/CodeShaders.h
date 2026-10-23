//
//  CodeShaders.h
//  Square
//
//  The shaders the engine compiles from a source in its code, not from a resource: the ones it
//  draws before the resources are there (the splash screen) or of its own (the UI). Their sources
//  in one place (source/Render/CodeShaders/*.hlsl): the ones that use them and the shader pack
//  (ShaderPack translates them too, no define: a release has no shader out of its pack).
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Render
{
namespace CodeShaders
{
	//the icon of the splash screen
	SQUARE_API const char* splash_screen();
	//the documents of the UI (RmlUi)
	SQUARE_API const char* ui();
	//all of them
	SQUARE_API Vector<String> all();
}
}
}
