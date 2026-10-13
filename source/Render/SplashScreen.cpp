//
//  SplashScreen.cpp
//  Square
//
//  See SplashScreen.h. The icon is put in the library by the build: on Windows a resource of
//  Square.dll (Square.rc), elsewhere a section filled by the assembler (.incbin of
//  SQUARE_SPLASH_IMAGE_PATH, from CMake).
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/Window.h"
#include "Square/Render/BasicMesh.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/SplashScreen.h"
#include "Square/Resource/Shader.h"
#include "Square/Resource/Texture.h"
#include "Square/System/RenderSystem.h"

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#else
	//the bytes of the icon, between two symbols of the library
	#if defined(__APPLE__)
		__asm__(
			".section __TEXT,__const\n"
			".p2align 4\n"
			".globl _square_splash_image_begin\n"
			"_square_splash_image_begin:\n"
			".incbin \"" SQUARE_SPLASH_IMAGE_PATH "\"\n"
			".globl _square_splash_image_end\n"
			"_square_splash_image_end:\n"
			".byte 0\n"
			".text\n"
		);
	#else
		__asm__(
			".pushsection .rodata\n"
			".balign 16\n"
			".globl square_splash_image_begin\n"
			"square_splash_image_begin:\n"
			".incbin \"" SQUARE_SPLASH_IMAGE_PATH "\"\n"
			".globl square_splash_image_end\n"
			"square_splash_image_end:\n"
			".byte 0\n"
			".popsection\n"
		);
	#endif
	extern "C" const unsigned char square_splash_image_begin[];
	extern "C" const unsigned char square_splash_image_end[];
#endif

namespace Square
{
namespace Render
{
namespace SplashScreen
{
	//the shader of the icon
	static const char* s_shader_source =
	#include "SplashScreen.hlsl"
	;

	//share of the shorter side of the window taken by the icon
	static constexpr float s_icon_scale = 0.4f;

	std::vector<unsigned char> image()
	{
	#if defined(_WIN32)
		//the module of this code (Square.dll), its resource SQUARE_SPLASH_IMAGE
		HMODULE module = nullptr;
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		                        reinterpret_cast<LPCWSTR>(&image), &module))
		{
			return {};
		}
		HRSRC resource = FindResourceW(module, L"SQUARE_SPLASH_IMAGE", MAKEINTRESOURCEW(10) /* RT_RCDATA */);
		if (!resource) return {};
		HGLOBAL data = LoadResource(module, resource);
		const DWORD size = SizeofResource(module, resource);
		const unsigned char* bytes = data ? static_cast<const unsigned char*>(LockResource(data)) : nullptr;
		if (!bytes || !size) return {};
		return std::vector<unsigned char>(bytes, bytes + size);
	#else
		return std::vector<unsigned char>(square_splash_image_begin, square_splash_image_end);
	#endif
	}

	bool show(Square::Context& context, const Vec4& background)
	{
		auto* render_system = System::get<RenderSystem>(context);
		auto* render = render_system ? render_system->render() : nullptr;
		auto* window = context.window();
		if (!render || !window) return false;
		//the icon
		std::vector<unsigned char> image_file = image();
		if (image_file.empty()) return false;
		auto texture = MakeShared<Resource::Texture>(context);
		if (!texture->load({ TMIN_LINEAR_MIPMAP_LINEAR, TMAG_LINEAR, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP, true, 1 }, image_file))
		{
			return false;
		}
		//the shader, the quad
		auto shader = MakeShared<Resource::Shader>(context);
		if (!shader->compile(s_shader_source, {})) return false;
		auto quad = BasicMesh::build_quad(context);
		if (!quad) return false;
		//the icon in the middle, s_icon_scale of the shorter side, its aspect
		unsigned int width = 0, height = 0;
		window->get_size(width, height);
		if (!width || !height) return false;
		const float side = float(std::min(width, height)) * s_icon_scale;
		const float aspect = float(texture->get_width()) / float(std::max<unsigned long>(texture->get_height(), 1));
		const Vec4 rect(side * aspect / float(width), side / float(height), 0.0f, 0.0f);
		//the frame: the background (linear on an sRGB target), the icon over it
		Vec4 clear_color = background;
		if (render->is_srgb_framebuffer())
		{
			clear_color = Vec4(glm::pow(Vec3(background), Vec3(2.2f)), background.w);
		}
		render->set_viewport_state({ Vec4(0.0f, 0.0f, float(width), float(height)) });
		render->set_clear_color_state({ clear_color });
		render->clear(CLEAR_COLOR_DEPTH);
		render->set_depth_buffer_state({ DM_DISABLE });
		render->set_cullface_state({ CF_DISABLE });
		render->set_blend_state(BlendState(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA));
		shader->bind();
		if (auto u = shader->uniform("g_splash"))    u->set(texture->get_context_texture());
		if (auto u = shader->uniform("splash_rect")) u->set(rect);
		quad->draw(*render);
		shader->unbind();
		//the states of the engine back
		render->set_blend_state({});
		render->set_cullface_state({ CF_BACK });
		render->set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
		render->print_errors();
		window->swap();
		return true;
	}
}
}
}
