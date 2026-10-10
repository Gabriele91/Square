//
//  PostEffectDOF.cpp
//  Square
//
//  See PostEffectDOF.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/Pipeline/GBuffer.h"
#include "Square/Render/Pipeline/DrawerPassDeferred.h"
#include "Square/Render/PostEffect/PostEffectDOF.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	DOF::DOF(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	DOF::~DOF()
	{
		on_release();
	}

	void DOF::on_release()
	{
		m_shader.reset();
		m_shader_composite.reset();
		m_shader_copy.reset();
		delete_color_target(m_blur_texture, m_blur_target);
		m_blur_size = IVec2(0, 0);
	}

	void DOF::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		if (!m_shader_copy) return;
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	void DOF::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		if (!m_shader) m_shader = load_shader("DOF");
		//forward (no G-Buffer), no shader: the frame as it is
		if (!frame.m_gbuffer || !frame.m_camera || !m_shader)
		{
			copy(frame);
			return;
		}
		if (m_settings.resolution == PER_FULL)
		{
			//one pass: the blur is the result
			blur(frame, frame.m_destination, frame.m_size);
		}
		else
		{
			if (!m_shader_composite) m_shader_composite = load_shader("DOFComposite");
			//its target, at its size
			const IVec2 size = post_effect_size(frame.m_size, m_settings.resolution);
			bool ready = m_blur_target && m_blur_size == size;
			if (!ready)
			{
				delete_color_target(m_blur_texture, m_blur_target);
				ready = create_color_target(size, TF_RGBA16F, m_blur_texture, m_blur_target);
				m_blur_size = ready ? size : IVec2(0, 0);
			}
			if (ready && m_shader_composite)
			{
				//the blur small, then over the frame by the circle of each of its pixels
				blur(frame, m_blur_target, size);
				Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
				draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
				{
					uniforms(shader, frame.m_size);
					if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
					if (auto u = shader.uniform("g_position")) u->set(position);
					if (auto u = shader.uniform("g_blur"))     u->set(m_blur_texture);
				});
			}
			else
			{
				//no target: at the size of the frame
				blur(frame, frame.m_destination, frame.m_size);
			}
		}
	}

	void DOF::blur(PostEffectFrame& frame, Target* target, const IVec2& size)
	{
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		PostEffectFrame pass = frame;
		pass.m_size = size;
		draw_fullscreen(pass, target, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			uniforms(shader, size);
			if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
			if (auto u = shader.uniform("g_position")) u->set(position);
		});
	}

	void DOF::uniforms(Resource::Shader& shader, const IVec2& size) const
	{
		//the radius and the motion in pixels of the pass (the settings: of a 1080p frame)
		const float pixels = float(size.y) / 1080.0f;
		const float radius = std::max(m_settings.max_radius, 0.0f) * pixels;
		const float motion = std::max(m_settings.motion, 0.0f) * pixels;
		const float half = std::max(m_settings.focus_range, 0.0f) * 0.5f;
		if (auto u = shader.uniform("dof_size"))   u->set(Vec2(float(size.x), float(size.y)));
		if (auto u = shader.uniform("dof_focus"))  u->set(Vec4(std::max(m_settings.focus_distance - half, 0.0f), m_settings.focus_distance + half,
		                                                        std::max(m_settings.near_range, 0.001f), std::max(m_settings.far_range, 0.001f)));
		if (auto u = shader.uniform("dof_params")) u->set(Vec4(radius, float(std::clamp(m_settings.samples, 4, 64)), m_settings.near ? 1.0f : 0.0f, 0.0f));
		if (auto u = shader.uniform("dof_motion")) u->set(Vec4(motion, m_settings.motion_from, std::max(m_settings.motion_to, m_settings.motion_from + 0.001f), 0.0f));
	}

	void DOF::debug_options(std::vector<DebugOption>& options)
	{
		options.push_back(DebugOption::choice("Resolution", { "Full", "Half", "Quarter" },
			[this]() { return int(m_settings.resolution); },
			[this](int value) { m_settings.resolution = PostEffectResolution(value); }));
	}
}
}
