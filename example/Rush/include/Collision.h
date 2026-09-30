//
//  Collision.h
//  Rush
//
//  Collisions of moving spheres (and ellipsoids) against triangle meshes and other spheres:
//  - CollisionMesh: triangles in world space, in a tree of boxes (16 per leaf); a sphere
//    moving along a segment against them: the face, the edges (cylinders) and the vertices
//    (spheres), the first contact along the segment;
//  - MeshCollider (component): the triangles of the meshes of its actor and children, and its
//    collision type;
//  - SphereCollider (component): a moving sphere, its collision type, and the collisions of
//    the last step (contact points and normals);
//  - CollisionWorld (per world): the rules (source type, destination type, method, response)
//    and the steps: every sphere collider goes from where it was at the last step to where it
//    is now, and on a hit it slides on the plane of the contact (up to 10 hits; on two planes
//    along their crease);
//  - CollisionSystem (game system, on demand): the steps of every world, every frame.
//  The time goes in fixed steps (CollisionWorld::Settings::step), whatever the frame rate: a
//  frame runs as many steps as its time holds; a FixedStepListener component moves its actor
//  at every step (before the collisions of the step), and after the steps of the frame it can
//  show a pose between the last two steps (no stutter when the frames are not multiples of
//  the steps). So the same input gives the same motion at 30 or at 120 frames per second.
//  Order in a frame: Component::on_update (input), the steps, Component::on_late_update.
//
#pragma once
#include <Square/Square.h>
#include <functional>
#include <unordered_map>
#include <vector>

class CollisionDebug;

//how a sphere tests the destination
enum class CollisionMethod
{
	SPHERE,  //against the other SphereColliders (a sphere where they are now)
	POLYGON  //against the triangles of a MeshCollider
};

//what a sphere does on a hit
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
		Square::Vec3 m_normal{ Square::Constants::axis_y };
		int          m_triangle{ -1 };
	};

	//a ray hit
	struct Hit
	{
		float        m_distance{ 0.0f };
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ Square::Constants::axis_y };
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
	//y_scale the line is in a space where y is scaled (an ellipsoid: a sphere of radius in a
	//space squeezed on y): the triangles are scaled too
	bool collide(const Line& line, float radius, Collision& collision, float y_scale = 1.0f) const;

	//closest hit along origin + t * direction (direction normalized), t in [0, max_distance]
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, Hit& hit) const;

	//bounds of the triangles transformed by transform (e.g. world to hull space); false if empty
	bool bounds(const Square::Mat4& transform, Square::Vec3& out_min, Square::Vec3& out_max) const;

	size_t size() const { return m_triangles.size(); }

	//the triangles in world space, three vertices each (e.g. to draw them)
	void triangles(std::vector<Square::Vec3>& out) const;

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

//a collision of the last step of a sphere collider
struct CollisionReport
{
	Square::Vec3                           m_point{ 0.0f };  //on the surface hit
	Square::Vec3                           m_normal{ Square::Constants::axis_y };
	Square::Weak<Square::Scene::Actor>     m_with;           //the actor hit
	int                                    m_type{ 0 };      //its collision type
};

//a moving sphere
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

	//radius, in world units: a sphere, or an ellipsoid (x/z radius, y radius)
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

	//the next step starts from where the actor is now (a teleport, not a move)
	void reset() { m_has_previous = false; }
	//the next step moves the sphere from `from` (its center, world) to where it is then
	void reset(const Square::Vec3& from) { m_previous = from; m_has_previous = true; }

	//the collisions of the last step
	const std::vector<CollisionReport>& collisions() const { return m_collisions; }
	//it hit a collider of type in the last step, with a contact normal.y of at least
	//min_normal_y (e.g. 0.5: a floor, not a wall in front or a ceiling)
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

//the triangles of the meshes of its actor and children, static: built from where they are
//the first time they are needed
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

//a component (also a Square::Scene::Component) moved at the fixed steps of the collisions
class FixedStepListener
{
public:
	virtual ~FixedStepListener() = default;
	//a step of step seconds: move the actor (the collisions of the step come after it)
	virtual void on_fixed_update(double step) = 0;
	//after the steps of a frame (also none): alpha in [0, 1), the time of the frame between
	//the last step and the next one (e.g. to show a pose between the last two steps)
	virtual void on_fixed_interpolate(double alpha) {}
};

//the collisions of a world
class CollisionWorld : public Square::SystemInstance
{
public:
	SQUARE_OBJECT(CollisionWorld)

	struct Settings
	{
		double step{ 1.0 / 60.0 }; //seconds of a step
		int    max_steps{ 8 };     //steps of a frame at most: a longer frame (loading, debugger) loses the rest of its time
	};

	CollisionWorld(Square::Context& context, Square::System& system, Square::Scene::World& world);
	virtual ~CollisionWorld();

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//a rule: the sphere colliders of src_type against the colliders of dst_type, with method
	//(SPHERE: the other sphere colliders, POLYGON: the mesh colliders) and response
	void collisions(int src_type, int dst_type, CollisionMethod method, CollisionResponse response);

	//a frame of delta_time seconds: the steps it holds (every step: the listeners, then every
	//sphere collider with a rule from where it was to where it is now), then the listeners
	//interpolate; the CollisionSystem calls it every frame
	void update(double delta_time);

	//closest hit of a ray with the mesh colliders
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, CollisionMesh::Hit& hit);

	//the colliders in the levels of the world
	const std::vector< Square::Weak<SphereCollider> >& spheres() const { return m_spheres; }
	const std::vector< Square::Weak<MeshCollider> >&   meshes() const { return m_meshes; }

	//debug view of the collisions (CollisionDebug), drawn over the frame of the world
	void debug(bool enable);
	bool debug() const { return m_debug != nullptr; }
	const Square::Shared<CollisionDebug>& debug_view() const { return m_debug; }

	//the colliders and the listeners that join / leave the levels of the world
	virtual void on_add_component(const Square::Shared<Square::Scene::Actor>& actor, const Square::Shared<Square::Scene::Component>& component) override;
	virtual void on_remove_component(const Square::Shared<Square::Scene::Actor>& actor, const Square::Shared<Square::Scene::Component>& component) override;

private:
	struct Rule
	{
		int               m_dst_type;
		CollisionMethod   m_method;
		CollisionResponse m_response;
	};
	std::unordered_map< int, std::vector<Rule> > m_rules;
	Settings m_settings;
	double   m_time{ 0.0 }; //time not stepped yet

	//the colliders and the listeners in the levels of the world (kept by the add/remove events,
	//like the render collection of a level)
	struct Listener
	{
		Square::Weak<Square::Scene::Component> m_component;
		FixedStepListener*                     m_listener{ nullptr };
	};
	std::vector< Square::Weak<SphereCollider> > m_spheres;
	std::vector< Square::Weak<MeshCollider> >   m_meshes;
	std::vector< Listener >                     m_listeners;
	Square::Shared<CollisionDebug>              m_debug;

	void step();
	void collide(SphereCollider& source);
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
