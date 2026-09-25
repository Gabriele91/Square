//
//  LightVolume.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/LightVolume.h"
#include <cmath>

namespace Square
{
namespace Render
{
namespace LightVolume
{
	Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings, unsigned int sectors)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		const float ring_step   = 1.0f / (float)(rings - 1);
		const float sector_step = 1.0f / (float)(sectors - 1);
		for (unsigned int ring = 0; ring < rings; ++ring)
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float y = std::sin(-Constants::pi<float>() * 0.5f + Constants::pi<float>() * ring * ring_step);
			const float x = std::cos(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			const float z = std::sin(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			vertices.push_back({ Vec3(x, y, z) });
		}
		for (unsigned int ring = 0; ring + 1 < rings; ++ring)
		for (unsigned int sector = 0; sector + 1 < sectors; ++sector)
		{
			unsigned int index_bottom_left  = ring * sectors + sector;
			unsigned int index_bottom_right = ring * sectors + (sector + 1);
			unsigned int index_top_right    = (ring + 1) * sectors + (sector + 1);
			unsigned int index_top_left     = (ring + 1) * sectors + sector;
			indices.insert(indices.end(), { index_bottom_left, index_bottom_right, index_top_right,
			                                index_bottom_left, index_top_right,    index_top_left });
		}
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		//apex (0) and base center (1)
		vertices.push_back({ Vec3(0.0f, 0.0f, 0.0f) });
		vertices.push_back({ Vec3(0.0f, 0.0f, 1.0f) });
		//base ring
		const unsigned int base_start = 2;
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float angle = 2.0f * Constants::pi<float>() * (float)sector / (float)sectors;
			vertices.push_back({ Vec3(std::cos(angle), std::sin(angle), 1.0f) });
		}
		//sides and base cap
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			unsigned int index_current = base_start + sector;
			unsigned int index_next    = base_start + (sector + 1) % sectors;
			indices.insert(indices.end(), { 0u, index_current, index_next });
			indices.insert(indices.end(), { 1u, index_next, index_current });
		}
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
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
		static constexpr Square::Vec3 cone_z_axis{ 0.0f, 0.0f, 1.0f };
		const Vec3  light_direction = normalize(uspot_light.m_direction);
		const float cone_height     = uspot_light.m_radius * 1.1f;
		const float cos_outer       = clamp(uspot_light.m_outer_cut_off, -1.0f, 1.0f);
		const float base_radius     = cone_height * std::tan(std::acos(cos_outer)) + cone_size_epsilon;
		//rotation from +z to the light direction
		const float cos_angle = clamp(dot(cone_z_axis, light_direction), -1.0f, 1.0f);
		Mat4 rotation(1.0f);
		if (cos_angle < -cone_rotation_epsilon)
		{
			rotation = to_mat4(angle_axis(Constants::pi<float>(), Vec3(1.0f, 0.0f, 0.0f)));
		}
		else if (cos_angle < cone_rotation_epsilon)
		{
			rotation = to_mat4(angle_axis(std::acos(cos_angle), normalize(cross(cone_z_axis, light_direction))));
		}
		return glm::translate(Mat4(1.0f), Vec3(uspot_light.m_position))
			 * rotation
			 * glm::scale(Mat4(1.0f), Vec3(base_radius, base_radius, cone_height));
	}
}
}
}
