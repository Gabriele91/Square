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
#include "Square/Render/ShadowBuffer.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Scene/Component.h"
#include <array>
#include <vector>

namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;

	//how the cascades of a directional light follow the camera
	enum class CascadeFit : int
	{
		//a square of constant size around the slice of the frustum (its box), every frame (snapped
		//to the texel): the sharpest, it moves with the camera
		FOLLOW = 0,
		//a square around the sphere of the slice, larger by a margin: it moves only when the slice
		//leaves it (the cache of its static casters stays valid), not when the camera turns
		STABLE = 1
	};
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

		//the cascades of its shadow (1 to DIRECTION_SHADOW_CSM_NUMBER_OF_FACES)
		void cascades(int cascades);
		int  cascades() const;

		//how far from the camera its shadow reaches (the cascades split over it, not over the
		//whole view: sharper near); 0: as far as the camera sees
		void  shadow_distance(float distance);
		float shadow_distance() const;

		//how its cascades follow the camera, the margin of a stable one (a share of the radius of
		//its slice: more, moved less often, its texels larger)
		void       cascade_fit(CascadeFit fit);
		CascadeFit cascade_fit() const;
		void       cascade_margin(float margin);
		float      cascade_margin() const;
		//its cascades stay (the cache of the shadow pass can keep them)
		virtual bool stable_cascades() const override;

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
		virtual void set(Render::UniformDirectionLight* data) const override;
		virtual void set(Render::UniformDirectionShadowLight* data, const Render::Camera* camera, bool draw_shadow_map = true) const override;

		virtual void set_scene_size(const Geometry::AABoundingBox& scene) override;

	protected:

		//the cascades of a stable fit (they stay while the slices are inside them)
		void stable_uniform(Render::UniformDirectionShadowLight& data, const Render::Camera& camera) const;
		//the depth of the light over the scene, the same while the scene stays in it
		Vec2 stable_depth(const Mat4& light_view) const;

		Vec3 m_direction;
		Mat3 m_rotation;
		//shadow
		Render::ShadowBuffer m_buffer;
		int m_cascades{ DIRECTION_SHADOW_CSM_DEFAULT_FACES };
		float m_shadow_distance{ 0.0f };
		Geometry::AABoundingBox m_scene_size;
		CascadeFit m_cascade_fit{ CascadeFit::FOLLOW };
		float m_cascade_margin{ 0.2f };
		mutable Render::UniformDirectionShadowLight m_cache_udirectionshadowlight;
		//a stable cascade: where it is (its center in the space of the light), its half side; what
		//all of them were made for (a change: made again); the depth of the light (its scene)
		struct StableCascade
		{
			Vec2  m_center{ 0.0f };
			float m_half{ 0.0f };
			bool  m_valid{ false };
		};
		mutable std::array<StableCascade, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES> m_stable;
		mutable std::vector<float> m_stable_key;
		mutable Vec2 m_stable_depth{ 0.0f };
		mutable bool m_stable_depth_valid{ false };
	};
}
}