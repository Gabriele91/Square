//
//  PostEffectSSAO.cpp
//  Square
//
//  See PostEffectSSAO.h for the high level description.
//  Passes (the first three at the occlusion size: half or full):
//   1) SSAODepth: G-Buffer position -> view depth (R32F; 0 = background);
//   2) SSAO:      view depth (positions rebuilt from the view rays) + G-Buffer normal -> raw
//                 occlusion (R8, noisy: few rotated samples);
//   3) SSAOBlur:  off, 2x2 or 4x4 box, or a separable bilateral gaussian (horizontal then vertical);
//   4) SSAOApply: up to the frame size (by the depth) -> Vec4(1, 1, 1, ao) blended
//                 (ZERO, SRC_COLOR) on the G-Buffer occlusion target: GT3.rgb unchanged,
//                 GT3.a *= ao.
//
#include <cmath>
#include "Square/Core/Context.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Pipeline/GBuffer.h"
#include "Square/Render/Pipeline/DrawerPassDeferred.h"
#include "Square/Render/PostEffect/PostEffectSSAO.h"
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
		delete_color_target(m_depth_texture, m_depth_target);
		delete_color_target(m_ao_texture, m_ao_target);
		delete_color_target(m_blur_texture, m_blur_target);
		delete_color_target(m_debug_texture, m_debug_target);
		m_size = IVec2(0, 0);
		m_ao_size = IVec2(0, 0);
		m_shader_depth.reset();
		m_shader_ssao.reset();
		m_shader_blur.reset();
		m_shader_apply.reset();
	}

	bool SSAO::create_targets(const IVec2& size, const IVec2& ao_size)
	{
		//nearest: the depth is not interpolated, every sample is a texel
		if (!create_color_target(ao_size, TF_R32F, m_depth_texture, m_depth_target, false)
		||  !create_color_target(ao_size, TF_R8,   m_ao_texture,    m_ao_target,    false)
		||  !create_color_target(ao_size, TF_R8,   m_blur_texture,  m_blur_target,  false))
		{
			m_size = m_ao_size = IVec2(0, 0);
			return false;
		}
		delete_color_target(m_debug_texture, m_debug_target);
		m_size = size;
		m_ao_size = ao_size;
		return true;
	}

	void SSAO::draw(PostEffectFrame& frame)
	{
		if (!frame.m_gbuffer || !frame.m_occlusion || !frame.m_camera) return;
		//shaders (again after a release)
		if (!m_shader_depth) m_shader_depth = load_shader("SSAODepth");
		if (!m_shader_ssao)  m_shader_ssao  = load_shader("SSAO");
		if (!m_shader_blur)  m_shader_blur  = load_shader("SSAOBlur");
		if (!m_shader_apply) m_shader_apply = load_shader("SSAOApply");
		if (!m_shader_depth || !m_shader_ssao || !m_shader_blur || !m_shader_apply) return;
		//targets of the occlusion size (half or full)
		const int   scale   = post_effect_scale(m_settings.resolution);
		const IVec2 ao_size = post_effect_size(frame.m_size, m_settings.resolution);
		if (!m_depth_target || m_size != frame.m_size || m_ao_size != ao_size)
		{
			if (!create_targets(frame.m_size, ao_size)) return;
		}
		if (m_settings.debug && !m_debug_target)
		{
			if (!create_color_target(frame.m_size, TF_RGBA16F, m_debug_texture, m_debug_target)) return;
		}
		//view rays (world, per unit of view depth): ray(ndc) = center + ndc.x * right + ndc.y * up,
		//from the inverse of projection * view (ndc z of 0.5: in front of the camera on every
		//backend; any point of the ray gives its direction)
		const Mat4& view = frame.m_camera->view();
		const Mat4  inverse_view_projection = inverse(frame.m_camera->projection() * view);
		const Vec3  eye = Vec3(inverse(view)[3]);
		auto ray = [&](float x, float y)
		{
			Vec4 world = inverse_view_projection * Vec4(x, y, 0.5f, 1.0f);
			return Vec3(world) / world.w - eye;
		};
		const Vec3 forward  = normalize(ray(0.0f, 0.0f));
		auto view_ray = [&](float x, float y) { Vec3 r = ray(x, y); return r / dot(r, forward); };
		const Vec4 ray_center(forward, 0.0f);
		const Vec4 ray_right(view_ray(1.0f, 0.0f) - forward, 0.0f);
		const Vec4 ray_up(view_ray(0.0f, 1.0f) - forward, 0.0f);
		//pixels of one world unit at distance 1: half the screen height times the focal (P[1][1])
		const float pixel_scale = 0.5f * float(frame.m_size.y) * std::abs(frame.m_camera->projection()[1][1]);
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		Texture* normal   = frame.m_gbuffer->texture(DrawerPassDeferred::GB_NORMAL);
		const Vec2 size(float(frame.m_size.x), float(frame.m_size.y));
		const Vec2 occlusion_size(float(ao_size.x), float(ao_size.y));
		auto common_uniforms = [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("ssao_full_size")) u->set(size);
			if (auto u = shader.uniform("ssao_scale"))     u->set(float(scale));
			if (auto u = shader.uniform("ssao_ray_c"))     u->set(ray_center);
			if (auto u = shader.uniform("ssao_ray_x"))     u->set(ray_right);
			if (auto u = shader.uniform("ssao_ray_y"))     u->set(ray_up);
		};
		//the passes at the occlusion size
		PostEffectFrame ao_frame = frame;
		ao_frame.m_size = ao_size;
		//1) view depth
		draw_fullscreen(ao_frame, m_depth_target, m_shader_depth.get(), {}, [&](Resource::Shader& shader)
		{
			common_uniforms(shader);
			if (auto u = shader.uniform("g_position")) u->set(position);
		});
		//2) raw occlusion
		draw_fullscreen(ao_frame, m_ao_target, m_shader_ssao.get(), {}, [&](Resource::Shader& shader)
		{
			common_uniforms(shader);
			if (auto u = shader.uniform("g_depth"))     u->set(m_depth_texture);
			if (auto u = shader.uniform("g_normal"))    u->set(normal);
			if (auto u = shader.uniform("ssao_size"))   u->set(occlusion_size);
			if (auto u = shader.uniform("ssao_params")) u->set(Vec4(m_settings.radius, m_settings.intensity, m_settings.bias, m_settings.contrast));
			if (auto u = shader.uniform("ssao_pixels")) u->set(Vec2(pixel_scale, m_settings.max_pixels));
		});
		//3) blur: from a texture to a target (mode 0: box, radius its side; 1: gaussian along direction)
		Texture* occlusion = m_ao_texture;
		auto blur = [&](Texture* source, Target* target, float mode, float radius, const Vec2& direction)
		{
			draw_fullscreen(ao_frame, target, m_shader_blur.get(), {}, [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_ao"))           u->set(source);
				if (auto u = shader.uniform("g_depth"))        u->set(m_depth_texture);
				if (auto u = shader.uniform("ssao_size"))      u->set(occlusion_size);
				if (auto u = shader.uniform("ssao_blur"))      u->set(Vec4(mode, radius, direction.x, direction.y));
				if (auto u = shader.uniform("ssao_sharpness")) u->set(m_settings.blur_sharpness);
			});
		};
		switch (m_settings.blur)
		{
		case Settings::BLUR_VERY_LOW:
		case Settings::BLUR_LOW:
			blur(m_ao_texture, m_blur_target, 0.0f, m_settings.blur == Settings::BLUR_LOW ? 4.0f : 2.0f, Vec2(0.0f));
			occlusion = m_blur_texture;
			break;
		case Settings::BLUR_MEDIUM:
		case Settings::BLUR_HIGH:
		{
			const float radius = m_settings.blur == Settings::BLUR_HIGH ? 6.0f : 3.0f;
			blur(m_ao_texture,   m_blur_target, 1.0f, radius, Vec2(1.0f, 0.0f));
			blur(m_blur_texture, m_ao_target,   1.0f, radius, Vec2(0.0f, 1.0f));
			occlusion = m_ao_texture;
		}
		break;
		case Settings::BLUR_OFF:
		default:
			break;
		}
		//4) up to the frame size, multiplied on the G-Buffer occlusion (alpha of GT3)
		auto apply = [&](Target* target, bool debug, const BlendState& blend)
		{
			draw_fullscreen(frame, target, m_shader_apply.get(), blend, [&](Resource::Shader& shader)
			{
				common_uniforms(shader);
				if (auto u = shader.uniform("ssao_debug"))     u->set(debug ? 1.0f : 0.0f);
				if (auto u = shader.uniform("g_ao"))           u->set(occlusion);
				if (auto u = shader.uniform("g_depth"))        u->set(m_depth_texture);
				if (auto u = shader.uniform("g_position"))     u->set(position);
				if (auto u = shader.uniform("ssao_size"))      u->set(occlusion_size);
				if (auto u = shader.uniform("ssao_sharpness")) u->set(m_settings.blur_sharpness);
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


	void SSAO::debug_options(std::vector<DebugOption>& options)
	{
		options.push_back(DebugOption::choice("Resolution", { "Full", "Half", "Quarter" },
			[this]() { return int(m_settings.resolution); },
			[this](int value) { m_settings.resolution = PostEffectResolution(value); }));
		options.push_back(DebugOption::choice("Blur", { "Off", "Very low", "Low", "Medium", "High" },
			[this]() { return int(m_settings.blur); },
			[this](int value) { m_settings.blur = Settings::BlurQuality(value); }));
		options.push_back(DebugOption::toggle("Occlusion only",
			[this]() { return m_settings.debug; },
			[this](bool value) { m_settings.debug = value; }));
	}
}
}
