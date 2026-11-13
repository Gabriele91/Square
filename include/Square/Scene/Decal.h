//
//  Decal.h
//  Square
//
//  A decal on an actor (Render::Decal): its box at the place and the turn of the actor (its scale
//  too), its half sizes (x, z: the plane of the texture; y: how far it projects, up and down from
//  the actor).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/Decal.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API Decal : public Component
	                       , public SharedObject<Decal>
	                       , public Render::Decal
	{
	public:
		//A square object
		SQUARE_OBJECT(Decal)
		static void object_registration(Context& ctx);

		Decal(Context& context);

		//its half sizes (x, z: its texture; y: how far it projects up and down)
		void size(const Vec3& half);
		const Vec3& size() const;

		//Render::Decal
		virtual Mat4 box() const override;
		virtual const Geometry::Sphere& bounding_sphere() const override;

		//serialize
		virtual void serialize(Data::Archive& archive) override;
		virtual void serialize_json(Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Data::Archive& archive) override;
		virtual void deserialize_json(Data::JsonValue& archive) override;

	protected:

		Vec3                     m_size{ 1.0f };
		mutable Geometry::Sphere m_sphere; //(of its box where it is now)
	};
}
}
