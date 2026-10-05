//
//  SnowTrails.cpp
//  Rush
//
//  See SnowTrails.h.
//
#include <algorithm>
#include <cmath>
#include <SnowTrails.h>

namespace AuxSnowTrails
{
	//a jump from a point to the next one longer than this (world units): a respawn, no groove
	static constexpr float s_max_step = 8.0f;
}

SnowTrails::SnowTrails(Square::Context& context)
: m_context(context)
{
}

bool SnowTrails::create(const Settings& settings)
{
	using namespace Square;
	m_settings = settings;
	const size_t side = size_t(std::max(m_settings.resolution, 2));
	m_map.assign(side * side, 0);
	m_tracks.clear();
	m_fill = 0.0f;
	m_changed = false;
	//linear (the sides of the grooves smooth), no mipmaps (updated every frame), clamped
	m_texture = MakeShared<Resource::Texture>(m_context);
	const Resource::Texture::Attributes attributes{ Render::TMIN_LINEAR, Render::TMAG_LINEAR, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, false, 1 };
	if (!m_texture->build(attributes, m_map.data(), (unsigned long)side, (unsigned long)side, Render::TF_R8, Render::TT_R))
	{
		m_texture.reset();
		return false;
	}
	return true;
}

void SnowTrails::attach(Square::Shared<Square::Scene::Actor> actor) const
{
	using namespace Square;
	if (!m_texture || !actor) return;
	//the corner of the map, 1 / its size: the uv of a world point (PBR.hlsl, SURFACE_TRAILS)
	const float half = m_settings.size * 0.5f;
	const Vec4  area(m_settings.center.x - half, m_settings.center.y - half, 1.0f / m_settings.size, 1.0f / m_settings.size);
	auto texture = m_texture;
	actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		for (auto& material : node->component<Scene::StaticMesh>()->m_materials)
		{
			if (!material) continue;
			auto map = material->parameter_by_name("trail_map");
			auto where = material->parameter_by_name("trail_area");
			if (!map || !where) continue;
			map->set(texture);
			where->set(area);
		}
		return true;
	});
}

void SnowTrails::press(size_t id, const Square::Vec3& position, bool on_ground)
{
	using namespace Square;
	if (m_map.empty()) return;
	if (id >= m_tracks.size()) m_tracks.resize(id + 1);
	Track& track = m_tracks[id];
	const Vec2 point(position.x, position.z);
	const bool near = length(point - track.m_previous) <= AuxSnowTrails::s_max_step;
	//a groove only on the ground, from where it was on the ground (not across a jump)
	if (on_ground && track.m_on_ground && near) groove(track.m_previous, point);
	track.m_previous = point;
	track.m_on_ground = on_ground;
}

void SnowTrails::update(double delta_time)
{
	//the snow fills the grooves back: levels per second of the deepest one in refill seconds
	if (m_settings.refill > 0.0f) m_fill += float(delta_time) * 255.0f / m_settings.refill;
	const int levels = int(m_fill);
	if (levels > 0)
	{
		m_fill -= float(levels);
		for (unsigned char& texel : m_map)
		{
			if (!texel) continue;
			texel = (unsigned char)std::max(int(texel) - levels, 0);
			m_changed = true;
		}
	}
	if (m_changed && m_texture) m_texture->update(m_map.data());
	m_changed = false;
}

Square::Vec2 SnowTrails::to_texel(const Square::Vec2& point) const
{
	const float half = m_settings.size * 0.5f;
	return (point - m_settings.center + Square::Vec2(half)) / m_settings.size * float(m_settings.resolution);
}

void SnowTrails::groove(const Square::Vec2& from, const Square::Vec2& to)
{
	using namespace Square;
	//a stamp every half texel along the way (the groove without gaps)
	const Vec2  a = to_texel(from);
	const Vec2  b = to_texel(to);
	const float texels = length(b - a);
	const int   steps = std::max(int(std::ceil(texels * 2.0f)), 1);
	for (int step = 1; step <= steps; ++step)
	{
		stamp(a + (b - a) * (float(step) / float(steps)));
	}
}

void SnowTrails::stamp(const Square::Vec2& texel)
{
	using namespace Square;
	const int   side = m_settings.resolution;
	const float radius = m_settings.radius / m_settings.size * float(side);
	const int   x0 = std::max(int(std::floor(texel.x - radius)), 0);
	const int   x1 = std::min(int(std::ceil(texel.x + radius)), side - 1);
	const int   y0 = std::max(int(std::floor(texel.y - radius)), 0);
	const int   y1 = std::min(int(std::ceil(texel.y + radius)), side - 1);
	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			//round: deepest in the middle, 0 at the radius (the snow pressed by a ball)
			const float d = length(Vec2(float(x) + 0.5f, float(y) + 0.5f) - texel) / radius;
			if (d >= 1.0f) continue;
			const int depth = int(255.0f * m_settings.depth * (1.0f - d * d));
			unsigned char& value = m_map[size_t(y) * size_t(side) + size_t(x)];
			if (depth <= int(value)) continue;
			value = (unsigned char)depth;
			m_changed = true;
		}
	}
}
