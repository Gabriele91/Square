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
		Vec3 r(m_rotation[0][0], m_rotation[1][0], m_rotation[2][0]);
		Vec3 u(m_rotation[0][1], m_rotation[1][1], m_rotation[2][1]);
		Vec3 f(m_rotation[0][2], m_rotation[1][2], m_rotation[2][2]);
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
		Vec3 r(m_rotation[0][0], m_rotation[1][0], m_rotation[2][0]);
		Vec3 u(m_rotation[0][1], m_rotation[1][1], m_rotation[2][1]);
		Vec3 f(m_rotation[0][2], m_rotation[1][2], m_rotation[2][2]);
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

	/*
	* Applay a matrix to OBoundingBox
	*/
	void OBoundingBox::applay(const Mat4& model)
	{
		Mat4
		transform = translate(Mat4(1), m_position);
		transform*= Mat4(m_rotation);
		//new box
		transform = model * transform;
#if 0
		//decompone
		Vec3 scale;
		Quat rotation;
		Vec3 translation;
		Vec3 skew;
		Vec4 perspective;
		glm::decompose(transform, scale, rotation, translation, skew, perspective);
		//to OBoundingBox
		m_position  = translation;
		m_rotation  = Mat3_cast( rotation );
		m_extension = scale * m_extension;
#elif 1
		Vec3 translation, scale;
		Quat rotation;
		decompose_mat4(transform, translation, rotation, scale);
		m_position  = translation;
		m_rotation  = to_mat3(rotation);
		m_extension = scale * m_extension;
#else
		//to OBoundingBox
		m_position   = Vec3(transform[3]);
		//rotation
		auto r_scale = Mat3(transform);
		auto scale   = Vec3(length(r_scale[0]),
							length(r_scale[1]),
							length(r_scale[2]));
		r_scale[0] /= scale[0];
		r_scale[1] /= scale[1];
		r_scale[2] /= scale[2];
		m_rotation = traspose(inverse(r_scale));
		//scale
		m_extension = scale * m_extension;
#endif
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
		//axis
		const Mat3& axis = transpose(m_rotation);
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
