//
//  Sprite.h
//  Square
//
//  A sprite of a scene: a quad at its actor, turned toward the camera (or around the axis of its
//  material), its size (world units), its color, its rotation; the frame of the flipbook of its
//  material its own (>= 0) or by the time. Its material a sprite effect (Sprite, SpriteAdditive:
//  translucent, unlit). Saved with its scene: the converter makes it from a node
//  "square_sprite" (a plane: its size, its material).
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/SpriteBatch.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API Sprite : public Square::Scene::Component
	                        , public Square::Render::Renderable
	{
	public:
		SQUARE_OBJECT(Sprite)

		Sprite(Square::Context& context);
		virtual ~Sprite();

		//its material (a sprite effect)
		void material(const Square::Shared<Square::Resource::Material>& material) { m_material = material; }
		const Square::Shared<Square::Resource::Material>& material_resource() const { return m_material; }

		//its size (world units), its color, its rotation (radians, around the view), the frame of
		//its flipbook (< 0: by the time)
		void size(const Square::Vec2& size);
		const Square::Vec2& size() const { return m_size; }
		void color(const Square::Vec4& color) { m_color = color; }
		const Square::Vec4& color() const { return m_color; }
		void rotation(float rotation) { m_rotation = rotation; }
		float rotation() const { return m_rotation; }
		void frame(float frame) { m_frame = frame; }
		float frame() const { return m_frame; }

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

		Square::Shared< Square::Resource::Material > m_material;
		Square::Vec2                                 m_size{ 1.0f, 1.0f };
		Square::Vec4                                 m_color{ 1.0f };
		float                                        m_rotation{ 0.0f };
		float                                        m_frame{ -1.0f };
		Square::Render::SpriteBatch                  m_batch;
		Square::Render::SpriteInstance               m_sprite;
		Square::Geometry::OBoundingBox               m_obb_global;
		Square::Weak< Square::Render::Transform >    m_transform;
		bool                                         m_obb_dirty{ true };
	};
}
}
