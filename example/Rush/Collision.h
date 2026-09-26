//
//  Collision.h
//  Rush
//
//  The static triangles of the scene in world space, with the collisions of Blitz3D
//  (collision.cpp, meshcollider.cpp, world.cpp):
//  - a sphere moving along a segment against the triangles: the face, the edges (cylinders)
//    and the vertices (spheres); the first contact along the segment;
//  - a tree of boxes over the triangles (16 per leaf) to test only the near ones;
//  - move(): the sphere goes from one point to another, and on a hit it slides on the plane
//    of the contact (up to 10 hits a move; on two planes it slides along their crease).
//  CollisionSystem (game system, on demand): every world gets a CollisionWorld, its
//  CollisionMesh (world().instance<CollisionWorld>()->add(context, actor)).
//
#pragma once
#include <Square/Square.h>
#include <vector>

class CollisionMesh
{
public:
	//first contact of a moving sphere: time along the move (0 start, 1 end), contact normal
	struct Collision
	{
		float        m_time{ 1.0f };
		Square::Vec3 m_normal{ 0.0f, 1.0f, 0.0f };
		int          m_triangle{ -1 };
	};

	//a contact during a move: the point on the surface and its normal
	struct Contact
	{
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ 0.0f, 1.0f, 0.0f };
	};

	//a ray hit
	struct Hit
	{
		float        m_distance{ 0.0f };
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ 0.0f, 1.0f, 0.0f };
	};

	//what a move does on a hit
	enum class Response
	{
		STOP,  //stops at the contact
		SLIDE  //slides on the contact plane(s)
	};

	//add the static meshes of an actor and its children, with their current world transform;
	//sub meshes with a non opaque material (glass, glows...) are not solid and are skipped
	void add(Square::Context& context, const Square::Shared<Square::Scene::Actor>& actor);

	//move a sphere of radius from from to to: where it ends, the contacts on the way
	Square::Vec3 move
	(
	  const Square::Vec3& from
	, const Square::Vec3& to
	, float radius
	, std::vector<Contact>* contacts = nullptr
	, Response response = Response::SLIDE
	) const;

	//first contact of a sphere of radius moving from from to to (collision.m_time < 1)
	bool sweep(const Square::Vec3& from, const Square::Vec3& to, float radius, Collision& collision) const;

	//closest hit along origin + t * direction (direction normalized), t in [0, max_distance]
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, Hit& hit) const;

	//bounds of the triangles transformed by transform (e.g. world to hull space); false if empty
	bool bounds(const Square::Mat4& transform, Square::Vec3& out_min, Square::Vec3& out_max) const;

	size_t size() const { return m_triangles.size(); }

	//a segment: origin + direction * t
	struct Line
	{
		Square::Vec3 m_origin{ 0.0f };
		Square::Vec3 m_direction{ 0.0f };
		Square::Vec3 at(float t) const { return m_origin + m_direction * t; }
	};

private:
	struct Triangle
	{
		Square::Vec3 m_a, m_b, m_c;
		Square::Vec3 m_min, m_max;
	};
	//tree of boxes: a leaf has triangles, a node two children
	struct Node
	{
		Square::Vec3     m_min{ 0.0f }, m_max{ 0.0f };
		int              m_left{ -1 }, m_right{ -1 };
		std::vector<int> m_triangles;
	};
	std::vector<Triangle> m_triangles;
	std::vector<Node>     m_nodes;

	void add_triangle(const Square::Vec3& a, const Square::Vec3& b, const Square::Vec3& c);
	void build();
	int  build_node(std::vector<int>& triangles);
	bool collide(const Line& line, float radius, Collision& collision) const;
	bool collide(const Line& line, float radius, const Square::Vec3& box_min, const Square::Vec3& box_max, int node, Collision& collision) const;
};

//the collisions of a world
class CollisionWorld : public Square::SystemInstance
{
public:
	SQUARE_OBJECT(CollisionWorld)

	CollisionWorld(Square::Context& context, Square::System& system, Square::Scene::World& world);

	//the solid triangles of an actor and its children (after it is placed)
	void add(Square::Context& context, const Square::Shared<Square::Scene::Actor>& actor);

	CollisionMesh& mesh() { return m_mesh; }
	const CollisionMesh& mesh() const { return m_mesh; }

private:
	CollisionMesh m_mesh;
};

//the collisions of the worlds: static geometry, answers queries (no update)
class CollisionSystem : public Square::System
{
public:
	SQUARE_SYSTEM(CollisionSystem, Square::SystemRing::GAME)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	CollisionSystem(Square::Context& context);

	virtual bool initialize() override { return true; }
	virtual void shutdown() override {}
	virtual Square::Shared<Square::SystemInstance> create_instance(Square::Scene::World& world) override;
};
