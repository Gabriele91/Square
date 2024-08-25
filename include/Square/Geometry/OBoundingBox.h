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
	class SQUARE_API OBoundingBox
	{
	public:
		/*		
		* Constructor of Oriented Bounding Box
		*/
		OBoundingBox() {} //default

		/**
		* Constructor of obb
		* @param rotation, set rotation of obb
		* @param position, set local position of obb
		* @param extension, set extension (aka size) of obb
		*/
		OBoundingBox(const Mat3& rotation, const Vec3& position, const Vec3& extension);

		/**
		* Set values of obb
		* @param rotation, set rotation of obb
		* @param position, set local position of obb
		* @param extension, set extension (aka size) of obb
		*/
		void set(const Mat3& rotation, const Vec3& position, const Vec3& extension);

		/**
		* Return the 8 point of obb
		*/
		std::vector< Vec3 > get_bounding_box() const;

		/**
		* Return the 8 points (of obb) multiplied to "model matrix"
		*/
		std::vector< Vec3 > get_bounding_box(const Mat4& model) const;

		// get volume
		float volume() const;

		/*
		* Applay a matrix to obb
		*/
		void applay(const Mat4& model);

		/*
		* Applay a matrix to obb and return the new Oriented Bounding Box
		*/
        OBoundingBox  operator*  (const Mat4& model) const;

		/*
		* Applay a matrix to obb and return it's self 
		*/
		OBoundingBox& operator*= (const Mat4& model);

		//test
		Vec3 closest_point(const Vec3& target) const;

		//get info
		const Mat3& get_rotation_matrix() const { return m_rotation; }
		const Vec3& get_position() const { return m_position; }
		const Vec3& get_extension() const { return m_extension; }

		// Return if it is a valid OBB
		bool valid() const;

	private:

		Mat3 m_rotation { 0 };// rotation matrix for the transformation
		Vec3 m_position { 0 };// translation component of the transformation
		Vec3 m_extension{ 0 };// bounding box extents

	};
}
}
