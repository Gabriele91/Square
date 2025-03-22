//
//  PointLight.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
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
namespace Constants
{
	const Vec3 DIRECTION_LIGHT_DIR(0.0, 0.0, 1.0);
	const Vec3 UP_LIGHT_DIR(0.0, 1.0, 0.0);
}

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
    }

	//light
	DirectionLight::DirectionLight(Context& context)
	: Component(context)
	, SharedObject<DirectionLight>(context.allocator())
	, m_buffer(context)
	{
		m_rotation = Mat3(1);
		m_direction = Constants::DIRECTION_LIGHT_DIR;
	}

	//all events
	void DirectionLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * Constants::DIRECTION_LIGHT_DIR;
		}
	}
	void DirectionLight::on_deattch()
	{
		m_rotation = Mat3(1);
		m_direction = Constants::DIRECTION_LIGHT_DIR;
	}
	void DirectionLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * Constants::DIRECTION_LIGHT_DIR;
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
				m_buffer.build(size, Render::ShadowBuffer::SB_TEXTURE_CSM);
			else
				m_buffer.destoy();
		}
	}

	const IVec2& DirectionLight::shadow_size() const
	{
		return m_buffer.size();
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
		return {};
	}
	
	const Geometry::Frustum& DirectionLight::frustum() const
	{
		return {};
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
		CSMCascadeDepth compute_cascade_depth(const Render::Camera& camera)
		{
			CSMCascadeDepth cascade_depth;
			const float cam_near = camera.viewport().near();
			const float cam_far = camera.viewport().far();
			const float clip_range = cam_far - cam_near;
			const float min_z = cam_near;
			const float max_z = cam_far;
			const float range = max_z - min_z;
			const float ratio = max_z / min_z;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
			{
				float p = static_cast<float>(i) / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
				// Log depth
				float log_depth = min_z * std::pow(ratio, p);
				// Linear depth
				float uniform = min_z + range * p;
				// Interpolation
				const float lambda = 0.5f;
				cascade_depth[i] = lerp(log_depth, uniform, lambda);
			}
			cascade_depth[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES] = cam_far;
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
			float& world_space_dim,
			float& world_space_depth, 
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

			world_space_dim = actual_size;
			world_space_depth = f - n;

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
						const IVec2& shadow_map_size)
		{
			// Depths
			auto cascade_vdepths = compute_cascade_depth(camera);
			// multiply by inverse projection*view matrix to find frustum vertices in world space
			// transform to light space
			// same pass, find minimum along each axis
			Mat4 light_space_transform = look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(direction), Constants::UP_LIGHT_DIR);
			// Scene AABB
			Mat4 scene_matrix = scene_size.to_matrix();
			// Cam view
			const Mat4& cam_view = camera.view();
			// Const
			const float base_bias = 0.0000020000f;
			const float slope_bias = 0.0000060000f;
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				Mat4 cam_projection = Square::perspective(camera.viewport().fov(), camera.viewport().aspect(), cascade_vdepths[i], cascade_vdepths[i + 1]);
				Mat4 cascade_cam = inverse(cam_projection * cam_view);
				float fs_size = 0;
				float fs_depth = 0;
				auto& [l_proj,l_view] = fit_light_proj_mat_to_camera_frustum(cascade_cam,
																			 light_space_transform, 
																			 shadow_map_size.x, 
																			 scene_matrix,
																			 fs_size, 
																			 fs_depth);
				data.m_projection[i] = l_proj;
				data.m_view[i] = l_view;
				data.m_data[i] = Vec3(cascade_vdepths[i + 1],
									  base_bias * fs_size,
					                  slope_bias * fs_size);
			}
		}
	}

	void DirectionLight::set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map) const
	{
		if (auto ptr_actor = actor().lock() && draw_shadow_map)
		{
			CSMAux::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction, m_buffer.size());
		}
		std::memcpy(data, &m_cache_udirectionshadowlight, sizeof(Render::UniformDirectionShadowLight));
	}

	void DirectionLight::set_scene_size(const Geometry::AABoundingBox& scene)
	{
		m_scene_size = scene;
	}

}
}