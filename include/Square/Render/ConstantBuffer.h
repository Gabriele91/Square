//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
// Includes
#include <array>
#include "Square/Config.h"
#include "Square/Math/Linear.h"

// Macro
#define CBAlignas alignas(16)
#define CBStruct  struct alignas(16)

// Definitions
namespace Square
{
	template < typename T, size_t A = 16 >
	struct alignas(A) CBAligned
	{
		// Array size
		CBAligned() = default;
		CBAligned(const T& value) : m_value(value) {}

		// Asign operation
		T& operator = (const T& value) { return (m_value = value); }

		// Cast operator
		operator const T& () const { return m_value; }
		operator T& ()             { return m_value; }

	protected:

		alignas(A) T m_value;

	};
               
	using CBBool      = uint32;
	using CBArrBool   = CBAligned < CBBool >;
	using CBArrFloat  = CBAligned < float  >;
	using CBArrDouble = CBAligned < double >;
	using CBArrInt8   = CBAligned < int8 >;
	using CBArrInt16  = CBAligned < int16 >;
	using CBArrInt32  = CBAligned < int32 >;
	using CBArrUint8  = CBAligned < uint8 >;
	using CBArrUint16 = CBAligned < uint16 >;
	using CBArrUint32 = CBAligned < uint32 >;
	using CBArrVec2   = CBAligned < Vec2 >;
	using CBArrVec3   = CBAligned < Vec3 >;
	using CBArrVec4	  = CBAligned < Vec4 >;
	using CBArrMat4	  = CBAligned < Mat4 >;

}