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
		release_levels();
		delete_color_target(m_trace_texture, m_trace_target);
		delete_color_target(m_denoise_texture, m_denoise_target);
		m_trace_size = IVec2(0, 0);
		m_shader_trace.reset();
		m_shader_denoise.reset();
		m_shader_downsample.reset();
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

	void SSR::release_levels()
	{
		for (Level& level : m_levels)
		{
			delete_color_target(level.m_texture, level.m_target);
		}
		m_levels.clear();
		m_levels_count = 0;
	}

	int SSR::blur_levels(Settings::BlurQuality blur)
	{
		switch (blur)
		{
		case Settings::BLUR_MEDIUM: return 3;
		case Settings::BLUR_HIGH:   return s_max_blur_levels;
		default:                    return 0;
		}
	}

	bool SSR::build_levels(const IVec2& trace_size)
	{
		const int levels = blur_levels(m_settings.blur);
		const IVec2 first_size = glm::max(trace_size / 2, IVec2(1, 1));
		if (m_levels_count == levels && (m_levels.empty() || m_levels[0].m_size == first_size)) return true;
		release_levels();
		//each level half the one before, while it is at least 2x2
		m_levels.reserve(levels);
		for (IVec2 size = first_size; int(m_levels.size()) < levels && size.x >= 2 && size.y >= 2; size = glm::max(size / 2, IVec2(1, 1)))
		{
			Level level;
			level.m_size = size;
			const bool created = create_color_target(size, TF_RGBA16F, level.m_texture, level.m_target);
			m_levels.push_back(level);
			if (!created)
			{
				release_levels();
				return false;
			}
		}
		m_levels_count = levels;
		return true;
	}

	void SSR::denoise(const PostEffectFrame& frame, Texture* position, Texture* normal, const Vec4& params)
	{
		//horizontal: the trace to the denoise target; vertical: back to the trace
		struct Pass { Texture* m_source; Target* m_target; Vec2 m_direction; };
		const Pass passes[]
		{
			{ m_trace_texture,   m_denoise_target, Vec2(1.0f, 0.0f) },
			{ m_denoise_texture, m_trace_target,   Vec2(0.0f, 1.0f) }
		};
		PostEffectFrame trace_frame = frame;
		trace_frame.m_size = m_trace_size;
		for (const auto& pass : passes)
		{
			draw_fullscreen(trace_frame, pass.m_target, m_shader_denoise.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))      u->set(pass.m_source);
				if (auto u = shader.uniform("g_position"))    u->set(position);
				if (auto u = shader.uniform("g_normal"))      u->set(normal);
				if (auto u = shader.uniform("ssr_size"))      u->set(Vec2(float(m_trace_size.x), float(m_trace_size.y)));
				if (auto u = shader.uniform("ssr_direction")) u->set(pass.m_direction);
				if (auto u = shader.uniform("ssr_params"))    u->set(params);
				if (auto u = shader.uniform("ssr_radius"))    u->set(std::clamp(m_settings.denoise_radius, 1.0f, 16.0f));
			});
		}
	}

	void SSR::blur(const PostEffectFrame& frame)
	{
		//every level from the one before, the first from the trace
		Texture* source = m_trace_texture;
		float premultiply = 1.0f;
		for (const Level& level : m_levels)
		{
			PostEffectFrame level_frame = frame;
			level_frame.m_size = level.m_size;
			draw_fullscreen(level_frame, level.m_target, m_shader_downsample.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))        u->set(source);
				if (auto u = shader.uniform("ssr_size"))        u->set(Vec2(float(level.m_size.x), float(level.m_size.y)));
				if (auto u = shader.uniform("ssr_premultiply")) u->set(premultiply);
			});
			source = level.m_texture;
			premultiply = 0.0f;
		}
	}

	void SSR::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shaders (again after a release)
		if (!m_shader_trace)      m_shader_trace      = load_shader("SSRTrace");
		if (!m_shader_denoise)    m_shader_denoise    = load_shader("SSRDenoise");
		if (!m_shader_downsample) m_shader_downsample = load_shader("SSRDownsample");
		if (!m_shader_composite)  m_shader_composite  = load_shader("SSRComposite");
		//forward (no G-Buffer), no shaders: the frame as it is
		if (!frame.m_gbuffer || !frame.m_camera || !m_shader_trace || !m_shader_denoise || !m_shader_downsample || !m_shader_composite)
		{
			copy(frame);
			return;
		}
		//reflections texture: full, half or a quarter of the frame size
		const IVec2 trace_size = post_effect_size(frame.m_size, m_settings.resolution);
		if (!m_trace_target || m_trace_size != trace_size)
		{
			delete_color_target(m_trace_texture, m_trace_target);
			delete_color_target(m_denoise_texture, m_denoise_target);
			if (!create_color_target(trace_size, TF_RGBA16F, m_trace_texture, m_trace_target)
			||  !create_color_target(trace_size, TF_RGBA16F, m_denoise_texture, m_denoise_target))
			{
				m_trace_size = IVec2(0, 0);
				copy(frame);
				return;
			}
			m_trace_size = trace_size;
		}
		//the blur chain (none: the trace only)
		if (!build_levels(trace_size)) release_levels();
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
		//2) denoise (not the projection check: its red is the error of a pixel)
		if (m_settings.denoise && m_settings.debug != 2) denoise(frame, position, normal, params);
		//3) blur
		blur(frame);
		//4) composite: the levels not built are the last one (never read, but bound)
		static const char* s_level_names[s_max_blur_levels]{ "g_reflection_1", "g_reflection_2", "g_reflection_3", "g_reflection_4", "g_reflection_5" };
		draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))     u->set(frame.m_source);
			if (auto u = shader.uniform("g_reflection")) u->set(m_trace_texture);
			Texture* level_texture = m_trace_texture;
			for (int i = 0; i != s_max_blur_levels; ++i)
			{
				if (i < int(m_levels.size())) level_texture = m_levels[i].m_texture;
				if (auto u = shader.uniform(s_level_names[i])) u->set(level_texture);
			}
			if (auto u = shader.uniform("ssr_blur"))     u->set(float(m_settings.blur));
			if (auto u = shader.uniform("ssr_levels"))   u->set(float(m_levels.size()));
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
