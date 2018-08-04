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

		Sphere();

		Sphere(const Vec3& in_position, float in_radius);

		void position(const Vec3& in_position);
        
		void center(const Vec3& in_center);

		void radius(float in_radius);

		const Vec3& get_position() const;
        
		const Vec3& get_center() const;

		float get_radius() const;

		float volume() const;

		/*
		* Applay a matrix to aabb and return the new sphere
		*/
		Sphere  operator*  (const Mat4& model) const;

		/*
		* Applay a matrix to sphere and return this
		*/
		Sphere& operator*= (const Mat4& model);

	protected:

		Vec3  m_position;
		float m_radius{ 0.0 };

	};
}
}
