//
//  PostEffectSSAO.cpp
//  Square
//
//  See PostEffectSSAO.h for the high level description.
//  Two passes:
//   1) SSAO:      G-Buffer position + normal -> raw occlusion (noisy: few rotated samples);
//   2) SSAOApply: 4x4 blur that keeps the edges -> Vec4(1, 1, 1, ao) blended (ZERO, SRC_COLOR)
//                 on the G-Buffer occlusion target: GT3.rgb unchanged, GT3.a *= ao.
//
#include <cmath>
#include "Square/Core/Context.h"
#include "Square/Render/Camera.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectSSAO.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	SSAO::SSAO(Square::Context& context)
	: PostEffect(context, PES_GBUFFER)
	{
	}

	SSAO::~SSAO()
	{
		on_release();
	}

	void SSAO::on_release()
	{
		delete_color_target(m_ao_texture, m_ao_target);
		delete_color_target(m_debug_texture, m_debug_target);
		m_size = IVec2(0, 0);
		m_shader_ssao.reset();
		m_shader_apply.reset();
	}

	void SSAO::draw(PostEffectFrame& frame)
	{
		if (!frame.m_gbuffer || !frame.m_occlusion || !frame.m_camera) return;
		//shaders (again after a release)
		if (!m_shader_ssao)  m_shader_ssao  = load_shader("SSAO");
		if (!m_shader_apply) m_shader_apply = load_shader("SSAOApply");
		if (!m_shader_ssao || !m_shader_apply) return;
		//raw occlusion texture of the frame size
		if (!m_ao_target || m_size != frame.m_size)
		{
			if (!create_color_target(frame.m_size, TF_RGBA16F, m_ao_texture, m_ao_target)) return;
			delete_color_target(m_debug_texture, m_debug_target);
			m_size = frame.m_size;
		}
		if (m_settings.debug && !m_debug_target)
		{
			if (!create_color_target(frame.m_size, TF_RGBA16F, m_debug_texture, m_debug_target)) return;
		}
		//pixels of one world unit at distance 1: half the screen height times the focal (P[1][1])
		const float pixel_scale = 0.5f * float(frame.m_size.y) * std::abs(frame.m_camera->projection()[1][1]);
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		Texture* normal   = frame.m_gbuffer->texture(DrawerPassDeferred::GB_NORMAL);
		const Vec2 size(float(frame.m_size.x), float(frame.m_size.y));
		//1) raw occlusion
		draw_fullscreen(frame, m_ao_target, m_shader_ssao.get(), {}, [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_position")) u->set(position);
			if (auto u = shader.uniform("g_normal"))   u->set(normal);
			if (auto u = shader.uniform("ssao_size"))  u->set(size);
			if (auto u = shader.uniform("ssao_params")) u->set(Vec4(m_settings.radius, m_settings.intensity, m_settings.bias, m_settings.contrast));
			if (auto u = shader.uniform("ssao_pixels")) u->set(Vec2(pixel_scale, m_settings.max_pixels));
		});
		//2) blur and multiply the G-Buffer occlusion (alpha of GT3)
		auto apply = [&](Target* target, bool debug, const BlendState& blend)
		{
			draw_fullscreen(frame, target, m_shader_apply.get(), blend, [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("ssao_debug")) u->set(debug ? 1.0f : 0.0f);
				if (auto u = shader.uniform("g_ao"))       u->set(m_ao_texture);
				if (auto u = shader.uniform("g_position")) u->set(position);
				if (auto u = shader.uniform("ssao_size"))  u->set(size);
				if (auto u = shader.uniform("ssao_blur"))  u->set(m_settings.blur ? 1.0f : 0.0f);
			});
		};
		apply(frame.m_occlusion, false, BlendState(BLEND_ZERO, BLEND_SRC_COLOR));
		//debug view: the same occlusion, in grey, on its texture
		if (m_settings.debug) apply(m_debug_target, true, BlendState());
	}

	Texture* SSAO::debug_texture() const
	{
		return m_settings.debug ? m_debug_texture : nullptr;
	}

}
}
