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
	using CSMCascadeDepth  = std::array<float, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES>;
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
		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, bool>
		("visible"
		, bool(0)
		, [](const DirectionLight* plight) -> bool       { return plight->visible(); }
		, [](DirectionLight* plight, const bool& visible){ plight->visible(visible); }));

		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](DirectionLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); }));

		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3        { return plight->specular(); }
		, [](DirectionLight* plight, const Vec3& specular){ plight->specular(specular); }));
    }

	//light
	DirectionLight::DirectionLight(Context& context)
	: Component(context)
	, m_buffer(context)
	{
		m_direction = Vec3(0, 0.0, 1.0);
	}

	//all events
	void DirectionLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true))* DIRECTION_LIGHT_DIR;
		}
	}
	void DirectionLight::on_deattch()
	{
		m_direction = DIRECTION_LIGHT_DIR;
	}
	void DirectionLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * DIRECTION_LIGHT_DIR;
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
	void DirectionLight::serialize(Data::Archive& archivie)
	{
		Data::serialize(archivie, this);
	}
	void DirectionLight::serialize_json(Data::Json& archivie)
	{ }
	//deserialize
	void DirectionLight::deserialize(Data::Archive& archivie)
	{
		Data::deserialize(archivie, this);
	}
	void DirectionLight::deserialize_json(Data::Json& archivie)
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

	/*
		// Build a matrix for cropping light's projection    
		// Given vectors are in light's clip space 
		Matrix Light::CalculateCropMatrix(Frustum splitFrustum) 
		{
			Matrix lightViewProjMatrix = viewMatrix * projMatrix;   
			// Find boundaries in light's clip space   
			BoundingBox cropBB = CreateAABB(splitFrustum.AABB, lightViewProjMatrix);   
			// Use default near-plane value   cropBB.min.z = 0.0f;  
			// Create the crop matrix  
			float scaleX, scaleY, scaleZ;   
			float offsetX, offsetY, offsetZ;   
			scaleX = 2.0f / (cropBB.max.x - cropBB.min.x);   
			scaleY = 2.0f / (cropBB.max.y - cropBB.min.y);   
			offsetX = -0.5f * (cropBB.max.x + cropBB.min.x) * scaleX;   
			offsetY = -0.5f * (cropBB.max.y + cropBB.min.y) * scaleY;   
			scaleZ = 1.0f / (cropBB.max.z - cropBB.min.z);   
			offsetZ = -cropBB.min.z * scaleZ;   
			return Matrix(  scaleX,     0.0f,     0.0f,  0.0f,
							  0.0f,   scaleY,     0.0f,  0.0f,
							  0.0f,     0.0f,   scaleZ,  0.0f,
							offsetX,  offsetY,  offsetZ, 1.0f); 
    
		}

		Matrix Light::CalculateCropMatrix(ObjectList casters, ObjectList receivers, Frustum frustum) {
			// Bounding boxes   BoundingBox receiverBB, casterBB, splitBB;
			Matrix lightViewProjMatrix = viewMatrix * projMatrix;
			// Merge all bounding boxes of casters into a bigger "casterBB".
			for(int i = 0; i < casters.size(); i++){
				BoundingBox bb = CreateAABB(casters[i]->AABB, lightViewProjMatrix);
				casterBB.Union(bb);
			}
			// Merge all bounding boxes of receivers into a bigger "receiverBB".
			for(int i = 0; i < receivers.size(); i++){
				bb = CreateAABB(receivers[i]->AABB, lightViewProjMatrix);
				receiverBB.Union(bb);
			}
			// Find the bounding box of the current split
			// in the light's clip space.
			splitBB = CreateAABB(splitFrustum.AABB, lightViewProjMatrix);
			// Scene-dependent bounding volume
			BoundingBox cropBB;
			cropBB.min.x = Max(Max(casterBB.min.x, receiverBB.min.x), splitBB.min.x);
			cropBB.max.x = Min(Min(casterBB.max.x, receiverBB.max.x), splitBB.max.x);
			cropBB.min.y = Max(Max(casterBB.min.y, receiverBB.min.y), splitBB.min.y);
			cropBB.max.y = Min(Min(casterBB.max.y, receiverBB.max.y), splitBB.max.y);
			cropBB.min.z = Min(casterBB.min.z, splitBB.min.z);
			cropBB.max.z = Min(receiverBB.max.z, splitBB.max.z);
			// Create the crop matrix.
			float scaleX, scaleY, scaleZ;
			float offsetX, offsetY, offsetZ;
			scaleX = 2.0f / (cropBB.max.x - cropBB.min.x);
			scaleY = 2.0f / (cropBB.max.y - cropBB.min.y);
			offsetX = -0.5f * (cropBB.max.x + cropBB.min.x) * scaleX;
			offsetY = -0.5f * (cropBB.max.y + cropBB.min.y) * scaleY;
			scaleZ = 1.0f / (cropBB.max.z – cropBB.min.z);
			offsetZ = -cropBB.min.z * scaleZ;
			return Matrix(scaleX, 0.0f, 0.0f, 0.0f,
			              0.0f, scaleY, 0.0f, 0.0f,
		                  0.0f, 0.0f, scaleZ, 0.0f,
			              offsetX, offsetY, offsetZ, 1.0f);
		}
	*/

#if 1

	static std::array<Vec4, 8> get_frustum_corners_worldapace(const Mat4& projview)
	{
		const auto inv = inverse(projview);
		std::array<Vec4, 8> corners;
		size_t i = 0;
		for (unsigned short x = 0; x < 2; ++x)
		for (unsigned short y = 0; y < 2; ++y)
		for (unsigned short z = 0; z < 2; ++z)
		{
			const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
			corners[i++] = pt / pt.w;
		};
		return std::move(corners);
	}

	static std::tuple<CSMCascadeDepth, CSMMatrixArray, CSMMatrixArray> get_light_space_matrices(const Vec3& light_direction,
																							    const unsigned int texture_width,
																							    const unsigned int texture_height,
																							    const Render::Camera* camera,
																							    const float z_distance_shadow = 100.0f)
	{
		CSMCascadeDepth  cascade_depth;
		CSMMatrixArray   cascade_projection;
		CSMMatrixArray   cascade_view;

		float near_clip = camera->viewport().near();
		float far_clip = camera->viewport().far();
		float clip_range = far_clip - near_clip;

		float min_z = near_clip;
		float max_z = near_clip + clip_range;

		float range = max_z - min_z;
		float ratio = max_z / min_z;


		for (uint32_t i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
		{
			float p = (i + 1) / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
			float uniform = range * p;
			cascade_depth[i] = uniform;
		}

		for (uint32_t i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
		{
			const float near = i ? cascade_depth[i-1] : near_clip;
			const float far = cascade_depth[i];
			Mat4 projection = Square::perspective_NO(camera->viewport().fov(), camera->viewport().aspect(), near, far);
			std::array<Vec4, 8> corners = get_frustum_corners_worldapace(projection * camera->view());

			glm::vec3 center = glm::vec3(0, 0, 0);
			for (const auto& v : corners)
				center += glm::vec3(v);
			center /= corners.size();

			Vec3 light_dir = -normalize(light_direction);
			const auto light_view = look_at(center + light_dir, center, UP_LIGHT_DIR);

			float min_x = std::numeric_limits<float>::max();
			float max_x = std::numeric_limits<float>::lowest();
			float min_y = std::numeric_limits<float>::max();
			float max_y = std::numeric_limits<float>::lowest();
			float min_z = std::numeric_limits<float>::max();
			float max_z = std::numeric_limits<float>::lowest();
			for (const auto& v : corners)
			{
				const auto trf = light_view * v;
				min_x = std::min(min_x, trf.x);
				max_x = std::max(max_x, trf.x);
				min_y = std::min(min_y, trf.y);
				max_y = std::max(max_y, trf.y);
				min_z = std::min(min_z, trf.z);
				max_z = std::max(max_z, trf.z);
			}

			// Tune this parameter according to the scene
			if (min_z < 0) min_z *= z_distance_shadow;
			else           min_z /= z_distance_shadow;

			if (max_z < 0) max_z /= z_distance_shadow;
			else		   max_z *= z_distance_shadow;

			// Shadow projection
			const Mat4 zo_light_projection = Square::perspective(camera->viewport().fov(), camera->viewport().aspect(), near, far);
			const Mat4 zo_light_view = Square::ortho(min_x, max_x, min_y, max_y, min_z, max_z);

			// Result
			cascade_projection[i] = zo_light_projection;
			cascade_view[i] = zo_light_view;
		}
		return { cascade_depth, cascade_projection, cascade_view };
	}
#else

	static std::tuple<CSMMatrixArray, CSMCascadeDepth> get_light_space_matrices(const Vec3&        light_direction,
																				const unsigned int texture_width,
																				const unsigned int texture_height,
																				const Render::Camera* camera)
	{
		// Compute global informations
		CSMCascadeSplits cascade_splits;
		CSMCascadeDepth  cascade_depth;
		CSMMatrixArray   cascade_matrix;

		float near_clip = camera->viewport().near();
		float far_clip  = camera->viewport().far();
		float clip_range = far_clip - near_clip;

		float min_z = near_clip;
		float max_z = near_clip + clip_range;

		float range = max_z - min_z;
		float ratio = max_z / min_z;

		float last_split_dist = 0.0;
		const float cascade_split_lambda = 0.99f;

		Mat4 inv_cam = inverse(camera->projection() * camera->view());
		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
		{
#if 0
			float p = (i + 1) / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
			float log = min_z * std::pow(ratio, p);
			float uniform = min_z + range * p;
			float d = cascade_split_lambda * (log - uniform) + uniform;
			cascade_splits[i] = (d - near_clip) / clip_range;
#else 
			float p = (i + 1) / static_cast<float>(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
			float uniform =  range * p;
			cascade_splits[i] = uniform / clip_range;
#endif
		}

		// Calculate orthographic projection matrix for each cascade
		for (uint32_t i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; i++)
		{
			float split_dist = cascade_splits[i];

			Vec3 frustum_corners[8] = 
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
			const size_t frustum_corners_size{ SqareArrayLangth(frustum_corners)  };

			// Project frustum corners into world space
			for (uint32_t i = 0; i < frustum_corners_size; i++)
			{
				Vec4 inv_corner = inv_cam * Vec4(frustum_corners[i], 1.0f);
				frustum_corners[i] = inv_corner / inv_corner.w;
			}

			for (uint32_t i = 0; i < 4; i++)
			{
				Vec3 dist = frustum_corners[i + 4] - frustum_corners[i];
				frustum_corners[i + 4] = frustum_corners[i] + (dist * split_dist);
				frustum_corners[i] = frustum_corners[i] + (dist * last_split_dist);
			}

			// Get frustum center
			Vec3 frustum_center = Vec3(0.0f);
			for (uint32_t i = 0; i < frustum_corners_size; i++)
			{
				frustum_center += frustum_corners[i];
			}
			frustum_center /= float(frustum_corners_size);

			float radius = 0.0f;
			for (uint32_t i = 0; i < frustum_corners_size; i++)
			{
				float distance = length(frustum_corners[i] - frustum_center);
				radius = Square::max(radius, distance);
			}
			radius = Square::ceil(radius * 16.0f) / 16.0f;

			Vec3 maxExtents = Vec3(radius);
			Vec3 minExtents = -maxExtents;

			Vec3 light_dir = normalize(light_direction);
			Mat4 light_view_matrix  = look_at(frustum_center - light_dir * maxExtents.z,
											  frustum_center, 
											  Vec3(0.0f, 1.0f, 0.0f));
			Mat4 light_ortho_matrix = ortho(minExtents.x,
											maxExtents.x, 
											minExtents.y, 
											maxExtents.y, 
											0.0f, 
											maxExtents.z - minExtents.z);

			// Store split distance and matrix in cascade
			cascade_depth [i] = (near_clip + split_dist * clip_range) * -1.0f;
			cascade_matrix[i] = light_ortho_matrix * light_view_matrix;

			last_split_dist = cascade_splits[i];
		}
		return { cascade_matrix, cascade_depth };
	}
#endif 
	void DirectionLight::set(Render::UniformDirectionLight* data) const
	{
		data->m_direction = m_direction;
		this->Render::DirectionLight::set(data);
	}

	void DirectionLight::set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera) const
	{

		if (auto ptr_actor = actor().lock())
		{
			// Compute cascades
			auto& [cascade_vdepths, cascade_vproj, cascade_vview] = get_light_space_matrices
			(
				m_direction, m_buffer.width(), m_buffer.height(), camera
			);
			// Init to 0
			std::memset(data, 0, sizeof(Render::UniformDirectionShadowLight));
			// Copy values
			for (unsigned int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
			{
				data->m_depths[i] = cascade_vdepths[i];
				data->m_projection[i] = cascade_vproj[i];
				data->m_view[i] = cascade_vview[i];
			}

		}
	}

}
}