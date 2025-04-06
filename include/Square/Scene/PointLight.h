//
//  PointLight.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/Light.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;
    //Camera
    class SQUARE_API PointLight : public Component
								, public SharedObject<PointLight>
								, public Render::PointLight
    {
	public:
		//A square object
		SQUARE_OBJECT(PointLight)
		static void object_registration(Context& ctx);

		//using
		using Render::PointLight::radius;
		using Render::PointLight::inside_radius;
		using Render::PointLight::diffuse;
		using Render::PointLight::specular;
		using Render::PointLight::constant;

		//Init
		PointLight(Context& context);

		//change radius aka change geometry
		virtual void radius(const float radius) override;

		//shadow map override
		virtual const Render::ShadowBuffer& shadow_buffer() const override;
		virtual bool shadow() const override;
		virtual Vec4 shadow_viewport() const override;

		//shadow map custom 
		void  shadow(const IVec2& size);
		const IVec2& shadow_size() const;

		//all events
		virtual void on_attach(Actor& entity)      override;
		virtual void on_deattch()                  override;
		virtual void on_transform()                override;
		virtual void on_message(const Message& msg)override;

		//object methods
		//serialize
		virtual void serialize(Data::Archive& archive)   override;
		virtual void serialize_json(Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Data::Archive& archive)   override;
		virtual void deserialize_json(Data::JsonValue& archive) override;

		//lights methods
		virtual const Geometry::Sphere& bounding_sphere() const override;
		virtual const Geometry::Frustum& frustum() const override;
		virtual Weak<Render::Transform> transform() const override;

		//Reg object
		virtual void set(Render::UniformPointLight* data) const override;
		virtual void set(Render::UniformPointShadowLight* data, bool draw_shadow_map = true) const override;

	protected:
		//info
		Vec3 m_position;
		Geometry::Sphere m_sphere;
		//shadow
		Render::ShadowBuffer m_buffer;
		//view matrix
		mutable Mat4 m_views[6];
		mutable bool m_views_is_dirty{ true };
		mutable Mat4 m_projection;
		mutable bool m_projection_is_dirty{ true };

		const Mat4& views(size_t i) const;
		const Mat4& projection() const;
	};
}
}