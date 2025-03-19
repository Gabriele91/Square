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

	namespace CSMAuxCameSpace
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
	
	namespace CSMAuxWorldSpace
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
			const float lambda = 0.85f;

			cascade_depth[0] = cam_near;
			for (uint32_t i = 1; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
			{
				float p = static_cast<float>(i) / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
				// Log depth
				float log_depth = min_z * std::pow(ratio, p);
				// Linear depth
				float uniform = min_z + range * p;
				// Interpolation
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

		constexpr std::array<Vec3, 8> get_ndc_box()
		{
			return
			{
				Vec3(-1.0f,  1.0f, 0.0f),
				Vec3(1.0f,  1.0f, 0.0f),
				Vec3(1.0f, -1.0f, 0.0f),
				Vec3(-1.0f, -1.0f, 0.0f),
				Vec3(-1.0f,  1.0f,  1.0f),
				Vec3(1.0f,  1.0f,  1.0f),
				Vec3(1.0f, -1.0f,  1.0f),
				Vec3(-1.0f, -1.0f,  1.0f),
			};
		}

		const std::array<Vec3, 8> get_frustum_box(const Render::Camera& camera)
		{
			Mat4 inv_cam_pv = inverse(camera.projection() * camera.view());
			std::array<Vec3, 8> frustum_corners = get_ndc_box();
			// Project frustum corners into world space
			for (uint32_t j = 0; j < 8; j++)
			{
				Vec4 inv_corner = inv_cam_pv * Vec4(frustum_corners[j], 1.0f);
				frustum_corners[j] = inv_corner / inv_corner.w;
			}
			return frustum_corners;
		}

		float get_radius(const std::array<Vec3, 8>& frustum_corners, const Vec3& center)
		{
			float radius = 0.0f;
			for (uint32_t j = 0; j < 8; j++)
			{
				float distance = length(frustum_corners[j] - center);
				radius = max(radius, distance);
			}
			return ceil(radius * 16.0f) / 16.0f;
		}

		float get_radius(const std::array<Vec3, 8>& frustum_corners)
		{
			return get_radius(frustum_corners, center_ndc(frustum_corners));
		}

		void extend_z(Vec3& max_extents,Vec3& min_extents, float z_mult)
		{
			if (min_extents.z < 0)
			{
				min_extents.z *= z_mult;
			}
			else
			{
				min_extents.z /= z_mult;
			}

			if (max_extents.z < 0)
			{
				max_extents.z /= z_mult;
			}
			else
			{
				max_extents.z *= z_mult;
			}
		}

		void ortho_with_rispect_texture_size(Vec3& min_extents, Vec3& max_extents, const IVec2& shadow_map_size)
		{
			// Texture fix
			Vec2 shadow_map_texel_size = Vec2
			{
				(max_extents.x - min_extents.x) / shadow_map_size.x,
				(max_extents.y - min_extents.y) / shadow_map_size.y
			};
			min_extents.x = std::floor(min_extents.x / shadow_map_texel_size.x) * shadow_map_texel_size.x;
			min_extents.y = std::floor(min_extents.y / shadow_map_texel_size.y) * shadow_map_texel_size.y;
			max_extents.x = std::ceil(max_extents.x / shadow_map_texel_size.x) * shadow_map_texel_size.x;
			max_extents.y = std::ceil(max_extents.y / shadow_map_texel_size.y) * shadow_map_texel_size.y;
		}

		std::tuple<Vec3, Vec3> ortho_box(const std::array<Vec3, 8>& frustum_corners, const Vec3& center)
		{
			float local_radius = get_radius(frustum_corners, center);
			Vec3 max_extents = Vec3(local_radius);
			Vec3 min_extents = -max_extents;
			extend_z(max_extents, min_extents, 4.0f);
			return { min_extents , max_extents };
		}

		std::tuple<Vec3, Vec3> ortho_box(const Geometry::AABoundingBox& scene_aabb,const std::array<Vec3, 8>& frustum_corners, const Vec3& frustum_center)
		{
			float local_radius = get_radius(frustum_corners, frustum_center);

			// Use scene bounds to optimize extents
			Vec3 scene_size = scene_aabb.get_extension();
			Vec3 scene_center = scene_aabb.get_center();

			// Calculate relative scale factor based on scene size
			float scene_max_dim = std::max({ scene_size.x, scene_size.y, scene_size.z });
			float adaptive_scale = std::min(4.0f, std::max(1.0f, scene_max_dim / (2.0f * local_radius)));

			// Create extents that consider scene dimensions
			Vec3 max_extents = Vec3(local_radius * adaptive_scale);
			Vec3 min_extents = -max_extents;

			// Adjust based on distance to scene center
			float dist_to_scene = length(frustum_center - scene_center);
			float z_extension_factor = std::min(4.0f, 1.0f + (dist_to_scene / scene_max_dim));
			extend_z(max_extents, min_extents, z_extension_factor);

			return { min_extents , max_extents };
		}
		
		std::tuple<Vec3, Vec3> ortho_box_2(const Geometry::AABoundingBox& scene_aabb, const std::array<Vec3, 8>& frustum_corners, const Vec3& frustum_center)
		{
			float local_radius = get_radius(frustum_corners, frustum_center);

			// Use scene bounds to optimize extents
			Vec3 scene_size = scene_aabb.get_extension();
			Vec3 scene_center = scene_aabb.get_center();

			// Calculate more adaptive scale factor based on view direction and scene size
			Vec3 view_dir = normalize(scene_center - frustum_center);
			float scene_max_dim = std::max({ scene_size.x, scene_size.y, scene_size.z });

			// Compute projected sizes along view direction
			float scene_proj_size = dot(scene_size, abs(view_dir));
			float frustum_proj_size = local_radius * 2.0f;

			// Compute adaptive scale that considers both scene size and view direction
			float adaptive_scale = std::min(4.0f, std::max(1.0f, scene_proj_size / frustum_proj_size));

			// Calculate distance to scene center and adjust scale accordingly
			float dist_to_scene = length(frustum_center - scene_center);
			float distance_factor = std::min(1.5f, 0.5f + dist_to_scene / scene_max_dim);

			// Apply combined scaling factor
			Vec3 max_extents = Vec3(local_radius * adaptive_scale * distance_factor);
			Vec3 min_extents = -max_extents;

			// Adjust Z extent based on view direction and scene depth
			float scene_depth = dot(scene_size, abs(view_dir));
			float z_extension_factor = std::min(4.0f, 1.0f + (scene_depth / scene_max_dim));
			extend_z(max_extents, min_extents, z_extension_factor);

			// Optional: Add directional bias to cover more of the scene in the viewing direction
			Vec3 directional_bias = view_dir * (scene_max_dim * 0.1f);
			max_extents += abs(directional_bias);
			min_extents -= abs(directional_bias);

			return { min_extents, max_extents };
		}

		/*
		Calculate frustum split depths and matrices for the shadow map cascades
		Based on https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
		*/
		void set_uniform(Render::UniformDirectionShadowLight& data,
			const Render::Camera& camera,
			const Geometry::AABoundingBox& scene_aabb,
			const Render::ShadowBuffer& buffer,
			const Mat3& rotation,
			const Vec3& direction,
			const IVec2& shadow_map_size)
		{
			const float cam_near = camera.viewport().near();
			const float cam_far = camera.viewport().far();
			const float clip_range = cam_far - cam_near;
			const Vec3 light_dir = normalize(direction);

			CSMCascadeDepth cascade_splits;
			cascade_splits = compute_cascade_depth(camera);
			const std::array<Vec3, 8> frustum_corners = get_frustum_box(camera);
			
			// Calculate orthographic projection matrix for each cascade
			float last_split_dist = cascade_splits[0];
			for (uint32_t i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				float split_dist = cascade_splits[i + 1];
				std::array<Vec3, 8> local_frustum_corners;
				for (uint32_t j = 0; j < 4; j++)
				{
					Vec3 dist = frustum_corners[j + 4] - frustum_corners[j];
					local_frustum_corners[j + 4] = frustum_corners[j] + (dist * split_dist / cam_far);
					local_frustum_corners[j] = frustum_corners[j] + (dist * last_split_dist / cam_far);
				}

				// Get frustum center
				Vec3 local_frustum_center = center_ndc(local_frustum_corners);
				auto [min_extents, max_extents] = ortho_box_2(scene_aabb, local_frustum_corners, local_frustum_center);
				ortho_with_rispect_texture_size(min_extents, max_extents, shadow_map_size);

				// Fix for LHS: Change the light position calculation
				// In LHS, the light should be positioned in front of the frustum
				// with the Z direction pointing toward the frustum
				Mat4 light_view = look_at(local_frustum_center - light_dir * max_extents.z,
										  local_frustum_center,
										  UP_LIGHT_DIR);

				// Orthographic projection matrix for LHS
				// In LHS, depth increases as you move away from the camera
				Mat4 light_ortho = ortho(min_extents.x, max_extents.x,
										 min_extents.y, max_extents.y,
										 0.0f, max_extents.z - min_extents.z);

				// Store split distance and matrix in cascade
				data.m_depths[i] = cascade_splits[i + 1];
				data.m_projection[i] = light_ortho;
				data.m_view[i] = light_view;
				last_split_dist = cascade_splits[i];
			}
		}
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

			#if 1
			Mat4 mat_ortho = Square::ortho(l, r, t, b, n, f);
			#else
			Mat4 mat_ortho =
			{
				2.0f / (r - l),	0.0f,			0.0f,			0.0f,
				0.0f,			2.0f / (t - b),	0.0f,			0.0f,
				0.0f,			0.0f,			2.0f / (f - n),	0.0f,
				-(r + l) / (r - l),	-(t + b) / (t - b),	-(f + n) / (f - n),	1.0f
			};
			mat_ortho = Mat4
			{
				1,0,0,0,
				0,1,0,0,
				0,0,.5f,0,
				0,0,.5f,1
			} * mat_ortho;
			#endif

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
			Mat4 light_space_transform = look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(direction), UP_LIGHT_DIR);
			// Scene AABB
			Mat4 scene_matrix = scene_size.to_matrix();
			// Cam view
			const Mat4& cam_view = camera.view();
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				Mat4 cam_projection = Square::perspective(camera.viewport().fov(), camera.viewport().aspect(), cascade_vdepths[i], cascade_vdepths[i + 1]);
				Mat4 cascade_cam = inverse(cam_projection * cam_view);
				float fs_size = 0;
				float fs_bais = 0;
				auto& [l_proj,l_view] = fit_light_proj_mat_to_camera_frustum(cascade_cam,
																			 light_space_transform, 
																			 shadow_map_size.x, 
																			 scene_matrix,
																			 fs_size, 
																			 fs_bais);
				data.m_projection[i] = l_proj;
				data.m_view[i] = l_view;
				data.m_depths[i] = cascade_vdepths[i + 1];
			}
		}
	}

	void DirectionLight::set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map) const
	{
		if (auto ptr_actor = actor().lock() && draw_shadow_map)
		{
#if 0
			CSMAuxCameSpace::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction, m_buffer.size());
#elif 0
			CSMAuxWorldSpace::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction, m_buffer.size());
#else
			CSMAux::set_uniform(m_cache_udirectionshadowlight, *camera, m_scene_size, m_buffer, m_rotation, m_direction, m_buffer.size());
#endif
		}
		std::memcpy(data, &m_cache_udirectionshadowlight, sizeof(Render::UniformDirectionShadowLight));
	}

	void DirectionLight::set_scene_size(const Geometry::AABoundingBox& scene)
	{
		m_scene_size = scene;
	}

}
}