//
//  InstancedMesh.h
//  Square
//
//  A mesh drawn many times in a draw call (GPU instancing): its instances, each its matrix in the
//  space of its actor. Its materials' effects draw it with the instanced variant of their
//  techniques ("variants instanced" in the .sqfx, SQ_INSTANCED defined: their vertex shaders read
//  the matrix of each instance, Instances.hlsl), at most instances_max a draw (more:
//  more draws). Culled as one: the box of all its instances.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include <vector>

namespace Square
{
namespace Render
{
	class ConstBuffer;
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

		//its instances: their matrices in the space of the actor; mesh_box: the box of the mesh
		//(its own space: the box of all the instances from it)
		void instances(const std::vector<Square::Mat4>& models, const Square::Geometry::OBoundingBox& mesh_box);
		const std::vector<Square::Mat4>& instances() const { return m_instances; }

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

		//serialize (not saved: made at runtime)
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		Square::Shared< Square::Resource::Mesh >                    m_mesh;
		std::vector< Square::Shared< Square::Resource::Material > > m_materials;
		std::vector< Square::Mat4 >                                 m_instances;
		Square::Shared< Square::Render::ConstBuffer >               m_buffer;   //the matrices of a draw
		std::vector< Square::Mat4 >                                 m_batch;    //... on the CPU (instances_max)
		Square::Geometry::OBoundingBox                              m_obb_local;
		Square::Geometry::OBoundingBox                              m_obb_global;
		Square::Weak< Square::Render::Transform >                   m_transform;
		bool                                                        m_obb_dirty{ true };
	};
}
}
