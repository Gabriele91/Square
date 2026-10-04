//
//  PostEffectFog.cpp
//  Square
//
//  See PostEffectFog.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectFog.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	Fog::Fog(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	Fog::~Fog()
	{
		on_release();
	}

	void Fog::on_release()
	{
		m_shader.reset();
		m_shader_copy.reset();
	}

	void Fog::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		if (!m_shader_copy) return;
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	void Fog::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shader (again after a release)
		if (!m_shader) m_shader = load_shader("Fog");
		//forward (no G-Buffer), no shader: the frame as it is
		if (!frame.m_gbuffer || !frame.m_camera || !m_shader)
		{
			copy(frame);
			return;
		}
		Texture* position = frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION);
		//the sun: toward it, a direction (none: straight down)
		const float sun_length = length(m_settings.sun_direction);
		const Vec3  sun_direction = sun_length > 0.0001f ? m_settings.sun_direction / sun_length : Vec3(0.0f, -1.0f, 0.0f);
		draw_fullscreen(frame, frame.m_destination, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))          u->set(frame.m_source);
			if (auto u = shader.uniform("g_position"))        u->set(position);
			if (auto u = shader.uniform("fog_size"))          u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("fog_color"))         u->set(Vec4(m_settings.color, std::clamp(m_settings.sky, 0.0f, 1.0f)));
			if (auto u = shader.uniform("fog_params"))        u->set(Vec4(std::max(m_settings.density, 0.0f), m_settings.height, std::max(m_settings.falloff, 0.0f), std::max(m_settings.start, 0.0f)));
			if (auto u = shader.uniform("fog_sun_color"))     u->set(Vec4(m_settings.sun_color, std::max(m_settings.sun_exponent, 1.0f)));
			if (auto u = shader.uniform("fog_sun_direction")) u->set(Vec4(sun_direction, std::clamp(m_settings.max_opacity, 0.0f, 1.0f)));
		});
	}
}
}
