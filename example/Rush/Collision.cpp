//
//  Collision.cpp
//  Rush
//
//  The collisions of Blitz3D (github.com/blitz-research/blitz3d: collision.cpp,
//  meshcollider.cpp, world.cpp), with the vectors of Square.
//
#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Square;

using Line = CollisionMesh::Line;

namespace
{
	//a contact plane is moved out of this much (Blitz3D COLLISION_EPSILON)
	const float COLLISION_EPSILON = 0.001f;
	const float EPSILON = 0.000001f;
	//triangles in a leaf of the tree
	const size_t MAX_LEAF_TRIANGLES = 16;
	//hits of a move before it gives up (and goes back to where it was)
	const int MAX_HITS = 10;

	Vec3 to_vec3(const Vec2& v) { return Vec3(v, 0.0f); }
	Vec3 to_vec3(const Vec3& v) { return v; }

	//nearest point of a line to q
	Vec3 nearest(const Line& line, const Vec3& q)
	{
		const float dd = dot(line.m_direction, line.m_direction);
		if (dd <= 0.0f) return line.m_origin;
		return line.m_origin + line.m_direction * (dot(line.m_direction, q - line.m_origin) / dd);
	}

	//plane n.x + d = 0 (n normalized)
	struct Plane
	{
		Vec3  m_normal{ 0.0f, 1.0f, 0.0f };
		float m_d{ 0.0f };

		Plane() = default;
		Plane(const Vec3& point, const Vec3& normal) : m_normal(normal), m_d(-dot(normal, point)) {}
		//through three points, normal (v1 - v0) x (v2 - v0)
		static Plane from(const Vec3& v0, const Vec3& v1, const Vec3& v2)
		{
			return Plane(v0, normalize(cross(v1 - v0, v2 - v0)));
		}

		float distance(const Vec3& v) const { return dot(m_normal, v) + m_d; }
		float t_intersect(const Line& line) const { return -distance(line.m_origin) / dot(m_normal, line.m_direction); }
		Vec3  nearest(const Vec3& v) const { return v - m_normal * distance(v); }
		//the line of two planes (not parallel)
		Line intersect(const Plane& q) const
		{
			const float c = dot(m_normal, q.m_normal);
			const float det = 1.0f - c * c;
			const float h1 = -m_d, h2 = -q.m_d;
			Line line;
			line.m_origin = (m_normal * (h1 - h2 * c) + q.m_normal * (h2 - h1 * c)) / det;
			line.m_direction = normalize(cross(m_normal, q.m_normal));
			return line;
		}
	};

	//smaller root of a t^2 + b t + c, false if none
	bool smaller_root(float a, float b, float c, float& t)
	{
		const float d = b * b - 4.0f * a * c;
		if (d < 0.0f) return false;
		const float s = std::sqrt(d);
		const float t1 = (-b + s) / (2.0f * a);
		const float t2 = (-b - s) / (2.0f * a);
		t = std::min(t1, t2);
		return true;
	}

	//Blitz3D Collision::update: a contact at t with normal n, if it is the first one and the
	//sphere is going into it (and it was not already behind it)
	bool update(CollisionMesh::Collision& collision, const Line& line, float t, const Vec3& n)
	{
		if (t > collision.m_time) return false;
		const Plane plane(line.at(t), n);
		if (dot(plane.m_normal, line.m_direction) >= 0.0f) return false;
		if (plane.distance(line.m_origin) < -COLLISION_EPSILON) return false;
		collision.m_time = t;
		collision.m_normal = n;
		return true;
	}

	//Blitz3D edgeTest: the sphere against the edge v0 v1 (a cylinder of radius), and the
	//vertex v0 (a sphere); pn is the triangle normal, en the edge plane normal
	bool edge_test(const Vec3& v0, const Vec3& v1, const Vec3& pn, const Vec3& en, const Line& line, float radius, CollisionMesh::Collision& collision)
	{
		//edge space: x the edge plane normal, y along the edge, z the triangle normal
		const Mat3 basis(en, normalize(v1 - v0), pn);
		const Mat3 to_edge = transpose(basis);
		const Vec3 sv = to_edge * (line.m_origin - v0);
		const Vec3 dv = to_edge * (line.m_origin + line.m_direction - v0);
		Line l;
		l.m_origin = sv;
		l.m_direction = dv - sv;
		//cylinder around y
		float a = l.m_direction.x * l.m_direction.x + l.m_direction.z * l.m_direction.z;
		if (a <= 0.0f) return false; //parallel to the cylinder
		float b = (l.m_origin.x * l.m_direction.x + l.m_origin.z * l.m_direction.z) * 2.0f;
		float c = (l.m_origin.x * l.m_origin.x + l.m_origin.z * l.m_origin.z) - radius * radius;
		float t = 0.0f;
		if (!smaller_root(a, b, c, t)) return false;   //misses the cylinder
		if (t > collision.m_time) return false;        //too far
		Vec3 i = l.at(t), p(0.0f);
		if (i.y > length(v1 - v0)) return false;       //over the end of the edge
		if (i.y >= 0.0f)
		{
			p.y = i.y;
		}
		else
		{
			//under the start: the sphere of the vertex
			a = dot(l.m_direction, l.m_direction);
			if (a <= 0.0f) return false;
			b = dot(l.m_origin, l.m_direction) * 2.0f;
			c = dot(l.m_origin, l.m_origin) - radius * radius;
			if (!smaller_root(a, b, c, t)) return false;
			if (t > collision.m_time) return false;
			i = l.at(t);
		}
		return update(collision, line, t, normalize(basis * (i - p)));
	}

	//Blitz3D Collision::triangleCollide, both faces: the face facing the move is used
	bool triangle_collide(const Line& line, float radius, Vec3 v0, Vec3 v1, Vec3 v2, CollisionMesh::Collision& collision)
	{
		Plane p = Plane::from(v0, v1, v2);
		if (dot(p.m_normal, line.m_direction) >= 0.0f)
		{
			//the other face
			std::swap(v1, v2);
			p = Plane::from(v0, v1, v2);
			if (dot(p.m_normal, line.m_direction) >= 0.0f) return false; //parallel
		}
		//move the plane out of radius
		Plane moved = p;
		moved.m_d -= radius;
		const float t = moved.t_intersect(line);
		if (t > collision.m_time) return false;
		//edge planes (pointing inside)
		const Plane p0 = Plane::from(v0 + p.m_normal, v1, v0);
		const Plane p1 = Plane::from(v1 + p.m_normal, v2, v1);
		const Plane p2 = Plane::from(v2 + p.m_normal, v0, v2);
		//on the face?
		const Vec3 i = line.at(t);
		if (p0.distance(i) >= 0.0f && p1.distance(i) >= 0.0f && p2.distance(i) >= 0.0f)
		{
			return update(collision, line, t, p.m_normal);
		}
		if (radius <= 0.0f) return false;
		//the edges and the vertices (all of them: the first contact wins)
		const bool e0 = edge_test(v0, v1, p.m_normal, p0.m_normal, line, radius, collision);
		const bool e1 = edge_test(v1, v2, p.m_normal, p1.m_normal, line, radius, collision);
		const bool e2 = edge_test(v2, v0, p.m_normal, p2.m_normal, line, radius, collision);
		return e0 || e1 || e2;
	}

	bool boxes_overlap(const Vec3& a_min, const Vec3& a_max, const Vec3& b_min, const Vec3& b_max)
	{
		return a_min.x <= b_max.x && a_max.x >= b_min.x
		    && a_min.y <= b_max.y && a_max.y >= b_min.y
		    && a_min.z <= b_max.z && a_max.z >= b_min.z;
	}

	//Blitz3D World::collide (sphere, no y scale): the sphere moves from sv to dv; collide(line,
	//collision, response) finds the first contact along line (with the response of what it
	//hit), report(line, collision) gets every hit; where it ends
	template < class Collide, class Report >
	Vec3 slide_move(Vec3 sv, Vec3 dv, Collide collide, Report report)
	{
		if (sv == dv) return dv;
		const Vec3 panic = sv;

		int n_hit = 0;
		Plane planes[2];
		Line coll_line;
		coll_line.m_origin = sv;
		coll_line.m_direction = dv - sv;
		const Vec3 dir = coll_line.m_direction;
		float td = length(coll_line.m_direction);
		float td_xz = length(Vec3(coll_line.m_direction.x, 0.0f, coll_line.m_direction.z));

		int hits = 0;
		for (;;)
		{
			CollisionMesh::Collision coll;
			CollisionResponse response = CollisionResponse::SLIDE;
			if (!collide(coll_line, coll, response)) break;

			//register collision
			if (++hits == MAX_HITS) break;
			report(coll_line, coll);

			Plane coll_plane(coll_line.at(coll.m_time), coll.m_normal);
			coll_plane.m_d -= COLLISION_EPSILON;
			coll.m_time = coll_plane.t_intersect(coll_line);

			if (coll.m_time > 0.0f)
			{
				//update source position - only if ahead
				sv = coll_line.at(coll.m_time);
				td *= 1.0f - coll.m_time;
				td_xz *= 1.0f - coll.m_time;
			}

			//nearest point on the plane to the destination
			const Vec3 nv = coll_plane.nearest(dv);

			if (n_hit == 0)
			{
				dv = nv;
			}
			else if (n_hit == 1)
			{
				if (planes[0].distance(nv) >= 0.0f)
				{
					dv = nv;
					n_hit = 0;
				}
				else if (std::abs(dot(planes[0].m_normal, coll_plane.m_normal)) < 1.0f - EPSILON)
				{
					//along the crease of the two planes
					dv = nearest(coll_plane.intersect(planes[0]), dv);
				}
				else
				{
					//squished
					hits = MAX_HITS;
					break;
				}
			}
			else if (planes[0].distance(nv) >= 0.0f && planes[1].distance(nv) >= 0.0f)
			{
				dv = nv;
				n_hit = 0;
			}
			else
			{
				dv = sv;
				break;
			}

			Vec3 dd = dv - sv;

			//going behind the initial direction
			if (dot(dd, dir) <= 0.0f)
			{
				dv = sv;
				break;
			}

			if (response == CollisionResponse::SLIDE)
			{
				const float d = length(dd);
				if (d <= EPSILON)
				{
					dv = sv;
					break;
				}
				if (d > td) dd *= td / d;
			}
			else if (response == CollisionResponse::SLIDEXZ)
			{
				const float d = length(Vec3(dd.x, 0.0f, dd.z));
				if (d <= EPSILON)
				{
					dv = sv;
					break;
				}
				if (d > td_xz) dd *= td_xz / d;
			}

			coll_line.m_origin = sv;
			coll_line.m_direction = dd;
			dv = sv + dd;
			planes[n_hit++] = coll_plane;
		}

		if (hits >= MAX_HITS) return panic;
		return dv;
	}

	//the world position of an actor (in its parent space if it has one)
	void world_position(Scene::Actor& actor, const Vec3& position)
	{
		if (auto parent = actor.parent().lock())
		{
			actor.position(Vec3(inverse(parent->global_model_matrix()) * Vec4(position, 1.0f)));
		}
		else
		{
			actor.position(position);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionMesh: triangles
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
	//the tree of the triangles
	build();
}

void CollisionMesh::clear()
{
	m_triangles.clear();
	m_nodes.clear();
}

void CollisionMesh::add_triangle(const Vec3& a, const Vec3& b, const Vec3& c)
{
	//degenerate triangle
	if (length(cross(b - a, c - a)) < 1e-8f) return;
	Triangle triangle;
	triangle.m_a = a;
	triangle.m_b = b;
	triangle.m_c = c;
	triangle.m_min = glm::min(a, glm::min(b, c));
	triangle.m_max = glm::max(a, glm::max(b, c));
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

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionMesh: tree (Blitz3D MeshCollider)
void CollisionMesh::build()
{
	m_nodes.clear();
	if (m_triangles.empty()) return;
	std::vector<int> all(m_triangles.size());
	for (size_t i = 0; i < all.size(); ++i) all[i] = int(i);
	build_node(all);
}

int CollisionMesh::build_node(std::vector<int>& triangles)
{
	const int node_id = int(m_nodes.size());
	m_nodes.emplace_back();
	//box of the triangles
	Vec3 box_min(std::numeric_limits<float>::max()), box_max(std::numeric_limits<float>::lowest());
	for (int id : triangles)
	{
		box_min = glm::min(box_min, m_triangles[id].m_min);
		box_max = glm::max(box_max, m_triangles[id].m_max);
	}
	m_nodes[node_id].m_min = box_min;
	m_nodes[node_id].m_max = box_max;
	//leaf
	if (triangles.size() <= MAX_LEAF_TRIANGLES)
	{
		m_nodes[node_id].m_triangles = triangles;
		return node_id;
	}
	//split on the longest axis, by the centers of the triangles
	const Vec3 size = box_max - box_min;
	const int axis = (size.y > size.x && size.y >= size.z) ? 1 : (size.z > size.x && size.z > size.y) ? 2 : 0;
	auto center = [&](int id) { const Triangle& t = m_triangles[id]; return (t.m_a[axis] + t.m_b[axis] + t.m_c[axis]) / 3.0f; };
	std::sort(triangles.begin(), triangles.end(), [&](int l, int r) { return center(l) < center(r); });
	std::vector<int> left(triangles.begin(), triangles.begin() + triangles.size() / 2);
	std::vector<int> right(triangles.begin() + triangles.size() / 2, triangles.end());
	const int left_id = build_node(left);
	const int right_id = build_node(right);
	m_nodes[node_id].m_left = left_id;
	m_nodes[node_id].m_right = right_id;
	return node_id;
}

bool CollisionMesh::collide(const Line& line, float radius, Collision& collision, float y_scale) const
{
	if (m_nodes.empty()) return false;
	//box of the move, back in world space (the tree is in world space)
	const Vec3 end = line.at(1.0f);
	const Vec3 unscale(1.0f, 1.0f / y_scale, 1.0f);
	const Vec3 box_min = (glm::min(line.m_origin, end) - Vec3(radius)) * unscale;
	const Vec3 box_max = (glm::max(line.m_origin, end) + Vec3(radius)) * unscale;
	return collide(line, radius, y_scale, box_min, box_max, 0, collision);
}

bool CollisionMesh::collide(const Line& line, float radius, float y_scale, const Vec3& box_min, const Vec3& box_max, int node_id, Collision& collision) const
{
	const Node& node = m_nodes[node_id];
	if (!boxes_overlap(box_min, box_max, node.m_min, node.m_max)) return false;
	bool hit = false;
	if (node.m_triangles.empty())
	{
		if (node.m_left >= 0)  hit |= collide(line, radius, y_scale, box_min, box_max, node.m_left, collision);
		if (node.m_right >= 0) hit |= collide(line, radius, y_scale, box_min, box_max, node.m_right, collision);
		return hit;
	}
	const Vec3 scale(1.0f, y_scale, 1.0f);
	for (int id : node.m_triangles)
	{
		const Triangle& triangle = m_triangles[id];
		if (!boxes_overlap(box_min, box_max, triangle.m_min, triangle.m_max)) continue;
		if (!triangle_collide(line, radius, triangle.m_a * scale, triangle.m_b * scale, triangle.m_c * scale, collision)) continue;
		collision.m_triangle = id;
		hit = true;
	}
	return hit;
}

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionMesh: queries
bool CollisionMesh::raycast(const Vec3& origin, const Vec3& direction, float max_distance, Hit& hit) const
{
	Line line;
	line.m_origin = origin;
	line.m_direction = direction * max_distance;
	Collision collision;
	if (!collide(line, 0.0f, collision)) return false;
	hit.m_distance = collision.m_time * max_distance;
	hit.m_point = line.at(collision.m_time);
	hit.m_normal = collision.m_normal;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//SphereCollider
SQUARE_CLASS_OBJECT_REGISTRATION(SphereCollider);

void SphereCollider::object_registration(Context& ctx)
{
	//factory: actor->component<SphereCollider>()
	ctx.add_object<SphereCollider>();
	//attributes
	ctx.add_attribute_function<SphereCollider, int>
	("type"
	, 0
	, [](const SphereCollider* collider) -> int { return collider->type(); }
	, [](SphereCollider* collider, const int& type) { collider->type(type); });
	ctx.add_attribute_function<SphereCollider, float>
	("radius"
	, 1.0f
	, [](const SphereCollider* collider) -> float { return collider->radius(); }
	, [](SphereCollider* collider, const float& radius) { collider->radius(radius); });
}

SphereCollider::SphereCollider(Context& context) : Component(context)
{
}

Vec3 SphereCollider::center() const
{
	auto owner = actor().lock();
	if (!owner) return m_offset;
	return owner->position(true) + owner->rotation(true) * m_offset;
}

bool SphereCollider::collided(int type, float min_normal_y) const
{
	for (const CollisionReport& report : m_collisions)
	{
		if (report.m_type == type && report.m_normal.y >= min_normal_y) return true;
	}
	return false;
}

void SphereCollider::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void SphereCollider::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void SphereCollider::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void SphereCollider::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }

//////////////////////////////////////////////////////////////////////////////////////////
//MeshCollider
SQUARE_CLASS_OBJECT_REGISTRATION(MeshCollider);

void MeshCollider::object_registration(Context& ctx)
{
	//factory: actor->component<MeshCollider>()
	ctx.add_object<MeshCollider>();
	//attributes
	ctx.add_attribute_function<MeshCollider, int>
	("type"
	, 0
	, [](const MeshCollider* collider) -> int { return collider->type(); }
	, [](MeshCollider* collider, const int& type) { collider->type(type); });
}

MeshCollider::MeshCollider(Context& context) : Component(context)
{
}

const CollisionMesh& MeshCollider::mesh()
{
	if (!m_built)
	{
		m_mesh.clear();
		if (auto owner = actor().lock()) m_mesh.add(context(), owner);
		m_built = true;
	}
	return m_mesh;
}

void MeshCollider::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void MeshCollider::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void MeshCollider::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void MeshCollider::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionWorld
CollisionWorld::CollisionWorld(Context& context, System& system, Scene::World& world)
: SystemInstance(context, system, world)
{
}

void CollisionWorld::collisions(int src_type, int dst_type, CollisionMethod method, CollisionResponse response)
{
	std::vector<Rule>& rules = m_rules[src_type];
	for (const Rule& rule : rules)
	{
		if (rule.m_dst_type == dst_type) return;
	}
	rules.push_back(Rule{ dst_type, method, response });
}

CollisionWorld::Colliders CollisionWorld::colliders() const
{
	Colliders colliders;
	for (const Shared<Scene::Level>& level : m_world.levels())
	{
		level->visit([&](Shared<Scene::Actor> actor) -> bool
		{
			if (actor->contains<SphereCollider>()) colliders.m_spheres.push_back(actor->component<SphereCollider>());
			if (actor->contains<MeshCollider>())   colliders.m_meshes.push_back(actor->component<MeshCollider>());
			return true;
		});
	}
	return colliders;
}

void CollisionWorld::update()
{
	const Colliders all = colliders();
	for (const Shared<SphereCollider>& sphere : all.m_spheres)
	{
		sphere->m_collisions.clear();
		collide(*sphere, all);
	}
}

void CollisionWorld::collide(SphereCollider& source, const Colliders& colliders)
{
	auto actor = source.actor().lock();
	if (!actor) return;
	const Vec3 position = source.center();
	//first update, or a reset: from here
	if (!source.m_has_previous)
	{
		source.m_previous = position;
		source.m_has_previous = true;
		return;
	}
	const Vec3 from = source.m_previous;
	auto rules = m_rules.find(source.type());
	if (rules == m_rules.end())
	{
		source.m_previous = position;
		return;
	}
	//an ellipsoid: a sphere of radius in a space where y is scaled by radius / radius_y
	//(Blitz3D y_scale)
	const float radius = source.radius();
	const float y_scale = radius / std::max(source.radius_y(), 1e-5f);
	const Vec3  scale(1.0f, y_scale, 1.0f), unscale(1.0f, 1.0f / y_scale, 1.0f);
	Weak<Scene::Actor> hit_actor;
	int hit_type = 0;
	const Vec3 resolved = unscale * slide_move(from * scale, position * scale,
	[&](const Line& line, CollisionMesh::Collision& collision, CollisionResponse& response) -> bool
	{
		bool hit = false;
		for (const Rule& rule : rules->second)
		{
			for (const Shared<MeshCollider>& mesh : colliders.m_meshes)
			{
				if (mesh->type() != rule.m_dst_type) continue;
				if (!mesh->mesh().collide(line, radius, collision, y_scale)) continue;
				hit = true;
				response = rule.m_response;
				hit_actor = mesh->actor();
				hit_type = mesh->type();
			}
		}
		return hit;
	},
	[&](const Line& line, const CollisionMesh::Collision& collision)
	{
		CollisionReport report;
		report.m_point = (line.at(collision.m_time) - collision.m_normal * radius) * unscale;
		report.m_normal = normalize(collision.m_normal * scale);
		report.m_with = hit_actor;
		report.m_type = hit_type;
		source.m_collisions.push_back(report);
	});
	//the actor follows the center
	if (resolved != position) world_position(*actor, actor->position(true) + (resolved - position));
	source.m_previous = resolved;
}

bool CollisionWorld::raycast(const Vec3& origin, const Vec3& direction, float max_distance, CollisionMesh::Hit& hit)
{
	bool found = false;
	float best = max_distance;
	for (const Shared<MeshCollider>& mesh : colliders().m_meshes)
	{
		CollisionMesh::Hit mesh_hit;
		if (mesh->mesh().raycast(origin, direction, best, mesh_hit) && mesh_hit.m_distance <= best)
		{
			best = mesh_hit.m_distance;
			hit = mesh_hit;
			found = true;
		}
	}
	return found;
}

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionSystem
SQUARE_CLASS_OBJECT_REGISTRATION(CollisionSystem);

void CollisionSystem::object_registration(Context& ctx)
{
	//system: when the game starts it
	ctx.add_system<CollisionSystem>(SystemStartup::ON_DEMAND);
}

CollisionSystem::CollisionSystem(Context& context) : System(context)
{
}

void CollisionSystem::shutdown()
{
	m_instances.clear();
}

void CollisionSystem::update(double delta_time)
{
	//worlds gone
	m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(), [](const Weak<CollisionWorld>& instance) { return instance.expired(); }), m_instances.end());
	for (const Weak<CollisionWorld>& weak_instance : m_instances)
	{
		if (auto instance = weak_instance.lock()) instance->update();
	}
}

Shared<SystemInstance> CollisionSystem::create_instance(Scene::World& world)
{
	auto instance = MakeShared<CollisionWorld>(context(), *this, world);
	m_instances.push_back(instance);
	return instance;
}
