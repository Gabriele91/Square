#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API StaticMesh : public Square::Scene::Component
								 , public Square::Render::Renderable
	{
	public:
		SQUARE_OBJECT(StaticMesh)

		Square::Geometry::OBoundingBox m_obb_local;
		Square::Geometry::OBoundingBox m_obb_global;
		Square::Shared< Square::Resource::Mesh >       m_mesh;
		Square::Shared< Square::Resource::Material >   m_material;
		Square::Weak< Square::Render::Transform >      m_transform;

		StaticMesh(Square::Context& context);

		virtual size_t materials_count() const override;
		virtual Square::Weak<Square::Render::Material> material(size_t i = 0) const override;

		virtual void draw
		(
			Square::Render::Context& render
			, size_t material_id
			, Square::Render::EffectPassInputs& input
			, Square::Render::EffectPass& pass
		) override;

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
		virtual void serialize(Square::Data::Archive& archivie)  override;
		virtual void serialize_json(Square::Data::Json& archivie) override;
		//deserialize
		virtual void deserialize(Square::Data::Archive& archivie) override;
		virtual void deserialize_json(Square::Data::Json& archivie) override;

		// build bbox
		bool build_local_obounding_box();
	};
}
}