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
	class SQUARE_API AABoundingBox
	{
	public:

		AABoundingBox();
		AABoundingBox(AABoundingBox&&);
		AABoundingBox(const AABoundingBox&);
		AABoundingBox(const Vec3& min, const Vec3& max);
		AABoundingBox& operator = (AABoundingBox&&) = default;
		AABoundingBox& operator = (const AABoundingBox&) = default;

		const Vec3& get_min() const;

		const Vec3& get_max() const;

		Vec3 get_center() const;

		Vec3 get_extension() const;

		/**
		* Return the 8 point of aabb
		*/
		std::array< Vec3, 8 > get_bounding_box() const;

		/**
		* Return the 8 points (of aabb) multiplied to "model matrix"
		*/
		std::array< Vec3, 8 > get_bounding_box(const Mat4& model) const;

		/*
		* Applay a matrix to aabb
		*/
		void applay(const Mat4& model);

		/**
		* Create an AABB that contains this AABB and another AABB
		* @param other, the other AABB
		* @return A new AABoundingBox that encloses both AABBs
		*/
		AABoundingBox merge(const AABoundingBox& other) const;

		/*
		* Applay a matrix to aabb and return the new aabb
		*/
		AABoundingBox  operator*  (const Mat4& model) const;

		/*
		* Applay a matrix to obb aabb return it's self
		*/
		AABoundingBox& operator*= (const Mat4& model);

		Vec3 closest_point(const Vec3& point) const;

	protected:

		Vec3 m_min;
		Vec3 m_max;

	};
}
}
