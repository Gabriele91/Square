//
//  PostEffectFXAA.cpp
//  Square
//
//  See PostEffectFXAA.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/PostEffectFXAA.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	FXAA::FXAA(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	FXAA::~FXAA()
	{
		on_release();
	}

	void FXAA::on_release()
	{
		m_shader.reset();
	}

	void FXAA::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		if (!m_shader) m_shader = load_shader("FXAA");
		if (!m_shader) return;
		draw_fullscreen(frame, frame.m_destination, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))    u->set(frame.m_source);
			if (auto u = shader.uniform("fxaa_size"))   u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("fxaa_params")) u->set(Vec4(std::max(m_settings.edge_threshold, 0.0f), std::max(m_settings.edge_threshold_min, 0.0f),
			                                                        std::max(m_settings.span_max, 1.0f), 0.0f));
		});
	}
}
}
