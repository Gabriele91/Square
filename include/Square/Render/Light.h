//
//  Light.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"

namespace Square
{
    namespace Render
    {
        class Material;
        class Transform;
        class Camera;
    }
    namespace Geometry
    {
        class Sphere;
        class Frustum;
    }
}

namespace Square
{
namespace Render
{
	enum class LightType
	{
		SPOT,
		POINT,
		DIRECTION
	};

    class SQUARE_API Light : public BaseObject
    {
	public:
		
		SQUARE_OBJECT(Light)

        virtual const Geometry::Sphere& bounding_sphere() const = 0;
        
        virtual const Geometry::Frustum& frustum() const = 0;

        virtual Weak<Transform> transform() const = 0;
        
        virtual Weak<Camera> camera() const = 0;

		virtual LightType type() const = 0;

        bool visible() const { return m_visible; }
        
        void visible(bool enable) { m_visible = enable; }

        bool shadow_caster() const { return m_shadow_caster; }
        
        void shadow_caster(bool enable) { m_shadow_caster = enable; }


	private:

		bool m_visible{ true };
        bool m_shadow_caster{ false };
	};
}
}
