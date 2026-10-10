//
//  Occluder.h
//  Square
//
//  An occluder of a scene (Render::Occluder): its triangles in the space of its actor, a simple
//  shape inside what it stands for (a wall, a cliff, the ground under a hill), never drawn; the
//  software occlusion of each camera hides what is behind it. Saved with its scene: the
//  converter makes it from a node "square_occluder" (its mesh: drawn too, or "proxy": only the
//  occluder).
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Render/Occluder.h"
#include "Square/Render/Transform.h"
#include <vector>

namespace Square
{
namespace Scene
{
	class SQUARE_API Occluder : public Square::Scene::Component
	                          , public Square::Render::Occluder
	{
	public:
		SQUARE_OBJECT(Occluder)

		Occluder(Square::Context& context);
		virtual ~Occluder();

		//its triangles in the space of its actor (3 points each)
		void triangles(const std::vector<Square::Vec3>& triangles);
		const std::vector<Square::Vec3>& triangles() const { return m_triangles; }

		//it hides (false: not drawn into the occlusion)
		void enabled(bool enabled) { m_enabled = enabled; }
		bool enabled() const { return m_enabled; }

		//Render::Occluder
		virtual const std::vector<Square::Vec3>& occluder_triangles() override;
		virtual const Square::Geometry::AABoundingBox& occluder_box() override;
		virtual bool can_occlude() const override;

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

		//its triangles and their box in world space (again when its actor moved)
		void build_world();

		std::vector< Square::Vec3 >               m_triangles;
		std::vector< Square::Vec3 >               m_world;
		Square::Geometry::AABoundingBox           m_world_box;
		Square::Weak< Square::Render::Transform > m_transform;
		bool                                      m_enabled{ true };
		bool                                      m_dirty{ true };
	};
}
}
