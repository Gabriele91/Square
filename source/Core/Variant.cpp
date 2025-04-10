//
//  Square
//
//  Created by Gabriele on 09/07/176.
//  Copyright ? 2017 Gabriele. All rights reserved.
//
#pragma once
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

	Variant::Variant(const std::vector< char >& v_c)
	{
		set_type(VR_STD_VECTOR_CHAR);
		*((std::vector< char >*)(m_ptr)) = v_c;
	}

	Variant::Variant(const std::vector< short >& v_s)
	{
		set_type(VR_STD_VECTOR_SHORT);
		*((std::vector< short >*)(m_ptr)) = v_s;
	}

	Variant::Variant(const std::vector< int >& v_i)
	{
		set_type(VR_STD_VECTOR_INT);
		*((std::vector< int >*)(m_ptr)) = v_i;
	}

	Variant::Variant(const std::vector< long >& v_l)
	{
		set_type(VR_STD_VECTOR_LONG);
		*((std::vector< long >*)(m_ptr)) = v_l;
	}

	Variant::Variant(const std::vector< long long >& v_ll)
	{
		set_type(VR_STD_VECTOR_LONGLONG);
		*((std::vector< long long >*)(m_ptr)) = v_ll;
	}

	Variant::Variant(const std::vector< unsigned char >& v_uc)
	{
		set_type(VR_STD_VECTOR_UCHAR);
		*((std::vector< unsigned char >*)(m_ptr)) = v_uc;
	}

	Variant::Variant(const std::vector< unsigned short >& v_us)
	{
		set_type(VR_STD_VECTOR_USHORT);
		*((std::vector< unsigned short >*)(m_ptr)) = v_us;
	}

	Variant::Variant(const std::vector< unsigned int >& v_ui)
	{
		set_type(VR_STD_VECTOR_UINT);
		*((std::vector< unsigned int >*)(m_ptr)) = v_ui;
	}

	Variant::Variant(const std::vector< unsigned long >& v_ul)
	{
		set_type(VR_STD_VECTOR_ULONG);
		*((std::vector< unsigned long >*)(m_ptr)) = v_ul;
	}

	Variant::Variant(const std::vector< unsigned long long >& v_ull)
	{
		set_type(VR_STD_VECTOR_ULONGLONG);
		*((std::vector< unsigned long long >*)(m_ptr)) = v_ull;
	}

	Variant::Variant(const std::vector< float >& v_f)
	{
		set_type(VR_STD_VECTOR_FLOAT);
		*((std::vector< float >*)(m_ptr)) = v_f;
	}

	Variant::Variant(const std::vector< double >& v_d)
	{
		set_type(VR_STD_VECTOR_DOUBLE);
		*((std::vector< double >*)(m_ptr)) = v_d;
	}

	Variant::Variant(const std::vector< long double >& v_ld)
	{
		set_type(VR_STD_VECTOR_LONG_DOUBLE);
		*((std::vector< long double >*)(m_ptr)) = v_ld;
	}

	Variant::Variant(const std::vector< Mat4 >& v_m4)
	{
		set_type(VR_STD_VECTOR_FLOAT_MATRIX);
		*((std::vector< Mat4 >*)(m_ptr)) = v_m4;
	}

	Variant::Variant(const std::vector< DMat4 >& v_dm4)
	{
		set_type(VR_STD_VECTOR_DOUBLE_MATRIX);
		*((std::vector< DMat4 >*)(m_ptr)) = v_dm4;
	}

	Variant::Variant(const std::vector< Vec2 >& v_v2)
	{
		set_type(VR_STD_VECTOR_VEC2);
		*((std::vector< Vec2 >*)(m_ptr)) = v_v2;
	}

	Variant::Variant(const std::vector< Vec3 >& v_v3)
	{
		set_type(VR_STD_VECTOR_VEC3);
		*((std::vector< Vec3 >*)(m_ptr)) = v_v3;
	}

	Variant::Variant(const std::vector< Vec4 >& v_v4)
	{
		set_type(VR_STD_VECTOR_VEC4);
		*((std::vector< Vec4 >*)(m_ptr)) = v_v4;
	}

	Variant::Variant(const std::vector< IVec2 >& v_iv2)
	{
		set_type(VR_STD_VECTOR_IVEC2);
		*((std::vector< IVec2 >*)(m_ptr)) = v_iv2;
	}

	Variant::Variant(const std::vector< IVec3 >& v_iv3)
	{
		set_type(VR_STD_VECTOR_IVEC3);
		*((std::vector< IVec3 >*)(m_ptr)) = v_iv3;
	}

	Variant::Variant(const std::vector< IVec4 >& v_iv4)
	{
		set_type(VR_STD_VECTOR_IVEC4);
		*((std::vector< IVec4 >*)(m_ptr)) = v_iv4;
	}

	Variant::Variant(const std::vector< DVec2 >& v_dv2)
	{
		set_type(VR_STD_VECTOR_DVEC2);
		*((std::vector< DVec2 >*)(m_ptr)) = v_dv2;
	}

	Variant::Variant(const std::vector< DVec3 >& v_dv3)
	{
		set_type(VR_STD_VECTOR_DVEC3);
		*((std::vector< DVec3 >*)(m_ptr)) = v_dv3;
	}

	Variant::Variant(const std::vector< DVec4 >& v_dv4)
	{
		set_type(VR_STD_VECTOR_DVEC4);
		*((std::vector< DVec4 >*)(m_ptr)) = v_dv4;
	}

	Variant::Variant(const char* c_str)
	{
		set_type(VR_C_STRING);
		*((std::string*)(m_ptr)) = c_str;
	}

	Variant::Variant(const std::string& str)
	{
		set_type(VR_STD_STRING);
		*((std::string*)(m_ptr)) = str;
	}

	Variant::Variant(const std::vector< std::string >& v_str)
	{
		set_type(VR_STD_VECTOR_STRING);
		*((std::vector< std::string >*)(m_ptr)) = v_str;
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
			get< std::vector<char> >() = std::move(in.get< std::vector<char> >());
			break;
		case VariantType::VR_STD_VECTOR_SHORT:
			get< std::vector<short> >() = std::move(in.get< std::vector<short> >());
			break;
		case VariantType::VR_STD_VECTOR_INT:
			get< std::vector<int> >() = std::move(in.get< std::vector<int> >());
			break;
		case VariantType::VR_STD_VECTOR_LONG:
			get< std::vector<long> >() = std::move(in.get< std::vector<long> >());
			break;
		case VariantType::VR_STD_VECTOR_LONGLONG:
			get< std::vector<long long> >() = std::move(in.get< std::vector<long long> >());
			break;
		case VariantType::VR_STD_VECTOR_UCHAR:
			get< std::vector<unsigned char> >() = std::move(in.get< std::vector<unsigned char> >());
			break;
		case VariantType::VR_STD_VECTOR_USHORT:
			get< std::vector<unsigned short> >() = std::move(in.get< std::vector<unsigned short> >());
			break;
		case VariantType::VR_STD_VECTOR_UINT:
			get< std::vector<unsigned int> >() = std::move(in.get< std::vector<unsigned int> >());
			break;
		case VariantType::VR_STD_VECTOR_ULONG:
			get< std::vector<unsigned long> >() = std::move(in.get< std::vector<unsigned long> >());
			break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:
			get< std::vector<unsigned long long> >() = std::move(in.get< std::vector<unsigned long long> >());
			break;
		case VariantType::VR_STD_VECTOR_FLOAT:
			get< std::vector<float> >() = std::move(in.get< std::vector<float> >());
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE:
			get< std::vector<double> >() = std::move(in.get< std::vector<double> >());
			break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:
			get< std::vector<long double> >() = std::move(in.get< std::vector<long double> >());
			break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:
			get< std::vector<Mat4> >() = std::move(in.get< std::vector<Mat4> >());
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:
			get< std::vector<DMat4> >() = std::move(in.get< std::vector<DMat4> >());
			break;

		case VariantType::VR_STD_VECTOR_VEC2:
			get< std::vector<Vec2> >() = std::move(in.get< std::vector<Vec2> >());
			break;
		case VariantType::VR_STD_VECTOR_VEC3:
			get< std::vector<Vec3> >() = std::move(in.get< std::vector<Vec3> >());
			break;
		case VariantType::VR_STD_VECTOR_VEC4:
			get< std::vector<Vec4> >() = std::move(in.get< std::vector<Vec4> >());
			break;

		case VariantType::VR_STD_VECTOR_IVEC2:
			get< std::vector<IVec2> >() = std::move(in.get< std::vector<IVec2> >());
			break;
		case VariantType::VR_STD_VECTOR_IVEC3:
			get< std::vector<IVec3> >() = std::move(in.get< std::vector<IVec3> >());
			break;
		case VariantType::VR_STD_VECTOR_IVEC4:
			get< std::vector<IVec4> >() = std::move(in.get< std::vector<IVec4> >());
			break;

		case VariantType::VR_STD_VECTOR_DVEC2:
			get< std::vector<DVec2> >() = std::move(in.get< std::vector<DVec2> >());
			break;
		case VariantType::VR_STD_VECTOR_DVEC3:
			get< std::vector<DVec3> >() = std::move(in.get< std::vector<DVec3> >());
			break;
		case VariantType::VR_STD_VECTOR_DVEC4:
			get< std::vector<DVec4> >() = std::move(in.get < std::vector<DVec4> >());
			break;

		case VariantType::VR_C_STRING:
			square_assert(0);
			break;
		case VariantType::VR_STD_STRING:
			get<std::string>() = std::move(in.get<std::string>());
			break;
		case VariantType::VR_STD_VECTOR_STRING:
			get< std::vector<std::string> >() = std::move(in.get< std::vector<std::string> >());
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
			get< std::vector<char> >() = (const std::vector<char>&)in;
			break;
		case VariantType::VR_STD_VECTOR_SHORT:
			get< std::vector<short> >() = (const std::vector<short>&)in;
			break;
		case VariantType::VR_STD_VECTOR_INT:
			get< std::vector<int> >() = (const std::vector<int>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONG:
			get< std::vector<long> >() = (const std::vector<long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONGLONG:
			get< std::vector<long long> >() = (const std::vector<long long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_UCHAR:
			get< std::vector<unsigned char> >() = (const std::vector<unsigned char>&)in;
			break;
		case VariantType::VR_STD_VECTOR_USHORT:
			get< std::vector<unsigned short> >() = (const std::vector<unsigned short>&)in;
			break;
		case VariantType::VR_STD_VECTOR_UINT:
			get< std::vector<unsigned int> >() = (const std::vector<unsigned int>&)in;
			break;
		case VariantType::VR_STD_VECTOR_ULONG:
			get< std::vector<unsigned long> >() = (const std::vector<unsigned long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:
			get< std::vector<unsigned long long> >() = (const std::vector<unsigned long long>&)in;
			break;
		case VariantType::VR_STD_VECTOR_FLOAT:
			get< std::vector<float> >() = (const std::vector<float>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE:
			get< std::vector<double> >() = (const std::vector<double>&)in;
			break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:
			get< std::vector<long double> >() = (const std::vector<long double>&)in;
			break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:
			get< std::vector<Mat4> >() = (const std::vector<Mat4>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:
			get< std::vector<DMat4> >() = (const std::vector<DMat4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_VEC2:
			get< std::vector<Vec2> >() = (const std::vector<Vec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_VEC3:
			get< std::vector<Vec3> >() = (const std::vector<Vec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_VEC4:
			get< std::vector<Vec4> >() = (const std::vector<Vec4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_IVEC2:
			get< std::vector<IVec2> >() = (const std::vector<IVec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_IVEC3:
			get< std::vector<IVec3> >() = (const std::vector<IVec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_IVEC4:
			get< std::vector<IVec4> >() = (const std::vector<IVec4>&)in;
			break;

		case VariantType::VR_STD_VECTOR_DVEC2:
			get< std::vector<DVec2> >() = (const std::vector<DVec2>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DVEC3:
			get< std::vector<DVec3> >() = (const std::vector<DVec3>&)in;
			break;
		case VariantType::VR_STD_VECTOR_DVEC4:
			get< std::vector<DVec4> >() = (const std::vector<DVec4>&)in;
			break;

		case VariantType::VR_C_STRING:
			square_assert(0);
			break;
		case VariantType::VR_STD_STRING:
			get<std::string>() = (const std::string&)in;
			break;
		case VariantType::VR_STD_VECTOR_STRING:
			get< std::vector<std::string> >() = (const std::vector<std::string>&)in;
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

			case VariantType::VR_STD_VECTOR_CHAR:    delete (std::vector<char>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_SHORT:    delete (std::vector<short>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_INT:    delete (std::vector<int>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_LONG:    delete (std::vector<long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_LONGLONG:    delete (std::vector<long long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_UCHAR:    delete (std::vector<unsigned char>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_USHORT:    delete (std::vector<unsigned short>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_UINT:    delete (std::vector<unsigned int>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_ULONG:    delete (std::vector<unsigned long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_ULONGLONG:    delete (std::vector<unsigned long long>*)m_ptr;    break;
			case VariantType::VR_STD_VECTOR_FLOAT:  delete (std::vector<float>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_DOUBLE:  delete (std::vector<double>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE:  delete (std::vector<long double>*)m_ptr;  break;
			case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:        delete (std::vector<Mat4>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:        delete (std::vector<DMat4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_VEC2:   delete (std::vector<Vec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_VEC3:   delete (std::vector<Vec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_VEC4:   delete (std::vector<Vec4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_IVEC2:   delete (std::vector<IVec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_IVEC3:   delete (std::vector<IVec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_IVEC4:   delete (std::vector<IVec4>*)m_ptr;   break;

			case VariantType::VR_STD_VECTOR_DVEC2:   delete (std::vector<DVec2>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DVEC3:   delete (std::vector<DVec3>*)m_ptr;   break;
			case VariantType::VR_STD_VECTOR_DVEC4:   delete (std::vector<DVec4>*)m_ptr;   break;

			case VariantType::VR_C_STRING:          square_assert(0);                        break;
			case VariantType::VR_STD_STRING:        delete (std::string*)m_ptr;			     break;
			case VariantType::VR_STD_VECTOR_STRING: delete (std::vector<std::string>*)m_ptr; break;
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

			case VariantType::VR_STD_VECTOR_CHAR:            m_ptr = new std::vector<char>;      break;
			case VariantType::VR_STD_VECTOR_SHORT:           m_ptr = new std::vector<short>;      break;
			case VariantType::VR_STD_VECTOR_INT:             m_ptr = new std::vector<int>;	     break;
			case VariantType::VR_STD_VECTOR_LONG:            m_ptr = new std::vector<long>;       break;
			case VariantType::VR_STD_VECTOR_LONGLONG:        m_ptr = new std::vector<long long>;	 break;
			case VariantType::VR_STD_VECTOR_UCHAR:           m_ptr = new std::vector<unsigned char>;     break;
			case VariantType::VR_STD_VECTOR_USHORT:          m_ptr = new std::vector<unsigned short>;     break;
			case VariantType::VR_STD_VECTOR_UINT:            m_ptr = new std::vector<unsigned int>;	     break;
			case VariantType::VR_STD_VECTOR_ULONG:           m_ptr = new std::vector<unsigned long>;      break;
			case VariantType::VR_STD_VECTOR_ULONGLONG:       m_ptr = new std::vector<unsigned long long>; break;
			case VariantType::VR_STD_VECTOR_FLOAT:           m_ptr = new std::vector<float>;	     break;
			case VariantType::VR_STD_VECTOR_DOUBLE:          m_ptr = new std::vector<double>;     break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE:     m_ptr = new std::vector<long double>; break;
			case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:    m_ptr = new std::vector<Mat4>;    break;
			case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:   m_ptr = new std::vector<DMat4>;	 break;

			case VariantType::VR_STD_VECTOR_VEC2: m_ptr = new std::vector<Vec2>; break;
			case VariantType::VR_STD_VECTOR_VEC3: m_ptr = new std::vector<Vec3>; break;
			case VariantType::VR_STD_VECTOR_VEC4: m_ptr = new std::vector<Vec4>; break;

			case VariantType::VR_STD_VECTOR_IVEC2: m_ptr = new std::vector<IVec2>; break;
			case VariantType::VR_STD_VECTOR_IVEC3: m_ptr = new std::vector<IVec3>; break;
			case VariantType::VR_STD_VECTOR_IVEC4: m_ptr = new std::vector<IVec4>; break;

			case VariantType::VR_STD_VECTOR_DVEC2: m_ptr = new std::vector<DVec2>; break;
			case VariantType::VR_STD_VECTOR_DVEC3: m_ptr = new std::vector<DVec3>; break;
			case VariantType::VR_STD_VECTOR_DVEC4: m_ptr = new std::vector<DVec4>; break;

			case VariantType::VR_C_STRING:          square_assert(0);                      break;
			case VariantType::VR_STD_STRING:        m_ptr = new std::string();			   break;
			case VariantType::VR_STD_VECTOR_STRING: m_ptr = new std::vector<std::string>(); break;
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

		case VariantType::VR_STD_VECTOR_CHAR:              (*this) = ref.get< std::vector<char> >(); break;
		case VariantType::VR_STD_VECTOR_SHORT:             (*this) = ref.get< std::vector<short> >(); break;
		case VariantType::VR_STD_VECTOR_INT:               (*this) = ref.get< std::vector<int> >(); break;
		case VariantType::VR_STD_VECTOR_LONG:              (*this) = ref.get< std::vector<long> >(); break;
		case VariantType::VR_STD_VECTOR_LONGLONG:          (*this) = ref.get< std::vector<long long> >(); break;
		case VariantType::VR_STD_VECTOR_UCHAR:             (*this) = ref.get< std::vector<unsigned char> >(); break;
		case VariantType::VR_STD_VECTOR_USHORT:            (*this) = ref.get< std::vector<unsigned short> >(); break;
		case VariantType::VR_STD_VECTOR_UINT:              (*this) = ref.get< std::vector<unsigned int> >(); break;
		case VariantType::VR_STD_VECTOR_ULONG:             (*this) = ref.get< std::vector<unsigned long> >(); break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:         (*this) = ref.get< std::vector<unsigned long long> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT:             (*this) = ref.get< std::vector<float> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE:            (*this) = ref.get< std::vector<double> >(); break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       (*this) = ref.get< std::vector<long double> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      (*this) = ref.get< std::vector<Mat4> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     (*this) = ref.get< std::vector<DMat4> >(); break;

		case VariantType::VR_STD_VECTOR_VEC2:(*this) = ref.get< std::vector<Vec2> >(); break;
		case VariantType::VR_STD_VECTOR_VEC3:(*this) = ref.get< std::vector<Vec3> >(); break;
		case VariantType::VR_STD_VECTOR_VEC4:(*this) = ref.get< std::vector<Vec4> >(); break;

		case VariantType::VR_STD_VECTOR_IVEC2:(*this) = ref.get< std::vector<IVec2> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC3:(*this) = ref.get< std::vector<IVec3> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC4:(*this) = ref.get< std::vector<IVec4> >(); break;

		case VariantType::VR_STD_VECTOR_DVEC2:(*this) = ref.get< std::vector<DVec2> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC3:(*this) = ref.get< std::vector<DVec3> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC4:(*this) = ref.get< std::vector<DVec4> >(); break;

		case VariantType::VR_C_STRING:
		case VariantType::VR_STD_STRING:           (*this) = ref.get< std::string >(); break;
		case VariantType::VR_STD_VECTOR_STRING:    (*this) = ref.get< std::vector<std::string> >(); break;

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

	VariantRef::VariantRef(const std::vector< char >&v_c)
	{
		m_ptr = (void*)&v_c;
		m_type = VariantType::VR_STD_VECTOR_CHAR;
	}

	VariantRef::VariantRef(const std::vector< short > &v_s)
	{
		m_ptr = (void*)&v_s;
		m_type = VariantType::VR_STD_VECTOR_SHORT;
	}

	VariantRef::VariantRef(const std::vector< int > &v_i)
	{
		m_ptr = (void*)&v_i;
		m_type = VariantType::VR_STD_VECTOR_INT;
	}

	VariantRef::VariantRef(const std::vector< long > &v_l)
	{
		m_ptr = (void*)&v_l;
		m_type = VariantType::VR_STD_VECTOR_LONG;
	}

	VariantRef::VariantRef(const std::vector< long long > &v_ll)
	{
		m_ptr = (void*)&v_ll;
		m_type = VariantType::VR_STD_VECTOR_LONGLONG;
	}

	VariantRef::VariantRef(const std::vector< unsigned char >&v_uc)
	{
		m_ptr = (void*)&v_uc;
		m_type = VariantType::VR_STD_VECTOR_UCHAR;
	}

	VariantRef::VariantRef(const std::vector< unsigned short >&v_us)
	{
		m_ptr = (void*)&v_us;
		m_type = VariantType::VR_STD_VECTOR_USHORT;
	}

	VariantRef::VariantRef(const std::vector< unsigned int > &v_ui)
	{
		m_ptr = (void*)&v_ui;
		m_type = VariantType::VR_STD_VECTOR_UINT;
	}

	VariantRef::VariantRef(const std::vector< unsigned long > &v_ul)
	{
		m_ptr = (void*)&v_ul;
		m_type = VariantType::VR_STD_VECTOR_ULONG;
	}

	VariantRef::VariantRef(const std::vector< unsigned long long > &v_ull)
	{
		m_ptr = (void*)&v_ull;
		m_type = VariantType::VR_STD_VECTOR_ULONGLONG;
	}

	VariantRef::VariantRef(const std::vector< float > &v_f)
	{
		m_ptr = (void*)&v_f;
		m_type = VariantType::VR_STD_VECTOR_FLOAT;
	}

	VariantRef::VariantRef(const std::vector< double > &v_d)
	{
		m_ptr = (void*)&v_d;
		m_type = VariantType::VR_STD_VECTOR_DOUBLE;
	}

	VariantRef::VariantRef(const std::vector< long double > &v_ld)
	{
		m_ptr = (void*)&v_ld;
		m_type = VariantType::VR_STD_VECTOR_LONG_DOUBLE;
	}

	VariantRef::VariantRef(const std::vector< Mat4 > &v_fm)
	{
		m_ptr = (void*)&v_fm;
		m_type = VariantType::VR_STD_VECTOR_FLOAT_MATRIX;
	}

	VariantRef::VariantRef(const std::vector< DMat4 > &v_dm)
	{
		m_ptr = (void*)&v_dm;
		m_type = VariantType::VR_STD_VECTOR_DOUBLE_MATRIX;
	}

	VariantRef::VariantRef(const std::vector< Vec2 > &v_v2)
	{
		m_ptr = (void*)&v_v2;
		m_type = VariantType::VR_STD_VECTOR_VEC2;
	}

	VariantRef::VariantRef(const std::vector< Vec3 > &v_v3)
	{
		m_ptr = (void*)&v_v3;
		m_type = VariantType::VR_STD_VECTOR_VEC3;
	}

	VariantRef::VariantRef(const std::vector< Vec4 > &v_v4)
	{
		m_ptr = (void*)&v_v4;
		m_type = VariantType::VR_STD_VECTOR_VEC4;
	}

	VariantRef::VariantRef(const std::vector< IVec2 > &v_iv2)
	{
		m_ptr = (void*)&v_iv2;
		m_type = VariantType::VR_STD_VECTOR_IVEC2;
	}

	VariantRef::VariantRef(const std::vector< IVec3 > &v_iv3)
	{
		m_ptr = (void*)&v_iv3;
		m_type = VariantType::VR_STD_VECTOR_IVEC3;
	}

	VariantRef::VariantRef(const std::vector< IVec4 > &v_iv4)
	{
		m_ptr = (void*)&v_iv4;
		m_type = VariantType::VR_STD_VECTOR_IVEC4;
	}

	VariantRef::VariantRef(const std::vector< DVec2 > &v_dv2)
	{
		m_ptr = (void*)&v_dv2;
		m_type = VariantType::VR_STD_VECTOR_DVEC2;
	}

	VariantRef::VariantRef(const std::vector< DVec3 > &v_dv3)
	{
		m_ptr = (void*)&v_dv3;
		m_type = VariantType::VR_STD_VECTOR_DVEC3;
	}

	VariantRef::VariantRef(const std::vector< DVec4 > &v_dv4)
	{
		m_ptr = (void*)&v_dv4;
		m_type = VariantType::VR_STD_VECTOR_DVEC4;
	}

	VariantRef::VariantRef(const char* c_str)
	{
		m_ptr = (void*)c_str;
		m_type = VariantType::VR_C_STRING;
	}

	VariantRef::VariantRef(const std::string & str)
	{
		m_ptr = (void*)&str;
		m_type = VariantType::VR_STD_STRING;
	}

	VariantRef::VariantRef(const std::vector< std::string >&v_str)
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

		case VariantType::VR_STD_VECTOR_CHAR:              ((std::vector<char>&)(*this)) = ref.get< std::vector<char> >(); break;
		case VariantType::VR_STD_VECTOR_SHORT:             ((std::vector<short>&)(*this)) = ref.get< std::vector<short> >(); break;
		case VariantType::VR_STD_VECTOR_INT:               ((std::vector<int>&)(*this)) = ref.get< std::vector<int> >(); break;
		case VariantType::VR_STD_VECTOR_LONG:              ((std::vector<long>&)(*this)) = ref.get< std::vector<long> >(); break;
		case VariantType::VR_STD_VECTOR_LONGLONG:          ((std::vector<long long>&)(*this)) = ref.get< std::vector<long long> >(); break;
		case VariantType::VR_STD_VECTOR_UCHAR:             ((std::vector<unsigned char>&)(*this)) = ref.get< std::vector<unsigned char> >(); break;
		case VariantType::VR_STD_VECTOR_USHORT:            ((std::vector<unsigned short>&)(*this)) = ref.get< std::vector<unsigned short> >(); break;
		case VariantType::VR_STD_VECTOR_UINT:              ((std::vector<unsigned int>&)(*this)) = ref.get< std::vector<unsigned int> >(); break;
		case VariantType::VR_STD_VECTOR_ULONG:             ((std::vector<unsigned long>&)(*this)) = ref.get< std::vector<unsigned long> >(); break;
		case VariantType::VR_STD_VECTOR_ULONGLONG:         ((std::vector<unsigned long long>&)(*this)) = ref.get< std::vector<unsigned long long> >(); break;
		case VariantType::VR_STD_VECTOR_FLOAT:             ((std::vector<float>&)(*this)) = ref.get< std::vector<float> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE:            ((std::vector<double>&)(*this)) = ref.get< std::vector<double> >(); break;
		case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       ((std::vector<long double>&)(*this)) = ref.get< std::vector<long double> >(); break;

		case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      ((std::vector<Mat4>&)(*this)) = ref.get< std::vector<Mat4> >(); break;
		case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     ((std::vector<DMat4>&)(*this)) = ref.get< std::vector<DMat4> >(); break;

		case VariantType::VR_STD_VECTOR_VEC2:((std::vector<Vec2>&)(*this)) = ref.get< std::vector<Vec2> >(); break;
		case VariantType::VR_STD_VECTOR_VEC3:((std::vector<Vec3>&)(*this)) = ref.get< std::vector<Vec3> >(); break;
		case VariantType::VR_STD_VECTOR_VEC4:((std::vector<Vec4>&)(*this)) = ref.get< std::vector<Vec4> >(); break;

		case VariantType::VR_STD_VECTOR_IVEC2:((std::vector<IVec2>&)(*this)) = ref.get< std::vector<IVec2> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC3:((std::vector<IVec3>&)(*this)) = ref.get< std::vector<IVec3> >(); break;
		case VariantType::VR_STD_VECTOR_IVEC4:((std::vector<IVec4>&)(*this)) = ref.get< std::vector<IVec4> >(); break;

		case VariantType::VR_STD_VECTOR_DVEC2:((std::vector<DVec2>&)(*this)) = ref.get< std::vector<DVec2> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC3:((std::vector<DVec3>&)(*this)) = ref.get< std::vector<DVec3> >(); break;
		case VariantType::VR_STD_VECTOR_DVEC4:((std::vector<DVec4>&)(*this)) = ref.get< std::vector<DVec4> >(); break;

		case VariantType::VR_C_STRING:
		case VariantType::VR_STD_STRING:           ((std::string&)(*this)) = ref.get< std::string >(); break;
		case VariantType::VR_STD_VECTOR_STRING:    ((std::vector<std::string>&)(*this)) = ref.get< std::vector<std::string> >(); break;

		case VariantType::VR_RESOURCE: ((Shared<ResourceObject>&)(*this)) = ref.get< Shared<ResourceObject> >(); break;

		case VariantType::VR_PTR:                   m_ptr = (void*)ref.get_ptr<void>(); break;
		default: break;
		}
	}
#pragma endregion
}