//
//  BasicMesh.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/BasicMesh.h"
#include <cmath>

namespace Square
{
namespace Render
{
namespace BasicMesh
{
	Shared<Mesh> build_quad(Square::Context& context)
	{
		Mesh::Vertex3DList vertices
		{
			{ Vec3(-1.0f, -1.0f, 0.0f) },
			{ Vec3( 1.0f, -1.0f, 0.0f) },
			{ Vec3( 1.0f,  1.0f, 0.0f) },
			{ Vec3(-1.0f,  1.0f, 0.0f) },
		};
		Mesh::IndexList indices{ 0, 2, 1, 0, 3, 2 };
		auto mesh = MakeShared<Mesh>(context);
		return mesh->build(vertices, indices) ? mesh : nullptr;
	}

	Shared<Mesh> build_box(Square::Context& context, float size)
	{
		Mesh::Vertex3DList vertices
		{
			{ Vec3(-size,  size, -size) }, { Vec3( size,  size, -size) }, { Vec3( size,  size,  size) }, { Vec3(-size,  size,  size) }, // +Y
			{ Vec3(-size, -size,  size) }, { Vec3( size, -size,  size) }, { Vec3( size, -size, -size) }, { Vec3(-size, -size, -size) }, // -Y
			{ Vec3( size,  size,  size) }, { Vec3( size,  size, -size) }, { Vec3( size, -size, -size) }, { Vec3( size, -size,  size) }, // +X
			{ Vec3(-size,  size, -size) }, { Vec3(-size,  size,  size) }, { Vec3(-size, -size,  size) }, { Vec3(-size, -size, -size) }, // -X
			{ Vec3(-size,  size,  size) }, { Vec3( size,  size,  size) }, { Vec3( size, -size,  size) }, { Vec3(-size, -size,  size) }, // +Z
			{ Vec3( size,  size, -size) }, { Vec3(-size,  size, -size) }, { Vec3(-size, -size, -size) }, { Vec3( size, -size, -size) }  // -Z
		};
		Mesh::IndexList indices
		{
			0, 2, 1,    0, 3, 2,
			4, 6, 5,    4, 7, 6,
			8, 10, 9,   8, 11, 10,
			12, 14, 13, 12, 15, 14,
			16, 18, 17, 16, 19, 18,
			20, 22, 21, 20, 23, 22
		};
		SubMesh submesh{ DrawType::DRAW_TRIANGLES, uint32(indices.size()) };
		auto mesh = MakeShared<Mesh>(context);
		return mesh->build(vertices, indices, { submesh }, false) ? mesh : nullptr;
	}

	Shared<Mesh> build_frustum_box(Square::Context& context)
	{
		Mesh::Vertex3DList vertices
		{
			{ Vec3(-1,  1, 0) }, { Vec3( 1,  1, 0) }, { Vec3( 1,  1, 1) }, { Vec3(-1,  1, 1) }, // +Y
			{ Vec3(-1, -1, 1) }, { Vec3( 1, -1, 1) }, { Vec3( 1, -1, 0) }, { Vec3(-1, -1, 0) }, // -Y
			{ Vec3( 1,  1, 1) }, { Vec3( 1,  1, 0) }, { Vec3( 1, -1, 0) }, { Vec3( 1, -1, 1) }, // +X
			{ Vec3(-1,  1, 0) }, { Vec3(-1,  1, 1) }, { Vec3(-1, -1, 1) }, { Vec3(-1, -1, 0) }, // -X
			{ Vec3(-1,  1, 1) }, { Vec3( 1,  1, 1) }, { Vec3( 1, -1, 1) }, { Vec3(-1, -1, 1) }, // far
			{ Vec3( 1,  1, 0) }, { Vec3(-1,  1, 0) }, { Vec3(-1, -1, 0) }, { Vec3( 1, -1, 0) }  // near
		};
		Mesh::IndexList indices
		{
			0, 2, 1,    0, 3, 2,
			4, 6, 5,    4, 7, 6,
			8, 10, 9,   8, 11, 10,
			12, 14, 13, 12, 15, 14,
			16, 18, 17, 16, 19, 18,
			20, 22, 21, 20, 23, 22
		};
		SubMesh submesh{ DrawType::DRAW_TRIANGLES, uint32(indices.size()) };
		auto mesh = MakeShared<Mesh>(context);
		return mesh->build(vertices, indices, { submesh }, false) ? mesh : nullptr;
	}

	Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings, unsigned int sectors, bool circumscribed)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		const float ring_step   = 1.0f / (float)(rings - 1);
		const float sector_step = 1.0f / (float)(sectors - 1);
		//the flat faces between the vertices lie inside the unit sphere: circumscribed, the
		//vertices are pushed out so that the faces enclose it
		const float scale = circumscribed
		                  ? 1.124f / (std::cos(Constants::pi<float>() * sector_step)          // half sector angle
		                            * std::cos(Constants::pi<float>() * 0.5f * ring_step))    // half ring angle
		                  : 1.0f;
		for (unsigned int ring = 0; ring < rings; ++ring)
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float y = std::sin(-Constants::pi<float>() * 0.5f + Constants::pi<float>() * ring * ring_step);
			const float x = std::cos(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			const float z = std::sin(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			vertices.push_back({ Vec3(x, y, z) * scale });
		}
		for (unsigned int ring = 0; ring + 1 < rings; ++ring)
		for (unsigned int sector = 0; sector + 1 < sectors; ++sector)
		{
			const unsigned int index_bottom_left  = ring * sectors + sector;
			const unsigned int index_bottom_right = ring * sectors + (sector + 1);
			const unsigned int index_top_right    = (ring + 1) * sectors + (sector + 1);
			const unsigned int index_top_left     = (ring + 1) * sectors + sector;
			indices.insert(indices.end(), { index_bottom_left, index_bottom_right, index_top_right,
			                                index_bottom_left, index_top_right,    index_top_left });
		}
		auto mesh = MakeShared<Mesh>(context);
		return mesh->build(vertices, indices) ? mesh : nullptr;
	}

	Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors, bool circumscribed)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		vertices.reserve(2 + sectors);
		indices.reserve(6 * sectors);
		//apex (0) and base center (1)
		vertices.push_back({ Vec3(0.0f) });
		vertices.push_back({ Constants::axis_z });
		//base ring: circumscribed, a polygon whose sides touch the circle (an inscribed one
		//cuts the circle with its flat sides)
		const unsigned int base_start = 2;
		const float scale = circumscribed ? 1.124f / std::cos(Constants::pi<float>() / (float)sectors) : 1.0f;
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float angle = 2.0f * Constants::pi<float>() * (float)sector / (float)sectors;
			vertices.push_back({ Vec3(std::cos(angle) * scale, std::sin(angle) * scale, 1.0f) });
		}
		//sides and base cap
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const unsigned int index_current = base_start + sector;
			const unsigned int index_next    = base_start + (sector + 1) % sectors;
			indices.insert(indices.end(), { 0u, index_current, index_next });
			indices.insert(indices.end(), { 1u, index_next, index_current });
		}
		auto mesh = MakeShared<Mesh>(context);
		return mesh->build(vertices, indices) ? mesh : nullptr;
	}
}
}
}
