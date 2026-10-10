//
//  PostEffectMotionBlur.cpp
//  Square
//
//  See PostEffectMotionBlur.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/PostEffect/PostEffectMotionBlur.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	MotionBlur::MotionBlur(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	MotionBlur::~MotionBlur()
	{
		on_release();
	}

	void MotionBlur::on_release()
	{
		m_shader.reset();
		m_shader_copy.reset();
	}

	void MotionBlur::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		if (!m_shader_copy) return;
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	void MotionBlur::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		if (!m_shader) m_shader = load_shader("MotionBlur");
		//no velocity (nothing with its motion blur on, forward), no shader: the frame as it is
		if (!frame.m_velocity || !m_shader)
		{
			copy(frame);
			return;
		}
		//the pixels of this frame (the settings: of a 1080p one)
		const float max_pixels = std::max(m_settings.max_pixels, 0.0f) * float(frame.m_size.y) / 1080.0f;
		draw_fullscreen(frame, frame.m_destination, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))   u->set(frame.m_source);
			if (auto u = shader.uniform("g_velocity")) u->set(frame.m_velocity);
			if (auto u = shader.uniform("mb_size"))    u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("mb_params"))  u->set(Vec4(std::max(m_settings.shutter, 0.0f), max_pixels, std::max(m_settings.min_pixels, 0.0f),
			                                                       float(std::clamp(m_settings.samples, 2, 32))));
		});
	}
}
}
