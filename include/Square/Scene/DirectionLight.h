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
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;
    //Camera
    class SQUARE_API DirectionLight : public Component
								    , public SharedObject<DirectionLight>
								    , public Render::DirectionLight
    {
	public:
		//A square object
		SQUARE_OBJECT(DirectionLight)
		static void object_registration(Context& ctx);

		//using
		using Render::DirectionLight::diffuse;
		using Render::DirectionLight::specular;

		//Init
		DirectionLight(Context& context);

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
		virtual void serialize(Data::Archive& archivie)   override;
		virtual void serialize_json(Data::Json& archivie) override;
		//deserialize
		virtual void deserialize(Data::Archive& archivie)   override;
		virtual void deserialize_json(Data::Json& archivie) override;

		//lights methods
		virtual const Geometry::Sphere& bounding_sphere() const override;
		virtual const Geometry::Frustum& frustum() const override;
		virtual Weak<Render::Transform> transform() const override;

		//Reg object
		virtual void set(Render::UniformDirectionLight* data) const override;
		virtual void set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map = true) const override;

		virtual void set_scene_size(const Geometry::AABoundingBox& scene) override;

	protected:

		Vec3 m_direction;
		Mat3 m_rotation;
		//shadow
		Render::ShadowBuffer m_buffer;
		Geometry::AABoundingBox m_scene_size;
		mutable Render::UniformDirectionShadowLight m_cache_udirectionshadowlight;
	};
}
}