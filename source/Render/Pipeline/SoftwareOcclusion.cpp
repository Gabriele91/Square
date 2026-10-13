//
//  SoftwareOcclusion.cpp
//  Square
//
//  See SoftwareOcclusion.h for the high level description.
//
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Occluder.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Geometry/Intersection.h"
#include "Square/Render/Pipeline/SoftwareOcclusion.h"

//the vector instructions of the raster (CMake SQUARE_SIMD, or what the compiler targets): AVX (8
//pixels at once), SSE 4.1 (4), NEON (4); none: the scalar raster
#if defined(SQUARE_SIMD_AVX) || defined(__AVX__)
	#define SQUARE_OCCLUSION_AVX
	#include <immintrin.h>
#elif defined(SQUARE_SIMD_SSE4) || defined(__SSE4_1__)
	#define SQUARE_OCCLUSION_SSE4
	#include <smmintrin.h>
#elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64)
	#define SQUARE_OCCLUSION_NEON
	#include <arm_neon.h>
#endif

namespace Square
{
namespace Render
{
	namespace AuxSoftwareOcclusion
	{
		//the lanes of a vector of floats: some pixels of a row at once (the same code for every
		//instruction set: raster_row)
#if defined(SQUARE_OCCLUSION_AVX)
		struct Lanes
		{
			static constexpr int count = 8;
			using Float = __m256;
			static Float set(float value) { return _mm256_set1_ps(value); }
			static Float ramp() { return _mm256_setr_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f); }
			static Float add(Float a, Float b) { return _mm256_add_ps(a, b); }
			static Float mul(Float a, Float b) { return _mm256_mul_ps(a, b); }
			static Float max(Float a, Float b) { return _mm256_max_ps(a, b); }
			//a mask: every bit of a lane where it is >= 0
			static Float positive(Float a) { return _mm256_cmp_ps(a, _mm256_setzero_ps(), _CMP_GE_OQ); }
			static Float both(Float a, Float b) { return _mm256_and_ps(a, b); }
			//where mask: when, else otherwise
			static Float select(Float mask, Float when, Float otherwise) { return _mm256_blendv_ps(otherwise, when, mask); }
			static bool none(Float mask) { return _mm256_movemask_ps(mask) == 0; }
			static Float load(const float* at) { return _mm256_loadu_ps(at); }
			static void store(float* at, Float value) { _mm256_storeu_ps(at, value); }
		};
#elif defined(SQUARE_OCCLUSION_SSE4)
		struct Lanes
		{
			static constexpr int count = 4;
			using Float = __m128;
			static Float set(float value) { return _mm_set1_ps(value); }
			static Float ramp() { return _mm_setr_ps(0.0f, 1.0f, 2.0f, 3.0f); }
			static Float add(Float a, Float b) { return _mm_add_ps(a, b); }
			static Float mul(Float a, Float b) { return _mm_mul_ps(a, b); }
			static Float max(Float a, Float b) { return _mm_max_ps(a, b); }
			static Float positive(Float a) { return _mm_cmpge_ps(a, _mm_setzero_ps()); }
			static Float both(Float a, Float b) { return _mm_and_ps(a, b); }
			static Float select(Float mask, Float when, Float otherwise) { return _mm_blendv_ps(otherwise, when, mask); }
			static bool none(Float mask) { return _mm_movemask_ps(mask) == 0; }
			static Float load(const float* at) { return _mm_loadu_ps(at); }
			static void store(float* at, Float value) { _mm_storeu_ps(at, value); }
		};
#elif defined(SQUARE_OCCLUSION_NEON)
		struct Lanes
		{
			static constexpr int count = 4;
			using Float = float32x4_t;
			static Float set(float value) { return vdupq_n_f32(value); }
			static Float ramp() { const float values[4]{ 0.0f, 1.0f, 2.0f, 3.0f }; return vld1q_f32(values); }
			static Float add(Float a, Float b) { return vaddq_f32(a, b); }
			static Float mul(Float a, Float b) { return vmulq_f32(a, b); }
			static Float max(Float a, Float b) { return vmaxq_f32(a, b); }
			static Float positive(Float a) { return vreinterpretq_f32_u32(vcgeq_f32(a, vdupq_n_f32(0.0f))); }
			static Float both(Float a, Float b) { return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b))); }
			static Float select(Float mask, Float when, Float otherwise) { return vbslq_f32(vreinterpretq_u32_f32(mask), when, otherwise); }
			static bool none(Float mask)
			{
				const uint32x4_t bits = vreinterpretq_u32_f32(mask);
				const uint32x2_t half = vorr_u32(vget_low_u32(bits), vget_high_u32(bits));
				return (vget_lane_u32(half, 0) | vget_lane_u32(half, 1)) == 0;
			}
			static Float load(const float* at) { return vld1q_f32(at); }
			static void store(float* at, Float value) { vst1q_f32(at, value); }
		};
#endif

		//an edge of a triangle along a row: its value at the center of a pixel x is
		//m_at_zero + m_step * (x + 0.5), its sign by the winding (inside: >= 0)
		struct RowEdge
		{
			float m_at_zero{ 0.0f };
			float m_step{ 0.0f };
		};

		//the edge from a to b on the row of centers py, by the winding (sign: 1 or -1)
		static RowEdge row_edge(const Vec3& a, const Vec3& b, float py, float sign)
		{
			//edge(a, b, px, py) = (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x)
			RowEdge out;
			out.m_at_zero = ((b.x - a.x) * (py - a.y) + (b.y - a.y) * a.x) * sign;
			out.m_step = -(b.y - a.y) * sign;
			return out;
		}

		//the pixels of a row from low_x to high_x inside the three edges: the inverse of their depth
		//(the depths of the vertices by the edges, by scale) kept where it is nearer. Some pixels
		//at once (Lanes), the rest one by one
		static void raster_row(float* row, int low_x, int high_x, const RowEdge (&edges)[3], const Vec3& depths, float scale)
		{
			int x = low_x;
#if defined(SQUARE_OCCLUSION_AVX) || defined(SQUARE_OCCLUSION_SSE4) || defined(SQUARE_OCCLUSION_NEON)
			const Lanes::Float ramp = Lanes::ramp();
			const Lanes::Float at0 = Lanes::set(edges[0].m_at_zero), step0 = Lanes::set(edges[0].m_step);
			const Lanes::Float at1 = Lanes::set(edges[1].m_at_zero), step1 = Lanes::set(edges[1].m_step);
			const Lanes::Float at2 = Lanes::set(edges[2].m_at_zero), step2 = Lanes::set(edges[2].m_step);
			const Lanes::Float z0 = Lanes::set(depths.x), z1 = Lanes::set(depths.y), z2 = Lanes::set(depths.z);
			const Lanes::Float factor = Lanes::set(scale);
			for (; x + Lanes::count - 1 <= high_x; x += Lanes::count)
			{
				const Lanes::Float px = Lanes::add(Lanes::set(float(x) + 0.5f), ramp);
				const Lanes::Float w0 = Lanes::add(at0, Lanes::mul(step0, px));
				const Lanes::Float w1 = Lanes::add(at1, Lanes::mul(step1, px));
				const Lanes::Float w2 = Lanes::add(at2, Lanes::mul(step2, px));
				const Lanes::Float inside = Lanes::both(Lanes::both(Lanes::positive(w0), Lanes::positive(w1)), Lanes::positive(w2));
				if (!Lanes::none(inside))
				{
					const Lanes::Float sum = Lanes::add(Lanes::add(Lanes::mul(w0, z0), Lanes::mul(w1, z1)), Lanes::mul(w2, z2));
					const Lanes::Float value = Lanes::mul(sum, factor);
					const Lanes::Float old = Lanes::load(row + x);
					Lanes::store(row + x, Lanes::select(inside, Lanes::max(old, value), old));
				}
			}
#endif
			for (; x <= high_x; ++x)
			{
				const float px = float(x) + 0.5f;
				const float w0 = edges[0].m_at_zero + edges[0].m_step * px;
				const float w1 = edges[1].m_at_zero + edges[1].m_step * px;
				const float w2 = edges[2].m_at_zero + edges[2].m_step * px;
				if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
				{
					const float value = (w0 * depths.x + w1 * depths.y + w2 * depths.z) * scale;
					row[x] = std::max(row[x], value);
				}
			}
		}
		//a point of clip space on the depth buffer: x, y its pixels (y down), z the inverse of
		//its depth along the view
		static Vec3 to_screen(const Vec4& clip, const IVec2& size)
		{
			const float inverse_w = 1.0f / clip.w;
			return Vec3
			(
				  (clip.x * inverse_w * 0.5f + 0.5f) * float(size.x)
				, (0.5f - clip.y * inverse_w * 0.5f) * float(size.y)
				, inverse_w
			);
		}

		//twice the signed area of a, b, p (which side of a -> b p is)
		static float edge(const Vec3& a, const Vec3& b, float x, float y)
		{
			return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
		}

		//the level where a rectangle of pixels (level 0) is at most 4 texels across
		static size_t level_of(const IVec2& low, const IVec2& high, size_t levels)
		{
			size_t level = 0;
			while (level + 1 < levels && (((high.x >> level) - (low.x >> level)) > 3 || ((high.y >> level) - (low.y >> level)) > 3))
			{
				++level;
			}
			return level;
		}
	}

	const char* SoftwareOcclusion::instructions()
	{
#if defined(SQUARE_OCCLUSION_AVX)
		return "AVX";
#elif defined(SQUARE_OCCLUSION_SSE4)
		return "SSE4.1";
#elif defined(SQUARE_OCCLUSION_NEON)
		return "NEON";
#else
		return "scalar";
#endif
	}

	void SoftwareOcclusion::draw(const Camera& camera, const std::vector< Weak<Occluder> >& occluders)
	{
		++m_version;
		m_active = false;
		m_stats = Stats();
		//only a perspective camera (its w: the depth along the view)
		const Mat4& projection = camera.projection();
		const bool perspective = projection[2][3] != 0.0f;
		if (m_settings.enabled && perspective && !occluders.empty())
		{
			//the depth buffer at the aspect of the camera, cleared (0: nothing)
			const int width = std::clamp(m_settings.width, 16, 2048);
			const float aspect = std::max(camera.viewport().aspect(), 0.01f);
			m_size = IVec2(width, std::max(int(std::round(float(width) / aspect)), 1));
			m_levels.resize(1);
			m_levels[0].assign(size_t(m_size.x) * size_t(m_size.y), 0.0f);
			m_view_projection = projection * camera.view();
			m_near = std::max(camera.viewport().near_and_far().x, 0.001f);
			//the occluders it sees
			const Geometry::Frustum& frustum = camera.frustum();
			for (const Weak<Occluder>& weak_occluder : occluders)
			{
				auto occluder = weak_occluder.lock();
				const bool seen = occluder
				               && occluder->can_occlude()
				               && Geometry::Intersection::check(frustum, occluder->occluder_box()) != Geometry::Intersection::OUTSIDE;
				if (seen)
				{
					const std::vector<Vec3>& triangles = occluder->occluder_triangles();
					for (size_t i = 0; i + 2 < triangles.size(); i += 3)
					{
						draw_triangle
						(
							  m_view_projection * Vec4(triangles[i], 1.0f)
							, m_view_projection * Vec4(triangles[i + 1], 1.0f)
							, m_view_projection * Vec4(triangles[i + 2], 1.0f)
						);
					}
					m_stats.m_occluders += 1;
					m_stats.m_triangles += triangles.size() / 3;
				}
			}
			build_levels();
			m_active = m_stats.m_triangles > 0;
		}
	}

	void SoftwareOcclusion::draw_triangle(const Vec4& a, const Vec4& b, const Vec4& c)
	{
		//cut by the near plane (w >= near): nothing, a triangle or a quad
		const std::array<Vec4, 3> points{ a, b, c };
		std::array<Vec4, 4> kept;
		size_t count = 0;
		for (size_t i = 0; i != points.size(); ++i)
		{
			const Vec4& current = points[i];
			const Vec4& next = points[(i + 1) % points.size()];
			const bool current_in = current.w >= m_near;
			const bool next_in = next.w >= m_near;
			if (current_in)
			{
				kept[count++] = current;
			}
			if (current_in != next_in)
			{
				const float t = (m_near - current.w) / (next.w - current.w);
				kept[count++] = current + (next - current) * t;
			}
		}
		if (count >= 3)
		{
			raster_triangle(kept[0], kept[1], kept[2]);
		}
		if (count == 4)
		{
			raster_triangle(kept[0], kept[2], kept[3]);
		}
	}

	void SoftwareOcclusion::raster_triangle(const Vec4& a, const Vec4& b, const Vec4& c)
	{
		using namespace AuxSoftwareOcclusion;
		const Vec3 p0 = to_screen(a, m_size);
		const Vec3 p1 = to_screen(b, m_size);
		const Vec3 p2 = to_screen(c, m_size);
		const float area = edge(p0, p1, p2.x, p2.y);
		//its pixels (their centers inside it, either winding: both faces hide)
		const int low_x = std::max(int(std::floor(std::min({ p0.x, p1.x, p2.x }))), 0);
		const int low_y = std::max(int(std::floor(std::min({ p0.y, p1.y, p2.y }))), 0);
		const int high_x = std::min(int(std::floor(std::max({ p0.x, p1.x, p2.x }))), m_size.x - 1);
		const int high_y = std::min(int(std::floor(std::max({ p0.y, p1.y, p2.y }))), m_size.y - 1);
		if (std::abs(area) > 1e-8f && low_x <= high_x && low_y <= high_y)
		{
			//the edges by the winding (inside: >= 0 either way); the inverse of the depth linear on
			//the screen: the depths of the vertices by the edges, by the sign over the area
			const float sign = area > 0.0f ? 1.0f : -1.0f;
			const float scale = sign / area;
			const Vec3 depths(p0.z, p1.z, p2.z);
			std::vector<float>& depth = m_levels[0];
			for (int y = low_y; y <= high_y; ++y)
			{
				const float py = float(y) + 0.5f;
				const RowEdge edges[3]
				{
					  row_edge(p1, p2, py, sign)
					, row_edge(p2, p0, py, sign)
					, row_edge(p0, p1, py, sign)
				};
				raster_row(depth.data() + size_t(y) * size_t(m_size.x), low_x, high_x, edges, depths, scale);
			}
		}
	}

	void SoftwareOcclusion::build_levels()
	{
		//each level half the one before, each texel the farthest (the smallest inverse) of the
		//four under it, down to 1 x 1
		m_level_sizes.assign(1, m_size);
		m_levels.resize(1);
		IVec2 size = m_size;
		while (size.x > 1 || size.y > 1)
		{
			const IVec2 next_size(std::max((size.x + 1) / 2, 1), std::max((size.y + 1) / 2, 1));
			std::vector<float> next(size_t(next_size.x) * size_t(next_size.y), 0.0f);
			const std::vector<float>& source = m_levels.back();
			for (int y = 0; y != next_size.y; ++y)
			{
				for (int x = 0; x != next_size.x; ++x)
				{
					const int x0 = std::min(x * 2, size.x - 1), x1 = std::min(x * 2 + 1, size.x - 1);
					const int y0 = std::min(y * 2, size.y - 1), y1 = std::min(y * 2 + 1, size.y - 1);
					const float farthest = std::min
					({
						  source[size_t(y0) * size_t(size.x) + size_t(x0)]
						, source[size_t(y0) * size_t(size.x) + size_t(x1)]
						, source[size_t(y1) * size_t(size.x) + size_t(x0)]
						, source[size_t(y1) * size_t(size.x) + size_t(x1)]
					});
					next[size_t(y) * size_t(next_size.x) + size_t(x)] = farthest;
				}
			}
			m_levels.push_back(std::move(next));
			m_level_sizes.push_back(next_size);
			size = next_size;
		}
	}

	bool SoftwareOcclusion::hidden(const Geometry::AABoundingBox& box, bool counted) const
	{
		using namespace AuxSoftwareOcclusion;
		bool hidden = false;
		if (m_active)
		{
			if (counted)
			{
				m_stats.m_tested += 1;
			}
			//its corners on the screen: its rectangle, its nearest point (the largest inverse);
			//a corner over the near plane: in front of the camera, seen
			const Vec3& low = box.get_min();
			const Vec3& high = box.get_max();
			bool in_front = true;
			Vec2 screen_low(std::numeric_limits<float>::max());
			Vec2 screen_high(std::numeric_limits<float>::lowest());
			float nearest = 0.0f;
			for (int corner = 0; corner != 8 && in_front; ++corner)
			{
				const Vec3 point
				(
					  (corner & 1) ? high.x : low.x
					, (corner & 2) ? high.y : low.y
					, (corner & 4) ? high.z : low.z
				);
				const Vec4 clip = m_view_projection * Vec4(point, 1.0f);
				in_front = clip.w >= m_near;
				if (in_front)
				{
					const Vec3 screen = to_screen(clip, m_size);
					screen_low = glm::min(screen_low, Vec2(screen));
					screen_high = glm::max(screen_high, Vec2(screen));
					nearest = std::max(nearest, screen.z);
				}
			}
			//its texels (a pixel more around it: the borders of the pixels), on the screen
			const IVec2 pixel_low(int(std::floor(screen_low.x)) - 1, int(std::floor(screen_low.y)) - 1);
			const IVec2 pixel_high(int(std::floor(screen_high.x)) + 1, int(std::floor(screen_high.y)) + 1);
			const bool on_screen = in_front
			                    && pixel_high.x >= 0 && pixel_high.y >= 0
			                    && pixel_low.x < m_size.x && pixel_low.y < m_size.y;
			if (on_screen)
			{
				const IVec2 clamped_low = glm::max(pixel_low, IVec2(0, 0));
				const IVec2 clamped_high = glm::min(pixel_high, m_size - IVec2(1, 1));
				//hidden while every texel is nearer than it (a farther one or nothing: seen)
				const size_t level = level_of(clamped_low, clamped_high, m_levels.size());
				const IVec2& level_size = m_level_sizes[level];
				const std::vector<float>& texels = m_levels[level];
				hidden = true;
				for (int y = (clamped_low.y >> level); y <= (clamped_high.y >> level) && hidden; ++y)
				{
					for (int x = (clamped_low.x >> level); x <= (clamped_high.x >> level) && hidden; ++x)
					{
						hidden = texels[size_t(y) * size_t(level_size.x) + size_t(x)] > nearest;
					}
				}
			}
			if (hidden && counted)
			{
				m_stats.m_hidden += 1;
			}
		}
		return hidden;
	}

	bool SoftwareOcclusion::hidden(const Geometry::Sphere& sphere) const
	{
		const Vec3 radius(sphere.get_radius());
		const bool hide = hidden(Geometry::AABoundingBox(sphere.get_position() - radius, sphere.get_position() + radius), false);
		if (m_active)
		{
			m_stats.m_instances_tested += 1;
			if (hide)
			{
				m_stats.m_instances_hidden += 1;
			}
		}
		return hide;
	}
}
}
