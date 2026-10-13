//
//  PostEffectBloom.cpp
//  Square
//
//  See PostEffectBloom.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Render/PostEffect/PostEffectBloom.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	Bloom::Bloom(Square::Context& context)
	: PostEffect(context, PES_COLOR)
	{
	}

	Bloom::~Bloom()
	{
		on_release();
	}

	void Bloom::on_release()
	{
		release_levels();
		m_shader_prefilter.reset();
		m_shader_downsample.reset();
		m_shader_upsample.reset();
		m_shader_composite.reset();
		m_shader_copy.reset();
	}

	void Bloom::copy(PostEffectFrame& frame)
	{
		if (!m_shader_copy) m_shader_copy = load_shader("PostCopy");
		draw_fullscreen(frame, frame.m_destination, m_shader_copy.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source")) u->set(frame.m_source);
		});
	}

	void Bloom::release_levels()
	{
		for (Level& level : m_levels)
		{
			delete_color_target(level.m_down_texture, level.m_down_target);
			delete_color_target(level.m_up_texture, level.m_up_target);
		}
		m_levels.clear();
		m_size = IVec2(0, 0);
		m_levels_count = 0;
	}

	bool Bloom::build_levels(const IVec2& size)
	{
		if (m_size == size && m_levels_count == m_settings.levels && !m_levels.empty()) return true;
		release_levels();
		//each level half the one before, while it is at least 2x2
		IVec2 level_size = glm::max(size / 2, IVec2(1, 1));
		for (int i = 0; i < m_settings.levels && level_size.x >= 2 && level_size.y >= 2; ++i)
		{
			Level level;
			level.m_size = level_size;
			if (!create_color_target(level_size, TF_RGBA16F, level.m_down_texture, level.m_down_target)
			||  !create_color_target(level_size, TF_RGBA16F, level.m_up_texture, level.m_up_target))
			{
				m_levels.push_back(level);
				release_levels();
				return false;
			}
			m_levels.push_back(level);
			level_size = glm::max(level_size / 2, IVec2(1, 1));
		}
		m_size = size;
		m_levels_count = m_settings.levels;
		return !m_levels.empty();
	}

	void Bloom::draw(PostEffectFrame& frame)
	{
		if (!frame.m_source || !frame.m_destination) return;
		//shaders (again after a release)
		if (!m_shader_prefilter)  m_shader_prefilter  = load_shader("BloomPrefilter");
		if (!m_shader_downsample) m_shader_downsample = load_shader("BloomDownsample");
		if (!m_shader_upsample)   m_shader_upsample   = load_shader("BloomUpsample");
		if (!m_shader_composite)  m_shader_composite  = load_shader("BloomComposite");
		if (!m_shader_prefilter || !m_shader_downsample || !m_shader_upsample || !m_shader_composite || !build_levels(frame.m_size))
		{
			copy(frame);
			return;
		}
		//the frame of a level: its size (draw_fullscreen sets the viewport from it)
		auto level_frame = [&](const Level& level)
		{
			PostEffectFrame level_frame = frame;
			level_frame.m_size = level.m_size;
			return level_frame;
		};
		auto size_of = [](const IVec2& size) { return Vec2(float(size.x), float(size.y)); };
		//1) prefilter: the frame to the first level, the light over the threshold
		{
			PostEffectFrame first = level_frame(m_levels[0]);
			draw_fullscreen(first, m_levels[0].m_down_target, m_shader_prefilter.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))        u->set(frame.m_source);
				if (auto u = shader.uniform("bloom_size"))      u->set(size_of(m_levels[0].m_size));
				if (auto u = shader.uniform("bloom_threshold")) u->set(Vec4(m_settings.threshold, std::max(m_settings.knee, 0.0f), m_settings.clamp, 0.0f));
			});
		}
		//2) downsample: every level from the one before
		for (size_t i = 1; i < m_levels.size(); ++i)
		{
			PostEffectFrame current = level_frame(m_levels[i]);
			draw_fullscreen(current, m_levels[i].m_down_target, m_shader_downsample.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))   u->set(m_levels[i - 1].m_down_texture);
				if (auto u = shader.uniform("bloom_size")) u->set(size_of(m_levels[i].m_size));
			});
		}
		//3) upsample: from the last level back to the first; the last one is only downsampled
		Texture* lower = m_levels.back().m_down_texture;
		for (size_t i = m_levels.size() - 1; i-- > 0;)
		{
			PostEffectFrame current = level_frame(m_levels[i]);
			draw_fullscreen(current, m_levels[i].m_up_target, m_shader_upsample.get(), BlendState(), [&](Resource::Shader& shader)
			{
				if (auto u = shader.uniform("g_source"))      u->set(m_levels[i].m_down_texture);
				if (auto u = shader.uniform("g_lower"))       u->set(lower);
				if (auto u = shader.uniform("bloom_size"))    u->set(size_of(m_levels[i].m_size));
				if (auto u = shader.uniform("bloom_scatter")) u->set(std::clamp(m_settings.scatter, 0.0f, 1.0f));
			});
			lower = m_levels[i].m_up_texture;
		}
		//4) composite: the frame plus the bloom (the first level, upsampled)
		draw_fullscreen(frame, frame.m_destination, m_shader_composite.get(), BlendState(), [&](Resource::Shader& shader)
		{
			if (auto u = shader.uniform("g_source"))        u->set(frame.m_source);
			if (auto u = shader.uniform("g_bloom"))         u->set(lower);
			if (auto u = shader.uniform("bloom_size"))      u->set(size_of(frame.m_size));
			if (auto u = shader.uniform("bloom_intensity")) u->set(m_settings.intensity);
			if (auto u = shader.uniform("bloom_debug"))     u->set(m_settings.debug ? 1.0f : 0.0f);
		});
	}

	void Bloom::debug_options(std::vector<DebugOption>& options)
	{
		options.push_back(DebugOption::toggle("Bloom only",
			[this]() { return m_settings.debug; },
			[this](bool value) { m_settings.debug = value; }));
	}
}
}
