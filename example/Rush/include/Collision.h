//
//  Collision.h
//  Rush
//
//  Collisions of moving spheres (and ellipsoids) against triangle meshes and other spheres:
//  - CollisionMesh: objects (a mesh where it is, or a sphere) in a tree of their boxes in the
//    world; each mesh its triangles in its own space in a tree of boxes, shared by its objects
//    (the instances of a prop: one tree); a sphere moving along a segment: the objects its box
//    meets, in each one the triangles (in the world) the face, the edges (cylinders) and the
//    vertices (spheres), the first contact along the segment;
//  - MeshCollider (component): the triangles of the meshes of its actor and children (the solid
//    ones, or every one), its collision type, one sided or not;
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

//the grounds of a map, by the names of its meshes: "water..." (deep water), "shallow..." (shallow
//water), "mud...", "sand...", "dirt...", any other the ground (rock, road, grass). The water ones
//are solid for the collisions also with a translucent material (the hovercraft glide on them)
enum class Surface : unsigned char
{
	GROUND,
	DIRT,
	SAND,
	MUD,
	SHALLOW,
	WATER,
	ICE,
	COUNT
};
Surface surface_of(const std::string& name);
//water: solid though not opaque
bool is_water(Surface surface);

class CollisionMesh
{
public:
	//first contact of a moving sphere: time along the move (0 start, 1 end), contact normal
	struct Collision
	{
		float        m_time{ 1.0f };
		Square::Vec3 m_normal{ Square::Constants::axis_y };
		int          m_triangle{ -1 };
		Surface      m_surface{ Surface::GROUND };
	};

	//a ray hit
	struct Hit
	{
		float        m_distance{ 0.0f };
		Square::Vec3 m_point{ 0.0f };
		Square::Vec3 m_normal{ Square::Constants::axis_y };
		Surface      m_surface{ Surface::GROUND };
	};

	//a segment: origin + direction * t
	struct Line
	{
		Square::Vec3 m_origin{ 0.0f };
		Square::Vec3 m_direction{ 0.0f };
		Square::Vec3 at(float t) const { return m_origin + m_direction * t; }
	};

	//add the meshes of an actor and its children (static and instanced: an object each, each
	//instance one), where they are now; sub meshes with a non opaque material (glass, glows...)
	//are not solid and are skipped (not the water: its surface by the name of its node, see
	//Surface). Only the first level of a level of detail (Scene::LodGroup). The property of the
	//game "collision" of a node (Scene::Properties, Blender "game_collision"), its children too:
	//false not solid, true / "mesh" its triangles (also not opaque), "box" its box, "sphere" a
	//sphere in its box.
	//solid_only: only the opaque surfaces (not the translucent, not the alpha tested ones);
	//false: every surface (e.g. the triangles of a navmesh)
	void add(Square::Context& context, const Square::Shared<Square::Scene::Actor>& actor, bool solid_only = true);
	void clear();

	//first contact of a sphere of radius moving along line, if before collision.m_time; with
	//y_scale the line is in a space where y is scaled (an ellipsoid: a sphere of radius in a
	//space squeezed on y): the triangles are scaled too
	bool collide(const Line& line, float radius, Collision& collision, float y_scale = 1.0f) const;

	//one sided: a sphere hits only the front of its triangles (moving against their normal, the
	//winding of the mesh); from behind it goes through (a wall: out of it free to come in, in it
	//it does not go out); false: both faces
	void one_sided(bool one_sided) { m_one_sided = one_sided; }
	bool one_sided() const { return m_one_sided; }

	//closest hit along origin + t * direction (direction normalized), t in [0, max_distance]
	bool raycast(const Square::Vec3& origin, const Square::Vec3& direction, float max_distance, Hit& hit) const;

	//bounds of the triangles transformed by transform (e.g. world to hull space); false if empty
	bool bounds(const Square::Mat4& transform, Square::Vec3& out_min, Square::Vec3& out_max) const;

	//its triangles in the world (the ones of every object), its objects, its shapes (the meshes:
	//the objects of the same mesh share one)
	size_t size() const { return m_triangle_count; }
	size_t objects() const { return m_objects.size(); }
	size_t shapes() const { return m_shapes; }

	//the triangles in world space, three vertices each (e.g. to draw them; not the spheres)
	void triangles(std::vector<Square::Vec3>& out) const;

	//a box of a tree: its two children, or its items (first, count: in the order of the tree)
	struct Node
	{
		Square::Vec3 m_min{ 0.0f }, m_max{ 0.0f };
		int          m_left{ -1 }, m_right{ -1 };
		int          m_first{ 0 }, m_count{ 0 };
	};
	//a tree of boxes over some items (by their boxes)
	struct Tree
	{
		std::vector<Node> m_nodes;
		std::vector<int>  m_order;
		void build(const std::vector<Square::Vec3>& mins, const std::vector<Square::Vec3>& maxs, size_t leaf);
		int  build_node(const std::vector<Square::Vec3>& mins, const std::vector<Square::Vec3>& maxs, int first, int count, size_t leaf);
	};
	//a triangle in the space of its shape
	struct Triangle
	{
		Square::Vec3 m_a, m_b, m_c;
	};
	//the triangles of a mesh in its own space and their tree (the objects of that mesh share it)
	struct Shape
	{
		std::vector<Triangle> m_triangles;
		Tree                  m_tree;
	};

private:
	//an object: a shape where it is (its matrix), or a sphere; its box in the world, its ground
	struct Object
	{
		Square::Shared<const Shape> m_shape;
		Square::Mat4                m_model{ 1.0f };
		Square::Mat4                m_inverse{ 1.0f };
		bool                        m_mirrored{ false }; //(its matrix turns the winding)
		bool                        m_similar{ false };  //its matrix turns, moves and scales the same on every axis
		float                       m_scale{ 1.0f };     //(its scale then)
		bool                        m_sphere{ false };
		Square::Vec3                m_center{ 0.0f };
		float                       m_radius{ 0.0f };
		Square::Vec3                m_min{ 0.0f }, m_max{ 0.0f };
		Surface                     m_surface{ Surface::GROUND };
	};
	std::vector<Object> m_objects;
	Tree                m_tree;  //of the objects
	size_t              m_shapes{ 0 };
	size_t              m_triangle_count{ 0 };
	bool                m_one_sided{ false };

	//an object added (its box in the world made)
	void add_object(Object object);
	//the tree of the objects
	void build();
	//the first contact with an object, if before collision.m_time
	bool collide(const Object& object, const Line& line, float radius, float y_scale, const Square::Vec3& box_min, const Square::Vec3& box_max, Collision& collision) const;
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

	//its triangles: only the opaque surfaces (true, the default), or every surface (e.g. an
	//invisible alpha tested wall); before its mesh is built
	void solid_only(bool solid_only) { m_solid_only = solid_only; }
	bool solid_only() const { return m_solid_only; }

	//one sided (see CollisionMesh::one_sided)
	void one_sided(bool one_sided) { m_mesh.one_sided(one_sided); }
	bool one_sided() const { return m_mesh.one_sided(); }

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
	bool          m_solid_only{ true };
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
		int    max_steps{ 10 };     //steps of a frame at most: a longer frame (loading, debugger) loses the rest of its time
	};

	CollisionWorld(Square::Context& context, Square::System& system, Square::Scene::World& world);
	virtual ~CollisionWorld();

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//paused: no steps, its time not counted (a pause of the game: nothing moves, nothing to
	//catch up after)
	void paused(bool paused) { m_paused = paused; }
	bool paused() const { return m_paused; }

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
	bool     m_paused{ false };

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
