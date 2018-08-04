//
//  PointLight.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/Light.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;
    //Camera
    class SQUARE_API SpotLight : public Component
								, public SharedObject<SpotLight>
								, public Render::SpotLight
    {
	public:
		//A square object
		SQUARE_OBJECT(SpotLight)
		static void object_registration(Context& ctx);

		//using
		using Render::SpotLight::cut_off;
		using Render::SpotLight::inner_cut_off;
		using Render::SpotLight::outer_cut_off;
		using Render::SpotLight::radius;
		using Render::SpotLight::inside_radius;
		using Render::SpotLight::diffuse;
		using Render::SpotLight::specular;
		using Render::SpotLight::constant;

		//Init
		SpotLight(Context& context);

		//change radius aka change geometry
		virtual void radius(const float radius) override;

		//all events
		virtual void on_attach(Actor& entity)      override;
		virtual void on_deattch()                  override;
		virtual void on_transform()                override;
		virtual void on_message(const Message& msg)override;

		//object methods
		//serialize
		virtual void serialize(Data::Archive& archivie)   override;
		virtual void serialize_json(Data::Json& archivie) override;
		//deserialize
		virtual void deserialize(Data::Archive& archivie)   override;
		virtual void deserialize_json(Data::Json& archivie) override;

		//lights methods
		virtual const Geometry::Sphere& bounding_sphere() const override;
		virtual const Geometry::Frustum& frustum() const override;
		virtual Weak<Render::Transform> transform() const override;
		virtual Weak<Render::Camera> camera() const override;
		//Reg object
		virtual void set(Render::UniformSpotLight* data) const override;

	protected:

		Vec3 m_position;
		Vec3 m_direction;
		Geometry::Sphere m_sphere;
	};
}
}