//
//  PostEffectDOF.cpp
//  Square
//
//  See PostEffectDOF.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectDOF.h"
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
		m_shader_copy.reset();
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
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		//the radius in pixels of this frame (the settings: of a 1080p one)
		const float radius = std::max(m_settings.max_radius, 0.0f) * float(frame.m_size.y) / 1080.0f;
		const float half = std::max(m_settings.focus_range, 0.0f) * 0.5f;
		draw_fullscreen(frame, frame.m_destination, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))    u->set(frame.m_source);
			if (auto u = shader.uniform("g_position"))  u->set(position);
			if (auto u = shader.uniform("dof_size"))    u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("dof_focus"))   u->set(Vec4(std::max(m_settings.focus_distance - half, 0.0f), m_settings.focus_distance + half,
			                                                            std::max(m_settings.near_range, 0.001f), std::max(m_settings.far_range, 0.001f)));
			if (auto u = shader.uniform("dof_params"))  u->set(Vec4(radius, float(std::clamp(m_settings.samples, 4, 64)), m_settings.near ? 1.0f : 0.0f, 0.0f));
			if (auto u = shader.uniform("dof_motion"))  u->set(Vec4(std::max(m_settings.motion, 0.0f) * float(frame.m_size.y) / 1080.0f, m_settings.motion_from,
			                                                         std::max(m_settings.motion_to, m_settings.motion_from + 0.001f), 0.0f));
		});
	}
}
}
