//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Geometry
{
	AABoundingBox::AABoundingBox()
	{
	}

	AABoundingBox::AABoundingBox(AABoundingBox&& aabb)
	{
		std::swap(m_min, aabb.m_min);
		std::swap(m_max, aabb.m_max);
	}

	AABoundingBox::AABoundingBox(const AABoundingBox& aabb)
	: m_min(aabb.m_min)
	, m_max(aabb.m_max)
	{

	}

	AABoundingBox::AABoundingBox(const Vec3& min, const Vec3& max)
	{
		m_min = min;
		m_max = max;
	}

	const Vec3& AABoundingBox::get_min() const
	{
		return m_min;
	}

	const Vec3& AABoundingBox::get_max() const
	{
		return m_max;
	}

	Vec3 AABoundingBox::get_center() const
	{
		return (m_max + m_min) / 2.0f;
	}

	Vec3 AABoundingBox::get_extension() const
	{
		return abs(m_max - get_center());
	}

	std::array< Vec3, 8 > AABoundingBox::get_bounding_box() const
	{
		std::array< Vec3, 8 > p;
		
		Vec3  r(1, 0, 0);
		Vec3  u(0, 1, 0);
		Vec3  f(0, 0, 1);
		const Vec3& extension = get_extension();
		const Vec3& position  = get_center();

		p[0] = position - r*extension[0] - u*extension[1] - f*extension[2];
		p[1] = position + r*extension[0] - u*extension[1] - f*extension[2];
		p[2] = position + r*extension[0] - u*extension[1] + f*extension[2];
		p[3] = position - r*extension[0] - u*extension[1] + f*extension[2];
		p[4] = position - r*extension[0] + u*extension[1] - f*extension[2];
		p[5] = position + r*extension[0] + u*extension[1] - f*extension[2];
		p[6] = position + r*extension[0] + u*extension[1] + f*extension[2];
		p[7] = position - r*extension[0] + u*extension[1] + f*extension[2];

		return p;

	}

	std::array< Vec3, 8 > AABoundingBox::get_bounding_box(const Mat4& model) const
	{
		auto output = get_bounding_box();
		for (Vec3& point : output) point = Vec3(model * Vec4(point, 1.0));
		return output;
	}

	void AABoundingBox::applay(const Mat4& model)
	{
#if 0
		//info aabb
		Vec3 center     = get_center();
		Vec3 extension  = m_max - center;
		//info transormation
		Mat3 scale_rotation(model);
		Vec3 position(model[3]);
		//compute new coors
		Vec3 new_center    = center*scale_rotation + position;
		Vec3 new_extension = extension*scale_rotation;
		//save new aabb
		m_max = new_center + new_extension;
		m_min = new_center - new_extension;
#else
		Vec3 min{ std::numeric_limits<float>::max() };
		Vec3 max{ std::numeric_limits<float>::lowest() };
		for (const Vec3& point : get_bounding_box(model))
		{
			min = Square::min(min, point);
			max = Square::max(max, point);
		}
		m_min = min;
		m_max = max;
#endif
	}

	AABoundingBox AABoundingBox::merge(const AABoundingBox& other) const
	{
		// Compute the new min and max points
		Vec3 new_min = min(m_min, other.get_min()); // element-wise min
		Vec3 new_max = max(m_max, other.get_max()); // element-wise max

		// Return the new AABB
		return AABoundingBox(new_min, new_max);
	}

	AABoundingBox  AABoundingBox::operator*  (const Mat4& model) const
	{
		AABoundingBox out(*this); 
		out.applay(model); 
		return out;
	}

	AABoundingBox& AABoundingBox::operator*= (const Mat4& model)
	{
		this->applay(model); return *this;
	}

	Vec3 AABoundingBox::closest_point(const Vec3& point) const
	{
		Vec3 result;

		     if (point.x > m_max.x) result.x = m_max.x;
	    else if (point.x < m_min.x) result.x = m_min.x;
		else  result.x = point.x;

			 if (point.y > m_max.y) result.y = m_max.y;
		else if (point.y < m_min.y) result.y = m_min.y;
		else  result.y = point.y;

			 if (point.z > m_max.z) result.z = m_max.z;
		else if (point.z < m_min.z) result.z = m_min.z;
		else  result.z = point.z;

		return result;

	}
}
}
