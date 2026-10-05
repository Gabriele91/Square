//
//  PostEffectSnow.cpp
//  Square
//
//  See PostEffectSnow.h for the high level description.
//
#include <algorithm>
#include <cmath>
#include "Square/Core/Context.h"
#include "Square/Render/Camera.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/PostEffectSnow.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	Snow::Snow(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	Snow::~Snow()
	{
		on_release();
	}

	void Snow::on_release()
	{
		m_shader.reset();
		m_shader_copy.reset();
	}

	void Snow::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		if (!m_shader_copy) return;
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	float Snow::advance_time()
	{
		const Clock::time_point now = Clock::now();
		if (m_started) m_time += std::min(std::chrono::duration<float>(now - m_last).count(), 0.1f);
		m_last = now;
		m_started = true;
		//a period of the flakes far longer than a race, short enough for the precision of a float
		m_time = std::fmod(m_time, 3600.0f);
		return m_time;
	}

	void Snow::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shader (again after a release)
		if (!m_shader) m_shader = load_shader("Snow");
		if (!frame.m_camera || !m_shader)
		{
			copy(frame);
			return;
		}
		const float time = advance_time();
		//the view rays: the inverse of the view projection (a point of the screen back in the world)
		const Mat4& projection = frame.m_camera->projection();
		const Mat4  inverse_view_projection = inverse(projection * frame.m_camera->view());
		//the angle of a pixel (a flake at least that wide)
		const float fov_y = 2.0f * std::atan(1.0f / std::max(std::abs(projection[1][1]), 0.0001f));
		const float pixel_angle = frame.m_size.y > 0 ? 2.0f * std::tan(fov_y * 0.5f) / float(frame.m_size.y) : 0.001f;
		Texture* position = frame.m_gbuffer ? frame.m_gbuffer->texture(DrawerPassDeferred::GB_POSITION) : nullptr;
		draw_fullscreen(frame, frame.m_destination, m_shader.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))     u->set(frame.m_source);
			if (auto u = shader.uniform("g_position"); u && position) u->set(position);
			if (auto u = shader.uniform("snow_inverse")) u->set(inverse_view_projection);
			if (auto u = shader.uniform("snow_size"))    u->set(Vec2(float(frame.m_size.x), float(frame.m_size.y)));
			if (auto u = shader.uniform("snow_time"))    u->set(time);
			if (auto u = shader.uniform("snow_params"))  u->set(Vec4(std::clamp(m_settings.density, 0.0f, 1.0f), std::max(m_settings.spacing, 0.1f), std::max(m_settings.near_distance, 0.0f), std::max(m_settings.max_distance, 1.0f)));
			if (auto u = shader.uniform("snow_motion"))  u->set(Vec4(m_settings.speed, std::max(m_settings.size, 0.001f), m_settings.wind.x, m_settings.wind.y));
			if (auto u = shader.uniform("snow_color"))   u->set(Vec4(m_settings.color, std::clamp(m_settings.intensity, 0.0f, 1.0f)));
			if (auto u = shader.uniform("snow_extra"))   u->set(Vec4(frame.m_gbuffer ? 1.0f : 0.0f, pixel_angle, std::max(m_settings.shutter, 0.0f), std::max(m_settings.focus_distance, 0.01f)));
		});
	}
}
}
