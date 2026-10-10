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
#include <map>

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

namespace AuxCollisionMesh
{
	//items in a leaf of a tree: the triangles of a shape, the objects
	static constexpr size_t s_shape_leaf = 8;
	static constexpr size_t s_object_leaf = 4;

	//how a node collides (the property of the game "collision", its children too)
	enum class Mode
	{
		SOLID,  //its triangles, only the solid ones (or every one: not solid only)
		EVERY,  //its triangles, every one (also not opaque)
		BOX,    //its box
		SPHERE, //a sphere in its box
		NONE    //not solid
	};

	//the mode of a node: its property "collision", else the one of its parent
	static Mode mode_of(const Shared<Scene::Actor>& node, Mode parent)
	{
		Mode mode = parent;
		if (node->contains<Scene::Properties>())
		{
			auto properties = node->component<Scene::Properties>();
			if (properties->has("collision"))
			{
				const std::string value = properties->string("collision", "true");
				if (value == "false" || value == "0" || value == "none")
				{
					mode = Mode::NONE;
				}
				else if (value == "box")
				{
					mode = Mode::BOX;
				}
				else if (value == "sphere")
				{
					mode = Mode::SPHERE;
				}
				else
				{
					mode = Mode::EVERY;
				}
			}
		}
		return mode;
	}

	//a sub mesh solid: only the opaque surfaces, not the translucent ones, nor the alpha tested
	//ones (mask >= 0: grass, foliage, drawn as opaque)
	static bool solid(const Render::Renderable& renderable, size_t submesh_id)
	{
		bool solid = true;
		if (auto material = renderable.material(submesh_id).lock())
		{
			const auto* mask       = material->parameter_by_name("mask");
			const bool  opaque     = material->queue().m_type == Render::RQ_OPAQUE;
			const bool  alpha_test = mask && mask->get_float() >= 0.0f;
			solid = opaque && !alpha_test;
		}
		return solid;
	}

	//the sub meshes taken of a renderable (a bit each, the first 64), every one or the solid ones
	static uint64 taken(const Render::Renderable& renderable, size_t submeshes, bool every)
	{
		uint64 mask = 0;
		for (size_t i = 0; i < submeshes && i < 64; ++i)
		{
			if (every || solid(renderable, i))
			{
				mask |= uint64(1) << i;
			}
		}
		return mask;
	}

	//the shapes made by an add, by their mesh and what of it (its sub meshes, or its box: ~0 + 1)
	using ShapeKey = std::pair<const Resource::Mesh*, uint64>;
	using Shapes = std::map<ShapeKey, Shared<const CollisionMesh::Shape>>;
	static constexpr uint64 s_box_key = ~uint64(0) - 1;

	//a shape of triangles, its tree
	static Shared<CollisionMesh::Shape> shape_of(std::vector<CollisionMesh::Triangle>&& triangles)
	{
		auto shape = std::make_shared<CollisionMesh::Shape>();
		shape->m_triangles = std::move(triangles);
		std::vector<Vec3> mins, maxs;
		mins.reserve(shape->m_triangles.size());
		maxs.reserve(shape->m_triangles.size());
		for (const auto& triangle : shape->m_triangles)
		{
			mins.push_back(glm::min(triangle.m_a, glm::min(triangle.m_b, triangle.m_c)));
			maxs.push_back(glm::max(triangle.m_a, glm::max(triangle.m_b, triangle.m_c)));
		}
		shape->m_tree.build(mins, maxs, s_shape_leaf);
		return shape;
	}

	//the triangles of the sub meshes taken of a mesh (its own space); nullptr: none, not read
	static Shared<const CollisionMesh::Shape> mesh_shape(const Resource::Mesh& mesh, uint64 mask, Shapes& shapes)
	{
		const ShapeKey key(&mesh, mask);
		auto found = shapes.find(key);
		if (found != shapes.end())
		{
			return found->second;
		}
		std::vector<Vec3> points;
		Shared<const CollisionMesh::Shape> made;
		const bool read = mesh.local_triangles(points, [mask](size_t submesh) { return submesh < 64 && (mask >> submesh) & 1; });
		if (read)
		{
			std::vector<CollisionMesh::Triangle> triangles;
			triangles.reserve(points.size() / 3);
			for (size_t i = 0; i + 2 < points.size(); i += 3)
			{
				//(degenerate: none)
				if (length(cross(points[i + 1] - points[i], points[i + 2] - points[i])) >= 1e-8f)
				{
					triangles.push_back({ points[i], points[i + 1], points[i + 2] });
				}
			}
			made = shape_of(std::move(triangles));
		}
		shapes[key] = made;
		return made;
	}

	//the 12 triangles of a box (its own space)
	static Shared<const CollisionMesh::Shape> box_shape(const Resource::Mesh& mesh, const Geometry::OBoundingBox& box, Shapes& shapes)
	{
		const ShapeKey key(&mesh, s_box_key);
		auto found = shapes.find(key);
		if (found != shapes.end())
		{
			return found->second;
		}
		const Mat3 axes = box.get_rotation_matrix();
		const Vec3 center = box.get_position();
		const Vec3 half = box.get_extension();
		Vec3 corner[8];
		for (int i = 0; i < 8; ++i)
		{
			const Vec3 sign((i & 1) ? 1.0f : -1.0f, (i & 2) ? 1.0f : -1.0f, (i & 4) ? 1.0f : -1.0f);
			corner[i] = center + axes[0] * (half.x * sign.x) + axes[1] * (half.y * sign.y) + axes[2] * (half.z * sign.z);
		}
		//its faces (two triangles each), out of the box
		static const int faces[6][4] = { {0,2,6,4}, {1,5,7,3}, {0,4,5,1}, {2,3,7,6}, {0,1,3,2}, {4,6,7,5} };
		std::vector<CollisionMesh::Triangle> triangles;
		for (const auto& face : faces)
		{
			triangles.push_back({ corner[face[0]], corner[face[1]], corner[face[2]] });
			triangles.push_back({ corner[face[0]], corner[face[2]], corner[face[3]] });
		}
		Shared<const CollisionMesh::Shape> made = shape_of(std::move(triangles));
		shapes[key] = made;
		return made;
	}

	//the box of points by a matrix (the box of the corners of a box)
	static void transformed_box(const Mat4& matrix, const Vec3& low, const Vec3& high, Vec3& out_min, Vec3& out_max)
	{
		out_min = Vec3(std::numeric_limits<float>::max());
		out_max = Vec3(std::numeric_limits<float>::lowest());
		for (int i = 0; i < 8; ++i)
		{
			const Vec3 corner((i & 1) ? high.x : low.x, (i & 2) ? high.y : low.y, (i & 4) ? high.z : low.z);
			const Vec3 point = Vec3(matrix * Vec4(corner, 1.0f));
			out_min = glm::min(out_min, point);
			out_max = glm::max(out_max, point);
		}
	}

	//a node with something drawn
	static bool drawn(const Shared<Scene::Actor>& node)
	{
		return node->contains<Scene::StaticMesh>() || node->contains<Scene::InstancedMesh>();
	}

	//the levels of detail of a node not taken (all but the first one): their names
	static std::vector<std::string> other_levels(const Shared<Scene::Actor>& node)
	{
		std::vector<std::string> names;
		if (node->contains<Scene::LodGroup>())
		{
			const auto& levels = node->component<Scene::LodGroup>()->levels();
			for (size_t i = 1; i < levels.size(); ++i)
			{
				names.push_back(levels[i].m_actor);
			}
		}
		return names;
	}
}

void CollisionMesh::add(Context& context, const Shared<Scene::Actor>& actor, bool solid_only)
{
	using namespace AuxCollisionMesh;
	Shapes shapes;
	//a node and its children (its mode passed to them)
	std::function<void(const Shared<Scene::Actor>&, Mode)> walk = [&](const Shared<Scene::Actor>& node, Mode parent_mode)
	{
		const Mode mode = mode_of(node, parent_mode);
		if (mode == Mode::NONE)
		{
			return;
		}
		if (drawn(node))
		{
			//its ground (by its name, or by the one of its parent: a node of the exporter under it)
			Surface surface = surface_of(node->name());
			bool blocker = is_blocker(node->name());
			if (auto parent = node->parent().lock())
			{
				if (surface == Surface::GROUND)
				{
					surface = surface_of(parent->name());
				}
				blocker = blocker || is_blocker(parent->name());
			}
			//the water, the walls, the property: solid, also translucent
			const bool every = !solid_only || is_water(surface) || blocker || mode == Mode::EVERY;
			const Mat4& model = node->global_model_matrix();
			//its mesh, its box, its places (a static mesh: one, an instanced one: its instances)
			Shared<Resource::Mesh> mesh;
			Geometry::OBoundingBox box;
			std::vector<Mat4> places;
			uint64 mask = 0;
			if (node->contains<Scene::StaticMesh>())
			{
				auto static_mesh = node->component<Scene::StaticMesh>();
				mesh = static_mesh->m_mesh;
				box = static_mesh->local_bounding_box();
				places.push_back(model);
				mask = mesh ? taken(*static_mesh, mesh->number_of_sub_meshs(), every) : 0;
			}
			else
			{
				auto instanced = node->component<Scene::InstancedMesh>();
				mesh = instanced->mesh();
				box = instanced->mesh_box();
				for (const Mat4& instance : instanced->instances())
				{
					places.push_back(model * instance);
				}
				mask = mesh ? taken(*instanced, mesh->number_of_sub_meshs(), every) : 0;
			}
			Shared<const Shape> shape;
			if (mesh && mask && mode == Mode::BOX)
			{
				shape = box_shape(*mesh, box, shapes);
			}
			else if (mesh && mask && mode != Mode::SPHERE)
			{
				shape = mesh_shape(*mesh, mask, shapes);
				if (!shape)
				{
					context.logger()->warning("CollisionMesh: unable to read the mesh of " + node->name());
				}
			}
			const bool sphere = mesh && mask && mode == Mode::SPHERE;
			for (const Mat4& place : places)
			{
				Object object;
				object.m_surface = surface;
				if (sphere)
				{
					//the sphere in its box: its center, its half side the longest in the world
					object.m_sphere = true;
					object.m_center = Vec3(place * Vec4(box.get_position(), 1.0f));
					const Mat3 axes = box.get_rotation_matrix();
					const Vec3 half = box.get_extension();
					for (int axis = 0; axis < 3; ++axis)
					{
						object.m_radius = std::max(object.m_radius, length(Vec3(place * Vec4(axes[axis] * half[axis], 0.0f))));
					}
					add_object(object);
				}
				else if (shape && !shape->m_triangles.empty())
				{
					object.m_shape = shape;
					object.m_model = place;
					add_object(object);
				}
			}
		}
		//its children, but the other levels of detail
		const std::vector<std::string> skipped = other_levels(node);
		for (const auto& child : node->childs())
		{
			if (std::find(skipped.begin(), skipped.end(), child->name()) == skipped.end())
			{
				walk(child, mode);
			}
		}
	};
	walk(actor, Mode::SOLID);
	m_shapes += shapes.size();
	//the tree of the objects
	build();
}

void CollisionMesh::add_object(Object object)
{
	if (object.m_sphere)
	{
		object.m_min = object.m_center - Vec3(object.m_radius);
		object.m_max = object.m_center + Vec3(object.m_radius);
	}
	else
	{
		object.m_inverse = inverse(object.m_model);
		object.m_mirrored = determinant(Mat3(object.m_model)) < 0.0f;
		//a similarity: its axes as long, square to each other (the moves tested in its space)
		const Mat3 axes(object.m_model);
		const float x = length(axes[0]), y = length(axes[1]), z = length(axes[2]);
		const float tolerance = 1e-4f * std::max(x, 1e-6f);
		object.m_scale = x;
		object.m_similar = x > 1e-6f
		                && std::abs(x - y) < tolerance && std::abs(x - z) < tolerance
		                && std::abs(dot(axes[0], axes[1])) < tolerance * x
		                && std::abs(dot(axes[0], axes[2])) < tolerance * x
		                && std::abs(dot(axes[1], axes[2])) < tolerance * x;
		const Node& root = object.m_shape->m_tree.m_nodes.front();
		AuxCollisionMesh::transformed_box(object.m_model, root.m_min, root.m_max, object.m_min, object.m_max);
		m_triangle_count += object.m_shape->m_triangles.size();
	}
	m_objects.push_back(std::move(object));
}

void CollisionMesh::clear()
{
	m_objects.clear();
	m_tree = Tree();
	m_shapes = 0;
	m_triangle_count = 0;
}

void CollisionMesh::triangles(std::vector<Vec3>& out) const
{
	out.reserve(out.size() + m_triangle_count * 3);
	for (const Object& object : m_objects)
	{
		if (object.m_shape)
		{
			for (const Triangle& triangle : object.m_shape->m_triangles)
			{
				out.push_back(Vec3(object.m_model * Vec4(triangle.m_a, 1.0f)));
				out.push_back(Vec3(object.m_model * Vec4(triangle.m_b, 1.0f)));
				out.push_back(Vec3(object.m_model * Vec4(triangle.m_c, 1.0f)));
			}
		}
	}
}

bool CollisionMesh::bounds(const Mat4& transform, Vec3& out_min, Vec3& out_max) const
{
	if (m_objects.empty()) return false;
	out_min = Vec3(std::numeric_limits<float>::max());
	out_max = Vec3(std::numeric_limits<float>::lowest());
	for (const Object& object : m_objects)
	{
		if (object.m_shape)
		{
			const Mat4 matrix = transform * object.m_model;
			for (const Triangle& triangle : object.m_shape->m_triangles)
			{
				for (const Vec3& vertex : { triangle.m_a, triangle.m_b, triangle.m_c })
				{
					const Vec3 point = Vec3(matrix * Vec4(vertex, 1.0f));
					out_min = glm::min(out_min, point);
					out_max = glm::max(out_max, point);
				}
			}
		}
		else
		{
			Vec3 low, high;
			AuxCollisionMesh::transformed_box(transform, object.m_min, object.m_max, low, high);
			out_min = glm::min(out_min, low);
			out_max = glm::max(out_max, high);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionMesh: trees
void CollisionMesh::Tree::build(const std::vector<Vec3>& mins, const std::vector<Vec3>& maxs, size_t leaf)
{
	m_nodes.clear();
	m_order.resize(mins.size());
	for (size_t i = 0; i < m_order.size(); ++i) m_order[i] = int(i);
	if (!mins.empty())
	{
		m_nodes.reserve(mins.size() * 2 / std::max<size_t>(leaf, 1) + 1);
		build_node(mins, maxs, 0, int(mins.size()), leaf);
	}
}

int CollisionMesh::Tree::build_node(const std::vector<Vec3>& mins, const std::vector<Vec3>& maxs, int first, int count, size_t leaf)
{
	const int node_id = int(m_nodes.size());
	m_nodes.emplace_back();
	//box of the items
	Vec3 box_min(std::numeric_limits<float>::max()), box_max(std::numeric_limits<float>::lowest());
	for (int i = first; i < first + count; ++i)
	{
		box_min = glm::min(box_min, mins[m_order[i]]);
		box_max = glm::max(box_max, maxs[m_order[i]]);
	}
	m_nodes[node_id].m_min = box_min;
	m_nodes[node_id].m_max = box_max;
	if (size_t(count) <= leaf)
	{
		//a leaf
		m_nodes[node_id].m_first = first;
		m_nodes[node_id].m_count = count;
	}
	else
	{
		//split on the longest axis, at the middle item by the centers of the boxes
		const Vec3 size = box_max - box_min;
		const int axis = (size.y > size.x && size.y >= size.z) ? 1 : (size.z > size.x && size.z > size.y) ? 2 : 0;
		auto begin = m_order.begin() + first;
		std::nth_element(begin, begin + count / 2, begin + count, [&](int l, int r)
		{
			return mins[l][axis] + maxs[l][axis] < mins[r][axis] + maxs[r][axis];
		});
		const int left = build_node(mins, maxs, first, count / 2, leaf);
		const int right = build_node(mins, maxs, first + count / 2, count - count / 2, leaf);
		m_nodes[node_id].m_left = left;
		m_nodes[node_id].m_right = right;
	}
	return node_id;
}

void CollisionMesh::build()
{
	std::vector<Vec3> mins, maxs;
	mins.reserve(m_objects.size());
	maxs.reserve(m_objects.size());
	for (const Object& object : m_objects)
	{
		mins.push_back(object.m_min);
		maxs.push_back(object.m_max);
	}
	m_tree.build(mins, maxs, AuxCollisionMesh::s_object_leaf);
}

//////////////////////////////////////////////////////////////////////////////////////////
//CollisionMesh: queries
bool CollisionMesh::collide(const Line& line, float radius, Collision& collision, float y_scale) const
{
	if (m_tree.m_nodes.empty()) return false;
	//box of the move, back in world space (the trees are in world space, or in their own)
	const Vec3 end = line.at(1.0f);
	const Vec3 unscale(1.0f, 1.0f / y_scale, 1.0f);
	const Vec3 box_min = (glm::min(line.m_origin, end) - Vec3(radius)) * unscale;
	const Vec3 box_max = (glm::max(line.m_origin, end) + Vec3(radius)) * unscale;
	bool hit = false;
	int stack[64];
	int top = 0;
	stack[top++] = 0;
	while (top > 0)
	{
		const Node& node = m_tree.m_nodes[stack[--top]];
		if (!boxes_overlap(box_min, box_max, node.m_min, node.m_max)) continue;
		if (node.m_left >= 0)
		{
			stack[top++] = node.m_left;
			stack[top++] = node.m_right;
			continue;
		}
		for (int i = node.m_first; i < node.m_first + node.m_count; ++i)
		{
			const Object& object = m_objects[m_tree.m_order[i]];
			if (!boxes_overlap(box_min, box_max, object.m_min, object.m_max)) continue;
			hit |= collide(object, line, radius, y_scale, box_min, box_max, collision);
		}
	}
	return hit;
}

bool CollisionMesh::collide(const Object& object, const Line& line, float radius, float y_scale, const Vec3& box_min, const Vec3& box_max, Collision& collision) const
{
	const Vec3 scale(1.0f, y_scale, 1.0f);
	bool hit = false;
	if (object.m_sphere)
	{
		//a point against a sphere of the two radii (in the space of the ellipsoid)
		hit = sphere_collide(line, radius + object.m_radius, object.m_center * scale, collision);
	}
	else
	{
		//the box of the move in the space of the shape, its triangles there
		Vec3 local_min, local_max;
		AuxCollisionMesh::transformed_box(object.m_inverse, box_min, box_max, local_min, local_max);
		const Shape& shape = *object.m_shape;
		//a similarity and a sphere (not an ellipsoid): the move in the space of the shape (the
		//time along it the same, the radius by its scale), else its triangles in the world
		const bool local = object.m_similar && y_scale == 1.0f;
		Line local_line;
		Collision local_collision;
		float local_radius = radius;
		if (local)
		{
			local_line.m_origin = Vec3(object.m_inverse * Vec4(line.m_origin, 1.0f));
			local_line.m_direction = Vec3(object.m_inverse * Vec4(line.m_direction, 0.0f));
			local_radius = radius / object.m_scale;
			local_collision.m_time = collision.m_time;
		}
		int stack[64];
		int top = 0;
		stack[top++] = 0;
		while (top > 0)
		{
			const Node& node = shape.m_tree.m_nodes[stack[--top]];
			if (!boxes_overlap(local_min, local_max, node.m_min, node.m_max)) continue;
			if (node.m_left >= 0)
			{
				stack[top++] = node.m_left;
				stack[top++] = node.m_right;
				continue;
			}
			for (int i = node.m_first; i < node.m_first + node.m_count; ++i)
			{
				const Triangle& triangle = shape.m_triangles[shape.m_tree.m_order[i]];
				const Vec3 low = glm::min(triangle.m_a, glm::min(triangle.m_b, triangle.m_c));
				const Vec3 high = glm::max(triangle.m_a, glm::max(triangle.m_b, triangle.m_c));
				if (!boxes_overlap(local_min, local_max, low, high)) continue;
				if (local)
				{
					hit |= triangle_collide(local_line, local_radius, triangle.m_a, triangle.m_b, triangle.m_c, m_one_sided, local_collision);
					continue;
				}
				const Vec3 a = Vec3(object.m_model * Vec4(triangle.m_a, 1.0f)) * scale;
				Vec3 b = Vec3(object.m_model * Vec4(triangle.m_b, 1.0f)) * scale;
				Vec3 c = Vec3(object.m_model * Vec4(triangle.m_c, 1.0f)) * scale;
				//(a mirror: its front the same face)
				if (object.m_mirrored)
				{
					std::swap(b, c);
				}
				hit |= triangle_collide(line, radius, a, b, c, m_one_sided, collision);
			}
		}
		//the contact back in the world
		if (local && hit)
		{
			collision.m_time = local_collision.m_time;
			collision.m_normal = normalize(Vec3(object.m_model * Vec4(local_collision.m_normal, 0.0f)));
		}
	}
	if (hit)
	{
		collision.m_surface = object.m_surface;
	}
	return hit;
}

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
	hit.m_surface = collision.m_surface;
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
