//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Square
{
namespace Geometry
{
	class SQUARE_API Sphere
	{
	public:

		Sphere() {}

		Sphere(const Vec3& in_position, float in_radius)
		{
			m_position = in_position;
			m_radius   = in_radius;
		}

        void position(const Vec3& in_position)
        {
            m_position = in_position;
        }
        
        void center(const Vec3& in_center)
        {
            m_position = in_center;
        }

		void radius(float in_radius)
		{
			m_radius = in_radius;
		}

        const Vec3& get_position() const
        {
            return m_position;
        }
        
        const Vec3& get_center() const
        {
            return m_position;
        }

		float get_radius() const
		{
			return m_radius;
		}

		float volume() const
		{
			return 4.0f * Constants::pi<float>() * std::pow(m_radius, 2);
		}

	protected:

		Vec3  m_position;
		float m_radius{ 0.0 };

	};
}
}
