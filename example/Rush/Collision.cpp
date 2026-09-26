//
//  Collision.cpp
//  Rush
//
#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Square;

static Vec3 to_vec3(const Vec2& v) { return Vec3(v, 0.0f); }
static Vec3 to_vec3(const Vec3& v) { return v; }

//ray (origin + t * direction) vs triangle abc, both faces: true on a hit with t >= 0,
//out_t is the distance along the ray in units of direction (Moller-Trumbore)
static bool ray_triangle(const Vec3& origin, const Vec3& direction, const Vec3& a, const Vec3& b, const Vec3& c, float& out_t)
{
	const float epsilon = 1e-7f;
	const Vec3 edge1 = b - a;
	const Vec3 edge2 = c - a;
	const Vec3 p = cross(direction, edge2);
	const float det = dot(edge1, p);
	//ray parallel to the triangle
	if (std::abs(det) < epsilon) return false;
	const float inv_det = 1.0f / det;
	const Vec3 s = origin - a;
	const float u = dot(s, p) * inv_det;
	if (u < 0.0f || u > 1.0f) return false;
	const Vec3 q = cross(s, edge1);
	const float v = dot(direction, q) * inv_det;
	if (v < 0.0f || u + v > 1.0f) return false;
	const float t = dot(edge2, q) * inv_det;
	if (t < 0.0f) return false;
	out_t = t;
	return true;
}

//point of the triangle abc closest to point (Ericson, Real-Time Collision Detection 5.1.5)
static Vec3 closest_point_triangle(const Vec3& point, const Vec3& a, const Vec3& b, const Vec3& c)
{
	//vertex region a
	const Vec3 ab = b - a;
	const Vec3 ac = c - a;
	const Vec3 ap = point - a;
	const float d1 = dot(ab, ap);
	const float d2 = dot(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f) return a;
	//vertex region b
	const Vec3 bp = point - b;
	const float d3 = dot(ab, bp);
	const float d4 = dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) return b;
	//edge region ab
	const float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) return a + ab * (d1 / (d1 - d3));
	//vertex region c
	const Vec3 cp = point - c;
	const float d5 = dot(ab, cp);
	const float d6 = dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) return c;
	//edge region ac
	const float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) return a + ac * (d2 / (d2 - d6));
	//edge region bc
	const float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) return b + (c - b) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));
	//inside the face
	const float denom = 1.0f / (va + vb + vc);
	return a + ab * (vb * denom) + ac * (vc * denom);
}

void CollisionMesh::add(Context& context, const Shared<Scene::Actor>& actor)
{
	actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		auto static_mesh = node->component<Scene::StaticMesh>();
		if (!static_mesh->m_mesh) return true;
		//the mesh file: the GPU mesh keeps no copy of its triangles
		const std::string& path = context.resource_path<Resource::Mesh>(static_mesh->m_mesh->resource_untyped_name());
		if (path.empty()) return true;
		const bool compressed = Filesystem::get_extension(path) == ".sm3dgz";
		const std::vector<unsigned char> bytes = compressed ? Filesystem::binary_compress_file_read_all(path)
		                                                    : Filesystem::binary_file_read_all(path);
		Parser::StaticMesh::Context mesh;
		if (!Parser::StaticMesh().parse(mesh, bytes))
		{
			context.logger()->warning("CollisionMesh: unable to read " + path);
			return true;
		}
		//positions in world space
		const Mat4 model = node->global_model_matrix();
		std::vector<Vec3> positions;
		std::visit([&](const auto& vertices)
		{
			positions.reserve(vertices.size());
			for (const auto& vertex : vertices)
			{
				positions.push_back(Vec3(model * Vec4(to_vec3(vertex.m_position), 1.0f)));
			}
		}, mesh.m_vertex);
		//one sub mesh per material (the whole mesh when there are none)
		Render::Mesh::SubMeshList submeshes = mesh.m_submesh;
		if (submeshes.empty())
		{
			const size_t count = mesh.m_index.empty() ? positions.size() : mesh.m_index.size();
			submeshes.push_back(Render::SubMesh(Render::DRAW_TRIANGLES, (unsigned int)count, 0));
		}
		for (size_t submesh_id = 0; submesh_id < submeshes.size(); ++submesh_id)
		{
			const Render::SubMesh& submesh = submeshes[submesh_id];
			if (submesh.m_draw_type != Render::DRAW_TRIANGLES) continue;
			//only opaque surfaces are solid
			if (auto material = static_mesh->material(submesh_id).lock())
			{
				if (material->queue().m_type != Render::RQ_OPAQUE) continue;
			}
			auto vertex_id = [&](unsigned int i) -> size_t
			{
				const size_t n = size_t(submesh.m_index_offset) + i;
				return mesh.m_index.empty() ? n : size_t(mesh.m_index[n]);
			};
			for (unsigned int i = 0; i + 2 < submesh.m_index_count; i += 3)
			{
				const size_t a = vertex_id(i), b = vertex_id(i + 1), c = vertex_id(i + 2);
				if (a < positions.size() && b < positions.size() && c < positions.size())
				{
					add_triangle(positions[a], positions[b], positions[c]);
				}
			}
		}
		return true;
	});
}

void CollisionMesh::add_triangle(const Vec3& a, const Vec3& b, const Vec3& c)
{
	const Vec3 normal = cross(b - a, c - a);
	const float area2 = length(normal);
	//degenerate triangle
	if (area2 < 1e-8f) return;
	Triangle triangle;
	triangle.m_a = a;
	triangle.m_b = b;
	triangle.m_c = c;
	triangle.m_normal = normal / area2;
	triangle.m_min = Vec3(std::min({ a.x, b.x, c.x }), std::min({ a.y, b.y, c.y }), std::min({ a.z, b.z, c.z }));
	triangle.m_max = Vec3(std::max({ a.x, b.x, c.x }), std::max({ a.y, b.y, c.y }), std::max({ a.z, b.z, c.z }));
	m_triangles.push_back(triangle);
}

bool CollisionMesh::bounds(const Mat4& transform, Vec3& out_min, Vec3& out_max) const
{
	if (m_triangles.empty()) return false;
	out_min = Vec3(std::numeric_limits<float>::max());
	out_max = Vec3(std::numeric_limits<float>::lowest());
	for (const Triangle& triangle : m_triangles)
	{
		for (const Vec3& vertex : { triangle.m_a, triangle.m_b, triangle.m_c })
		{
			const Vec3 point = Vec3(transform * Vec4(vertex, 1.0f));
			out_min = glm::min(out_min, point);
			out_max = glm::max(out_max, point);
		}
	}
	return true;
}

bool CollisionMesh::raycast(const Vec3& origin, const Vec3& direction, float max_distance, Hit& hit) const
{
	//bounds of the segment, to skip most triangles
	const Vec3 end = origin + direction * max_distance;
	const Vec3 segment_min(std::min(origin.x, end.x), std::min(origin.y, end.y), std::min(origin.z, end.z));
	const Vec3 segment_max(std::max(origin.x, end.x), std::max(origin.y, end.y), std::max(origin.z, end.z));
	bool found = false;
	float best = max_distance;
	for (const Triangle& triangle : m_triangles)
	{
		if (triangle.m_max.x < segment_min.x || triangle.m_min.x > segment_max.x
		||  triangle.m_max.y < segment_min.y || triangle.m_min.y > segment_max.y
		||  triangle.m_max.z < segment_min.z || triangle.m_min.z > segment_max.z)
			continue;
		float distance = 0.0f;
		if (ray_triangle(origin, direction, triangle.m_a, triangle.m_b, triangle.m_c, distance)
		&&  distance <= best)
		{
			best = distance;
			hit.m_normal = triangle.m_normal;
			found = true;
		}
	}
	if (found)
	{
		hit.m_distance = best;
		hit.m_point = origin + direction * best;
	}
	return found;
}

bool CollisionMesh::push_out(Vec3& center, float radius, float max_normal_y) const
{
	bool moved = false;
	for (const Triangle& triangle : m_triangles)
	{
		//floors and ceilings: the ground probes take care of them
		if (std::abs(triangle.m_normal.y) >= max_normal_y) continue;
		if (center.x + radius < triangle.m_min.x || center.x - radius > triangle.m_max.x
		||  center.y + radius < triangle.m_min.y || center.y - radius > triangle.m_max.y
		||  center.z + radius < triangle.m_min.z || center.z - radius > triangle.m_max.z)
			continue;
		const Vec3 closest = closest_point_triangle(center, triangle.m_a, triangle.m_b, triangle.m_c);
		//horizontal separation from the wall
		Vec3 delta = center - closest;
		delta.y = 0.0f;
		float distance = length(delta);
		if (distance >= radius) continue;
		Vec3 direction;
		if (distance > 1e-5f)
		{
			direction = delta / distance;
		}
		else
		{
			//center on the wall: push along its (horizontal) normal
			direction = Vec3(triangle.m_normal.x, 0.0f, triangle.m_normal.z);
			const float normal_length = length(direction);
			if (normal_length < 1e-5f) continue;
			direction /= normal_length;
			distance = 0.0f;
		}
		center += direction * (radius - distance);
		moved = true;
	}
	return moved;
}
