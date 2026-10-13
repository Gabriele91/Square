//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Geometry
{

	OBoundingBox::OBoundingBox()
	{
	}

	OBoundingBox::OBoundingBox(OBoundingBox&& obb)
	{
		std::swap(m_rotation, obb.m_rotation);
		std::swap(m_position, obb.m_position);
		std::swap(m_extension, obb.m_extension);
	}
	
	OBoundingBox::OBoundingBox(const OBoundingBox& obb)
	: m_rotation(obb.m_rotation)
	, m_position(obb.m_position)
	, m_extension(obb.m_extension)
	{
	}

	OBoundingBox::OBoundingBox(const Mat3& rotation, const Vec3& position, const Vec3& extension)
	{
		m_rotation = rotation;
		m_position = position;
		m_extension = extension;
	}

	void OBoundingBox::set(const Mat3& rotation, const Vec3& position, const Vec3& extension)
	{
		m_rotation = rotation;
		m_position = position;
		m_extension = extension;
	}

	float OBoundingBox::volume() const
	{
		return 8 * m_extension[0] * m_extension[1] * m_extension[2];
	}

	std::array< Vec3, 8 > OBoundingBox::get_bounding_box() const
	{
		std::array< Vec3, 8 > p;
		//its axes: the columns of the rotation
		const Vec3& r = m_rotation[0];
		const Vec3& u = m_rotation[1];
		const Vec3& f = m_rotation[2];
		p[0] = m_position - r*m_extension[0] - u*m_extension[1] - f*m_extension[2];
		p[1] = m_position + r*m_extension[0] - u*m_extension[1] - f*m_extension[2];
		p[2] = m_position + r*m_extension[0] - u*m_extension[1] + f*m_extension[2];
		p[3] = m_position - r*m_extension[0] - u*m_extension[1] + f*m_extension[2];
		p[4] = m_position - r*m_extension[0] + u*m_extension[1] - f*m_extension[2];
		p[5] = m_position + r*m_extension[0] + u*m_extension[1] - f*m_extension[2];
		p[6] = m_position + r*m_extension[0] + u*m_extension[1] + f*m_extension[2];
		p[7] = m_position - r*m_extension[0] + u*m_extension[1] + f*m_extension[2];
		return p;
	}

	std::array< Vec3, 8 > OBoundingBox::get_bounding_box(const Mat4& model) const
	{
		std::array< Vec3, 8 > p;
		//its axes: the columns of the rotation
		const Vec3& r = m_rotation[0];
		const Vec3& u = m_rotation[1];
		const Vec3& f = m_rotation[2];
		p[0] = m_position - r*m_extension[0] - u*m_extension[1] - f*m_extension[2];
		p[1] = m_position + r*m_extension[0] - u*m_extension[1] - f*m_extension[2];
		p[2] = m_position + r*m_extension[0] - u*m_extension[1] + f*m_extension[2];
		p[3] = m_position - r*m_extension[0] - u*m_extension[1] + f*m_extension[2];
		p[4] = m_position - r*m_extension[0] + u*m_extension[1] - f*m_extension[2];
		p[5] = m_position + r*m_extension[0] + u*m_extension[1] - f*m_extension[2];
		p[6] = m_position + r*m_extension[0] + u*m_extension[1] + f*m_extension[2];
		p[7] = m_position - r*m_extension[0] + u*m_extension[1] + f*m_extension[2];
		//mul by model matrix
		for (Vec3& point : p)
		{
			point = (Vec3)(model * Vec4(point, 1.0));
		}		
		return p;
	}

	namespace AuxOBoundingBox
	{
		//the half axes of a box are still a box: none flat, each orthogonal to the others (a non
		//uniform scale on a rotated box shears them)
		bool still_a_box(const Vec3 (&half)[3])
		{
			const float flat = 0.000001f;
			const float tolerance = 0.001f;
			for (const Vec3& axis : half)
			{
				if (length(axis) <= flat) return false;
			}
			const Vec3 x = normalize(half[0]);
			const Vec3 y = normalize(half[1]);
			const Vec3 z = normalize(half[2]);
			const bool xy = std::abs(dot(x, y)) < tolerance;
			const bool xz = std::abs(dot(x, z)) < tolerance;
			const bool yz = std::abs(dot(y, z)) < tolerance;
			return xy && xz && yz;
		}
	}

	/*
	* Applay a matrix to OBoundingBox: its center and its half axes through the matrix (no
	* decomposition: a non uniform scale on a rotated box is a shear)
	*/
	void OBoundingBox::applay(const Mat4& model)
	{
		const Mat3 linear(model);
		Vec3 half[3];
		for (int i = 0; i != 3; ++i)
		{
			half[i] = linear * (m_rotation[i] * m_extension[i]);
		}
		m_position = Vec3(model * Vec4(m_position, 1.0f));
		if (AuxOBoundingBox::still_a_box(half))
		{
			for (int i = 0; i != 3; ++i)
			{
				m_extension[i] = length(half[i]);
				m_rotation[i] = half[i] / m_extension[i];
			}
		}
		else
		{
			//sheared or flat: the box on the axes of the world that holds it
			m_rotation = Mat3(1.0f);
			m_extension = glm::abs(half[0]) + glm::abs(half[1]) + glm::abs(half[2]);
		}
	}

	/*
	* Applay a matrix to OBoundingBox and return the new OBoundingBox
	*/
	OBoundingBox  OBoundingBox::operator*  (const Mat4& model) const
	{
		OBoundingBox new_obb(*this);
		new_obb.applay(model);
		return new_obb;
	}

	/*
	* Applay a matrix to obb and return this
	*/
	OBoundingBox& OBoundingBox::operator*= (const Mat4& model)
	{
		this->applay(model);
		return *this;
	}

	// The implementation of this function is from Christer Ericson's Real-Time Collision Detection, p.133.
	Vec3 OBoundingBox::closest_point(const Vec3& target) const
	{
		// Best: 33.412 nsecs / 89.952 ticks, Avg: 33.804 nsecs, Worst: 34.180 nsecs
		Vec3 d = target - m_position;
		// Start at the center point of the OBB.
		Vec3 closest_point = m_position;
		//its axes: the columns of the rotation
		const Mat3& axis = m_rotation;
		// Project the target onto the OBB axes and walk towards that point.
		for (int i = 0; i < 3; ++i)
		{
			closest_point += clamp(dot(d, axis[i]), -m_extension[i], m_extension[i]) * axis[i];
		}

		return closest_point;
	}


	// Create AABB from a OBB
	AABoundingBox OBoundingBox::to_aabb() const
	{
		// Get the 8 vertices of the OBB
		auto obb_vertices = std::move(get_bounding_box());

		// Initialize the min and max points for the AABB
		Vec3 aabb_min = obb_vertices[0];
		Vec3 aabb_max = obb_vertices[0];

		// Find the min and max coordinates for the AABB
		for (const auto& vertex : obb_vertices)
		{
			aabb_min = Square::min<Vec3>(aabb_min, vertex); // element-wise min
			aabb_max = Square::max<Vec3>(aabb_max, vertex); // element-wise max
		}

		// Return the new AABB
		return AABoundingBox(aabb_min, aabb_max);
	}

	// Test is valid OBB
	bool OBoundingBox::valid() const
	{
		static const auto epsilon = Constants::epsilon<float>() * 100.0f;
		// Check if rows (or columns) are orthogonal to each other
		if (!epsilon_equal(dot(m_rotation[0], m_rotation[1]), 0.0f, epsilon) ||
			!epsilon_equal(dot(m_rotation[0], m_rotation[2]), 0.0f, epsilon) ||
			!epsilon_equal(dot(m_rotation[1], m_rotation[2]), 0.0f, epsilon)) {
			return false;
		}

		// Check if each row (or column) is a unit vector
		if (!epsilon_equal(length(m_rotation[0]), 1.0f, epsilon) ||
			!epsilon_equal(length(m_rotation[1]), 1.0f, epsilon) ||
			!epsilon_equal(length(m_rotation[2]), 1.0f, epsilon)) {
			return false;
		}

		// Check if the determinant is close to 1
		float det = determinant(m_rotation);
		if (!epsilon_equal(det, 1.0f, epsilon)) {
			return false;
		}

		// All conditions passed, so the matrix is a valid rotation matrix
		return true;
	}

}
}
