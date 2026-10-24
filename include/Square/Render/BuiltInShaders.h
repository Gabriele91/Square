//
//  BuiltInShaders.h
//  Square
//
//  The shaders the engine compiles from a source in its code, not from a resource: the ones it
//  draws before the resources are there (the splash screen) or of its own (the UI). Their sources
//  in one place (source/Render/BuiltInShaders/*.hlsl): the ones that use them and the shader pack
//  (ShaderPack translates them too, no define: a release has no shader out of its pack).
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Render
{
namespace BuiltInShaders
{
	enum Type
	{
		SPLASH_SCREEN_SHADER,
		UI_SHADER,
		MAX_SHADER
	};

	// Return the static shader 
	SQUARE_API const char* get(Type type);
	// All shaders
	SQUARE_API const Array<const char*, MAX_SHADER>& all();
}
}
}
