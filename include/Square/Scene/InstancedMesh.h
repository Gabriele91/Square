//
//  InstancedMesh.h
//  Square
//
//  A mesh drawn many times in a draw call (GPU instancing): its instances, each its matrix in the
//  space of its actor. Its materials' effects draw it with the instanced variant of their
//  techniques ("variants instanced" in the .sqfx, SQ_INSTANCED defined: their vertex shaders read
//  the matrix of each instance, Instances.hlsl), at most instances_max a draw (more:
//  more draws). Culled as one (the box of all its instances), then, in a pass that says what it
//  sees (EffectPassInputs::m_frustum: the camera), each instance by its sphere: only the ones in
//  it drawn (the shadows: all of them). Saved with its scene (its mesh, its
//  materials, the box of its mesh, its instances): the converter makes it from the children of a
//  node "square_instances" that share a mesh.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include <array>
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include <functional>
#include <vector>

namespace Square
{
namespace Render
{
	class ConstBuffer;
	class SoftwareOcclusion;
}
namespace Scene
{
	class SQUARE_API InstancedMesh : public Square::Scene::Component
								    , public Square::Render::Renderable
	{
	public:
		SQUARE_OBJECT(InstancedMesh)

		//the instances of a draw at most (Instances.hlsl: INSTANCES_MAX)
		static constexpr size_t instances_max = 192;

		InstancedMesh(Square::Context& context);
		virtual ~InstancedMesh();

		//the mesh, its materials (one a sub mesh)
		void mesh(const Square::Shared<Square::Resource::Mesh>& mesh, const std::vector< Square::Shared<Square::Resource::Material> >& materials);
		const Square::Shared<Square::Resource::Mesh>& mesh() const { return m_mesh; }
		const std::vector< Square::Shared<Square::Resource::Material> >& materials() const { return m_materials; }

		//the box of the mesh (its own space: the box of all the instances from it)
		void mesh_box(const Square::Geometry::OBoundingBox& box);
		const Square::Geometry::OBoundingBox& mesh_box() const { return m_mesh_box; }

		//its instances: their matrices in the space of the actor
		void instances(const std::vector<Square::Mat4>& models);
		const std::vector<Square::Mat4>& instances() const { return m_instances; }

		//the triangles of all its instances in world space (3 points each, added to out), read from
		//the file of its mesh; filter: the sub meshes taken (by their index), none: all of them;
		//false: no mesh, or it cannot be read
		bool triangles(std::vector<Square::Vec3>& out, const std::function<bool(size_t submesh)>& filter = nullptr);

		//Renderable
		virtual size_t materials_count() const override;
		virtual Square::Weak<Square::Render::Material> material(size_t i = 0) const override;
		virtual void draw
		(
			  Square::Render::Context& render
			, size_t material_id
			, Square::Render::EffectPassInputs& input
			, Square::Render::EffectPass& pass
			, int draw_id = 0
		) override;
		virtual bool support_culling() const override;
		virtual bool visible() const override;
		virtual bool instanced() const override { return true; }
		virtual const Square::Geometry::OBoundingBox& bounding_box() override;
		virtual Square::Weak<Square::Render::Transform> transform() const override;

		//events
		virtual void on_transform() override;
		virtual void on_attach(Square::Scene::Actor& entity) override;
		virtual void on_deattch() override;

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		//the box of all the instances (the space of the actor), from the box of the mesh; the
		//sphere of each instance
		void build_local_box();
		//the spheres of the instances in world space (again when the actor moved)
		const std::vector<Square::Geometry::Sphere>& world_spheres();
		//the instances in a frustum not hidden by an occlusion (none: nothing hidden; the last ones
		//while they, the frustum and the actor stay)
		const std::vector<Square::Mat4>& visible_instances(const Square::Geometry::Frustum& frustum, const Square::Render::SoftwareOcclusion* occlusion);

		Square::Shared< Square::Resource::Mesh >                    m_mesh;
		std::vector< Square::Shared< Square::Resource::Material > > m_materials;
		std::vector< Square::Mat4 >                                 m_instances;
		Square::Shared< Square::Render::ConstBuffer >               m_buffer;   //the matrices of a draw
		std::vector< Square::Mat4 >                                 m_batch;    //... on the CPU (instances_max)
		Square::Geometry::OBoundingBox                              m_mesh_box;
		Square::Geometry::OBoundingBox                              m_obb_local;
		Square::Geometry::OBoundingBox                              m_obb_global;
		Square::Weak< Square::Render::Transform >                   m_transform;
		bool                                                        m_obb_dirty{ true };
		//the sphere of each instance: in the space of the actor, in world space
		std::vector< Square::Geometry::Sphere >                     m_spheres_local;
		std::vector< Square::Geometry::Sphere >                     m_spheres_world;
		bool                                                        m_spheres_dirty{ true };
		//the instances in the last frustum (its planes), still right while valid
		std::vector< Square::Mat4 >                                 m_visible;
		std::array< Square::Vec4, Square::Geometry::Frustum::N_PLANES > m_visible_planes;
		const Square::Render::SoftwareOcclusion*                    m_visible_occlusion{ nullptr };
		Square::uint64                                              m_visible_version{ 0 };
		bool                                                        m_visible_valid{ false };
	};
}
}
