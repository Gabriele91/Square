//
//  Square
//
//  Created by Gabriele on 09/07/176.
//  Copyright ? 2017 Gabriele. All rights reserved.
//
#include <optional>
#include "Square/Config.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Resource.h"
#include "Square/Math/Linear.h"

namespace Square
{
#pragma region Variant  
	Variant::Variant()
	{
		set_type(VR_NONE);
	}

	Variant::~Variant()
	{
		set_type(VR_NONE);
	}

	Variant::Variant(VariantType type)
	{
		set_type(type);
	}

	Variant::Variant(const Variant& in)
	{
		copy_from(in);
	}

	Variant::Variant(Variant&& in) noexcept
	{
		move_from(std::forward<Variant>(in));
	}

	Variant::Variant(bool b)
	{
		set_type(VR_BOOL);
		m_b = b;
	}

	Variant::Variant(char c)
	{
		set_type(VR_CHAR);
		m_c = c;
	}

	Variant::Variant(short s)
	{
		set_type(VR_SHORT);
		m_s = s;
	}

	Variant::Variant(int i)
	{
		set_type(VR_INT);
		m_i = i;
	}

	Variant::Variant(long l)
	{
		set_type(VR_LONG);
		m_l = l;
	}

	Variant::Variant(long long l)
	{
		set_type(VR_LONGLONG);
		m_ll = l;
	}

	Variant::Variant(unsigned char uc)
	{
		set_type(VR_UCHAR);
		m_uc = uc;
	}

	Variant::Variant(unsigned short us)
	{
		set_type(VR_USHORT);
		m_us = us;
	}

	Variant::Variant(unsigned int ui)
	{
		set_type(VR_UINT);
		m_ui = ui;
	}

	Variant::Variant(unsigned long ul)
	{
		set_type(VR_ULONG);
		m_ul = ul;
	}

	Variant::Variant(unsigned long long ull)
	{
		set_type(VR_ULONGLONG);
		m_ull = ull;
	}

	Variant::Variant(float f)
	{
		set_type(VR_FLOAT);
		m_f = f;
	}

	Variant::Variant(double d)
	{
		set_type(VR_DOUBLE);
		m_d = d;
	}

	Variant::Variant(long double ld)
	{
		set_type(VR_LONG_DOUBLE);
		m_ld = ld;
	}

	Variant::Variant(const Vec2& v2)
	{
		set_type(VR_VEC2);
		m_v2 = v2;
	}

	Variant::Variant(const Vec3& v3)
	{
		set_type(VR_VEC3);
		m_v3 = v3;
	}

	Variant::Variant(const Vec4& v4)
	{
		set_type(VR_VEC4);
		m_v4 = v4;
	}

	Variant::Variant(const IVec2& iv2)
	{
		set_type(VR_IVEC2);
		m_iv2 = iv2;
	}

	Variant::Variant(const IVec3& iv3)
	{
		set_type(VR_IVEC3);
		m_iv3 = iv3;
	}

	Variant::Variant(const IVec4& iv4)
	{
		set_type(VR_IVEC4);
		m_iv4 = iv4;
	}

	Variant::Variant(const DVec2& dv2)
	{
		set_type(VR_DVEC2);
		m_dv2 = dv2;
	}

	Variant::Variant(const DVec3& dv3)
	{
		set_type(VR_DVEC3);
		m_dv3 = dv3;
	}

	Variant::Variant(const DVec4& dv4)
	{
		set_type(VR_DVEC4);
		m_dv4 = dv4;
	}

	Variant::Variant(const Quat& quat)
	{
		set_type(VR_QUAT);
		m_quat = quat;
	}

	Variant::Variant(const DQuat& dquat)
	{
		set_type(VR_DQUAT);
		m_dquat = dquat;
	}

	Variant::Variant(const Mat4& fm)
	{
		set_type(VR_FLOAT_MATRIX);
		*((Mat4*)(m_ptr)) = fm;
	}

	Variant::Variant(const DMat4& dm)
	{
		set_type(VR_DOUBLE_MATRIX);
		*((DMat4*)(m_ptr)) = dm;
	}

	Variant::Variant(const Vector< char >& v_c)
	{
		set_type(VR_STD_VECTOR_CHAR);
		*((Vector< char >*)(m_ptr)) = v_c;
	}

	Variant::Variant(const Vector< short >& v_s)
	{
		set_type(VR_STD_VECTOR_SHORT);
		*((Vector< short >*)(m_ptr)) = v_s;
	}

	Variant::Variant(const Vector< int >& v_i)
	{
		set_type(VR_STD_VECTOR_INT);
		*((Vector< int >*)(m_ptr)) = v_i;
	}

	Variant::Variant(const Vector< long >& v_l)
	{
		set_type(VR_STD_VECTOR_LONG);
		*((Vector< long >*)(m_ptr)) = v_l;
	}

	Variant::Variant(const Vector< long long >& v_ll)
	{
		set_type(VR_STD_VECTOR_LONGLONG);
		*((Vector< long long >*)(m_ptr)) = v_ll;
	}

	Variant::Variant(const Vector< unsigned char >& v_uc)
	{
		set_type(VR_STD_VECTOR_UCHAR);
		*((Vector< unsigned char >*)(m_ptr)) = v_uc;
	}

	Variant::Variant(const Vector< unsigned short >& v_us)
	{
		set_type(VR_STD_VECTOR_USHORT);
		*((Vector< unsigned short >*)(m_ptr)) = v_us;
	}

	Variant::Variant(const Vector< unsigned int >& v_ui)
	{
		set_type(VR_STD_VECTOR_UINT);
		*((Vector< unsigned int >*)(m_ptr)) = v_ui;
	}

	Variant::Variant(const Vector< unsigned long >& v_ul)
	{
		set_type(VR_STD_VECTOR_ULONG);
		*((Vector< unsigned long >*)(m_ptr)) = v_ul;
	}

	Variant::Variant(const Vector< unsigned long long >& v_ull)
	{
		set_type(VR_STD_VECTOR_ULONGLONG);
		*((Vector< unsigned long long >*)(m_ptr)) = v_ull;
	}

	Variant::Variant(const Vector< float >& v_f)
	{
		set_type(VR_STD_VECTOR_FLOAT);
		*((Vector< float >*)(m_ptr)) = v_f;
	}

	Variant::Variant(const Vector< double >& v_d)
	{
		set_type(VR_STD_VECTOR_DOUBLE);
		*((Vector< double >*)(m_ptr)) = v_d;
	}

	Variant::Variant(const Vector< long double >& v_ld)
	{
		set_type(VR_STD_VECTOR_LONG_DOUBLE);
		*((Vector< long double >*)(m_ptr)) = v_ld;
	}

	Variant::Variant(const Vector< Mat4 >& v_m4)
	{
		set_type(VR_STD_VECTOR_FLOAT_MATRIX);
		*((Vector< Mat4 >*)(m_ptr)) = v_m4;
	}

	Variant::Variant(const Vector< DMat4 >& v_dm4)
	{
		set_type(VR_STD_VECTOR_DOUBLE_MATRIX);
		*((Vector< DMat4 >*)(m_ptr)) = v_dm4;
	}

	Variant::Variant(const Vector< Vec2 >& v_v2)
	{
		set_type(VR_STD_VECTOR_VEC2);
		*((Vector< Vec2 >*)(m_ptr)) = v_v2;
	}

	Variant::Variant(const Vector< Vec3 >& v_v3)
	{
		set_type(VR_STD_VECTOR_VEC3);
		*((Vector< Vec3 >*)(m_ptr)) = v_v3;
	}

	Variant::Variant(const Vector< Vec4 >& v_v4)
	{
		set_type(VR_STD_VECTOR_VEC4);
		*((Vector< Vec4 >*)(m_ptr)) = v_v4;
	}

	Variant::Variant(const Vector< IVec2 >& v_iv2)
	{
		set_type(VR_STD_VECTOR_IVEC2);
		*((Vector< IVec2 >*)(m_ptr)) = v_iv2;
	}

	Variant::Variant(const Vector< IVec3 >& v_iv3)
	{
		set_type(VR_STD_VECTOR_IVEC3);
		*((Vector< IVec3 >*)(m_ptr)) = v_iv3;
	}

	Variant::Variant(const Vector< IVec4 >& v_iv4)
	{
		set_type(VR_STD_VECTOR_IVEC4);
		*((Vector< IVec4 >*)(m_ptr)) = v_iv4;
	}

	Variant::Variant(const Vector< DVec2 >& v_dv2)
	{
		set_type(VR_STD_VECTOR_DVEC2);
		*((Vector< DVec2 >*)(m_ptr)) = v_dv2;
	}

	Variant::Variant(const Vector< DVec3 >& v_dv3)
	{
		set_type(VR_STD_VECTOR_DVEC3);
		*((Vector< DVec3 >*)(m_ptr)) = v_dv3;
	}

	Variant::Variant(const Vector< DVec4 >& v_dv4)
	{
		set_type(VR_STD_VECTOR_DVEC4);
		*((Vector< DVec4 >*)(m_ptr)) = v_dv4;
	}

	Variant::Variant(const char* c_str)
	{
		set_type(VR_C_STRING);
		*((String*)(m_ptr)) = c_str;
	}

	Variant::Variant(const String& str)
	{
		set_type(VR_STD_STRING);
		*((String*)(m_ptr)) = str;
	}

	Variant::Variant(const Vector< String >& v_str)
	{
		set_type(VR_STD_VECTOR_STRING);
		*((Vector< String >*)(m_ptr)) = v_str;
	}

	Variant::Variant(const Shared< ResourceObject >& resouce)
	{
		set_type(VR_RESOURCE);
		*((Shared< ResourceObject >*)(m_ptr)) = resouce;
	}

	Variant::Variant(void* ptr)
	{
		set_type(VR_PTR);
		m_ptr = ptr;
	}

	//assignament op
	Variant& Variant::operator = (const Variant& value)
	{
		copy_from(value);
		return *this;
	}
	//move op
	Variant& Variant::operator = (Variant&& in) noexcept
	{
		move_from(std::forward<Variant>(in));
		return *this;
	}

	//type
	VariantType Variant::get_type() const
	{
		return m_type;
	}

	//query
	bool Variant::is_none() const
	{
		return m_type == VariantType::VR_NONE;
	}

	bool Variant::is_null() const
	{
		return m_type == VariantType::VR_NONE;
	}

	bool Variant::is_heap_value() const
	{
		switch (m_type)
		{
		case VariantType::VR_FLOAT_MATRIX:
		case VariantType::VR_DOUBLE_MATRIX:
		case VariantType::VR_RESOURCE:
		case VariantType::VR_C_STRING:
		case VariantType::VR_STD_STRING:
		case VariantType::VR_STD_VECTOR_CHAR:
		case VariantType::VR_STD_VECTOR_SHORT:
		case VariantType::VR_STD_VECTOR_INT:
		case VariantType::VR_STD_VECTOR_LONG:
		case VariantType::VR_STD_VECTOR_LONGLONG:
		case VariantType::VR_STD_VECTOR_UCHAR:
		case VariantType::VR_STD_VECTOR_USHORT:
		case VariantType::VR_STD_VECTOR_UINT:
		case VariantType::VR_STD_VECTOR_ULONG:
		case VariantType::VR_STD_VECTOR_ULONGLONG:
		case VariantType::VR_STD_VECTOR_FLOAT:
		case VariantType::VR_STD_VECTOR_DOUBLE:
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:
		case VariantType::VR_STD_VECTOR_VEC2:
		case VariantType::VR_STD_VECTOR_VEC3:
		case VariantType::VR_STD_VECTOR_VEC4:
		case VariantType::VR_STD_VECTOR_DVEC2:
		case VariantType::VR_STD_VECTOR_DVEC3:
		case VariantType::VR_STD_VECTOR_DVEC4:
		case VariantType::VR_STD_VECTOR_IVEC2:
		case VariantType::VR_STD_VECTOR_IVEC3:
		case VariantType::VR_STD_VECTOR_IVEC4:
		case VariantType::VR_STD_VECTOR_STRING: return true; break;
		default: return false; break;
		}
	}

	//get reference pointer
	void* Variant::get_ref() const
	{
		if (is_heap_value()) return m_ptr;
		else				 return (void*)&m_c;
	}

	//move from other variant
	void Variant::move_from(Variant&& in)
	{
		//alloc
		set_type(in.get_type());
		//copy

		switch (m_type)
		{
			//copy from heap
		case VariantType::VR_FLOAT_MATRIX:    get<Mat4>() = std::move(in.get< Mat4 >()); break;
		case VariantType::VR_DOUBLE_MATRIX:   get<DMat4>() = std::move(in.get< DMat4 >()); break;

		case VariantType::VR_STD_VECTOR_CHAR:
			get< Vector<char> >() = std::move(in.get< Vector<char> >());
			break;
		case VariantType::VR_STD_VECTOR_SHORT:
			get< Vector<short> >() = std::move(in.get< Vector<short> >());
			break;
		case VariantType::VR_STD_VECTOR_INT:
			get< Vector<int> >() = std::move(in.get< Vector<int> >());
			break;
		case VariantType::VR_STD_VECTOR_LONG:
			get< Vector<long> >() = std::move(in.get< Vector<long> >());
			break;
		case VariantType::VR_STD_VECTOR_LONGLONG:
			get< Vector<long long> >() = std::move(in.get< Vector<long long> >());
			break;
		case VariantType::VR_STD_VECTOR_UCHAR:
			get< Vector<unsigned char> >() = std::move(in.get< Vector<unsigned char> >());
			break;
		case VariantType::VR_STD_VECTOR_USHORT:
			get< Vector<unsigned short> >() = std::move(in.get< Vector<unsigned short> >());
			break;
		case VariantType::VR_STD_VECTOR_UINT:
			get< Vector<unsigned int> >() = std::move(in.get< Vector<unsigned int> >());
			break;
		case VariantType::VR_STD_VECTOR_ULONG:
			get< Vector<unsigned long> >() = std::move(in.get< Vector<unsigned long> >());
			break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:
			get< Vector<unsigned long long> >() = std::move(in.get< Vector<unsigned long long> >());
			break;
		case VariantType::VR_STD_VECTOR_FLOAT:
			get< Vector<float> >() = std::move(in.get< Vector<float> >());
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE:
			get< Vector<double> >() = std::move(in.get< Vector<double> >());
			break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:
			get< Vector<long double> >() = std::move(in.get< Vector<long double> >());
			break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:
			get< Vector<Mat4> >() = std::move(in.get< Vector<Mat4> >());
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:
			get< Vector<DMat4> >() = std::move(in.get< Vector<DMat4> >());
			break;

		case VariantType::VR_STD_VECTOR_VEC2:
			get< Vector<Vec2> >() = std::move(in.get< Vector<Vec2> >());
			break;
		case VariantType::VR_STD_VECTOR_VEC3:
			get< Vector<Vec3> >() = std::move(in.get< Vector<Vec3> >());
			break;
		case VariantType::VR_STD_VECTOR_VEC4:
			get< Vector<Vec4> >() = std::move(in.get< Vector<Vec4> >());
			break;

		case VariantType::VR_STD_VECTOR_IVEC2:
			get< Vector<IVec2> >() = std::move(in.get< Vector<IVec2> >());
			break;
		case VariantType::VR_STD_VECTOR_IVEC3:
			get< Vector<IVec3> >() = std::move(in.get< Vector<IVec3> >());
			break;
		case VariantType::VR_STD_VECTOR_IVEC4:
			get< Vector<IVec4> >() = std::move(in.get< Vector<IVec4> >());
			break;

		case VariantType::VR_STD_VECTOR_DVEC2:
			get< Vector<DVec2> >() = std::move(in.get< Vector<DVec2> >());
			break;
		case VariantType::VR_STD_VECTOR_DVEC3:
			get< Vector<DVec3> >() = std::move(in.get< Vector<DVec3> >());
			break;
		case VariantType::VR_STD_VECTOR_DVEC4:
			get< Vector<DVec4> >() = std::move(in.get < Vector<DVec4> >());
			break;

		case VariantType::VR_C_STRING:
			square_assert(0);
			break;
		case VariantType::VR_STD_STRING:
			get<String>() = std::move(in.get<String>());
			break;
		case VariantType::VR_STD_VECTOR_STRING:
			get< Vector<String> >() = std::move(in.get< Vector<String> >());
			break;
		case VariantType::VR_RESOURCE:
			get< Shared<ResourceObject> >() = std::move(in.get< Shared<ResourceObject> >());
			break;
			//copy stack
		default:
			std::memcpy(this, &in, sizeof(Variant));
			break;
		}
		//reinit in
		std::memset(&in, 0, sizeof(Variant));
		in.m_type = VariantType::VR_NONE;
	}

	//copy from other variant
	void Variant::copy_from(const Variant& in)
	{
		//alloc
		set_type(in.get_type());
		//copy
		switch (m_type)
		{
			//copy from heap
		case VariantType::VR_FLOAT_MATRIX:			   get<Mat4>() = (const  Mat4&)in; break;
		case VariantType::VR_DOUBLE_MATRIX:			   get<DMat4>() = (const  DMat4&)in; break;

		case VariantType::VR_STD_VECTOR_CHAR:
			get< Vector<char> >() = (const Vector<char>&)in;
			break;
		case VariantType::VR_STD_VECTOR_SHORT:
			get< Vector<short> >() = (const Vector<short>&)in;
			break;
		case VariantType::VR_STD_VECTOR_INT:
			get< Vector<int> >() = (const Vector<int>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONG:
			get< Vector<long> >() = (const Vector<long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONGLONG:
			get< Vector<long long> >() = (const Vector<long long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_UCHAR:
			get< Vector<unsigned char> >() = (const Vector<unsigned char>&)in;
			break;
		case VariantType::VR_STD_VECTOR_USHORT:
			get< Vector<unsigned short> >() = (const Vector<unsigned short>&)in;
			break;
		case VariantType::VR_STD_VECTOR_UINT:
			get< Vector<unsigned int> >() = (const Vector<unsigned int>&)in;
			break;
		case VariantType::VR_STD_VECTOR_ULONG:
			get< Vector<unsigned long> >() = (const Vector<unsigned long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:
			get< Vector<unsigned long long> >() = (const Vector<unsigned long long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_FLOAT:
			get< Vector<float> >() = (const Vector<float>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE:
			get< Vector<double> >() = (const Vector<double>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:
			get< Vector<long double> >() = (const Vector<long double>&)in;
			break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:
			get< Vector<Mat4> >() = (const Vector<Mat4>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:
			get< Vector<DMat4> >() = (const Vector<DMat4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_VEC2:
			get< Vector<Vec2> >() = (const Vector<Vec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_VEC3:
			get< Vector<Vec3> >() = (const Vector<Vec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_VEC4:
			get< Vector<Vec4> >() = (const Vector<Vec4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_IVEC2:
			get< Vector<IVec2> >() = (const Vector<IVec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_IVEC3:
			get< Vector<IVec3> >() = (const Vector<IVec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_IVEC4:
			get< Vector<IVec4> >() = (const Vector<IVec4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_DVEC2:
			get< Vector<DVec2> >() = (const Vector<DVec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DVEC3:
			get< Vector<DVec3> >() = (const Vector<DVec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DVEC4:
			get< Vector<DVec4> >() = (const Vector<DVec4>&)in;
			break;

		case VariantType::VR_C_STRING:
			square_assert(0);
			break;
		case VariantType::VR_STD_STRING:
			get<String>() = (const String&)in;
			break;
		case VariantType::VR_STD_VECTOR_STRING:
			get< Vector<String> >() = (const Vector<String>&)in;
			break;

		case VariantType::VR_RESOURCE:
			get< Shared<ResourceObject> >() = in.get< Shared<ResourceObject> >();
			break;
			//copy stack
		default:
			std::memcpy(this, &in, sizeof(Variant));
			break;
		}
	}

	//set type
	void Variant::set_type(VariantType type)
		{
			//dealloc
			switch (m_type)
			{
			case VariantType::VR_FLOAT_MATRIX:			   delete (Mat4*)(m_ptr);  break;
			case VariantType::VR_DOUBLE_MATRIX:			   delete (DMat4*)(m_ptr);  break;

			case VariantType::VR_STD_VECTOR_CHAR:    delete (Vector<char>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_SHORT:    delete (Vector<short>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_INT:    delete (Vector<int>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_LONG:    delete (Vector<long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_LONGLONG:    delete (Vector<long long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_UCHAR:    delete (Vector<unsigned char>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_USHORT:    delete (Vector<unsigned short>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_UINT:    delete (Vector<unsigned int>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_ULONG:    delete (Vector<unsigned long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_ULONGLONG:    delete (Vector<unsigned long long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_FLOAT:  delete (Vector<float>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_DOUBLE:  delete (Vector<double>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE:  delete (Vector<long double>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:        delete (Vector<Mat4>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:        delete (Vector<DMat4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_VEC2:   delete (Vector<Vec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_VEC3:   delete (Vector<Vec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_VEC4:   delete (Vector<Vec4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_IVEC2:   delete (Vector<IVec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_IVEC3:   delete (Vector<IVec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_IVEC4:   delete (Vector<IVec4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_DVEC2:   delete (Vector<DVec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DVEC3:   delete (Vector<DVec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DVEC4:   delete (Vector<DVec4>*)m_ptr;   break;

			case VariantType::VR_C_STRING:          square_assert(0);                        break;
			case VariantType::VR_STD_STRING:        delete (String*)m_ptr;			     break;
			case VariantType::VR_STD_VECTOR_STRING: delete (Vector<String>*)m_ptr; break;
			case VariantType::VR_RESOURCE:          delete (Shared<ResourceObject>*)m_ptr;   break;
			default: break;
			}
			//change type
			m_type = type;
			//alloc
			switch (m_type)
			{
			case VariantType::VR_FLOAT_MATRIX:			   m_ptr = new Mat4; break;
			case VariantType::VR_DOUBLE_MATRIX:			   m_ptr = new DMat4; break;

			case VariantType::VR_STD_VECTOR_CHAR:            m_ptr = new Vector<char>;      break;
			case VariantType::VR_STD_VECTOR_SHORT:           m_ptr = new Vector<short>;      break;
			case VariantType::VR_STD_VECTOR_INT:             m_ptr = new Vector<int>;	     break;
			case VariantType::VR_STD_VECTOR_LONG:            m_ptr = new Vector<long>;       break;
			case VariantType::VR_STD_VECTOR_LONGLONG:        m_ptr = new Vector<long long>;	 break;
			case VariantType::VR_STD_VECTOR_UCHAR:           m_ptr = new Vector<unsigned char>;     break;
			case VariantType::VR_STD_VECTOR_USHORT:          m_ptr = new Vector<unsigned short>;     break;
			case VariantType::VR_STD_VECTOR_UINT:            m_ptr = new Vector<unsigned int>;	     break;
			case VariantType::VR_STD_VECTOR_ULONG:           m_ptr = new Vector<unsigned long>;      break;
			case VariantType::VR_STD_VECTOR_ULONGLONG:       m_ptr = new Vector<unsigned long long>; break;
			case VariantType::VR_STD_VECTOR_FLOAT:           m_ptr = new Vector<float>;	     break;
			case VariantType::VR_STD_VECTOR_DOUBLE:          m_ptr = new Vector<double>;     break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE:     m_ptr = new Vector<long double>; break;
			case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:    m_ptr = new Vector<Mat4>;    break;
			case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:   m_ptr = new Vector<DMat4>;	 break;

			case VariantType::VR_STD_VECTOR_VEC2: m_ptr = new Vector<Vec2>; break;
			case VariantType::VR_STD_VECTOR_VEC3: m_ptr = new Vector<Vec3>; break;
			case VariantType::VR_STD_VECTOR_VEC4: m_ptr = new Vector<Vec4>; break;

			case VariantType::VR_STD_VECTOR_IVEC2: m_ptr = new Vector<IVec2>; break;
			case VariantType::VR_STD_VECTOR_IVEC3: m_ptr = new Vector<IVec3>; break;
			case VariantType::VR_STD_VECTOR_IVEC4: m_ptr = new Vector<IVec4>; break;

			case VariantType::VR_STD_VECTOR_DVEC2: m_ptr = new Vector<DVec2>; break;
			case VariantType::VR_STD_VECTOR_DVEC3: m_ptr = new Vector<DVec3>; break;
			case VariantType::VR_STD_VECTOR_DVEC4: m_ptr = new Vector<DVec4>; break;

			case VariantType::VR_C_STRING:          square_assert(0);                      break;
			case VariantType::VR_STD_STRING:        m_ptr = new String();			   break;
			case VariantType::VR_STD_VECTOR_STRING: m_ptr = new Vector<String>(); break;
			case VariantType::VR_RESOURCE:          m_ptr = new Shared<ResourceObject>();  break;
			default: break;
			}
		}
	
	//to variant ref
	VariantRef Variant::as_variant_ref() const
	{
		if (is_heap_value())
		{
			return VariantRef(m_type, m_ptr);
		}
		else
		{
			return VariantRef(m_type, (void*)&m_ptr);
		}
	}

	Variant::Variant(const VariantRef& ref)
	{
		switch (ref.get_type())
		{
		case VariantType::VR_NONE:     (*this) = Variant();        break;
		case VariantType::VR_BOOL:     (*this) = ref.get<bool>();  break;
		case VariantType::VR_CHAR:     (*this) = ref.get<char>();  break;
		case VariantType::VR_SHORT:    (*this) = ref.get<short>(); break;
		case VariantType::VR_INT:      (*this) = ref.get<int>();   break;
		case VariantType::VR_LONG:     (*this) = ref.get<long>();  break;
		case VariantType::VR_LONGLONG: (*this) = ref.get<long long>(); break;
		case VariantType::VR_UCHAR:    (*this) = ref.get<unsigned char>(); break;
		case VariantType::VR_USHORT:   (*this) = ref.get<unsigned short>(); break;
		case VariantType::VR_UINT:     (*this) = ref.get<unsigned int>(); break;
		case VariantType::VR_ULONG:    (*this) = ref.get<unsigned long>(); break;
		case VariantType::VR_ULONGLONG:(*this) = ref.get<unsigned long long>(); break;
		case VariantType::VR_FLOAT:    (*this) = ref.get<float>(); break;
		case VariantType::VR_DOUBLE:   (*this) = ref.get<double>();  break;

		case VariantType::VR_VEC2:   (*this) = ref.get<Vec2>();  break;
		case VariantType::VR_VEC3:   (*this) = ref.get<Vec3>();  break;
		case VariantType::VR_VEC4:   (*this) = ref.get<Vec4>();  break;

		case VariantType::VR_IVEC2:   (*this) = ref.get<IVec2>();  break;
		case VariantType::VR_IVEC3:   (*this) = ref.get<IVec3>();  break;
		case VariantType::VR_IVEC4:   (*this) = ref.get<IVec4>();  break;

		case VariantType::VR_DVEC2:   (*this) = ref.get<DVec2>();  break;
		case VariantType::VR_DVEC3:   (*this) = ref.get<DVec3>();  break;
		case VariantType::VR_DVEC4:   (*this) = ref.get<DVec4>();  break;

		case VariantType::VR_QUAT:    (*this) = ref.get<Quat>();  break;
		case VariantType::VR_DQUAT:   (*this) = ref.get<DQuat>();  break;

		case VariantType::VR_FLOAT_MATRIX:      (*this) = ref.get<Mat4>(); break;
		case VariantType::VR_DOUBLE_MATRIX:     (*this) = ref.get<DMat4>(); break;

		case VariantType::VR_STD_VECTOR_CHAR:              (*this) = ref.get< Vector<char> >(); break;
		case VariantType::VR_STD_VECTOR_SHORT:             (*this) = ref.get< Vector<short> >(); break;
		case VariantType::VR_STD_VECTOR_INT:               (*this) = ref.get< Vector<int> >(); break;
		case VariantType::VR_STD_VECTOR_LONG:              (*this) = ref.get< Vector<long> >(); break;
		case VariantType::VR_STD_VECTOR_LONGLONG:          (*this) = ref.get< Vector<long long> >(); break;
		case VariantType::VR_STD_VECTOR_UCHAR:             (*this) = ref.get< Vector<unsigned char> >(); break;
		case VariantType::VR_STD_VECTOR_USHORT:            (*this) = ref.get< Vector<unsigned short> >(); break;
		case VariantType::VR_STD_VECTOR_UINT:              (*this) = ref.get< Vector<unsigned int> >(); break;
		case VariantType::VR_STD_VECTOR_ULONG:             (*this) = ref.get< Vector<unsigned long> >(); break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:         (*this) = ref.get< Vector<unsigned long long> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT:             (*this) = ref.get< Vector<float> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE:            (*this) = ref.get< Vector<double> >(); break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       (*this) = ref.get< Vector<long double> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      (*this) = ref.get< Vector<Mat4> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     (*this) = ref.get< Vector<DMat4> >(); break;

		case VariantType::VR_STD_VECTOR_VEC2:(*this) = ref.get< Vector<Vec2> >(); break;
		case VariantType::VR_STD_VECTOR_VEC3:(*this) = ref.get< Vector<Vec3> >(); break;
		case VariantType::VR_STD_VECTOR_VEC4:(*this) = ref.get< Vector<Vec4> >(); break;

		case VariantType::VR_STD_VECTOR_IVEC2:(*this) = ref.get< Vector<IVec2> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC3:(*this) = ref.get< Vector<IVec3> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC4:(*this) = ref.get< Vector<IVec4> >(); break;

		case VariantType::VR_STD_VECTOR_DVEC2:(*this) = ref.get< Vector<DVec2> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC3:(*this) = ref.get< Vector<DVec3> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC4:(*this) = ref.get< Vector<DVec4> >(); break;

		case VariantType::VR_C_STRING:
		case VariantType::VR_STD_STRING:           (*this) = ref.get< String >(); break;
		case VariantType::VR_STD_VECTOR_STRING:    (*this) = ref.get< Vector<String> >(); break;

		case VariantType::VR_RESOURCE: (*this).get< Shared<ResourceObject> >() = ref.get< Shared<ResourceObject> >(); break;

		case VariantType::VR_PTR: (*this) = (void*)ref.get_ptr<void>(); break;
		default: break;
		}
	}
#pragma endregion

#pragma region VariantRef
	VariantRef::VariantRef()
	{
		m_type = VariantType::VR_NONE;
	}

	VariantRef::VariantRef(VariantType type, void* ptr)
	{
		m_type = type;
		m_ptr = ptr;
	}

	VariantRef::VariantRef(const bool& b)
	{
		m_ptr = (void*)&b;
		m_type = VariantType::VR_BOOL;
	}

	VariantRef::VariantRef(const char& c)
	{
		m_ptr = (void*)&c;
		m_type = VariantType::VR_CHAR;
	}

	VariantRef::VariantRef(const short& s)
	{
		m_ptr = (void*)&s;
		m_type = VariantType::VR_SHORT;
	}

	VariantRef::VariantRef(const int& i)
	{
		m_ptr = (void*)&i;
		m_type = VariantType::VR_INT;
	}

	VariantRef::VariantRef(const long& l)
	{
		m_ptr = (void*)&l;
		m_type = VariantType::VR_LONG;
	}

	VariantRef::VariantRef(const long long& l)
	{
		m_ptr = (void*)&l;
		m_type = VariantType::VR_LONGLONG;
	}

	VariantRef::VariantRef(const unsigned char& uc)
	{
		m_ptr = (void*)&uc;
		m_type = VariantType::VR_UCHAR;
	}

	VariantRef::VariantRef(const unsigned short& us)
	{
		m_ptr = (void*)&us;
		m_type = VariantType::VR_USHORT;
	}

	VariantRef::VariantRef(const unsigned int& ui)
	{
		m_ptr = (void*)&ui;
		m_type = VariantType::VR_UINT;
	}

	VariantRef::VariantRef(const unsigned long& ul)
	{
		m_ptr = (void*)&ul;
		m_type = VariantType::VR_ULONG;
	}

	VariantRef::VariantRef(const unsigned long long& ull)
	{
		m_ptr = (void*)&ull;
		m_type = VariantType::VR_ULONGLONG;
	}

	VariantRef::VariantRef(const float& f)
	{
		m_ptr = (void*)&f;
		m_type = VariantType::VR_FLOAT;
	}

	VariantRef::VariantRef(const double& d)
	{
		m_ptr = (void*)&d;
		m_type = VariantType::VR_DOUBLE;
	}

	VariantRef::VariantRef(const long double& ld)
	{
		m_ptr = (void*)&ld;
		m_type = VariantType::VR_LONG_DOUBLE;
	}

	VariantRef::VariantRef(const Vec2 & v2)
	{
		m_ptr = (void*)&v2;
		m_type = VariantType::VR_VEC2;
	}

	VariantRef::VariantRef(const Vec3 & v3)
	{
		m_ptr = (void*)&v3;
		m_type = VariantType::VR_VEC3;
	}

	VariantRef::VariantRef(const Vec4 & v4)
	{
		m_ptr = (void*)&v4;
		m_type = VariantType::VR_VEC4;
	}

	VariantRef::VariantRef(const IVec2 & iv2)
	{
		m_ptr = (void*)&iv2;
		m_type = VariantType::VR_IVEC2;
	}

	VariantRef::VariantRef(const IVec3 & iv3)
	{
		m_ptr = (void*)&iv3;
		m_type = VariantType::VR_IVEC3;
	}

	VariantRef::VariantRef(const IVec4 & iv4)
	{
		m_ptr = (void*)&iv4;
		m_type = VariantType::VR_IVEC4;
	}

	VariantRef::VariantRef(const DVec2 & dv2)
	{
		m_ptr = (void*)&dv2;
		m_type = VariantType::VR_DVEC2;
	}

	VariantRef::VariantRef(const DVec3 & dv3)
	{
		m_ptr = (void*)&dv3;
		m_type = VariantType::VR_DVEC3;
	}

	VariantRef::VariantRef(const DVec4 & dv4)
	{
		m_ptr = (void*)&dv4;
		m_type = VariantType::VR_DVEC4;
	}

	VariantRef::VariantRef(const Quat & quat)
	{
		m_ptr = (void*)&quat;
		m_type = VariantType::VR_QUAT;
	}

	VariantRef::VariantRef(const DQuat & dquat)
	{
		m_ptr = (void*)&dquat;
		m_type = VariantType::VR_DQUAT;
	}

	VariantRef::VariantRef(const Mat4 & fm)
	{
		m_ptr = (void*)&fm;
		m_type = VariantType::VR_FLOAT_MATRIX;
	}

	VariantRef::VariantRef(const DMat4 & dm)
	{
		m_ptr = (void*)&dm;
		m_type = VariantType::VR_DOUBLE_MATRIX;
	}

	VariantRef::VariantRef(const Vector< char >&v_c)
	{
		m_ptr = (void*)&v_c;
		m_type = VariantType::VR_STD_VECTOR_CHAR;
	}

	VariantRef::VariantRef(const Vector< short > &v_s)
	{
		m_ptr = (void*)&v_s;
		m_type = VariantType::VR_STD_VECTOR_SHORT;
	}

	VariantRef::VariantRef(const Vector< int > &v_i)
	{
		m_ptr = (void*)&v_i;
		m_type = VariantType::VR_STD_VECTOR_INT;
	}

	VariantRef::VariantRef(const Vector< long > &v_l)
	{
		m_ptr = (void*)&v_l;
		m_type = VariantType::VR_STD_VECTOR_LONG;
	}

	VariantRef::VariantRef(const Vector< long long > &v_ll)
	{
		m_ptr = (void*)&v_ll;
		m_type = VariantType::VR_STD_VECTOR_LONGLONG;
	}

	VariantRef::VariantRef(const Vector< unsigned char >&v_uc)
	{
		m_ptr = (void*)&v_uc;
		m_type = VariantType::VR_STD_VECTOR_UCHAR;
	}

	VariantRef::VariantRef(const Vector< unsigned short >&v_us)
	{
		m_ptr = (void*)&v_us;
		m_type = VariantType::VR_STD_VECTOR_USHORT;
	}

	VariantRef::VariantRef(const Vector< unsigned int > &v_ui)
	{
		m_ptr = (void*)&v_ui;
		m_type = VariantType::VR_STD_VECTOR_UINT;
	}

	VariantRef::VariantRef(const Vector< unsigned long > &v_ul)
	{
		m_ptr = (void*)&v_ul;
		m_type = VariantType::VR_STD_VECTOR_ULONG;
	}

	VariantRef::VariantRef(const Vector< unsigned long long > &v_ull)
	{
		m_ptr = (void*)&v_ull;
		m_type = VariantType::VR_STD_VECTOR_ULONGLONG;
	}

	VariantRef::VariantRef(const Vector< float > &v_f)
	{
		m_ptr = (void*)&v_f;
		m_type = VariantType::VR_STD_VECTOR_FLOAT;
	}

	VariantRef::VariantRef(const Vector< double > &v_d)
	{
		m_ptr = (void*)&v_d;
		m_type = VariantType::VR_STD_VECTOR_DOUBLE;
	}

	VariantRef::VariantRef(const Vector< long double > &v_ld)
	{
		m_ptr = (void*)&v_ld;
		m_type = VariantType::VR_STD_VECTOR_LONG_DOUBLE;
	}

	VariantRef::VariantRef(const Vector< Mat4 > &v_fm)
	{
		m_ptr = (void*)&v_fm;
		m_type = VariantType::VR_STD_VECTOR_FLOAT_MATRIX;
	}

	VariantRef::VariantRef(const Vector< DMat4 > &v_dm)
	{
		m_ptr = (void*)&v_dm;
		m_type = VariantType::VR_STD_VECTOR_DOUBLE_MATRIX;
	}

	VariantRef::VariantRef(const Vector< Vec2 > &v_v2)
	{
		m_ptr = (void*)&v_v2;
		m_type = VariantType::VR_STD_VECTOR_VEC2;
	}

	VariantRef::VariantRef(const Vector< Vec3 > &v_v3)
	{
		m_ptr = (void*)&v_v3;
		m_type = VariantType::VR_STD_VECTOR_VEC3;
	}

	VariantRef::VariantRef(const Vector< Vec4 > &v_v4)
	{
		m_ptr = (void*)&v_v4;
		m_type = VariantType::VR_STD_VECTOR_VEC4;
	}

	VariantRef::VariantRef(const Vector< IVec2 > &v_iv2)
	{
		m_ptr = (void*)&v_iv2;
		m_type = VariantType::VR_STD_VECTOR_IVEC2;
	}

	VariantRef::VariantRef(const Vector< IVec3 > &v_iv3)
	{
		m_ptr = (void*)&v_iv3;
		m_type = VariantType::VR_STD_VECTOR_IVEC3;
	}

	VariantRef::VariantRef(const Vector< IVec4 > &v_iv4)
	{
		m_ptr = (void*)&v_iv4;
		m_type = VariantType::VR_STD_VECTOR_IVEC4;
	}

	VariantRef::VariantRef(const Vector< DVec2 > &v_dv2)
	{
		m_ptr = (void*)&v_dv2;
		m_type = VariantType::VR_STD_VECTOR_DVEC2;
	}

	VariantRef::VariantRef(const Vector< DVec3 > &v_dv3)
	{
		m_ptr = (void*)&v_dv3;
		m_type = VariantType::VR_STD_VECTOR_DVEC3;
	}

	VariantRef::VariantRef(const Vector< DVec4 > &v_dv4)
	{
		m_ptr = (void*)&v_dv4;
		m_type = VariantType::VR_STD_VECTOR_DVEC4;
	}

	VariantRef::VariantRef(const char* c_str)
	{
		m_ptr = (void*)c_str;
		m_type = VariantType::VR_C_STRING;
	}

	VariantRef::VariantRef(const String & str)
	{
		m_ptr = (void*)&str;
		m_type = VariantType::VR_STD_STRING;
	}

	VariantRef::VariantRef(const Vector< String >&v_str)
	{
		m_ptr = (void*)&v_str;
		m_type = VariantType::VR_STD_VECTOR_STRING;
	}

	VariantRef::VariantRef(const void* ptr)
	{
		m_ptr = (void*)ptr;
		m_type = VariantType::VR_PTR;
	}

	VariantRef::VariantRef(const Variant & var)
	{
		m_ptr = (void*)var.get_ref();
		m_type = var.get_type();
	}

	VariantType VariantRef::get_type() const
	{
		return m_type;
	}

	void VariantRef::copy_from(const VariantRef & ref)
	{
		switch (ref.get_type())
		{
			//case VariantType::VR_NONE:     (*this) = Variant();        break;
		case VariantType::VR_BOOL:     ((bool&)(*this)) = ref.get<bool>();  break;
		case VariantType::VR_CHAR:     ((char&)(*this)) = ref.get<char>();  break;
		case VariantType::VR_SHORT:    ((short&)(*this)) = ref.get<short>(); break;
		case VariantType::VR_INT:      ((int&)(*this)) = ref.get<int>();   break;
		case VariantType::VR_LONG:     ((long&)(*this)) = ref.get<long>();  break;
		case VariantType::VR_LONGLONG: ((long long&)(*this)) = ref.get<long long>(); break;
		case VariantType::VR_UCHAR:    ((unsigned char&)(*this)) = ref.get<unsigned char>(); break;
		case VariantType::VR_USHORT:   ((unsigned short&)(*this)) = ref.get<unsigned short>(); break;
		case VariantType::VR_UINT:     ((unsigned int&)(*this)) = ref.get<unsigned int>(); break;
		case VariantType::VR_ULONG:    ((unsigned long&)(*this)) = ref.get<unsigned long>(); break;
		case VariantType::VR_ULONGLONG:((unsigned long long&)(*this)) = ref.get<unsigned long long>(); break;
		case VariantType::VR_FLOAT:    ((float&)(*this)) = ref.get<float>(); break;
		case VariantType::VR_DOUBLE:   ((double&)(*this)) = ref.get<double>();  break;

		case VariantType::VR_VEC2:   ((Vec2&)(*this)) = ref.get<Vec2>();  break;
		case VariantType::VR_VEC3:   ((Vec3&)(*this)) = ref.get<Vec3>();  break;
		case VariantType::VR_VEC4:   ((Vec4&)(*this)) = ref.get<Vec4>();  break;

		case VariantType::VR_IVEC2:   ((IVec2&)(*this)) = ref.get<IVec2>();  break;
		case VariantType::VR_IVEC3:   ((IVec3&)(*this)) = ref.get<IVec3>();  break;
		case VariantType::VR_IVEC4:   ((IVec4&)(*this)) = ref.get<IVec4>();  break;

		case VariantType::VR_DVEC2:   ((DVec2&)(*this)) = ref.get<DVec2>();  break;
		case VariantType::VR_DVEC3:   ((DVec3&)(*this)) = ref.get<DVec3>();  break;
		case VariantType::VR_DVEC4:   ((DVec4&)(*this)) = ref.get<DVec4>();  break;

		case VariantType::VR_QUAT:    ((Quat&)(*this)) = ref.get<Quat>();  break;
		case VariantType::VR_DQUAT:   ((DQuat&)(*this)) = ref.get<DQuat>();  break;

		case VariantType::VR_FLOAT_MATRIX:      ((Mat4&)(*this)) = ref.get<Mat4>(); break;
		case VariantType::VR_DOUBLE_MATRIX:     ((DMat4&)(*this)) = ref.get<DMat4>(); break;

		case VariantType::VR_STD_VECTOR_CHAR:              ((Vector<char>&)(*this)) = ref.get< Vector<char> >(); break;
		case VariantType::VR_STD_VECTOR_SHORT:             ((Vector<short>&)(*this)) = ref.get< Vector<short> >(); break;
		case VariantType::VR_STD_VECTOR_INT:               ((Vector<int>&)(*this)) = ref.get< Vector<int> >(); break;
		case VariantType::VR_STD_VECTOR_LONG:              ((Vector<long>&)(*this)) = ref.get< Vector<long> >(); break;
		case VariantType::VR_STD_VECTOR_LONGLONG:          ((Vector<long long>&)(*this)) = ref.get< Vector<long long> >(); break;
		case VariantType::VR_STD_VECTOR_UCHAR:             ((Vector<unsigned char>&)(*this)) = ref.get< Vector<unsigned char> >(); break;
		case VariantType::VR_STD_VECTOR_USHORT:            ((Vector<unsigned short>&)(*this)) = ref.get< Vector<unsigned short> >(); break;
		case VariantType::VR_STD_VECTOR_UINT:              ((Vector<unsigned int>&)(*this)) = ref.get< Vector<unsigned int> >(); break;
		case VariantType::VR_STD_VECTOR_ULONG:             ((Vector<unsigned long>&)(*this)) = ref.get< Vector<unsigned long> >(); break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:         ((Vector<unsigned long long>&)(*this)) = ref.get< Vector<unsigned long long> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT:             ((Vector<float>&)(*this)) = ref.get< Vector<float> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE:            ((Vector<double>&)(*this)) = ref.get< Vector<double> >(); break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       ((Vector<long double>&)(*this)) = ref.get< Vector<long double> >(); break;

		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      ((Vector<Mat4>&)(*this)) = ref.get< Vector<Mat4> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     ((Vector<DMat4>&)(*this)) = ref.get< Vector<DMat4> >(); break;

		case VariantType::VR_STD_VECTOR_VEC2:((Vector<Vec2>&)(*this)) = ref.get< Vector<Vec2> >(); break;
		case VariantType::VR_STD_VECTOR_VEC3:((Vector<Vec3>&)(*this)) = ref.get< Vector<Vec3> >(); break;
		case VariantType::VR_STD_VECTOR_VEC4:((Vector<Vec4>&)(*this)) = ref.get< Vector<Vec4> >(); break;

		case VariantType::VR_STD_VECTOR_IVEC2:((Vector<IVec2>&)(*this)) = ref.get< Vector<IVec2> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC3:((Vector<IVec3>&)(*this)) = ref.get< Vector<IVec3> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC4:((Vector<IVec4>&)(*this)) = ref.get< Vector<IVec4> >(); break;

		case VariantType::VR_STD_VECTOR_DVEC2:((Vector<DVec2>&)(*this)) = ref.get< Vector<DVec2> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC3:((Vector<DVec3>&)(*this)) = ref.get< Vector<DVec3> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC4:((Vector<DVec4>&)(*this)) = ref.get< Vector<DVec4> >(); break;

		case VariantType::VR_C_STRING:
		case VariantType::VR_STD_STRING:           ((String&)(*this)) = ref.get< String >(); break;
		case VariantType::VR_STD_VECTOR_STRING:    ((Vector<String>&)(*this)) = ref.get< Vector<String> >(); break;

		case VariantType::VR_RESOURCE: ((Shared<ResourceObject>&)(*this)) = ref.get< Shared<ResourceObject> >(); break;

		case VariantType::VR_PTR:                   m_ptr = (void*)ref.get_ptr<void>(); break;
		default: break;
		}
	}
#pragma endregion
}