//
//  Square
//
//  Created by Gabriele Di Bari on 13/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <cstring>
#include "Square/Data/Archive.h"
#include "Square/Core/Application.h"

namespace Square
{
namespace Data
{
    namespace AuxArchive
    {
        //the magic of the header
        constexpr char s_magic[4]{ 'S', 'Q', 'A', 'R' };

        //the type in the stream of a value (format 1): of a size by any compiler
        inline VariantType stored_type(VariantType type)
        {
            switch (type)
            {
            case VR_LONG:                  return VR_LONGLONG;
            case VR_ULONG:                 return VR_ULONGLONG;
            case VR_LONG_DOUBLE:           return VR_DOUBLE;
            case VR_STD_VECTOR_LONG:       return VR_STD_VECTOR_LONGLONG;
            case VR_STD_VECTOR_ULONG:      return VR_STD_VECTOR_ULONGLONG;
            case VR_STD_VECTOR_LONG_DOUBLE:return VR_STD_VECTOR_DOUBLE;
            default:                       return type;
            }
        }

        //a vector of another type
        template < class To, class From >
        Vector< To > convert(const Vector< From >& from)
        {
            Vector< To > to;
            to.reserve(from.size());
            for (const From& value : from) to.push_back(To(value));
            return to;
        }
    }

    template < class T >
    std::ostream& operator < (std::ostream& ostream, const T& value)
    {
        ostream.write((const char*)&value, sizeof(T));
        return ostream;
    }
    
    template <>
    std::ostream& operator < (std::ostream& ostream, const String& value)
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
    std::ostream& operator < (std::ostream& ostream, const Vector< V >& value)
    {
        //vector
        uint64 len = value.size();
        ostream < len;
        for (uint64 i = 0; i != len; ++i) ostream < value[(size_t)i];
        //ok
        return ostream;
    }
    
    ArchiveBinWrite::ArchiveBinWrite(Context& context, std::ostream& stream, bool header) : Archive(context), m_stream(stream)
    {
        if (!header) return;
        m_stream.write(AuxArchive::s_magic, sizeof(AuxArchive::s_magic));
        m_stream < uint32(format_version);
        m_stream < uint32(SQUARE_VERSION_MAJOR);
        m_stream < uint32(SQUARE_VERSION_MINOR);
        m_stream < uint32(SQUARE_VERSION_PATCH);
    }

    Archive& ArchiveBinWrite::block(String& data)
    {
        m_stream < data;
        return *this;
    }

    Archive& ArchiveBinWrite::operator % (VariantRef value)
    {
        //the types of the compiler: of their size by any compiler
        switch (value.get_type())
        {
            case Square::VR_LONG:
                m_stream < int(VR_LONGLONG);
                m_stream < (long long)(value.get<long>());
            return *this;
            case Square::VR_ULONG:
                m_stream < int(VR_ULONGLONG);
                m_stream < (unsigned long long)(value.get<unsigned long>());
            return *this;
            case Square::VR_LONG_DOUBLE:
                m_stream < int(VR_DOUBLE);
                m_stream < double(value.get<long double>());
            return *this;
            case Square::VR_STD_VECTOR_LONG:
                m_stream < int(VR_STD_VECTOR_LONGLONG);
                m_stream < AuxArchive::convert<long long>(value.get<Vector<long>>());
            return *this;
            case Square::VR_STD_VECTOR_ULONG:
                m_stream < int(VR_STD_VECTOR_ULONGLONG);
                m_stream < AuxArchive::convert<unsigned long long>(value.get<Vector<unsigned long>>());
            return *this;
            case Square::VR_STD_VECTOR_LONG_DOUBLE:
                m_stream < int(VR_STD_VECTOR_DOUBLE);
                m_stream < AuxArchive::convert<double>(value.get<Vector<long double>>());
            return *this;
            default: break;
        }
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
            case Square::VR_STD_VECTOR_SHORT: m_stream < value.get<Vector<short>>(); break;
            case Square::VR_STD_VECTOR_INT: m_stream < value.get<Vector<int>>(); break;
            case Square::VR_STD_VECTOR_LONG: m_stream < value.get<Vector<long>>(); break;
            case Square::VR_STD_VECTOR_LONGLONG: m_stream < value.get<Vector<long long>>(); break;
            case Square::VR_STD_VECTOR_USHORT: m_stream < value.get<Vector<unsigned short>>(); break;
            case Square::VR_STD_VECTOR_UINT: m_stream < value.get<Vector<unsigned int>>(); break;
            case Square::VR_STD_VECTOR_ULONG: m_stream < value.get<Vector<unsigned long>>(); break;
            case Square::VR_STD_VECTOR_ULONGLONG: m_stream < value.get<Vector<unsigned long long>>(); break;
            case Square::VR_STD_VECTOR_FLOAT: m_stream < value.get<Vector<float>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE: m_stream < value.get<Vector<double>>(); break;
            case Square::VR_STD_VECTOR_LONG_DOUBLE: m_stream < value.get<Vector<long double>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_FLOAT_MATRIX: m_stream < value.get<Vector<Mat4>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE_MATRIX: m_stream < value.get<Vector<DMat4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_VEC2: m_stream < value.get<Vector<Vec2>>(); break;
            case Square::VR_STD_VECTOR_VEC3: m_stream < value.get<Vector<Vec3>>(); break;
            case Square::VR_STD_VECTOR_VEC4: m_stream < value.get<Vector<Vec4>>(); break;
                
            case Square::VR_STD_VECTOR_DVEC2: m_stream < value.get<Vector<DVec2>>(); break;
            case Square::VR_STD_VECTOR_DVEC3: m_stream < value.get<Vector<DVec3>>(); break;
            case Square::VR_STD_VECTOR_DVEC4: m_stream < value.get<Vector<DVec4>>(); break;
                
            case Square::VR_STD_VECTOR_IVEC2: m_stream < value.get<Vector<IVec2>>(); break;
            case Square::VR_STD_VECTOR_IVEC3: m_stream < value.get<Vector<IVec3>>(); break;
            case Square::VR_STD_VECTOR_IVEC4: m_stream < value.get<Vector<IVec4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_C_STRING:
                throw std::runtime_error("ArchiveBinWrite, can't write a C string");
            break;
            case Square::VR_STD_STRING:
                m_stream < value.get<String>();
            break;
			case Square::VR_STD_VECTOR_STRING: m_stream < value.get<Vector<String>>(); break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_RESOURCE: 
				if (value.get< Shared<ResourceObject> >())
				{
					m_stream < true;
					m_stream < String(value.get< Shared<ResourceObject> >()->resource_name());
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
    std::istream& operator > (std::istream& istream, String& value)
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
    std::istream& operator > (std::istream& istream, Vector< V >& value)
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
    
    ArchiveBinRead::ArchiveBinRead(Context& context, std::istream& stream) : Archive(context, 0), m_stream(stream)
    {
        //its header, if it has it (else the format 0: from its start)
        const std::streampos start = m_stream.tellg();
        char magic[sizeof(AuxArchive::s_magic)]{};
        m_stream.read(magic, sizeof(magic));
        if (!m_stream || std::memcmp(magic, AuxArchive::s_magic, sizeof(magic)) != 0)
        {
            m_stream.clear();
            m_stream.seekg(start);
            return;
        }
        uint32 version = 0, major = 0, minor = 0, patch = 0;
        m_stream > version;
        m_stream > major;
        m_stream > minor;
        m_stream > patch;
        m_engine_version = to_string(major) + "." + to_string(minor) + "." + to_string(patch);
        //a format of a newer engine
        if (version > format_version)
            throw std::runtime_error(("ArchiveBinRead, format " + to_string(version) + " of the engine " + m_engine_version + ", newer than this one").c_str());
        m_version = version;
    }

    ArchiveBinRead::ArchiveBinRead(Context& context, std::istream& stream, uint32 version) : Archive(context, version), m_stream(stream)
    {
    }

    Archive& ArchiveBinRead::block(String& data)
    {
        m_stream > data;
        if (!m_stream) throw std::runtime_error("ArchiveBinRead, a block past the end of the stream");
        return *this;
    }

    Archive& ArchiveBinRead::operator % (VariantRef value)
    {
        //Type
        int type;
        //get type
        m_stream > type;
        //the types of the compiler, stored of their size by any compiler (format 1)
        if (m_version >= 1 && type != value.get_type() && type == AuxArchive::stored_type(value.get_type()))
        {
            switch (value.get_type())
            {
                case Square::VR_LONG:
                {
                    long long stored = 0;
                    m_stream > stored;
                    value.get<long>() = long(stored);
                }
                break;
                case Square::VR_ULONG:
                {
                    unsigned long long stored = 0;
                    m_stream > stored;
                    value.get<unsigned long>() = (unsigned long)(stored);
                }
                break;
                case Square::VR_LONG_DOUBLE:
                {
                    double stored = 0.0;
                    m_stream > stored;
                    value.get<long double>() = stored;
                }
                break;
                case Square::VR_STD_VECTOR_LONG:
                {
                    Vector<long long> stored;
                    m_stream > stored;
                    value.get<Vector<long>>() = AuxArchive::convert<long>(stored);
                }
                break;
                case Square::VR_STD_VECTOR_ULONG:
                {
                    Vector<unsigned long long> stored;
                    m_stream > stored;
                    value.get<Vector<unsigned long>>() = AuxArchive::convert<unsigned long>(stored);
                }
                break;
                case Square::VR_STD_VECTOR_LONG_DOUBLE:
                {
                    Vector<double> stored;
                    m_stream > stored;
                    value.get<Vector<long double>>() = AuxArchive::convert<long double>(stored);
                }
                break;
                default: break;
            }
            return *this;
        }
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
            case Square::VR_STD_VECTOR_SHORT: m_stream > value.get<Vector<short>>(); break;
            case Square::VR_STD_VECTOR_INT: m_stream > value.get<Vector<int>>(); break;
            case Square::VR_STD_VECTOR_LONG: m_stream > value.get<Vector<long>>(); break;
            case Square::VR_STD_VECTOR_LONGLONG: m_stream > value.get<Vector<long long>>(); break;
            case Square::VR_STD_VECTOR_USHORT: m_stream > value.get<Vector<unsigned short>>(); break;
            case Square::VR_STD_VECTOR_UINT: m_stream > value.get<Vector<unsigned int>>(); break;
            case Square::VR_STD_VECTOR_ULONG: m_stream > value.get<Vector<unsigned long>>(); break;
            case Square::VR_STD_VECTOR_ULONGLONG: m_stream > value.get<Vector<unsigned long long>>(); break;
            case Square::VR_STD_VECTOR_FLOAT: m_stream > value.get<Vector<float>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE: m_stream > value.get<Vector<double>>(); break;
            case Square::VR_STD_VECTOR_LONG_DOUBLE: m_stream > value.get<Vector<long double>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_FLOAT_MATRIX: m_stream > value.get<Vector<Mat4>>(); break;
            case Square::VR_STD_VECTOR_DOUBLE_MATRIX: m_stream > value.get<Vector<DMat4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_STD_VECTOR_VEC2: m_stream > value.get<Vector<Vec2>>(); break;
            case Square::VR_STD_VECTOR_VEC3: m_stream > value.get<Vector<Vec3>>(); break;
            case Square::VR_STD_VECTOR_VEC4: m_stream > value.get<Vector<Vec4>>(); break;
                
            case Square::VR_STD_VECTOR_DVEC2: m_stream > value.get<Vector<DVec2>>(); break;
            case Square::VR_STD_VECTOR_DVEC3: m_stream > value.get<Vector<DVec3>>(); break;
            case Square::VR_STD_VECTOR_DVEC4: m_stream > value.get<Vector<DVec4>>(); break;
                
            case Square::VR_STD_VECTOR_IVEC2: m_stream > value.get<Vector<IVec2>>(); break;
            case Square::VR_STD_VECTOR_IVEC3: m_stream > value.get<Vector<IVec3>>(); break;
            case Square::VR_STD_VECTOR_IVEC4: m_stream > value.get<Vector<IVec4>>(); break;
            ////////////////////////////////////////////////////////////////////////////////////////////
            case Square::VR_C_STRING:
                throw std::runtime_error("ArchiveBinWrite, can't write a C string");
                break;
            case Square::VR_STD_STRING:
                m_stream > value.get<String>();
                break;
            case Square::VR_STD_VECTOR_STRING: m_stream > value.get<Vector<String>>(); break;
			////////////////////////////////////////////////////////////////////////////////////////////
			case Square::VR_RESOURCE:
			{
				//test
				bool exists = false;
				m_stream > exists;
				if (!exists) break;
				//get name
				String resource_name;
				m_stream > resource_name;
				//load
				Shared<ResourceObject>  resource_object;
				//load resource
				if(!(resource_object = context().resource(resource_name)))
					throw std::runtime_error(("ArchiveBinWrite, can't load resource: " + resource_name + ", not exists").c_str());
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
