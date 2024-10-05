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
#include "Square/Render/Viewport.h"
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

		//change outer angle aka change frustum
		virtual void outer_cut_off(const float coff) override;

		//all events
		virtual void on_attach(Actor& entity)      override;
		virtual void on_deattch()                  override;
		virtual void on_transform()                override;
		virtual void on_message(const Message& msg)override;

		//shadow map override
		virtual const Render::ShadowBuffer& shadow_buffer() const override;
		virtual bool shadow() const override;
		virtual Vec4 shadow_viewport() const override;

		//shadow map custom 
		void  shadow(const IVec2& size);
		const IVec2& shadow_size() const;

		//object methods
		//serialize
		virtual void serialize(Data::Archive& archive)   override;
		virtual void serialize_json(Data::Json& archive) override;
		//deserialize
		virtual void deserialize(Data::Archive& archive)   override;
		virtual void deserialize_json(Data::Json& archive) override;

		//lights methods
		virtual const Geometry::Sphere& bounding_sphere() const override;
		virtual const Geometry::Frustum& frustum() const override;
		virtual Weak<Render::Transform> transform() const override;

		//Reg object
		virtual void set(Render::UniformSpotLight* data) const override;
		virtual void set(Render::UniformSpotShadowLight* data, bool draw_shadow_map = true) const override;


	protected:
		//light info
		Vec3 m_position;
		Vec3 m_direction;
		Geometry::Sphere m_sphere;
		//shadow
		mutable Mat4		         m_view;
		mutable bool			     m_view_is_dirty{ true };

		mutable Render::Viewport	 m_viewport;
		mutable bool			     m_viewport_is_dirty{ true };
		
		mutable Geometry::Frustum	 m_frustum;
		mutable bool			     m_frustum_is_dirty{ true };

		Render::ShadowBuffer         m_buffer;
		//help
		const Mat4& view() const;
		const Mat4& projection() const;
	};
}
}