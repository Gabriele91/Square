#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include <functional>
#include <vector>

namespace Square
{
namespace Scene
{
	class SQUARE_API StaticMesh : public Square::Scene::Component
								 , public Square::Render::Renderable
	{
	private:

		Square::Geometry::OBoundingBox m_obb_local;
		Square::Geometry::OBoundingBox m_obb_global;
		Square::Weak< Square::Render::Transform >      m_transform;
		bool										   m_obb_dirty;

	public:
		SQUARE_OBJECT(StaticMesh)

		Square::Shared< Square::Resource::Mesh >       m_mesh;
		std::vector< Square::Shared< Square::Resource::Material > >   m_materials;

		StaticMesh(Square::Context& context);

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

		//its sub meshes, the shader bound by the caller (the velocity pass)
		virtual bool draw_geometry(Square::Render::Context& render) const override;

		virtual bool support_culling() const override;
		virtual bool visible() const override;
		virtual void on_transform() override;

		virtual const Square::Geometry::OBoundingBox& bounding_box() override;
		virtual Square::Weak<Square::Render::Transform> transform() const override;

		//events
		virtual void on_attach(Square::Scene::Actor& entity) override;
		virtual void on_deattch() override;
		virtual void on_message(const Square::Scene::Message& msg) override;

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

		//its triangles in world space (3 points each, added to out), read from the file of its mesh
		//(the GPU mesh keeps no copy of them); filter: the sub meshes taken (by their index: e.g.
		//by their material), none: all of them; false: no mesh, or it cannot be read
		bool triangles(std::vector<Square::Vec3>& out, const std::function<bool(size_t submesh)>& filter = nullptr);
		// build bbox
		bool build_local_obounding_box(bool from_triangles=true);
		void set_obounding_box(const Square::Geometry::OBoundingBox& obb);
		//the box of its mesh in its own space
		const Square::Geometry::OBoundingBox& local_bounding_box() const { return m_obb_local; }
	};
}
}