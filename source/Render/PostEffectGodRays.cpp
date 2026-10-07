//
//  PostEffectGodRays.cpp
//  Square
//
//  See PostEffectGodRays.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectGodRays.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	GodRays::GodRays(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	GodRays::~GodRays()
	{
		on_release();
	}

	void GodRays::on_release()
	{
		delete_color_target(m_mask_texture, m_mask_target);
		delete_color_target(m_rays_texture, m_rays_target);
		m_size = IVec2(0, 0);
		m_shader_mask.reset();
		m_shader_blur.reset();
		m_shader_composite.reset();
		m_shader_copy.reset();
		m_shader_volume.reset();
		m_shader_volume_blur.reset();
	}

	void GodRays::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		if (!m_shader_copy) return;
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	bool GodRays::build(const IVec2& size)
	{
		if (size == m_size && m_mask_target && m_rays_target) return true;
		m_size = size;
		const bool mask = create_color_target(size, TF_RGBA16F, m_mask_texture, m_mask_target);
		const bool rays = create_color_target(size, TF_RGBA16F, m_rays_texture, m_rays_target);
		return mask && rays;
	}

	void GodRays::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shaders (again after a release)
		if (!m_shader_mask) m_shader_mask = load_shader("GodRaysMask");
		if (!m_shader_blur) m_shader_blur = load_shader("GodRaysBlur");
		if (!m_shader_composite) m_shader_composite = load_shader("GodRaysComposite");
		const bool shaders = m_shader_mask && m_shader_blur && m_shader_composite;
		//forward (no G-Buffer), no shaders: the frame as it is
		if (!frame.m_gbuffer || !frame.m_camera || !shaders)
		{
			copy(frame);
			return;
		}
		const IVec2 size = post_effect_size(frame.m_size, m_settings.resolution);
		if (!build(size))
		{
			copy(frame);
			return;
		}
		//volumetric: the sun with its shadow map
		const bool sun_shadow = frame.m_sun_shadow_map && frame.m_sun_shadow_buffer;
		if (m_settings.volumetric && sun_shadow)
		{
			draw_volumetric(frame, size);
			return;
		}
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		Texture* albedo = frame.m_gbuffer->texture(DrawerPassDeferred::GB_ALBEDO);
		Texture* emissive = frame.m_gbuffer->texture(DrawerPassDeferred::GB_EMISSIVE);
		//toward the sun (a direction; none: straight up)
		const float sun_length = length(m_settings.sun_direction);
		const Vec3  sun_direction = sun_length > 0.0001f ? m_settings.sun_direction / sun_length : Vec3(0.0f, -1.0f, 0.0f);
		const Vec4  sun = Vec4(-sun_direction, std::max(m_settings.fade, 0.001f));
		const Vec2  frame_size = Vec2(float(frame.m_size.x), float(frame.m_size.y));
		const Vec2  rays_size = Vec2(float(size.x), float(size.y));
		const float aspect = frame_size.x / std::max(frame_size.y, 1.0f);
		PostEffectFrame small = frame;
		small.m_size = size;
		//1) the mask: the sky around the sun
		draw_fullscreen(small, m_mask_target, m_shader_mask.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
			if (auto u = shader.uniform("g_position")) u->set(position);
			if (auto u = shader.uniform("g_albedo"))   u->set(albedo);
			if (auto u = shader.uniform("g_emissive")) u->set(emissive);
			if (auto u = shader.uniform("rays_size"))  u->set(rays_size);
			if (auto u = shader.uniform("rays_sun"))   u->set(sun);
			if (auto u = shader.uniform("rays_mask"))  u->set(Vec4(std::max(m_settings.threshold, 0.0f), std::max(m_settings.sun_radius, 0.001f), aspect, m_settings.emissive_sky ? 1.0f : 0.0f));
		});
		//2) the blur toward the sun
		const int samples = std::clamp(m_settings.samples, 1, 128);
		draw_fullscreen(small, m_rays_target, m_shader_blur.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_mask"))      u->set(m_mask_texture);
			if (auto u = shader.uniform("rays_size"))   u->set(rays_size);
			if (auto u = shader.uniform("rays_sun"))    u->set(sun);
			if (auto u = shader.uniform("rays_blur"))   u->set(Vec4(std::clamp(m_settings.density, 0.0f, 1.0f), std::clamp(m_settings.decay, 0.0f, 1.0f), std::max(m_settings.weight, 0.0f), float(samples)));
		});
		//3) the frame plus the rays
		draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))    u->set(frame.m_source);
			if (auto u = shader.uniform("g_rays"))      u->set(m_rays_texture);
			if (auto u = shader.uniform("rays_size"))   u->set(frame_size);
			if (auto u = shader.uniform("rays_sun"))    u->set(sun);
			if (auto u = shader.uniform("rays_color"))  u->set(Vec4(m_settings.color, std::max(m_settings.intensity, 0.0f)));
		});
	}

	void GodRays::draw_volumetric(PostEffectFrame& frame, const IVec2& size)
	{
		if (!m_shader_volume) m_shader_volume = load_shader("GodRaysVolume");
		if (!m_shader_volume_blur) m_shader_volume_blur = load_shader("GodRaysVolumeBlur");
		if (!m_shader_volume || !m_shader_volume_blur)
		{
			copy(frame);
			return;
		}
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		//toward the sun (the one of the frame), the hue of its color (not its strength: a strong sun
		//would burn the frame; how much light: the intensity) by the settings
		const float sun_length = length(frame.m_sun_direction);
		const Vec3  sun_direction = sun_length > 0.0001f ? frame.m_sun_direction / sun_length : Vec3(0.0f, -1.0f, 0.0f);
		//(w: negative, the rays always visible in the composite)
		const Vec4  sun = Vec4(-sun_direction, -1.0f);
		const float sun_peak = std::max({ frame.m_sun_color.x, frame.m_sun_color.y, frame.m_sun_color.z, 0.0001f });
		const Vec3  color = frame.m_sun_color / sun_peak * m_settings.color;
		const Vec2  frame_size = Vec2(float(frame.m_size.x), float(frame.m_size.y));
		const Vec2  rays_size = Vec2(float(size.x), float(size.y));
		const int   steps = std::clamp(m_settings.steps, 1, 128);
		const float anisotropy = std::clamp(m_settings.anisotropy, 0.0f, 0.95f);
		PostEffectFrame small = frame;
		small.m_size = size;
		//1) the light in the air along the rays
		draw_fullscreen(small, m_mask_target, m_shader_volume.get(), BlendState(), [&](Resource::Shader& shader)
		{
			render().bind_uniform_CB(frame.m_sun_shadow_buffer, shader.base_shader(), "DirectionShadowCamera");
			if (auto u = shader.uniform("direction_shadow_map")) u->set(frame.m_sun_shadow_map);
			if (auto u = shader.uniform("g_position"))  u->set(position);
			if (auto u = shader.uniform("rays_size"))   u->set(rays_size);
			if (auto u = shader.uniform("rays_sun"))    u->set(sun);
			if (auto u = shader.uniform("rays_volume")) u->set(Vec4(std::max(m_settings.max_distance, 1.0f), float(steps), anisotropy, std::max(m_settings.air, 0.0f)));
		});
		//2) its noise blurred
		draw_fullscreen(small, m_rays_target, m_shader_volume_blur.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_rays"))     u->set(m_mask_texture);
			if (auto u = shader.uniform("rays_size"))  u->set(rays_size);
		});
		//3) the frame plus the light
		draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
			if (auto u = shader.uniform("g_rays"))     u->set(m_rays_texture);
			if (auto u = shader.uniform("rays_size"))  u->set(frame_size);
			if (auto u = shader.uniform("rays_sun"))   u->set(sun);
			if (auto u = shader.uniform("rays_color")) u->set(Vec4(color, std::max(m_settings.intensity, 0.0f)));
		});
	}
}
}
