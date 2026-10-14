//
//  ShaderTarget.h
//  Square
//
//  What a shader is translated for: its language (GLSL, GLSL ES, HLSL, MSL for macOS or iOS,
//  SPIR-V), its version, the driver it stands for (the requirements of the effects choose their
//  sub effects by it: its geometry shaders, its layers written by the vertex shader...), the gamma
//  correction (the common header of the shaders).
//  - The runtime: the one of its driver (of); its stages read from the pack (if the game has it),
//    else from the cache of the user, else translated there.
//  - A tool (tools/ShaderPack): every one of a platform (of_platform), translated only (no shader
//    of the driver): the cross product of its languages, of the capabilities of their drivers, of
//    the gamma corrections; the stages of all of them in the pack of the game.
//
//  The translation depends only on it (and on the source): each language has its baseline version
//  (GLSL 4.1, ESSL 3.1 or 3.2, MSL 2.0, HLSL 5.0; an effect can ask another one), never the version
//  or the optional extensions of a driver; so the pack made on a machine serves every machine of
//  its platform.
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Resource
{
	struct SQUARE_API ShaderTarget
	{
		enum Language : int
		{
			L_GLSL = 0,  //desktop OpenGL (Windows, macOS, Linux)
			L_GLSL_ES,   //OpenGL ES (iOS, Linux, Android)
			L_HLSL,      //DirectX (Windows)
			L_MSL,       //Metal, macOS
			L_MSL_IOS,   //Metal, iOS
			L_SPIRV      //Vulkan (Windows, Linux, Android)
		};

		Language                 m_language{ L_GLSL };
		Render::RenderDriverInfo m_info;                    //the driver it stands for (the requirements of the effects)
		int                      m_version{ 410 };          //of the language (GLSL 410, ESSL 310/320, MSL 20000, HLSL 50)
		bool                     m_gamma{ true };           //the gamma correction of the shaders (ENABLE_GAMMA_CORRECTION)
		bool                     m_srgb_target{ true };     //the framebuffer encodes sRGB (ENABLE_TARGET_SRGB)
		bool                     m_translate_only{ false }; //a tool: its stages, no shader of the driver

		//its name: the folder of its translations ("GLSL", "GLSL_ES", "HLSL", "MSL", "MSL_IOS", "SPIRV")
		const char* name() const;
		static const char* name(Language language);
		//the baseline version of a language (ESSL: 3.2 for a driver of it, else 3.1)
		static int version(Language language, const Render::RenderDriverInfo& info);

		//the one of a driver (the runtime), its gamma correction (the window)
		static ShaderTarget of(const Render::Context& render, bool gamma);

		//every one of a platform ("windows", "macos", "ios", "linux", "android"): each language of it
		//x each capability of its drivers that changes the shaders (geometry shaders, layers written
		//by the vertex shader) x each gamma correction (none, by the framebuffer, by the shaders);
		//none: an unknown platform
		static Vector<ShaderTarget> of_platform(const String& platform);
		//the platform this is compiled for
		static String host_platform();
	};
}
}
