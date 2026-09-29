//
//  LightVolume.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/LightVolume.h"
#include "Square/Render/BasicMesh.h"
#include <cmath>

namespace Square
{
namespace Render
{
namespace LightVolume
{
	Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings, unsigned int sectors)
	{
		//circumscribed: the volume must never cut the light
		return BasicMesh::build_sphere(context, rings, sectors, true);
	}

	Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors)
	{
		return BasicMesh::build_cone(context, sectors, true);
	}

	Mat4 point_light_model(const UniformPointLight& upoint_light)
	{
		const float sphere_scale = upoint_light.m_radius * 1.1f;
		return glm::translate(Mat4(1.0f), upoint_light.m_position)
			 * glm::scale(Mat4(1.0f), Vec3(sphere_scale, sphere_scale, sphere_scale));
	}

	Mat4 spot_light_model(const UniformSpotLight& uspot_light)
	{
		static constexpr float cone_rotation_epsilon{ 0.9999f };
		static constexpr float cone_size_epsilon{  1e-3f };
		const Vec3  light_direction = normalize(uspot_light.m_direction);
		const float cone_height     = uspot_light.m_radius * 1.1f;
		const float cos_outer       = clamp(uspot_light.m_outer_cut_off, -1.0f, 1.0f);
		const float base_radius     = cone_height * std::tan(std::acos(cos_outer)) + cone_size_epsilon;
		//rotation from +z to the light direction
		const float cos_angle = clamp(dot(Constants::axis_z, light_direction), -1.0f, 1.0f);
		Mat4 rotation(1.0f);
		if (cos_angle < -cone_rotation_epsilon)
		{
			rotation = to_mat4(angle_axis(Constants::pi<float>(), Constants::axis_x));
		}
		else if (cos_angle < cone_rotation_epsilon)
		{
			rotation = to_mat4(angle_axis(std::acos(cos_angle), normalize(cross(Constants::axis_z, light_direction))));
		}
		return glm::translate(Mat4(1.0f), Vec3(uspot_light.m_position))
			 * rotation
			 * glm::scale(Mat4(1.0f), Vec3(base_radius, base_radius, cone_height));
	}
}
}
}
