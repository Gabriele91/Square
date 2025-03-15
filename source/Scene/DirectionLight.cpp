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

#define DIRECTION_LIGHT_DIR Vec3(0.0, 0.0, 1.0)
#define UP_LIGHT_DIR Vec3(0.0, 1.0, 0.0)
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
    }

	//light
	DirectionLight::DirectionLight(Context& context)
	: Component(context)
	, SharedObject<DirectionLight>(context.allocator())
	, m_buffer(context)
	{
		m_rotation = Mat3(1);
		m_direction = DIRECTION_LIGHT_DIR;
	}

	//all events
	void DirectionLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * DIRECTION_LIGHT_DIR;
		}
	}
	void DirectionLight::on_deattch()
	{
		m_rotation = Mat3(1);
		m_direction = DIRECTION_LIGHT_DIR;
	}
	void DirectionLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_rotation = to_mat3(ptr_actor->rotation(true));
			m_direction = m_rotation * DIRECTION_LIGHT_DIR;
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
		
		template<typename T>
		T center_ndc(const std::array<T, 8>& NDC)
		{
			T center(0.0f);
			for (const T& vec : NDC)
			{
				center += vec;
			}
			float div = 1.0f / NDC.size();
			return center * div;
		}

		Mat4 get_ortho(const Render::Camera& camera,
					   const Mat4& camera_inv_view,
				       float near,
				       float far,
				       const Mat4& light_view,
					   const Geometry::AABoundingBox& scene_size,
					   const IVec2& shadow_map_size)
		{
			float aspect = camera.viewport().aspect();
			float FOV = camera.viewport().fov();
			float tan_half_HFOV = std::tan(FOV / 2.0f);
			float tan_half_VFOV = std::tan(FOV / 2.0f) / aspect;

			float xn = near * tan_half_HFOV;
			float xf = far * tan_half_HFOV;
			float yn = near * tan_half_VFOV;
			float yf = far * tan_half_VFOV;

			const size_t frustum_corners_size = 8;
			std::array<Vec4, frustum_corners_size> frustum_corners
			{
				// near face
				Vec4(xn, yn, near, 1.0),
				Vec4(-xn, yn, near, 1.0),
				Vec4(xn, -yn, near, 1.0),
				Vec4(-xn, -yn, near, 1.0),

				// far face
				Vec4(xf, yf, far, 1.0),
				Vec4(-xf, yf, far, 1.0),
				Vec4(xf, -yf, far, 1.0),
				Vec4(-xf, -yf, far, 1.0)
			};

			Vec4 scene_min_light = light_view * Vec4(scene_size.get_min(), 1.0f);
			Vec4 scene_max_light = light_view * Vec4(scene_size.get_max(), 1.0f);
			Vec4 min{ Square::min(scene_min_light, scene_max_light) };
			Vec4 max{ Square::max(scene_min_light, scene_max_light) };

			for (size_t j = 0; j < frustum_corners_size; ++j)
			{
				Vec4 vc_world = camera_inv_view * frustum_corners[j];
				Vec4 vc_light = light_view * vc_world;

				min = Square::min(min, vc_light);
				max = Square::max(max, vc_light);
			}

			// Padding
			const float shadow_padding = 5.0f;
			Vec4 padding = Vec4(shadow_padding, shadow_padding, 0.0f, 0.0f);
			min -= padding;
			max += padding;

			// Texture fix
			Vec2 shadow_map_texel_size = Vec2(
				(max.x - min.x) / shadow_map_size.x,
				(max.y - min.y) / shadow_map_size.y
			);
			min.x = std::floor(min.x / shadow_map_texel_size.x) * shadow_map_texel_size.x;
			min.y = std::floor(min.y / shadow_map_texel_size.y) * shadow_map_texel_size.y;
			max.x = std::ceil(max.x / shadow_map_texel_size.x) * shadow_map_texel_size.x;
			max.y = std::ceil(max.y / shadow_map_texel_size.y) * shadow_map_texel_size.y;

			// Build projection
			return Square::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
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
			Mat4 light_model = Mat4(rotation);
			Mat4 light_view = inverse(light_model);
			Mat4 camera_inv_view = inverse(camera.view());
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				data.m_projection[i] = get_ortho(camera, 
												camera_inv_view,
												cascade_vdepths[i], 
					                            cascade_vdepths[i+1], 
					                            light_view,
					                            scene_size,
											    shadow_map_size);
				data.m_view[i] = light_view;
				data.m_depths[i] = cascade_vdepths[i + 1];
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