//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Variant.h"
#include <iostream>

namespace Square
{
namespace  Data
{
	class Archive
	{
	public:
		//archivie operator
		virtual Archive& operator % (VariantRef value) = 0;
	};

	class WriteArchive : Archive
	{
	public:

		//Input
		WriteArchive(std::ostream& stream) : m_stream(stream) {}
		//Archivie operator
		virtual Archive& operator % (VariantRef value)
		{
			//type
			m_stream << value.get_type();
			//serialize
			switch (value.get_type())
			{
			case Square::VR_NONE: /* none */ break;
			case Square::VR_BOOL:	      m_stream << value.get<bool>();  break;
			case Square::VR_CHAR:         m_stream << value.get<char>();  break;
			case Square::VR_SHORT:        m_stream << value.get<short>(); break;
			case Square::VR_INT:          m_stream << value.get<int>();   break;
			case Square::VR_LONG:         m_stream << value.get<long>(); break;
			case Square::VR_LONGLONG:     m_stream << value.get<long long>(); break;
			case Square::VR_UCHAR:        m_stream << value.get<unsigned char>(); break;
			case Square::VR_USHORT:       m_stream << value.get<unsigned short>();  break;
			case Square::VR_UINT:         m_stream << value.get<unsigned int>();  break;
			case Square::VR_ULONG:        m_stream << value.get<unsigned long>();  break;
			case Square::VR_ULONGLONG:    m_stream << value.get<unsigned long long>();  break;
			case Square::VR_FLOAT:        m_stream << value.get<float>();  break;
			case Square::VR_DOUBLE:       m_stream << value.get<double>();  break;
			case Square::VR_LONG_DOUBLE:  m_stream << value.get<long double>();  break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_VEC2:         m_stream << value.get<Vec2>().x; 
										  m_stream << value.get<Vec2>().y;  break;

			case Square::VR_VEC3:         m_stream << value.get<Vec3>().x;
										  m_stream << value.get<Vec3>().y;
										  m_stream << value.get<Vec3>().z;  break;

			case Square::VR_VEC4:         m_stream << value.get<Vec4>().x;
										  m_stream << value.get<Vec4>().y;
										  m_stream << value.get<Vec4>().z;
										  m_stream << value.get<Vec4>().w;  break
											  ;
			case Square::VR_IVEC2:        m_stream << value.get<IVec2>().x; 
										  m_stream << value.get<IVec2>().y;  break;

			case Square::VR_IVEC3:        m_stream << value.get<IVec3>().x;
										  m_stream << value.get<IVec3>().y;
										  m_stream << value.get<IVec3>().z;  break;

			case Square::VR_IVEC4:        m_stream << value.get<IVec4>().x;
										  m_stream << value.get<IVec4>().y;
										  m_stream << value.get<IVec4>().z;
										  m_stream << value.get<IVec4>().w;  break;
										  
			case Square::VR_DVEC2:        m_stream << value.get<DVec2>().x; 
										  m_stream << value.get<DVec2>().y;  break;

			case Square::VR_DVEC3:        m_stream << value.get<DVec3>().x;
										  m_stream << value.get<DVec3>().y;
										  m_stream << value.get<DVec3>().z;  break;

			case Square::VR_DVEC4:        m_stream << value.get<DVec4>().x;
										  m_stream << value.get<DVec4>().y;
										  m_stream << value.get<DVec4>().z;
										  m_stream << value.get<DVec4>().w;  break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_QUAT:         m_stream << value.get<Quat>().x;
										  m_stream << value.get<Quat>().y;
										  m_stream << value.get<Quat>().z;
										  m_stream << value.get<Quat>().w;  break;

			case Square::VR_DQUAT:        m_stream << value.get<DQuat>().x;
										  m_stream << value.get<DQuat>().y;
										  m_stream << value.get<DQuat>().z;
										  m_stream << value.get<DQuat>().w;  break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_FLOAT_MATRIX: for (size_t x = 0; x != 4; ++x)
										  for (size_t y = 0; y != 4; ++y)
											  m_stream << value.get<Mat4>()[x][y];
				break;
			case Square::VR_DOUBLE_MATRIX: for (size_t x = 0; x != 4; ++x) 
										   for (size_t y = 0; y != 4; ++y) 
											   m_stream << value.get<DMat4>()[x][y];
				break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_STD_VECTOR_SHORT: 
			{
				auto& vector = value.get<std::vector<short>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_INT:
			{
				auto& vector = value.get<std::vector<int>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_LONG:
			{
				auto& vector = value.get<std::vector<long>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_LONGLONG:
			{
				auto& vector = value.get<std::vector<long long>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_USHORT:
			{
				auto& vector = value.get<std::vector<unsigned short>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_UINT:
			{
				auto& vector = value.get<std::vector<unsigned int>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_ULONG:
			{
				auto& vector = value.get<std::vector<unsigned long>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_ULONGLONG:
			{
				auto& vector = value.get<std::vector<unsigned long long>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_FLOAT:
			{
				auto& vector = value.get<std::vector<float>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_DOUBLE:
			{
				auto& vector = value.get<std::vector<double>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			case Square::VR_STD_VECTOR_LONG_DOUBLE:
			{
				auto& vector = value.get<std::vector<long double>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) m_stream << vector[i];
			}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_STD_VECTOR_FLOAT_MATRIX:
			{
				auto& vector = value.get<std::vector<Mat4>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) 
				for (size_t x = 0; x != 4; ++x)
				for (size_t y = 0; y != 4; ++y) 
					m_stream << vector[i][x][y];
			}
			break;
			case Square::VR_STD_VECTOR_DOUBLE_MATRIX:
			{
				auto& vector = value.get<std::vector<DMat4>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i) 
				for (size_t x = 0; x != 4; ++x)
				for (size_t y = 0; y != 4; ++y) 
					m_stream << vector[i][x][y];
			}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_STD_VECTOR_VEC2:
			{
				auto& vector = value.get<std::vector<Vec2>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
				}
			}
			break;
			case Square::VR_STD_VECTOR_VEC3:
			{
				auto& vector = value.get<std::vector<Vec3>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
				}
			}
			break;
			case Square::VR_STD_VECTOR_VEC4:
			{
				auto& vector = value.get<std::vector<Vec4>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
					m_stream << vector[i].w;
				}
			}
			break;
			case Square::VR_STD_VECTOR_IVEC2:
			{
				auto& vector = value.get<std::vector<IVec2>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
				}
			}
			break;
			case Square::VR_STD_VECTOR_IVEC3:
			{
				auto& vector = value.get<std::vector<IVec3>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
				}
			}
			break;
			case Square::VR_STD_VECTOR_IVEC4:
			{
				auto& vector = value.get<std::vector<IVec4>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
					m_stream << vector[i].w;
				}
			}
			break;
			case Square::VR_STD_VECTOR_DVEC2:
			{
				auto& vector = value.get<std::vector<DVec2>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
				}
			}
			break;
			case Square::VR_STD_VECTOR_DVEC3:
			{
				auto& vector = value.get<std::vector<DVec3>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
				}
			}
			break;
			case Square::VR_STD_VECTOR_DVEC4:
			{
				auto& vector = value.get<std::vector<DVec4>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i].x;
					m_stream << vector[i].y;
					m_stream << vector[i].z;
					m_stream << vector[i].w;
				}
			}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_C_STRING:
				m_stream << value.get<const char*>();
			break;
			case Square::VR_STD_STRING:
				m_stream << value.get<std::string>().c_str();
			break;
			case Square::VR_STD_VECTOR_STRING:
				break;
			case Square::VR_PTR:
			{
				auto& vector = value.get<std::vector<std::string>>();
				uint64   len = vector.size();
				m_stream << len;
				for (size_t i = 0; i != len; ++i)
				{
					m_stream << vector[i];
				}
			}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
			default:
				//none
			break;
			}
		}

	private:

		//output stream
		std::ostream& m_stream;

	};
}
}