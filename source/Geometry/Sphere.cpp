//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Geometry/Sphere.h"

namespace Square
{
namespace Geometry
{

	Sphere::Sphere() {}

	Sphere::Sphere(const Vec3& in_position, float in_radius)
	{
		m_position = in_position;
		m_radius = in_radius;
	}

	void Sphere::position(const Vec3& in_position)
	{
		m_position = in_position;
	}

	void Sphere::center(const Vec3& in_center)
	{
		m_position = in_center;
	}

	void Sphere::radius(float in_radius)
	{
		m_radius = in_radius;
	}

	const Vec3& Sphere::get_position() const
	{
		return m_position;
	}

	const Vec3& Sphere::get_center() const
	{
		return m_position;
	}

	float Sphere::get_radius() const
	{
		return m_radius;
	}

	float Sphere::volume() const
	{
		return 4.0f * Constants::pi<float>() * std::pow(m_radius, 2);
	}
	/*
	* Applay a matrix to aabb and return the new aabb
	*/
	Sphere  Sphere::operator*  (const Mat4& model) const
	{
		Sphere new_sphere(*this);
		new_sphere *= model;
		return new_sphere;
		Vec3 position = model * Vec4(0, 0, 0, 1);
		Vec3 scale = model * Vec4(0, 0, 0, 0);
	}
	/*
	* Applay a matrix to obb and return this
	*/
	Sphere& Sphere::operator*= (const Mat4& model)
	{
		//position
		Vec3 new_position = model * Vec4(m_position, 1);
		//radius
		Mat3  rs_matrix(model);
		float max_scale = max(length(rs_matrix[0]), length(rs_matrix[1]), length(rs_matrix[2]));
		float new_radius = get_radius() * max_scale;
		//set
		position(new_position);
		radius(new_radius);
		//return it's self
		return *this;
	}
}
}