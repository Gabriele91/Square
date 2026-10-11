//
//  ShaderTarget.cpp
//  Square
//
//  See ShaderTarget.h for the high level description.
//
#include "Square/Config.h"
#include "Square/Resource/ShaderTarget.h"
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace Square
{
namespace Resource
{
	namespace AuxShaderTarget
	{
		//a driver of a language: what changes its shaders (the sub effects chosen by the requirements)
		struct Capabilities
		{
			bool m_geometry{ false };     //geometry shaders
			bool m_vertex_layer{ false }; //the layer (render target array index) written by the vertex shader
			int  m_version{ 0 };          //the version of its language (GLES: ESSL 310, 320; 0: the one of the language)
		};

		//a gamma correction: none, the encode by the framebuffer (sRGB), the encode by the shaders
		struct Gamma
		{
			bool m_gamma{ false };
			bool m_srgb_target{ false };
		};

		//the drivers of each language the engine supports (the ones of the platforms); without
		//geometry shaders and layers by the vertex shader the shadows draw a layer at a time
		//(RenderSystem::layers_by_target)
		static const std::vector<Capabilities> s_glsl{ { true, false }, { true, true } };         //GL 4.x, with ARB_shader_viewport_layer_array or not
		static const std::vector<Capabilities> s_glsl_macos{ { true, false } };                   //GL 4.1 of macOS
		static const std::vector<Capabilities> s_glsl_es{ { false, false, 310 }, { true, false, 310 }, { true, false, 320 } }; //GLES 3.1, 3.1 + EXT_geometry_shader (AEP), 3.2
		static const std::vector<Capabilities> s_hlsl{ { true, false }, { true, true } };         //DX11, DX11.3 (VPAndRTArrayIndexFromAnyShader) or not
		static const std::vector<Capabilities> s_msl{ { false, true } };                          //Metal: no geometry shaders, the layer by the vertex shader
		static const std::vector<Capabilities> s_spirv{ { true, false }, { true, true }, { false, false }, { false, true } }; //Vulkan: geometry shaders, shaderOutputLayer

		//the gamma corrections of each language (GL: a framebuffer can miss sRGB, the shaders encode)
		static const std::vector<Gamma> s_gamma_gl{ { false, false }, { true, true }, { true, false } };
		static const std::vector<Gamma> s_gamma{ { false, false }, { true, true } };

		//a driver as the requirements of the effects see it
		static Render::RenderDriverInfo info(ShaderTarget::Language language, const Capabilities& capabilities)
		{
			Render::RenderDriverInfo out;
			out.m_geometry_shader = capabilities.m_geometry;
			out.m_vertex_viewport_index = capabilities.m_vertex_layer;
			out.m_draw_instanced = true;
			switch (language)
			{
			case ShaderTarget::L_GLSL_ES:
				out.m_render_driver = Render::DR_OPENGL_ES;
				out.m_shader_language = "GLSL";
				out.m_major_version = 3;
				out.m_minor_version = capabilities.m_version >= 320 ? 2 : 1;
				out.m_shader_version = capabilities.m_version;
				break;
			case ShaderTarget::L_HLSL:
				out.m_render_driver = Render::DR_DIRECTX;
				out.m_shader_language = "HLSL";
				out.m_major_version = 11;
				out.m_minor_version = 0;
				out.m_shader_version = 50;
				break;
			case ShaderTarget::L_MSL:
			case ShaderTarget::L_MSL_IOS:
				out.m_render_driver = Render::DR_METAL;
				out.m_shader_language = "MSL";
				out.m_major_version = 3;
				out.m_minor_version = 0;
				out.m_shader_version = 20000;
				break;
			case ShaderTarget::L_SPIRV:
				out.m_render_driver = Render::DR_VULKAN;
				out.m_shader_language = "SPIRV";
				out.m_major_version = 1;
				out.m_minor_version = 0;
				out.m_shader_version = 100;
				break;
			case ShaderTarget::L_GLSL:
			default:
				out.m_render_driver = Render::DR_OPENGL;
				out.m_shader_language = "GLSL";
				out.m_major_version = 4;
				out.m_minor_version = 6;
				out.m_shader_version = 460;
				break;
			}
			out.m_name = Render::render_driver_str[out.m_render_driver];
			return out;
		}

		//a language: its drivers x its gamma corrections
		static void add
		(
			  std::vector<ShaderTarget>& out
			, ShaderTarget::Language language
			, const std::vector<Capabilities>& drivers
			, const std::vector<Gamma>& gammas
		)
		{
			for (const Capabilities& capabilities : drivers)
			{
				for (const Gamma& gamma : gammas)
				{
					ShaderTarget target;
					target.m_language = language;
					target.m_info = info(language, capabilities);
					target.m_version = ShaderTarget::version(language, target.m_info);
					target.m_gamma = gamma.m_gamma;
					target.m_srgb_target = gamma.m_srgb_target;
					target.m_translate_only = true;
					out.push_back(target);
				}
			}
		}
	}

	const char* ShaderTarget::name(Language language)
	{
		switch (language)
		{
		case L_GLSL_ES: return "GLSL_ES";
		case L_HLSL:    return "HLSL";
		case L_MSL:     return "MSL";
		case L_MSL_IOS: return "MSL_IOS";
		case L_SPIRV:   return "SPIRV";
		case L_GLSL:
		default:        return "GLSL";
		}
	}

	const char* ShaderTarget::name() const
	{
		return name(m_language);
	}

	int ShaderTarget::version(Language language, const Render::RenderDriverInfo& info)
	{
		switch (language)
		{
		case L_GLSL_ES: return info.m_shader_version >= 320 ? 320 : 310; //(3.1: the least)
		case L_HLSL:    return 50;
		case L_MSL:
		case L_MSL_IOS: return 20000;
		case L_SPIRV:   return 100;
		case L_GLSL:
		default:        return 410;
		}
	}

	ShaderTarget ShaderTarget::of(const Render::Context& render, bool gamma)
	{
		ShaderTarget out;
		out.m_info = render.get_render_driver_info();
		out.m_gamma = gamma;
		out.m_srgb_target = gamma && render.is_srgb_framebuffer();
		const std::string& language = out.m_info.m_shader_language;
		if (language == "HLSL")
		{
			out.m_language = L_HLSL;
		}
		else if (language == "MSL")
		{
#if defined(__APPLE__) && TARGET_OS_IPHONE
			out.m_language = L_MSL_IOS;
#else
			out.m_language = L_MSL;
#endif
		}
		else if (language == "SPIRV")
		{
			out.m_language = L_SPIRV;
		}
		else if (out.m_info.m_render_driver == Render::DR_OPENGL_ES)
		{
			out.m_language = L_GLSL_ES;
		}
		else
		{
			out.m_language = L_GLSL;
		}
		//(the baseline, not the one of the driver: the same translation of the pack)
		out.m_version = version(out.m_language, out.m_info);
		return out;
	}

	std::vector<ShaderTarget> ShaderTarget::of_platform(const std::string& platform)
	{
		using namespace AuxShaderTarget;
		std::vector<ShaderTarget> out;
		out.reserve(18 /* window case */);
		if (platform == "windows")
		{
			add(out, L_GLSL, s_glsl, s_gamma_gl);
			add(out, L_HLSL, s_hlsl, s_gamma);
			add(out, L_SPIRV, s_spirv, s_gamma);
		}
		else if (platform == "macos")
		{
			add(out, L_GLSL, s_glsl_macos, s_gamma_gl);
			add(out, L_MSL, s_msl, s_gamma);
		}
		else if (platform == "ios")
		{
			add(out, L_GLSL_ES, s_glsl_es, s_gamma_gl);
			add(out, L_MSL_IOS, s_msl, s_gamma);
		}
		else if (platform == "linux")
		{
			add(out, L_GLSL, s_glsl, s_gamma_gl);
			add(out, L_GLSL_ES, s_glsl_es, s_gamma_gl);
			add(out, L_SPIRV, s_spirv, s_gamma);
		}
		else if (platform == "android")
		{
			add(out, L_GLSL_ES, s_glsl_es, s_gamma_gl);
			add(out, L_SPIRV, s_spirv, s_gamma);
		}
		return out;
	}

	std::string ShaderTarget::host_platform()
	{
#if defined(_WIN32)
		return "windows";
#elif defined(__APPLE__) && TARGET_OS_IPHONE
		return "ios";
#elif defined(__APPLE__)
		return "macos";
#elif defined(__ANDROID__)
		return "android";
#else
		return "linux";
#endif
	}
}
}
