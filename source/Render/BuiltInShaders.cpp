//
//  BuiltInShaders.cpp
//  Square
//
//  See BuiltInShaders.h.
//
#include "Square/Render/BuiltInShaders.h"

namespace Square
{
namespace Render
{
namespace BuiltInShaders
{
	static const Array<const char*, MAX_SHADER> s_shaders
	{
		#include "BuiltInShaders/SplashScreen.hlsl"
		,
		#include "BuiltInShaders/UI.hlsl"
	};
	// Return the static shader 
	const char* get(Type type)
	{
		if (type < Type::MAX_SHADER)
		{
			return s_shaders[type];
		}
		return nullptr;
	}
	// All shaders
	const Array<const char*, MAX_SHADER> & all()
	{
		return s_shaders;
	}
}
}
}
