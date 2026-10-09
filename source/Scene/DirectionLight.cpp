//
//  PointLight.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include <array>
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Math/Transformation.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/DirectionLight.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"

namespace Square
{
	using CSMMatrixArray   = std::array<Mat4,  DIRECTION_SHADOW_CSM_NUMBER_OF_FACES>;
	using CSMCascadeSplits = std::array<float, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES>;
	using CSMCascadeDepth  = std::array<float, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES + 1>;
}

namespace Square
{
namespace Scene
{
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(DirectionLight);
    
    //Registration in context
    void DirectionLight::object_registration(Context& ctx)
    {
        //factory
        ctx.add_object<DirectionLight>();
        //Attributes
		ctx.add_attribute_function<DirectionLight, bool>
		("visible"
		, bool(0)
		, [](const DirectionLight* plight) -> bool       { return plight->visible(); }
		, [](DirectionLight* plight, const bool& visible){ plight->visible(visible); });

		ctx.add_attribute_function<DirectionLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](DirectionLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); });

		ctx.add_attribute_function<DirectionLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3        { return plight->specular(); }
		, [](DirectionLight* plight, const Vec3& specular){ plight->specular(specular); });
		
		ctx.add_attribute_function<DirectionLight, IVec2>
		("shadow"
		, IVec2(0)
		, [](const DirectionLight* plight) -> IVec2 { return plight->shadow_size(); }
		, [](DirectionLight* plight, const IVec2& shadow_size)  { plight->shadow(shadow_size);  });

		ctx.add_attribute_function<DirectionLight, int>
		("cascades"
		, int(DIRECTION_SHADOW_CSM_DEFAULT_FACES)
		, [](const DirectionLight* plight) -> int      { return plight->cascades(); }
		, [](DirectionLight* plight, const int& cascades){ plight->cascades(cascades); });

		//(in the binary archives after cascades: the scenes converted before it must be converted again)
		ctx.add_attribute_function<DirectionLight, float>
		("shadow_distance"
		, float(0.0f)
		, [](const DirectionLight* plight) -> float         { return plight->shadow_distance(); }
		, [](DirectionLight* plight, const float& distance){ plight->shadow_distance(distance); });

		ctx.add_attribute_function<DirectionLight, int>
		("cascade_fit"
		, int(CascadeFit::FOLLOW)
		, [](const DirectionLight* plight) -> int      { return int(plight->cascade_fit()); }
		, [](DirectionLight* plight, const int& fit)   { plight->cascade_fit(CascadeFit(fit)); });

		ctx.add_attribute_function<DirectionLight, float>
		("cascade_margin"
		, float(0.2f)
		, [](const DirectionLight* plight) -> float       { return plight->cascade_margin(); }
		, [](DirectionLight* plight, const float& margin) { plight->cascade_margin(margin); });
    }

	//light
	DirectionLight::DirectionLight(Context& context)
	: Component(context)
	, SharedObject<DirectionLight>(context.allocator())
	, m_buffer(context)
	{
		m_rotation = Mat3(1);
		m_direction = Constants::axis_z;
	}

	//all events
	void DirectionLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * Constants::axis_z;
		}
	}
	void DirectionLight::on_deattch()
	{
		m_rotation = Mat3(1);
		m_direction = Constants::axis_z;
	}
	void DirectionLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * Constants::axis_z;
		}
	}
	void DirectionLight::on_message(const Message& msg){}

	//Shadow
	const Render::ShadowBuffer& DirectionLight::shadow_buffer() const
	{
		return m_buffer;
	}

	Vec4 DirectionLight::shadow_viewport() const
	{
		return { 0, 0, m_buffer.width(), m_buffer.height() };
	}

	bool DirectionLight::shadow() const
	{
		return m_buffer.width() != 0 && m_buffer.height() != 0;
	}

	void DirectionLight::shadow(const IVec2& size)
	{
		if (m_buffer.size() != size)
		{
			if (size.x != 0 && size.y != 0)
				m_buffer.build(size, Render::ShadowBuffer::SB_TEXTURE_CSM, (unsigned int)m_cascades);
			else
				m_buffer.destoy();
		}
	}

	void DirectionLight::cascades(int cascades)
	{
		const int count = std::clamp(cascades, 1, int(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
		if (count == m_cascades) return;
		m_cascades = count;
		//a layer of the shadow map for each one (its size copied: build resets it)
		if (!shadow()) return;
		const IVec2 size = m_buffer.size();
		m_buffer.build(size, Render::ShadowBuffer::SB_TEXTURE_CSM, (unsigned int)m_cascades);
	}

	int DirectionLight::cascades() const
	{
		return m_cascades;
	}

	void DirectionLight::shadow_distance(float distance)
	{
		m_shadow_distance = (std::max)(distance, 0.0f);
	}

	float DirectionLight::shadow_distance() const
	{
		return m_shadow_distance;
	}

	const IVec2& DirectionLight::shadow_size() const
	{
		return m_buffer.size();
	}

	void DirectionLight::cascade_fit(CascadeFit fit)
	{
		if (m_cascade_fit != fit)
		{
			//stable again: its cascades made again (not the ones of before)
			m_cascade_fit = fit;
			m_stable_key.clear();
		}
	}

	CascadeFit DirectionLight::cascade_fit() const
	{
		return m_cascade_fit;
	}

	void DirectionLight::cascade_margin(float margin)
	{
		m_cascade_margin = std::clamp(margin, 0.0f, 2.0f);
	}

	float DirectionLight::cascade_margin() const
	{
		return m_cascade_margin;
	}

	bool DirectionLight::stable_cascades() const
	{
		return m_cascade_fit == CascadeFit::STABLE;
	}

	//object methods
	//serialize
	void DirectionLight::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}
	void DirectionLight::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}
	//deserialize
	void DirectionLight::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}
	void DirectionLight::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
	//methods
	const Geometry::Sphere& DirectionLight::bounding_sphere() const
	{
		static const Geometry::Sphere emptry;
		return emptry;
	}
	
	const Geometry::Frustum& DirectionLight::frustum() const
	{
		static const Geometry::Frustum emptry{};
		return emptry;
	}

	Weak<Render::Transform> DirectionLight::transform() const
	{
		return { DynamicPointerCast<Render::Transform>(actor().lock()) };
	}

	void DirectionLight::set(Render::UniformDirectionLight* data) const
	{
		data->m_direction = m_direction;
		this->Render::DirectionLight::set(data);
	}

	namespace CSMAux
	{
		CSMCascadeDepth compute_cascade_depth(const Render::Camera& camera, unsigned int cascades, float distance)
		{
			CSMCascadeDepth cascade_depth;
			const float cam_near = camera.viewport().near();
			//the shadow up to its distance (if any, within the view)
			const float cam_far = distance > cam_near ? (std::min)(camera.viewport().far(), distance) : camera.viewport().far();
			const float clip_range = cam_far - cam_near;
			const float min_z = cam_near;
			const float max_z = cam_far;
			const float range = max_z - min_z;
			const float ratio = max_z / min_z;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < cascades; i++)
			{
				float p = static_cast<float>(i) / static_cast<float>(cascades);
				// Log depth
				float log_depth = min_z * std::pow(ratio, p);
				// Linear depth
				float uniform = min_z + range * p;
				// Interpolation
				const float lambda = 0.5f;
				cascade_depth[i] = lerp(log_depth, uniform, lambda);
			}
			cascade_depth[cascades] = cam_far;
			return cascade_depth;
		}

		constexpr std::array<Vec4, 8> get_ndc_box()
		{
			return
			{
				Vec4{-1.0f,	-1.0f,	-1.0f,	1.0f},
				Vec4{-1.0f,	-1.0f,	1.0f,	1.0f},
				Vec4{-1.0f,	1.0f,	-1.0f,	1.0f},
				Vec4{-1.0f,	1.0f,	1.0f,	1.0f},
				Vec4{1.0f,	-1.0f,	-1.0f,	1.0f},
				Vec4{1.0f,	-1.0f,	1.0f,	1.0f},
				Vec4{1.0f,	1.0f,	-1.0f,	1.0f},
				Vec4{1.0f,	1.0f,	1.0f,	1.0f}
			};
		}

		inline Vec3 min_v3_v4(const Vec3& a, const Vec4& b)
		{
			return Vec3((min)(a.x, b.x), (min)(a.y, b.y), (min)(a.z, b.z));
		}

		inline Vec3 max_v3_v4(const Vec3& a, const Vec4& b)
		{
			return Vec3((max)(a.x, b.x), (max)(a.y, b.y), (max)(a.z, b.z));
		}

		template< class T >
		constexpr TMat4<T> csm_ortho(T l,T r,T t,T b, T n, T f)
		{
			return
			{
				T(2.0) / T(r - l),          T(0.0),                 T(0.0),                 T(0.0),
				T(0.0),                     T(2.0) / T(t - b),      T(0.0),                 T(0.0), 
				T(0.0),                     T(0.0),                 T(0.5) * T(2.0) / T(f - n), T(0.0),
				-T(r + l) / T(r - l),       -T(t + b) / T(t - b),   T(0.5) - T(0.5) * T(f + n) / T(f - n), T(1.0)
			};
		}

		std::tuple<Mat4,Mat4> fit_light_proj_mat_to_camera_frustum
		(
			const Mat4& frustum_mat, 
			const Mat4& light_space_transform,
			float texture_size, 
			const Mat4& sceneAABB,
			bool square = true,
			bool round_to_pixel_size = true,
			bool use_constant_size = true
		) 
		{
			bool first_processed = false;
			Vec3 boundingA(std::numeric_limits<float>::infinity());
			Vec3 boundingB(-std::numeric_limits<float>::infinity());

			// start with <-1 -1 -1> to <1 1 1> cube
			std::array<Vec4, 8> bounding_vertices = get_ndc_box();
			
			for (Vec4& vert : bounding_vertices) 
			{
				// clip space -> world space
				vert = frustum_mat * vert;
				vert /= vert.w;
			}

			for (Vec4& vert : bounding_vertices) 
			{
				// clip space -> world space -> light space
				vert = light_space_transform * vert;

				// initialize bounds without comparison, only for first transformed vertex
				if (!first_processed) 
				{
					boundingA = Vec3(vert);
					boundingB = Vec3(vert);
					first_processed = true;
					continue;
				}

				// expand bounding box to encompass everything in 3D
				boundingA = (min_v3_v4)(boundingA, vert);
				boundingB = (max_v3_v4)(boundingB, vert);
			}

			// fit z bounding to scene AABB
			for (uint32_t i = 0; i < 2; i++) 
			{
				for (uint32_t j = 0; j < 2; j++) 
				{
					for (uint32_t k = 0; k < 2; k++) 
					{
						Vec3 vert = Vec3(light_space_transform * sceneAABB * Vec4(i, j, k, 1));
						boundingA.z = std::min(vert.z, boundingA.z);
						boundingB.z = std::max(vert.z, boundingB.z);
					}
				}
			}

			// from https://en.wikipedia.org/wiki/Orthographic_projection#Geometry
			// because I don't trust GLM
			float l = boundingA.x;
			float r = boundingB.x;
			float b = boundingA.y;
			float t = boundingB.y;
			float n = boundingA.z;
			float f = boundingB.z;

			float actual_size = 0;
			if (use_constant_size) 
			{
				// keep constant world-size resolution, side length = diagonal of largest face of frustum
				// the other option looks good at high resolutions, but can result in shimmering as you look in different directions and the cascade changes size
				float farFaceDiagonal = length(Vec3(bounding_vertices[7]) - Vec3(bounding_vertices[1]));
				float forwardDiagonal = length(Vec3(bounding_vertices[7]) - Vec3(bounding_vertices[0]));
				actual_size = std::max(farFaceDiagonal, forwardDiagonal);
			}
			else 
			{
				actual_size = std::max(r - l, t - b);
			}

			// make it square
			if (square) 
			{
				const float W = r - l, H = t - b;
				float diff = actual_size - H;
				if (diff > 0) 
				{
					t += diff / 2.0f;
					b -= diff / 2.0f;
				}
				diff = actual_size - W;
				if (diff > 0) 
				{
					r += diff / 2.0f;
					l -= diff / 2.0f;
				}
			}

			// avoid shimmering
			if (round_to_pixel_size) 
			{
				const float pixelSize = actual_size / texture_size;
				l = std::round(l / pixelSize) * pixelSize;
				r = std::round(r / pixelSize) * pixelSize;
				b = std::round(b / pixelSize) * pixelSize;
				t = std::round(t / pixelSize) * pixelSize;
			}

			Mat4 mat_ortho = csm_ortho(l, r, t, b, n, f);
			return { mat_ortho, light_space_transform };
		}

		void set_uniform(Render::UniformDirectionShadowLight& data, 
						const Render::Camera& camera,
						const Geometry::AABoundingBox& scene_size,
						const Render::ShadowBuffer& buffer,
						const Mat3& rotation,
						const Vec3& direction,
						const IVec2& shadow_map_size,
						unsigned int cascades,
						float distance)
		{
			// Depths
			auto cascade_vdepths = compute_cascade_depth(camera, cascades, distance);
			// multiply by inverse projection*view matrix to find frustum vertices in world space
			// transform to light space
			// same pass, find minimum along each axis
			Mat4 light_space_transform = look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(direction), Constants::axis_y);
			// Scene AABB
			Mat4 scene_matrix = scene_size.to_matrix();
			scene_matrix *= Square::scale(Vec3{1.1f,1.1f,1.1f});
			// Cam view
			const Mat4& cam_view = camera.view();
			// Copy values (the depth bias: the shader, in texels of each cascade)
			for (unsigned int i = 0; i < cascades; ++i)
			{
				Mat4 cam_projection = Square::perspective(camera.viewport().fov(), camera.viewport().aspect(), cascade_vdepths[i], cascade_vdepths[i + 1]);
				Mat4 cascade_cam = inverse(cam_projection * cam_view);
				const auto& [l_proj,l_view] = fit_light_proj_mat_to_camera_frustum(cascade_cam,
                                                                                   light_space_transform,
                                                                                   shadow_map_size.x,
                                                                                   scene_matrix);
				data.m_projection[i] = l_proj;
				data.m_view[i] = l_view;
				data.m_data[i] = Vec3(cascade_vdepths[i + 1], 0.0f, 0.0f);
			}
		}
	}

	namespace CSMAux
	{
		//the corners of the slice of a cascade in world space (its frustum: the inverse of its
		//projection by the view)
		std::array<Vec3, 8> slice_corners(const Mat4& cascade_cam)
		{
			std::array<Vec3, 8> corners;
			const std::array<Vec4, 8> ndc = get_ndc_box();
			for (size_t i = 0; i != ndc.size(); ++i)
			{
				const Vec4 point = cascade_cam * ndc[i];
				corners[i] = Vec3(point) / point.w;
			}
			return corners;
		}

		//the sphere around a slice: its center, its radius (rounded up: the same every frame, it
		//depends only on the splits, the field of view, the aspect)
		std::tuple<Vec3, float> slice_sphere(const std::array<Vec3, 8>& corners)
		{
			Vec3 center(0.0f);
			for (const Vec3& corner : corners)
			{
				center += corner;
			}
			center /= float(corners.size());
			float radius = 0.0f;
			for (const Vec3& corner : corners)
			{
				radius = std::max(radius, length(corner - center));
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;
			return { center, radius };
		}
	}

	Vec2 DirectionLight::stable_depth(const Mat4& light_view) const
	{
		//the depth of the scene (a little larger, as the fit by the box) in the space of the light
		const Mat4 scene_matrix = m_scene_size.to_matrix() * Square::scale(Vec3{ 1.1f, 1.1f, 1.1f });
		Vec2 depth(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
		for (int corner = 0; corner != 8; ++corner)
		{
			const Vec4 unit(float(corner & 1), float((corner >> 1) & 1), float((corner >> 2) & 1), 1.0f);
			const float z = Vec3(light_view * scene_matrix * unit).z;
			depth.x = std::min(depth.x, z);
			depth.y = std::max(depth.y, z);
		}
		//the one kept while the scene stays in it, not much smaller (a hovercraft at its border: the
		//cascades do not move); else again, with a margin
		const float kept = m_stable_depth.y - m_stable_depth.x;
		const bool out = depth.x < m_stable_depth.x || depth.y > m_stable_depth.y;
		const bool smaller = (depth.y - depth.x) < kept * 0.5f;
		if (!m_stable_depth_valid || out || smaller)
		{
			const float pad = (depth.y - depth.x) * 0.1f;
			m_stable_depth = Vec2(depth.x - pad, depth.y + pad);
			m_stable_depth_valid = true;
		}
		return m_stable_depth;
	}

	void DirectionLight::stable_uniform(Render::UniformDirectionShadowLight& data, const Render::Camera& camera) const
	{
		const unsigned int cascades = (unsigned int)m_cascades;
		const IVec2 size = m_buffer.size();
		//what the cascades are made for: one changed, all of them again
		const Render::Viewport& viewport = camera.viewport();
		const Vec2 planes = viewport.near_and_far();
		const std::vector<float> key
		{
			  m_direction.x, m_direction.y, m_direction.z
			, float(cascades), float(size.x), m_shadow_distance, m_cascade_margin
			, viewport.fov(), viewport.aspect(), planes.x, planes.y
		};
		if (key != m_stable_key)
		{
			m_stable_key = key;
			m_stable_depth_valid = false;
			for (StableCascade& cascade : m_stable)
			{
				cascade.m_valid = false;
			}
		}
		const auto depths = CSMAux::compute_cascade_depth(camera, cascades, m_shadow_distance);
		const Mat4 light_view = look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(m_direction), Constants::axis_y);
		const Vec2 depth = stable_depth(light_view);
		const Mat4& camera_view = camera.view();
		for (unsigned int i = 0; i < cascades; ++i)
		{
			const Mat4 projection = Square::perspective(viewport.fov(), viewport.aspect(), depths[i], depths[i + 1]);
			const auto [center, radius] = CSMAux::slice_sphere(CSMAux::slice_corners(inverse(projection * camera_view)));
			const float half = radius * (1.0f + m_cascade_margin);
			const Vec2 at = Vec2(light_view * Vec4(center, 1.0f));
			//it stays while the slice is inside it; else around the slice, on the grid of its texels
			StableCascade& cascade = m_stable[i];
			const Vec2 offset = glm::abs(at - cascade.m_center);
			const bool inside = cascade.m_valid && offset.x <= half - radius && offset.y <= half - radius;
			if (!inside)
			{
				const float texel = (2.0f * half) / float(std::max(size.x, 1));
				cascade.m_center = glm::round(at / texel) * texel;
				cascade.m_half = half;
				cascade.m_valid = true;
			}
			const Vec2 low = cascade.m_center - Vec2(cascade.m_half);
			const Vec2 high = cascade.m_center + Vec2(cascade.m_half);
			data.m_projection[i] = CSMAux::csm_ortho(low.x, high.x, high.y, low.y, depth.x, depth.y);
			data.m_view[i] = light_view;
			data.m_data[i] = Vec3(depths[i + 1], 0.0f, 0.0f);
		}
	}

	void DirectionLight::set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map) const
	{
		const bool drawn = actor().lock() && draw_shadow_map;
		if (drawn && m_cascade_fit == CascadeFit::STABLE)
		{
			stable_uniform(m_cache_udirectionshadowlight, *camera);
		}
		else if (drawn)
		{
			CSMAux::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction, m_buffer.size(), (unsigned int)m_cascades, m_shadow_distance);
		}
		std::memcpy(data, &m_cache_udirectionshadowlight, sizeof(Render::UniformDirectionShadowLight));
		//the filter and the cascades: every frame (the filter changes without a new shadow map)
		data->m_options = IVec4(int(shadow_filter()), m_cascades, 0, 0);
	}

	void DirectionLight::set_scene_size(const Geometry::AABoundingBox& scene)
	{
		m_scene_size = scene;
	}

}
}