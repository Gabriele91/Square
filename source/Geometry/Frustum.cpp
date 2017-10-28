//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Geometry/Frustum.h"
namespace Square
{
namespace Geometry
{
	//plane distance
	static float plane_distance(const Vec4& plane, const Vec3& point)
	{
		return plane.w + dot((Vec3)plane, point);
	}
	//plane distance
	Vec4 plane_normalize(const Vec4& plane)
	{
		float  size = length((Vec3)plane);
		return plane / size;
	}
	//update planes
	void Frustum::update_frustum(const Mat4& projection)
	{
		const Vec4& p_x = row(projection, 0);
		const Vec4& p_y = row(projection, 1);
		const Vec4& p_z = row(projection, 2);
		const Vec4& p_w = row(projection, 3);

		m_planes[0] = plane_normalize(p_w + p_x);
		m_planes[1] = plane_normalize(p_w - p_x);
		m_planes[2] = plane_normalize(p_w + p_y);
		m_planes[3] = plane_normalize(p_w - p_y);
		m_planes[4] = plane_normalize(p_w + p_z);
		m_planes[5] = plane_normalize(p_w - p_z);
	}
	//distance
	float Frustum::distance_from_plane(Frustum::PlaneType plane, const Vec3& point) const
	{
		return plane_distance(m_planes[plane], point);
	}
	float Frustum::distance_from_near_plane(const Vec3& point) const
	{
		return plane_distance(m_planes[NEARP], point);
	}
}
}
