//
//  CollisionDebug.h
//  Rush
//
//  The debug view of the collisions of a world (CollisionWorld::debug): a post effect of the
//  color stage that copies the frame and draws over it, in wireframe (over everything, no
//  depth test):
//  - the triangles of the mesh colliders (the solid surfaces, as the spheres see them);
//  - the sphere colliders, spheres or ellipsoids (x/z radius, y radius), green, red when they
//    hit something in the last step;
//  - the contact points of the last step, small yellow spheres.
//  Only the engine API of the post effects: the engine knows nothing of the collisions.
//
#pragma once
#include <Square/Square.h>
#include <vector>

class CollisionWorld;
class MeshCollider;

class CollisionDebug : public Square::Render::PostEffect
{
public:
	SQUARE_OBJECT(CollisionDebug)

	struct Settings
	{
		Square::Vec4 mesh_color{ 0.35f, 0.35f, 0.45f, 1.0f };
		Square::Vec4 sphere_color{ 0.2f, 1.0f, 0.2f, 1.0f };
		Square::Vec4 hit_color{ 1.0f, 0.2f, 0.2f, 1.0f };
		Square::Vec4 contact_color{ 1.0f, 1.0f, 0.1f, 1.0f };
		float        contact_radius{ 0.1f };  //world units
		bool         meshes{ true };
		bool         spheres{ true };
		bool         contacts{ true };
	};

	CollisionDebug(Square::Context& context, CollisionWorld& world);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//the world is gone: nothing more to draw (the CollisionWorld calls it)
	void detach() { m_world = nullptr; }

	virtual void draw(Square::Render::PostEffectFrame& frame) override;

protected:
	virtual void on_release() override;

private:
	//a mesh collider and its triangles on the GPU (built the first time it is drawn)
	struct MeshShape
	{
		Square::Weak<MeshCollider>          m_collider;
		Square::Shared<Square::Render::Mesh> m_mesh;
	};

	CollisionWorld*                          m_world{ nullptr };
	Settings                                 m_settings;
	Square::Shared<Square::Resource::Shader> m_copy;
	Square::Shared<Square::Resource::Effect> m_effect;
	Square::Shared<Square::Render::ConstBuffer> m_transform;
	Square::Shared<Square::Render::Mesh>     m_sphere;  //radius 1
	std::vector<MeshShape>                   m_meshes;

	bool build();
	Square::Render::Mesh* mesh_of(const Square::Shared<MeshCollider>& collider);
	void draw_shape(Square::Render::PostEffectFrame& frame, Square::Render::Mesh& mesh, const Square::Mat4& model, const Square::Vec4& color);
};
