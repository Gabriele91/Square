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

#define DIRECTION_LIGHT_DIR Vec3(0.0,0.0,-1.0)
#define UP_LIGHT_DIR Vec3(0.0,-1.0,0.0)
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
	void DirectionLight::serialize_json(Data::Json& archive)
	{
		Data::serialize_json(archive, this);
	}
	//deserialize
	void DirectionLight::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}
	void DirectionLight::deserialize_json(Data::Json& archive)
	{ }
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

	namespace Aux1
	{
		CSMCascadeDepth compute_cascade_depth(const Render::Camera& camera)
		{
			CSMCascadeDepth  cascade_depth;

			const float cam_near = camera.viewport().near();
			const float cam_far = camera.viewport().far();
			const float clip_range = cam_far - cam_near;

			const float min_z = cam_near;
			const float max_z = cam_near + clip_range;

			const float range = max_z - min_z;
			const float ratio = max_z / min_z;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
			{
				float p = i / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
				float uniform = range * p;
				cascade_depth[i] = uniform;
			}
			cascade_depth[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES] = cam_far;

			return cascade_depth;
		}

		Vec3 center_ndc(const std::array<Vec3, 8>& NDC)
		{
			Vec3 center(0.0f);
			for (const Vec3& vec : NDC)
			{
				center += vec;
			}
			float div = 1.0f / NDC.size();
			return center * div;
		}

		float radius_ndc(const std::array<Vec3, 8>& NDC, const Vec3& center)
		{
			float radius = 0.0f;
			for (unsigned int i = 0; i < 8; ++i)
			{
				float distance = glm::length(NDC[i] - center);
				radius = (max)(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;
			return radius;
		}

		Mat4 get_ortho_matrix(const Vec3& min_point, const Vec3& max_point)
		{
			float distance = max_point.z - min_point.z;
			//Create the orthographic matrix
			return ortho(min_point.x, max_point.x, min_point.y, max_point.y, 0.0f, distance);
		}

		Mat4 get_round_matrix(Mat4 Ortho, const Mat4& View, const size_t texture_size)
		{
			Mat4 shadow_mat = Ortho * View;
			Vec4 shadow_start_Pos(0.0f, 0.0f, 0.0f, 1.0f);
			shadow_start_Pos = shadow_mat * shadow_start_Pos;
			shadow_start_Pos = (shadow_start_Pos * texture_size) / 2.0f;

			Vec4 round_start = glm::round(shadow_start_Pos);
			Vec4 offset = round_start - shadow_start_Pos;
			offset = Vec4(Vec2(offset * 2.0f / texture_size) , 0.0f, 0.0f);
			Ortho[3] += offset;

			return Ortho;
		}

		std::array<Vec3, 8> get_frustum_points(const Render::Camera& camera, float near, float far)
		{
			std::array<Vec3, 8> frustum_points
			{
				Vec3(-1.0f,  1.0f, -1.0f),
				Vec3(1.0f,  1.0f, -1.0f),
				Vec3(1.0f, -1.0f, -1.0f),
				Vec3(-1.0f, -1.0f, -1.0f),

				Vec3(-1.0f,  1.0f,  1.0f),
				Vec3(1.0f,  1.0f,  1.0f),
				Vec3(1.0f, -1.0f,  1.0f),
				Vec3(-1.0f, -1.0f,  1.0f),
			};

			//Move the frustum coordinates to world space
			Mat4 projection = glm::perspectiveRH_NO(camera.viewport().fov(), camera.viewport().aspect(), camera.viewport().near(), camera.viewport().far());
			Mat4 inv_view_proj = inverse(projection * camera.view());

			for (unsigned i = 0; i < 8; i++)
			{
				//Move to world
				Vec4 inverse_point = inv_view_proj * Vec4(frustum_points[i], 1.0f);
				//Perspective division
				frustum_points[i] = Vec3(inverse_point / inverse_point.w);
			}

			float range = camera.viewport().far() - camera.viewport().near();
			far = (far - camera.viewport().near()) / range;
			near = (near - camera.viewport().near()) / range;

			//Compute the real corners of the AABB
			for (unsigned i = 0; i < 4; i++)
			{
				Vec3 frustum_vec = frustum_points[i + 4] - frustum_points[i];
				//move vectors to real coordinates
				Vec3 far_vec = frustum_vec * far;
				Vec3 near_vec = frustum_vec * near;
				//Compute real points
				frustum_points[i + 4] = frustum_points[i] + far_vec;
				frustum_points[i] = frustum_points[i] + near_vec;
			}

			return frustum_points;
		}

		std::tuple<Mat4, Mat4> get_projection( const Render::Camera& camera, 
											   float near, 
											   float far,
											   const Vec3& direction,
											   size_t texture_size)
		{
			// looks Cascaded Shadow Maps - Sphere Based Bounding 
			std::array<Vec3, 8> NDC = get_frustum_points(camera, near, far);
			Vec3 center(center_ndc(NDC));
			float radius(radius_ndc(NDC, center));
			Vec3 max_point(radius);
			Vec3 min_point(-max_point);
			Mat4 light_projection = get_ortho_matrix(min_point, max_point);
			Mat4 light_view = look_at(center - normalize(direction) * max_point.z, center, -UP_LIGHT_DIR);
			light_projection = get_round_matrix(light_projection, light_view, texture_size);
			return { light_projection , light_view };
		}

		void set_uniform(Render::UniformDirectionShadowLight& data, 
						 const Render::Camera& camera,
						 const Geometry::AABoundingBox& scene_size,
						 const Render::ShadowBuffer& buffer,
						 const Mat3& rotation,
						 const Vec3& direction)
		{
			// Depths
			auto cascade_vdepths = compute_cascade_depth(camera);
			auto texture_size = Square::max<size_t>(buffer.width(), buffer.height());
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				auto& [p, v] = get_projection(camera, cascade_vdepths[i], cascade_vdepths[i + 1], direction, texture_size);
				data.m_projection[i] = p;
				data.m_view[i] = v;
				data.m_depths[i] = cascade_vdepths[i + 1];
			}
		}
	}

	namespace Aux2
	{
		CSMCascadeDepth compute_cascade_depth(const Render::Camera& camera)
		{
			CSMCascadeDepth  cascade_depth;

			const float cam_near = camera.viewport().near();
			const float cam_far = camera.viewport().far();
			const float clip_range = cam_far - cam_near;

			const float min_z = cam_near;
			const float max_z = cam_near + clip_range;

			const float range = max_z - min_z;
			const float ratio = max_z / min_z;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
			{
				float p = i / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
				float uniform = range * p;
				cascade_depth[i] = uniform;
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
				       const Mat4& light_view)
		{
			float aspect = camera.viewport().aspect();
			float FOV = camera.viewport().fov();
			float tan_half_HFOV = std::tan(FOV / 2.0f);
			float tan_half_VFOV = std::tan((FOV * aspect) / 2.0f);

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

			
			Vec4 center = camera_inv_view * center_ndc(frustum_corners);
			Vec4 min{ std::numeric_limits<float>::max() };
			Vec4 max{ std::numeric_limits<float>::lowest() };

			for (size_t j = 0; j < frustum_corners_size; ++j)
			{
				// Transform the frustum coordinate from view to world space
				Vec4 vc_world = camera_inv_view * frustum_corners[j];
				Vec4 vc_light = light_view * vc_world;

				min = Square::min(min, vc_light);
				max = Square::max(max, vc_light);
			}

			return Square::ortho_ZO(min.x, max.x, min.y, max.y, min.z, max.z);
		}

		void set_uniform(Render::UniformDirectionShadowLight& data, 
						 const Render::Camera& camera,
						 const Geometry::AABoundingBox& scene_size,
						 const Render::ShadowBuffer& buffer,
						 const Mat3& rotation,
						 const Vec3& direction)
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
					                            light_view);
				data.m_view[i] = light_view;
				data.m_depths[i] = cascade_vdepths[i + 1];
			}
		}
	}
	
	namespace Aux3
	{
		CSMCascadeDepth compute_cascade_depth(const Render::Camera& camera)
		{
			CSMCascadeDepth  cascade_depth;

			const float cam_near = camera.viewport().near();
			const float cam_far = camera.viewport().far();
			const float clip_range = cam_far - cam_near;

			const float min_z = cam_near;
			const float max_z = cam_near + clip_range;

			const float range = max_z - min_z;
			const float ratio = max_z / min_z;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
			{
				float p = i / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
				float uniform = range * p;
				cascade_depth[i] = uniform;
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
					   const Geometry::AABoundingBox& scene_size_light_space)
		{
			float aspect = camera.viewport().aspect();
			float FOV = camera.viewport().fov();
			float tan_half_HFOV = std::tan(FOV / 2.0f);
			float tan_half_VFOV = std::tan((FOV * aspect) / 2.0f);

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

			
			Vec4 center = camera_inv_view * center_ndc(frustum_corners);
			Vec4 min{ std::numeric_limits<float>::max() };
			Vec4 max{ std::numeric_limits<float>::lowest() };

			for (size_t j = 0; j < frustum_corners_size; ++j)
			{
				// Transform the frustum coordinate from view to world space
				Vec4 vc_world = camera_inv_view * frustum_corners[j];
				Vec4 vc_light = light_view * vc_world;

				min = Square::min(min, vc_light);
				max = Square::max(max, vc_light);
			}

			min.z = scene_size_light_space.get_min().z;
			max.z = scene_size_light_space.get_max().z;

			return Square::ortho(min.x, max.x, min.y, max.y, -max.z, -min.z);
		}

		Mat4 get_round_matrix(Mat4 Ortho, const Mat4& View, const size_t texture_size)
		{
			Mat4 shadow_mat = Ortho * View;
			Vec4 shadow_start_Pos(0.0f, 0.0f, 0.0f, 1.0f);
			shadow_start_Pos = shadow_mat * shadow_start_Pos;
			shadow_start_Pos = (shadow_start_Pos * texture_size) / 2.0f;

			Vec4 round_start = glm::round(shadow_start_Pos);
			Vec4 offset = round_start - shadow_start_Pos;
			offset = Vec4(Vec2(offset * 2.0f / texture_size), 0.0f, 0.0f);
			Ortho[3] += offset;

			return Ortho;
		}

		void set_uniform(Render::UniformDirectionShadowLight& data, 
						 const Render::Camera& camera,
						 const Geometry::AABoundingBox& scene_size,
						 const Render::ShadowBuffer& buffer,
						 const Mat3& rotation,
						 const Vec3& direction)
		{
			// Depths
			auto cascade_vdepths = compute_cascade_depth(camera);
			Mat4 light_model = Mat4(rotation);
			Mat4 light_view = inverse(light_model);
			Mat4 camera_inv_view = inverse(camera.view());
			auto scene_size_light_space = scene_size * light_view;			
			size_t texture_size = Square::max<size_t>(buffer.width(), buffer.height());
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				data.m_projection[i] = get_round_matrix(get_ortho(camera,
																  camera_inv_view,
																  cascade_vdepths[0], 
																  cascade_vdepths[i+1], 
																  light_view,
																  scene_size_light_space), light_view, texture_size);
				data.m_view[i] = light_view;
				data.m_depths[i] = cascade_vdepths[i + 1];
			}
		}
	}

	void DirectionLight::set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map) const
	{
		if (auto ptr_actor = actor().lock() && draw_shadow_map)
		{
			Aux3::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction);
		}
		std::memcpy(data, &m_cache_udirectionshadowlight, sizeof(Render::UniformDirectionShadowLight));
	}

	void DirectionLight::set_scene_size(const Geometry::AABoundingBox& scene)
	{
		m_scene_size = scene;
	}

}
}