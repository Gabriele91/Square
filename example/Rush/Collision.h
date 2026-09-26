//
//  Collision.h
//  Rush
//
//  The static triangles of the scene in world space, for the hovercraft:
//  ground probes (raycast) and walls (sphere push out).
//  CollisionSystem (game system, on demand): every world gets a CollisionWorld, its
//  CollisionMesh (world().instance<CollisionWorld>()->add(context, actor)).
//
#pragma once
#include <Square/Square.h>
#include <vector>

class CollisionMesh
{
public:
	struct Hit
	{
		float        m_distance{ 0.0f };
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ 0.0f, 1.0f, 0.0f };
	};

	//add the static meshes of an actor and its children, with their current world transform;
	//sub meshes with a non opaque material (glass, glows...) are not solid and are skipped
	void add(Square::Context& context, const Square::Shared<Square::Scene::Actor>& actor);

	//closest hit along origin + t * direction (direction normalized), t in [0, max_distance]
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, Hit& hit) const;

	//push a sphere out of the walls (triangles with |normal.y| < max_normal_y), horizontally:
	//floors and ceilings are left to the ground probes. True if the sphere moved
	bool push_out(Square::Vec3& center, float radius, float max_normal_y) const;

	//bounds of the triangles transformed by transform (e.g. world to hull space); false if empty
	bool bounds(const Square::Mat4& transform, Square::Vec3& out_min, Square::Vec3& out_max) const;

	size_t size() const { return m_triangles.size(); }

private:
	struct Triangle
	{
		Square::Vec3 m_a, m_b, m_c;
		Square::Vec3 m_normal;
		Square::Vec3 m_min, m_max;
	};
	std::vector<Triangle> m_triangles;

	void add_triangle(const Square::Vec3& a, const Square::Vec3& b, const Square::Vec3& c);
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
