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

	template < typename T, size_t A = 16, size_t P = 16 >
	PACKED(struct alignas(A) CBAlignedWithPadding
	{

		// Array size
		CBAlignedWithPadding() { std::memset(m_pad, 0, struct_padding); }
		CBAlignedWithPadding(const T& value) : m_value(value) { std::memset(m_pad, 0, struct_padding); }

		// Asign operation
		T& operator = (const T& value) { return (m_value = value); }

		// Cast operator
		operator const T& () const { return m_value; }
		operator T& () { return m_value; }

	protected:
		static constexpr std::size_t struct_padding = P - sizeof(T) % P;
		T m_value;
		unsigned char m_pad[struct_padding];
	});
               
	using CBBool      = uint32;
	using CBArrBool   = CBAlignedWithPadding < CBBool >;
	using CBArrFloat  = CBAlignedWithPadding < float  >;
	using CBArrDouble = CBAlignedWithPadding < double >;
	using CBArrInt8   = CBAlignedWithPadding < int8 >;
	using CBArrInt16  = CBAlignedWithPadding < int16 >;
	using CBArrInt32  = CBAlignedWithPadding < int32 >;
	using CBArrUint8  = CBAlignedWithPadding < uint8 >;
	using CBArrUint16 = CBAlignedWithPadding < uint16 >;
	using CBArrUint32 = CBAlignedWithPadding < uint32 >;
	using CBArrVec2   = CBAlignedWithPadding < Vec2 >;
	using CBArrVec3   = CBAlignedWithPadding < Vec3 >;
	using CBArrVec4	  = CBAligned < Vec4 >;
	using CBArrMat4	  = CBAligned < Mat4 >;

}