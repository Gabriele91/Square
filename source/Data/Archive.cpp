//
//  Square
//
//  Created by Gabriele Di Bari on 13/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/Archive.h"
#include "Square/Core/Application.h"

namespace Square
{
namespace Data
{
    template < class T >
    std::ostream& operator < (std::ostream& ostream, const T& value)
    {
        ostream.write((const char*)&value, sizeof(T));
        return ostream;
    }
    
    template <>
    std::ostream& operator < (std::ostream& ostream, const std::string& value)
    {
        int64 size = value.size();
        ostream.write((const char*)&size, sizeof(int64));
        ostream.write(value.c_str(), value.size());
        return ostream;
    }
    
    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const TVec2< V >& value)
    {
        ostream.write((const char*)&value.x, sizeof(V));
        ostream.write((const char*)&value.y, sizeof(V));
        return ostream;
    }
    
    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const TVec3< V >& value)
    {
        ostream.write((const char*)&value.x, sizeof(V));
        ostream.write((const char*)&value.y, sizeof(V));
        ostream.write((const char*)&value.z, sizeof(V));
        return ostream;
    }
    
    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const TVec4< V >& value)
    {
        ostream.write((const char*)&value.x, sizeof(V));
        ostream.write((const char*)&value.y, sizeof(V));
        ostream.write((const char*)&value.z, sizeof(V));
        ostream.write((const char*)&value.w, sizeof(V));
        return ostream;
    }

    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const TQuat< V >& value)
    {
        ostream.write((const char*)&value.x, sizeof(V));
        ostream.write((const char*)&value.y, sizeof(V));
        ostream.write((const char*)&value.z, sizeof(V));
        ostream.write((const char*)&value.w, sizeof(V));
        return ostream;
    }

    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const TMat4< V >& value)
    {
        for (int x = 0; x != 4; ++x)
        for (int y = 0; y != 4; ++y)
            ostream.write((const char*)&value[x][y], sizeof(V));
        return ostream;
    }
    
    template < typename V >
    std::ostream& operator < (std::ostream& ostream, const std::vector< V >& value)
    {
        //vector
        uint64 len = value.size();
        ostream < len;
        for (uint64 i = 0; i != len; ++i) ostream < value[(size_t)i];
        //ok
        return ostream;
    }
    
    ArchiveBinWrite::ArchiveBinWrite(Context& context, std::ostream& stream) : Archive(context), m_stream(stream) {}
    Archive& ArchiveBinWrite::operator % (VariantRef value)
    {
        //type
        m_stream < int(value.get_type());
        //serialize
        switch (value.get_type())
        {
            case Square::VR_NONE: /* none */ break;
            case Square::VR_BOOL:         m_stream < value.get<bool>();  break;
            case Square::VR_CHAR:         m_stream < value.get<char>();  break;
            case Square::VR_SHORT:        m_stream < value.get<short>(); break;
            case Square::VR_INT:          m_stream < value.get<int>();   break;
            case Square::VR_LONG:         m_stream < value.get<long>(); break;
            case Square::VR_LONGLONG:     m_stream < value.get<long long>(); break;
            case Square::VR_UCHAR:        m_stream < value.get<unsigned char>(); break;
            case Square::VR_USHORT:       m_stream < value.get<unsigned short>();  break;
            case Square::VR_UINT:         m_stream < value.get<unsigned int>();  break;
            case Square::VR_ULONG:        m_stream < value.get<unsigned long>();  break;
            case Square::VR_ULONGLONG:    m_stream < value.get<unsigned long long>();  break;
            case Square::VR_FLOAT:        m_stream < value.get<float>();  break;
            case Square::VR_DOUBLE:       m_stream < value.get<double>();  break;
            case Square::VR_LONG_DOUBLE:  m_stream < value.get<long double>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_VEC2: m_stream < value.get<Vec2>();  break;
            case Square::VR_VEC3: m_stream < value.get<Vec3>(); break;
            case Square::VR_VEC4: m_stream < value.get<Vec4>(); break;
            
            case Square::VR_IVEC2: m_stream < value.get<IVec2>();break;
            case Square::VR_IVEC3: m_stream < value.get<IVec3>(); break;
            case Square::VR_IVEC4: m_stream < value.get<IVec4>(); break;
                
            case Square::VR_DVEC2: m_stream < value.get<DVec2>(); break;
            case Square::VR_DVEC3: m_stream < value.get<DVec3>(); break;
            case Square::VR_DVEC4: m_stream < value.get<DVec4>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_QUAT: m_stream < value.get<Quat>(); break;
            case Square::VR_DQUAT: m_stream < value.get<DQuat>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_FLOAT_MATRIX:  m_stream < value.get<Mat4>(); break;
            case Square::VR_DOUBLE_MATRIX: m_stream < value.get<DMat4>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_SHORT: m_stream < value.get<std::vector<short>>(); break;
            case Square::VR_STD_VECTOR_INT: m_stream < value.get<std::vector<int>>(); break;
            case Square::VR_STD_VECTOR_LONG: m_stream < value.get<std::vector<long>>(); break;
            case Square::VR_STD_VECTOR_LONGLONG: m_stream < value.get<std::vector<long long>>(); break;
            case Square::VR_STD_VECTOR_USHORT: m_stream < value.get<std::vector<unsigned short>>(); break;
            case Square::VR_STD_VECTOR_UINT: m_stream < value.get<std::vector<unsigned int>>(); break;
            case Square::VR_STD_VECTOR_ULONG: m_stream < value.get<std::vector<unsigned long>>(); break;
            case Square::VR_STD_VECTOR_ULONGLONG: m_stream < value.get<std::vector<unsigned long long>>(); break;
            case Square::VR_STD_VECTOR_FLOAT: m_stream < value.get<std::vector<float>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE: m_stream < value.get<std::vector<double>>(); break;
            case Square::VR_STD_VECTOR_LONG_DOUBLE: m_stream < value.get<std::vector<long double>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_FLOAT_MATRIX: m_stream < value.get<std::vector<Mat4>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE_MATRIX: m_stream < value.get<std::vector<DMat4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_VEC2: m_stream < value.get<std::vector<Vec2>>(); break;
            case Square::VR_STD_VECTOR_VEC3: m_stream < value.get<std::vector<Vec3>>(); break;
            case Square::VR_STD_VECTOR_VEC4: m_stream < value.get<std::vector<Vec4>>(); break;
                
            case Square::VR_STD_VECTOR_DVEC2: m_stream < value.get<std::vector<DVec2>>(); break;
            case Square::VR_STD_VECTOR_DVEC3: m_stream < value.get<std::vector<DVec3>>(); break;
            case Square::VR_STD_VECTOR_DVEC4: m_stream < value.get<std::vector<DVec4>>(); break;
                
            case Square::VR_STD_VECTOR_IVEC2: m_stream < value.get<std::vector<IVec2>>(); break;
            case Square::VR_STD_VECTOR_IVEC3: m_stream < value.get<std::vector<IVec3>>(); break;
            case Square::VR_STD_VECTOR_IVEC4: m_stream < value.get<std::vector<IVec4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_C_STRING:
                throw std::runtime_error("ArchiveBinWrite, can't write a C string");
            break;
            case Square::VR_STD_STRING:
                m_stream < value.get<std::string>();
            break;
			case Square::VR_STD_VECTOR_STRING: m_stream < value.get<std::vector<std::string>>(); break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_RESOURCE: 
				if (value.get< Shared<ResourceObject> >())
				{
					m_stream < true;
					m_stream < std::string(value.get< Shared<ResourceObject> >()->resource_name());
				}
				else
				{
					m_stream < false;
				}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_PTR:
                throw std::runtime_error("ArchiveBinWrite, can't write a C pointer");
            break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            default:
                //none
                break;
        }
        return *this;
    }

    template < class T >
    std::istream& operator > (std::istream& istream, T& value)
    {
        istream.read((char*)&value, sizeof(T));
        return istream;
    }

    template <>
    std::istream& operator > (std::istream& istream, std::string& value)
    {
        //read size
		uint64 size = 0;
        istream.read((char*)&size, sizeof(uint64));
        //read value
        value.resize((size_t)size);
        istream.read((char*)value.c_str(), value.size());
        //ok
        return istream;
    }
    
    template < typename V >
    std::istream& operator > (std::istream& istream, TVec2< V >& value)
    {
        istream.read((char*)&value.x, sizeof(V));
        istream.read((char*)&value.y, sizeof(V));
        return istream;
    }
    
    template < typename V >
    std::istream& operator > (std::istream& istream, TVec3< V >& value)
    {
        istream.read((char*)&value.x, sizeof(V));
        istream.read((char*)&value.y, sizeof(V));
        istream.read((char*)&value.z, sizeof(V));
        return istream;
    }
    
    template < typename V >
    std::istream& operator > (std::istream& istream, TVec4< V >& value)
    {
        istream.read((char*)&value.x, sizeof(V));
        istream.read((char*)&value.y, sizeof(V));
        istream.read((char*)&value.z, sizeof(V));
        istream.read((char*)&value.w, sizeof(V));
        return istream;
    }
    
    template < typename V >
    std::istream& operator > (std::istream& istream, TQuat< V >& value)
    {
        istream.read((char*)&value.x, sizeof(V));
        istream.read((char*)&value.y, sizeof(V));
        istream.read((char*)&value.z, sizeof(V));
        istream.read((char*)&value.w, sizeof(V));
        return istream;
    }

    template < typename V >
    std::istream& operator > (std::istream& istream, TMat4< V >& value)
    {
        for (int x = 0; x != 4; ++x)
            for (int y = 0; y != 4; ++y)
                istream.read((char*)&value[x][y], sizeof(V));
        return istream;
    }
    
    template < typename V >
    std::istream& operator > (std::istream& istream, std::vector< V >& value)
    {
        //vector len
        uint64 len = 0;
        istream > len;
        //alloc
        value.resize((size_t)len);
        //read
        for (uint64 i = 0; i != len; ++i) istream > value[(size_t)i];
        //ok
        return istream;
    }
    
    ArchiveBinRead::ArchiveBinRead(Context& context, std::istream& stream) : Archive(context), m_stream(stream) {}
    Archive& ArchiveBinRead::operator % (VariantRef value)
    {
        //Type
        int type;
        //get type
        m_stream > type;
        //error
        if(type != value.get_type())  throw std::runtime_error("ArchiveBinRead, Not valid type");
        //serialize
        switch (value.get_type())
        {
            case Square::VR_NONE: /* none */ break;
            case Square::VR_BOOL:         m_stream > value.get<bool>();  break;
            case Square::VR_CHAR:         m_stream > value.get<char>();  break;
            case Square::VR_SHORT:        m_stream > value.get<short>(); break;
            case Square::VR_INT:          m_stream > value.get<int>();   break;
            case Square::VR_LONG:         m_stream > value.get<long>(); break;
            case Square::VR_LONGLONG:     m_stream > value.get<long long>(); break;
            case Square::VR_UCHAR:        m_stream > value.get<unsigned char>(); break;
            case Square::VR_USHORT:       m_stream > value.get<unsigned short>();  break;
            case Square::VR_UINT:         m_stream > value.get<unsigned int>();  break;
            case Square::VR_ULONG:        m_stream > value.get<unsigned long>();  break;
            case Square::VR_ULONGLONG:    m_stream > value.get<unsigned long long>();  break;
            case Square::VR_FLOAT:        m_stream > value.get<float>();  break;
            case Square::VR_DOUBLE:       m_stream > value.get<double>();  break;
            case Square::VR_LONG_DOUBLE:  m_stream > value.get<long double>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_VEC2: m_stream > value.get<Vec2>();  break;
            case Square::VR_VEC3: m_stream > value.get<Vec3>(); break;
            case Square::VR_VEC4: m_stream > value.get<Vec4>(); break;
                
            case Square::VR_IVEC2: m_stream > value.get<IVec2>();break;
            case Square::VR_IVEC3: m_stream > value.get<IVec3>(); break;
            case Square::VR_IVEC4: m_stream > value.get<IVec4>(); break;
                
            case Square::VR_DVEC2: m_stream > value.get<DVec2>(); break;
            case Square::VR_DVEC3: m_stream > value.get<DVec3>(); break;
            case Square::VR_DVEC4: m_stream > value.get<DVec4>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_QUAT: m_stream > value.get<Quat>(); break;
            case Square::VR_DQUAT: m_stream > value.get<DQuat>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_FLOAT_MATRIX:  m_stream > value.get<Mat4>(); break;
            case Square::VR_DOUBLE_MATRIX: m_stream > value.get<DMat4>();  break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_SHORT: m_stream > value.get<std::vector<short>>(); break;
            case Square::VR_STD_VECTOR_INT: m_stream > value.get<std::vector<int>>(); break;
            case Square::VR_STD_VECTOR_LONG: m_stream > value.get<std::vector<long>>(); break;
            case Square::VR_STD_VECTOR_LONGLONG: m_stream > value.get<std::vector<long long>>(); break;
            case Square::VR_STD_VECTOR_USHORT: m_stream > value.get<std::vector<unsigned short>>(); break;
            case Square::VR_STD_VECTOR_UINT: m_stream > value.get<std::vector<unsigned int>>(); break;
            case Square::VR_STD_VECTOR_ULONG: m_stream > value.get<std::vector<unsigned long>>(); break;
            case Square::VR_STD_VECTOR_ULONGLONG: m_stream > value.get<std::vector<unsigned long long>>(); break;
            case Square::VR_STD_VECTOR_FLOAT: m_stream > value.get<std::vector<float>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE: m_stream > value.get<std::vector<double>>(); break;
            case Square::VR_STD_VECTOR_LONG_DOUBLE: m_stream > value.get<std::vector<long double>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_FLOAT_MATRIX: m_stream > value.get<std::vector<Mat4>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE_MATRIX: m_stream > value.get<std::vector<DMat4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_VEC2: m_stream > value.get<std::vector<Vec2>>(); break;
            case Square::VR_STD_VECTOR_VEC3: m_stream > value.get<std::vector<Vec3>>(); break;
            case Square::VR_STD_VECTOR_VEC4: m_stream > value.get<std::vector<Vec4>>(); break;
                
            case Square::VR_STD_VECTOR_DVEC2: m_stream > value.get<std::vector<DVec2>>(); break;
            case Square::VR_STD_VECTOR_DVEC3: m_stream > value.get<std::vector<DVec3>>(); break;
            case Square::VR_STD_VECTOR_DVEC4: m_stream > value.get<std::vector<DVec4>>(); break;
                
            case Square::VR_STD_VECTOR_IVEC2: m_stream > value.get<std::vector<IVec2>>(); break;
            case Square::VR_STD_VECTOR_IVEC3: m_stream > value.get<std::vector<IVec3>>(); break;
            case Square::VR_STD_VECTOR_IVEC4: m_stream > value.get<std::vector<IVec4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_C_STRING:
                throw std::runtime_error("ArchiveBinWrite, can't write a C string");
                break;
            case Square::VR_STD_STRING:
                m_stream > value.get<std::string>();
                break;
            case Square::VR_STD_VECTOR_STRING: m_stream > value.get<std::vector<std::string>>(); break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_RESOURCE:
			{
				//test
				bool exists = false;
				m_stream > exists;
				if (!exists) break;
				//get name
				std::string resource_name;
				m_stream > resource_name;
				//load
				Shared<ResourceObject>  resource_object;
				//load resource
				if(!(resource_object = context().resource(resource_name)))
					throw std::runtime_error("ArchiveBinWrite, can't load resource: " + resource_name + ", not exists");
				//save
				value.get<Shared<ResourceObject>>() = resource_object;
			}
			break;
			////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_PTR:
                throw std::runtime_error("ArchiveBinWrite, can't write a C pointer");
                break;
                ////////////////////////////////////////////////////////////////////////////////////////////
            default:
                //none
                break;
        }
        return *this;
    }

}
}
