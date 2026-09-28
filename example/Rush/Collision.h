//
//  Collision.h
//  Rush
//
//  The collisions of Blitz3D (collision.cpp, meshcollider.cpp, world.cpp):
//  - CollisionMesh: triangles in world space, in a tree of boxes (16 per leaf); a sphere
//    moving along a segment against them: the face, the edges (cylinders) and the vertices
//    (spheres), the first contact along the segment;
//  - MeshCollider (component, EntityType + the mesh): the triangles of the meshes of its
//    actor and children, and its collision type;
//  - SphereCollider (component, EntityType + EntityRadius): a moving sphere, its collision
//    type, and the collisions of the last update (CountCollisions, CollisionNX...);
//  - CollisionWorld (per world): the rules (Collisions src, dst, method, response) and the
//    update (UpdateWorld): every sphere collider goes from where it was at the last update to
//    where it is now, and on a hit it slides on the plane of the contact (up to 10 hits; on
//    two planes along their crease);
//  - CollisionSystem (game system, on demand): the update of every world, every frame (after
//    the components moved their actors, Component::on_update; the results are there in
//    Component::on_late_update).
//
#pragma once
#include <Square/Square.h>
#include <functional>
#include <unordered_map>
#include <vector>

//how a sphere tests the destination (Blitz3D collision methods)
enum class CollisionMethod
{
	SPHERE,  //against the other SphereColliders (a sphere where they are now)
	POLYGON  //against the triangles of a MeshCollider
};

//what a sphere does on a hit (Blitz3D collision responses)
enum class CollisionResponse
{
	SLIDE,   //slides on the contact plane(s)
	SLIDEXZ  //slides, as far as it moved on x/z: it does not slide down a slope by falling
};

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

	//a ray hit
	struct Hit
	{
		float        m_distance{ 0.0f };
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ 0.0f, 1.0f, 0.0f };
	};

	//a segment: origin + direction * t
	struct Line
	{
		Square::Vec3 m_origin{ 0.0f };
		Square::Vec3 m_direction{ 0.0f };
		Square::Vec3 at(float t) const { return m_origin + m_direction * t; }
	};

	//add the static meshes of an actor and its children, with their current world transform;
	//sub meshes with a non opaque material (glass, glows...) are not solid and are skipped
	void add(Square::Context& context, const Square::Shared<Square::Scene::Actor>& actor);
	void clear();

	//first contact of a sphere of radius moving along line, if before collision.m_time; with
	//y_scale the line is in a space where y is scaled (an ellipsoid, Blitz3D y_scale): the
	//triangles are scaled too
	bool collide(const Line& line, float radius, Collision& collision, float y_scale = 1.0f) const;

	//closest hit along origin + t * direction (direction normalized), t in [0, max_distance]
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, Hit& hit) const;

	//bounds of the triangles transformed by transform (e.g. world to hull space); false if empty
	bool bounds(const Square::Mat4& transform, Square::Vec3& out_min, Square::Vec3& out_max) const;

	size_t size() const { return m_triangles.size(); }

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
	bool collide(const Line& line, float radius, float y_scale, const Square::Vec3& box_min, const Square::Vec3& box_max, int node, Collision& collision) const;
};

//a collision of the last update of a sphere collider
struct CollisionReport
{
	Square::Vec3                           m_point{ 0.0f };  //on the surface hit
	Square::Vec3                           m_normal{ 0.0f, 1.0f, 0.0f };
	Square::Weak<Square::Scene::Actor>     m_with;           //the actor hit
	int                                    m_type{ 0 };      //its collision type
};

//a moving sphere (Blitz3D EntityType + EntityRadius)
class SphereCollider : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(SphereCollider)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	SphereCollider(Square::Context& context);

	//collision type (the rules of the CollisionWorld)
	int type() const { return m_type; }
	void type(int type) { m_type = type; }

	//radius, in world units: a sphere, or an ellipsoid (x/z radius, y radius; Blitz3D
	//EntityRadius x,y)
	float radius() const { return m_radius; }
	float radius_y() const { return m_radius_y; }
	void radius(float radius) { m_radius = m_radius_y = radius; }
	void radius(float radius, float radius_y) { m_radius = radius; m_radius_y = radius_y; }

	//center of the sphere from the actor (world units, turned with the actor); the collisions
	//move the actor so that the center stays out
	const Square::Vec3& offset() const { return m_offset; }
	void offset(const Square::Vec3& offset) { m_offset = offset; }

	//center of the sphere now
	Square::Vec3 center() const;

	//the next update starts from where the actor is now (a teleport, not a move)
	void reset() { m_has_previous = false; }
	//the next update moves the sphere from `from` (its center, world) to where it is then
	void reset(const Square::Vec3& from) { m_previous = from; m_has_previous = true; }

	//the collisions of the last update
	const std::vector<CollisionReport>& collisions() const { return m_collisions; }
	//it hit a collider of type in the last update (Blitz3D EntityCollided), with a contact
	//normal.y of at least min_normal_y (e.g. 0.5: a floor, not a wall in front or a ceiling)
	bool collided(int type, float min_normal_y = -1.0f) const;

	//serialize (attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	int                          m_type{ 0 };
	float                        m_radius{ 1.0f }; 
	float                        m_radius_y{ 1.0f };
	Square::Vec3                 m_offset{ 0.0f };
	Square::Vec3                 m_previous{ 0.0f };
	bool                         m_has_previous{ false };
	std::vector<CollisionReport> m_collisions;
	friend class CollisionWorld;
};

//the triangles of the meshes of its actor and children (Blitz3D EntityType + mesh), static:
//built from where they are the first time they are needed
class MeshCollider : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(MeshCollider)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	MeshCollider(Square::Context& context);

	//collision type (the rules of the CollisionWorld)
	int type() const { return m_type; }
	void type(int type) { m_type = type; }

	//its triangles, in world space
	const CollisionMesh& mesh();

	//serialize (attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	int           m_type{ 0 };
	CollisionMesh m_mesh;
	bool          m_built{ false };
};

//the collisions of a world
class CollisionWorld : public Square::SystemInstance
{
public:
	SQUARE_OBJECT(CollisionWorld)

	CollisionWorld(Square::Context& context, Square::System& system, Square::Scene::World& world);

	//a rule (Blitz3D Collisions): the sphere colliders of src_type against the colliders of
	//dst_type, with method (SPHERE: the other sphere colliders, POLYGON: the mesh colliders) and
	//response
	void collisions(int src_type, int dst_type, CollisionMethod method, CollisionResponse response);

	//every sphere collider with a rule goes from where it was at the last update to where it
	//is now (Blitz3D UpdateWorld); the CollisionSystem calls it every frame
	void update();

	//closest hit of a ray with the mesh colliders (Blitz3D LinePick)
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, CollisionMesh::Hit& hit);

private:
	struct Rule
	{
		int               m_dst_type;
		CollisionMethod   m_method;
		CollisionResponse m_response;
	};
	std::unordered_map< int, std::vector<Rule> > m_rules;

	//the colliders of the world now (Blitz3D enumerates the entities every update)
	struct Colliders
	{
		std::vector< Square::Shared<SphereCollider> > m_spheres;
		std::vector< Square::Shared<MeshCollider> >   m_meshes;
	};
	Colliders colliders() const;
	void collide(SphereCollider& source, const Colliders& colliders);
};

//the collisions of the worlds, updated every frame
class CollisionSystem : public Square::System
{
public:
	SQUARE_SYSTEM(CollisionSystem, Square::SystemRing::GAME)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	CollisionSystem(Square::Context& context);

	virtual bool initialize() override { return true; }
	virtual void shutdown() override;
	virtual void update(double delta_time) override;
	virtual Square::Shared<Square::SystemInstance> create_instance(Square::Scene::World& world) override;

private:
	std::vector< Square::Weak<CollisionWorld> > m_instances;
};
