//
//  Collision.cpp
//  Rush
//
//  Moving spheres against triangles and spheres: the first contact along the move, then a
//  slide on the contact planes, with the vectors of Square.
//
#include <cstring>
#include <Collision.h>
#include <CollisionDebug.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Square;

using Line = CollisionMesh::Line;

namespace
{
	//a contact plane is moved out of this much: the sphere stops just before it
	const float CONTACT_EPSILON = 0.001f;
	const float EPSILON = 0.000001f;
	//triangles in a leaf of the tree
	const size_t MAX_LEAF_TRIANGLES = 16;
	//hits of a move before it gives up (and goes back to where it was)
	const int MAX_HITS = 10;

	Vec3 to_vec3(const Vec2& v) { return Vec3(v, 0.0f); }
	Vec3 to_vec3(const Vec3& v) { return v; }

	//nearest point of a line to point
	Vec3 nearest(const Line& line, const Vec3& point)
	{
		const float length2 = dot(line.m_direction, line.m_direction);
		if (length2 <= 0.0f) return line.m_origin;
		return line.m_origin + line.m_direction * (dot(line.m_direction, point - line.m_origin) / length2);
	}

	//plane dot(normal, x) + offset = 0 (normal normalized)
	struct Plane
	{
		Vec3  m_normal{ Constants::axis_y };
		float m_offset{ 0.0f };

		Plane() = default;
		Plane(const Vec3& point, const Vec3& normal) : m_normal(normal), m_offset(-dot(normal, point)) {}
		//through three points, normal (b - a) x (c - a)
		static Plane from(const Vec3& a, const Vec3& b, const Vec3& c)
		{
			return Plane(a, normalize(cross(b - a, c - a)));
		}

		float distance(const Vec3& point) const { return dot(m_normal, point) + m_offset; }
		//t of the line where it crosses the plane
		float intersect_time(const Line& line) const { return -distance(line.m_origin) / dot(m_normal, line.m_direction); }
		Vec3  nearest(const Vec3& point) const { return point - m_normal * distance(point); }
		//the line of two planes (not parallel)
		Line intersect(const Plane& other) const
		{
			const float cos_angle = dot(m_normal, other.m_normal);
			const float det = 1.0f - cos_angle * cos_angle;
			const float offset = -m_offset, other_offset = -other.m_offset;
			Line line;
			line.m_origin = (m_normal * (offset - other_offset * cos_angle) + other.m_normal * (other_offset - offset * cos_angle)) / det;
			line.m_direction = normalize(cross(m_normal, other.m_normal));
			return line;
		}
	};

	//smaller root of a t^2 + b t + c, false if none
	bool smaller_root(float a, float b, float c, float& t)
	{
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f) return false;
		const float root = std::sqrt(discriminant);
		t = std::min((-b + root) / (2.0f * a), (-b - root) / (2.0f * a));
		return true;
	}

	//a contact at time with normal: taken if it is the first one and the sphere is going into
	//it (and it was not already behind it)
	bool take_contact(CollisionMesh::Collision& collision, const Line& line, float time, const Vec3& normal)
	{
		if (time > collision.m_time) return false;
		const Plane plane(line.at(time), normal);
		if (dot(plane.m_normal, line.m_direction) >= 0.0f) return false;
		if (plane.distance(line.m_origin) < -CONTACT_EPSILON) return false;
		collision.m_time = time;
		collision.m_normal = normal;
		return true;
	}

	//the sphere against another one (at center, radius: the two radii), a point against a
	//sphere of the sum of the radii. Already inside: a contact at the start, it can only go out
	bool sphere_collide(const Line& line, float radius, const Vec3& center, CollisionMesh::Collision& collision)
	{
		Line local;
		local.m_origin = line.m_origin - center;
		local.m_direction = line.m_direction;
		const float a = dot(local.m_direction, local.m_direction);
		if (a <= 0.0f) return false;
		const float b = dot(local.m_origin, local.m_direction) * 2.0f;
		const float c = dot(local.m_origin, local.m_origin) - radius * radius;
		float time = 0.0f;
		if (c > 0.0f)
		{
			if (!smaller_root(a, b, c, time)) return false; //misses it
			if (time < 0.0f) return false;                  //behind
		}
		if (time > collision.m_time) return false;          //too far
		const Vec3 contact = local.at(time);
		if (dot(contact, contact) <= EPSILON) return false;  //same center: no direction
		return take_contact(collision, line, time, normalize(contact));
	}

	//the sphere against the edge from edge_start to edge_end (a cylinder of radius) and the
	//vertex edge_start (a sphere); triangle_normal and edge_normal (of the plane of the edge,
	//pointing inside) are the axes of the edge space
	bool edge_collide(const Vec3& edge_start, const Vec3& edge_end, const Vec3& triangle_normal, const Vec3& edge_normal, const Line& line, float radius, CollisionMesh::Collision& collision)
	{
		//edge space: x the edge normal, y along the edge, z the triangle normal
		const Mat3 edge_basis(edge_normal, normalize(edge_end - edge_start), triangle_normal);
		const Mat3 to_edge = transpose(edge_basis);
		const Vec3 local_start = to_edge * (line.m_origin - edge_start);
		const Vec3 local_end = to_edge * (line.m_origin + line.m_direction - edge_start);
		Line local;
		local.m_origin = local_start;
		local.m_direction = local_end - local_start;
		//cylinder around y
		float a = local.m_direction.x * local.m_direction.x + local.m_direction.z * local.m_direction.z;
		if (a <= 0.0f) return false; //parallel to the cylinder
		float b = (local.m_origin.x * local.m_direction.x + local.m_origin.z * local.m_direction.z) * 2.0f;
		float c = (local.m_origin.x * local.m_origin.x + local.m_origin.z * local.m_origin.z) - radius * radius;
		float time = 0.0f;
		if (!smaller_root(a, b, c, time)) return false;      //misses the cylinder
		if (time > collision.m_time) return false;           //too far
		Vec3 contact = local.at(time), axis_point(0.0f);
		if (contact.y > length(edge_end - edge_start)) return false; //over the end of the edge
		if (contact.y >= 0.0f)
		{
			axis_point.y = contact.y;
		}
		else
		{
			//under the start: the sphere of the vertex
			a = dot(local.m_direction, local.m_direction);
			if (a <= 0.0f) return false;
			b = dot(local.m_origin, local.m_direction) * 2.0f;
			c = dot(local.m_origin, local.m_origin) - radius * radius;
			if (!smaller_root(a, b, c, time)) return false;
			if (time > collision.m_time) return false;
			contact = local.at(time);
		}
		return take_contact(collision, line, time, normalize(edge_basis * (contact - axis_point)));
	}

	//the sphere against a triangle, both faces: the face facing the move is used; one sided: only
	//its front (its winding), from behind it is not there (its edges either)
	bool triangle_collide(const Line& line, float radius, Vec3 a, Vec3 b, Vec3 c, bool one_sided, CollisionMesh::Collision& collision)
	{
		Plane plane = Plane::from(a, b, c);
		if (dot(plane.m_normal, line.m_direction) >= 0.0f)
		{
			if (one_sided) return false;
			//the other face
			std::swap(b, c);
			plane = Plane::from(a, b, c);
			if (dot(plane.m_normal, line.m_direction) >= 0.0f) return false; //parallel
		}
		//the plane moved out of radius
		Plane moved_plane = plane;
		moved_plane.m_offset -= radius;
		const float time = moved_plane.intersect_time(line);
		if (time > collision.m_time) return false;
		//planes of the edges (pointing inside)
		const Plane edge_ab = Plane::from(a + plane.m_normal, b, a);
		const Plane edge_bc = Plane::from(b + plane.m_normal, c, b);
		const Plane edge_ca = Plane::from(c + plane.m_normal, a, c);
		//on the face?
		const Vec3 contact = line.at(time);
		if (edge_ab.distance(contact) >= 0.0f && edge_bc.distance(contact) >= 0.0f && edge_ca.distance(contact) >= 0.0f)
		{
			return take_contact(collision, line, time, plane.m_normal);
		}
		if (radius <= 0.0f) return false;
		//the edges and the vertices (all of them: the first contact wins)
		const bool hit_ab = edge_collide(a, b, plane.m_normal, edge_ab.m_normal, line, radius, collision);
		const bool hit_bc = edge_collide(b, c, plane.m_normal, edge_bc.m_normal, line, radius, collision);
		const bool hit_ca = edge_collide(c, a, plane.m_normal, edge_ca.m_normal, line, radius, collision);
		return hit_ab || hit_bc || hit_ca;
	}

	bool boxes_overlap(const Vec3& a_min, const Vec3& a_max, const Vec3& b_min, const Vec3& b_max)
	{
		return a_min.x <= b_max.x && a_max.x >= b_min.x
		    && a_min.y <= b_max.y && a_max.y >= b_min.y
		    && a_min.z <= b_max.z && a_max.z >= b_min.z;
	}

	//the sphere moves from `from` to `to`: collide(line, collision, response) finds the first
	//contact along line (with the response of what it hit), report(line, collision) gets every
	//hit; where it ends
	template < class Collide, class Report >
	Vec3 slide_move(Vec3 from, Vec3 to, Collide collide, Report report)
	{
		if (from == to) return to;
		const Vec3 start = from;

		int   plane_count = 0;
		Plane planes[2];
		Line  line;
		line.m_origin = from;
		line.m_direction = to - from;
		const Vec3 direction = line.m_direction;
		float distance = length(line.m_direction);
		float distance_xz = length(Vec3(line.m_direction.x, 0.0f, line.m_direction.z));

		int hit_count = 0;
		for (;;)
		{
			CollisionMesh::Collision collision;
			CollisionResponse response = CollisionResponse::SLIDE;
			if (!collide(line, collision, response)) break;

			//a hit
			if (++hit_count == MAX_HITS) break;
			report(line, collision);

			Plane plane(line.at(collision.m_time), collision.m_normal);
			plane.m_offset -= CONTACT_EPSILON;
			collision.m_time = plane.intersect_time(line);

			if (collision.m_time > 0.0f)
			{
				//the start goes to the contact (only forward)
				from = line.at(collision.m_time);
				distance *= 1.0f - collision.m_time;
				distance_xz *= 1.0f - collision.m_time;
			}

			//the destination on the plane
			const Vec3 on_plane = plane.nearest(to);

			if (plane_count == 0)
			{
				to = on_plane;
			}
			else if (plane_count == 1)
			{
				if (planes[0].distance(on_plane) >= 0.0f)
				{
					to = on_plane;
					plane_count = 0;
				}
				else if (std::abs(dot(planes[0].m_normal, plane.m_normal)) < 1.0f - EPSILON)
				{
					//along the crease of the two planes
					to = nearest(plane.intersect(planes[0]), to);
				}
				else
				{
					//squeezed between two parallel planes
					hit_count = MAX_HITS;
					break;
				}
			}
			else if (planes[0].distance(on_plane) >= 0.0f && planes[1].distance(on_plane) >= 0.0f)
			{
				to = on_plane;
				plane_count = 0;
			}
			else
			{
				to = from;
				break;
			}

			Vec3 move = to - from;

			//going back against the first direction
			if (dot(move, direction) <= 0.0f)
			{
				to = from;
				break;
			}

			if (response == CollisionResponse::SLIDE)
			{
				const float move_length = length(move);
				if (move_length <= EPSILON)
				{
					to = from;
					break;
				}
				if (move_length > distance) move *= distance / move_length;
			}
			else if (response == CollisionResponse::SLIDEXZ)
			{
				const float move_length = length(Vec3(move.x, 0.0f, move.z));
				if (move_length <= EPSILON)
				{
					to = from;
					break;
				}
				if (move_length > distance_xz) move *= distance_xz / move_length;
			}

			line.m_origin = from;
			line.m_direction = move;
			to = from + move;
			planes[plane_count++] = plane;
		}

		if (hit_count >= MAX_HITS) return start;
		return to;
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
Surface surface_of(const std::string& name)
{
	static const std::pair<const char*, Surface> s_prefixes[]
	{
		{ "water", Surface::WATER },
		{ "shallow", Surface::SHALLOW },
		{ "mud", Surface::MUD },
		{ "sand", Surface::SAND },
		{ "dirt", Surface::DIRT },
		{ "ice", Surface::ICE },
	};
	//the prefix alone, or before a "_" ("water", "water_lagoon": not "waterfall")
	for (const auto& prefix : s_prefixes)
	{
		const size_t length = std::strlen(prefix.first);
		if (name.rfind(prefix.first, 0) != 0) continue;
		if (name.size() == length || name[length] == '_' || name[length] == '-' || name[length] == '.') return prefix.second;
	}
	return Surface::GROUND;
}

bool is_water(Surface surface)
{
	return surface == Surface::WATER || surface == Surface::SHALLOW;
}

//a wall of a map ("blocker", "blocker_<n>"): solid whatever its material (invisible: translucent)
bool is_blocker(const std::string& name)
{
	static const std::string s_prefix = "blocker";
	if (name.rfind(s_prefix, 0) != 0) return false;
	return name.size() == s_prefix.size() || name[s_prefix.size()] == '_' || name[s_prefix.size()] == '.';
}

void CollisionMesh::add(Context& context, const Shared<Scene::Actor>& actor, bool solid_only)
{
	actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		auto static_mesh = node->component<Scene::StaticMesh>();
		//its ground (by its name, or by the one of its parent: a node of the exporter under it)
		Surface surface = surface_of(node->name());
		bool blocker = is_blocker(node->name());
		{
			auto parent = node->parent().lock();
			if (parent && surface == Surface::GROUND) surface = surface_of(parent->name());
			if (parent) blocker = blocker || is_blocker(parent->name());
		}
		//only opaque surfaces are solid: not the translucent ones, nor the alpha tested ones
		//(mask >= 0: grass, foliage, drawn as opaque)
		auto solid = [&static_mesh](size_t submesh_id) -> bool
		{
			auto material = static_mesh->material(submesh_id).lock();
			if (!material) return true;
			const auto* mask       = material->parameter_by_name("mask");
			const bool  opaque     = material->queue().m_type == Render::RQ_OPAQUE;
			const bool  alpha_test = mask && mask->get_float() >= 0.0f;
			return opaque && !alpha_test;
		};
		std::vector<Vec3> points;
		//the water, the walls: solid, also translucent
		const bool every = !solid_only || is_water(surface) || blocker;
		if (!static_mesh->triangles(points, every ? nullptr : std::function<bool(size_t)>(solid)))
		{
			if (static_mesh->m_mesh) context.logger()->warning("CollisionMesh: unable to read the mesh of " + node->name());
			return true;
		}
		for (size_t i = 0; i + 2 < points.size(); i += 3) add_triangle(points[i], points[i + 1], points[i + 2], surface);
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

void CollisionMesh::add_triangle(const Vec3& a, const Vec3& b, const Vec3& c, Surface surface)
{
	//degenerate triangle
	if (length(cross(b - a, c - a)) < 1e-8f) return;
	Triangle triangle;
	triangle.m_a = a;
	triangle.m_b = b;
	triangle.m_c = c;
	triangle.m_min = glm::min(a, glm::min(b, c));
	triangle.m_max = glm::max(a, glm::max(b, c));
	triangle.m_surface = surface;
	m_triangles.push_back(triangle);
}

void CollisionMesh::triangles(std::vector<Vec3>& out) const
{
	out.reserve(out.size() + m_triangles.size() * 3);
	for (const Triangle& triangle : m_triangles)
	{
		out.push_back(triangle.m_a);
		out.push_back(triangle.m_b);
		out.push_back(triangle.m_c);
	}
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
//CollisionMesh: tree
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
		if (!triangle_collide(line, radius, triangle.m_a * scale, triangle.m_b * scale, triangle.m_c * scale, m_one_sided, collision)) continue;
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
	if (collision.m_triangle >= 0) hit.m_surface = m_triangles[size_t(collision.m_triangle)].m_surface;
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
		if (auto owner = actor().lock()) m_mesh.add(context(), owner, m_solid_only);
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

CollisionWorld::~CollisionWorld()
{
	//the view can outlive it (the render instance keeps it): nothing more to draw
	if (m_debug) m_debug->detach();
}

void CollisionWorld::debug(bool enable)
{
	if (enable == debug()) return;
	auto render_instance = m_world.instance<RenderInstance>();
	if (enable)
	{
		m_debug = MakeShared<CollisionDebug>(context(), *this);
		if (render_instance) render_instance->add_post_effect(m_debug);
	}
	else
	{
		if (render_instance) render_instance->remove_post_effect(m_debug);
		m_debug->detach();
		m_debug.reset();
	}
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

void CollisionWorld::on_add_component(const Shared<Scene::Actor>& actor, const Shared<Scene::Component>& component)
{
	if (auto sphere = DynamicPointerCast<SphereCollider, Scene::Component>(component))  m_spheres.push_back(sphere);
	else if (auto mesh = DynamicPointerCast<MeshCollider, Scene::Component>(component)) m_meshes.push_back(mesh);
	if (auto listener = dynamic_cast<FixedStepListener*>(component.get())) m_listeners.push_back(Listener{ component, listener });
}

void CollisionWorld::on_remove_component(const Shared<Scene::Actor>& actor, const Shared<Scene::Component>& component)
{
	//it, and the ones gone
	const Scene::Component* removed = component.get();
	m_spheres.erase(std::remove_if(m_spheres.begin(), m_spheres.end(), [&](const Weak<SphereCollider>& weak_sphere)
	{
		auto sphere = weak_sphere.lock();
		return !sphere || static_cast<const Scene::Component*>(sphere.get()) == removed;
	}), m_spheres.end());
	m_meshes.erase(std::remove_if(m_meshes.begin(), m_meshes.end(), [&](const Weak<MeshCollider>& weak_mesh)
	{
		auto mesh = weak_mesh.lock();
		return !mesh || static_cast<const Scene::Component*>(mesh.get()) == removed;
	}), m_meshes.end());
	m_listeners.erase(std::remove_if(m_listeners.begin(), m_listeners.end(), [&](const Listener& listener)
	{
		auto listener_component = listener.m_component.lock();
		return !listener_component || listener_component.get() == removed;
	}), m_listeners.end());
}

void CollisionWorld::update(double delta_time)
{
	if (m_paused) return;
	//the steps the time of the frame holds
	m_time += delta_time;
	int steps = 0;
	while (m_time >= m_settings.step && steps < m_settings.max_steps)
	{
		step();
		m_time -= m_settings.step;
		++steps;
	}
	//a frame too long: the rest of its time is lost (no catching up after a loading)
	if (steps == m_settings.max_steps) m_time = std::min(m_time, m_settings.step * 0.999);
	//the pose between the last two steps
	const double alpha = m_time / m_settings.step;
	for (const Listener& listener : m_listeners)
	{
		if (!listener.m_component.expired()) listener.m_listener->on_fixed_interpolate(alpha);
	}
}

void CollisionWorld::step()
{
	//the listeners move their actors
	for (const Listener& listener : m_listeners)
	{
		if (!listener.m_component.expired()) listener.m_listener->on_fixed_update(m_settings.step);
	}
	//then the spheres collide, from where they were at the last step
	for (const Weak<SphereCollider>& weak_sphere : m_spheres)
	{
		if (auto sphere = weak_sphere.lock())
		{
			sphere->m_collisions.clear();
			collide(*sphere);
		}
	}
}

void CollisionWorld::collide(SphereCollider& source)
{
	auto actor = source.actor().lock();
	if (!actor) return;
	const Vec3 position = source.center();
	//first step, or a reset: from here
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
			switch (rule.m_method)
			{
			case CollisionMethod::SPHERE:
				//the other spheres, where they are now (in the space of this ellipsoid)
				for (const Weak<SphereCollider>& weak_sphere : m_spheres)
				{
					auto sphere = weak_sphere.lock();
					if (!sphere || sphere.get() == &source || sphere->type() != rule.m_dst_type) continue;
					if (!sphere_collide(line, radius + sphere->radius(), sphere->center() * scale, collision)) continue;
					hit = true;
					response = rule.m_response;
					hit_actor = sphere->actor();
					hit_type = sphere->type();
				}
				break;
			case CollisionMethod::POLYGON:
				for (const Weak<MeshCollider>& weak_mesh : m_meshes)
				{
					auto mesh = weak_mesh.lock();
					if (!mesh || mesh->type() != rule.m_dst_type) continue;
					if (!mesh->mesh().collide(line, radius, collision, y_scale)) continue;
					hit = true;
					response = rule.m_response;
					hit_actor = mesh->actor();
					hit_type = mesh->type();
				}
				break;
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
	for (const Weak<MeshCollider>& weak_mesh : m_meshes)
	{
		auto mesh = weak_mesh.lock();
		if (!mesh) continue;
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
		if (auto instance = weak_instance.lock()) instance->update(delta_time);
	}
}

Shared<SystemInstance> CollisionSystem::create_instance(Scene::World& world)
{
	auto instance = MakeShared<CollisionWorld>(context(), *this, world);
	m_instances.push_back(instance);
	return instance;
}
