//
//  PostEffectSSR.cpp
//  Square
//
//  See PostEffectSSR.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectSSR.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	SSR::SSR(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	SSR::~SSR()
	{
		on_release();
	}

	void SSR::on_release()
	{
		delete_color_target(m_trace_texture, m_trace_target);
		m_trace_size = IVec2(0, 0);
		m_shader_trace.reset();
		m_shader_composite.reset();
		m_shader_copy.reset();
	}

	void SSR::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	void SSR::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shaders (again after a release)
		if (!m_shader_trace)     m_shader_trace     = load_shader("SSRTrace");
		if (!m_shader_composite) m_shader_composite = load_shader("SSRComposite");
		//forward (no G-Buffer), no shaders: the frame as it is
		if (!frame.m_gbuffer || !frame.m_camera || !m_shader_trace || !m_shader_composite)
		{
			copy(frame);
			return;
		}
		//reflections texture, half or full size
		const IVec2 trace_size = m_settings.half_resolution ? glm::max(frame.m_size / 2, IVec2(1, 1)) : frame.m_size;
		if (!m_trace_target || m_trace_size != trace_size)
		{
			if (!create_color_target(trace_size, TF_RGBA16F, m_trace_texture, m_trace_target))
			{
				m_trace_size = IVec2(0, 0);
				copy(frame);
				return;
			}
			m_trace_size = trace_size;
		}
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		Texture* normal   = frame.m_gbuffer->texture(DrawerPassDeferred::GB_NORMAL);
		Texture* albedo   = frame.m_gbuffer->texture(DrawerPassDeferred::GB_ALBEDO);
		Texture* emissive = frame.m_gbuffer->texture(DrawerPassDeferred::GB_EMISSIVE);
		const Vec4 params(m_settings.max_distance, float(std::clamp(m_settings.steps, 1, 128)), m_settings.thickness, std::max(m_settings.max_roughness, 0.001f));
		//1) trace
		{
			PostEffectFrame trace_frame = frame;
			trace_frame.m_size = trace_size;
			draw_fullscreen(trace_frame, m_trace_target, m_shader_trace.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
				if (auto u = shader.uniform("g_position")) u->set(position);
				if (auto u = shader.uniform("g_normal"))   u->set(normal);
				if (auto u = shader.uniform("ssr_size"))   u->set(Vec2(float(trace_size.x), float(trace_size.y)));
				if (auto u = shader.uniform("ssr_params")) u->set(params);
				if (auto u = shader.uniform("ssr_fade"))   u->set(std::clamp(m_settings.edge_fade, 0.001f, 0.5f));
				if (auto u = shader.uniform("ssr_debug"))  u->set(float(m_settings.debug));
				if (auto u = shader.uniform("ssr_march"))  u->set(m_settings.screen_march ? 1.0f : 0.0f);
			});
		}
		//2) composite
		draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))     u->set(frame.m_source);
			if (auto u = shader.uniform("g_reflection")) u->set(m_trace_texture);
			if (auto u = shader.uniform("g_position"))   u->set(position);
			if (auto u = shader.uniform("g_normal"))     u->set(normal);
			if (auto u = shader.uniform("g_albedo"))     u->set(albedo);
			if (auto u = shader.uniform("g_emissive"))   u->set(emissive);
			if (auto u = shader.uniform("ssr_size"))     u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("ssr_params"))   u->set(params);
			if (auto u = shader.uniform("ssr_intensity")) u->set(m_settings.intensity);
			if (auto u = shader.uniform("ssr_debug"))    u->set(float(m_settings.debug));
		});
	}
}
}
