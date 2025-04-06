//
//  Square
//
//  Created by Gabriele on 09/07/176.
//  Copyright � 2017 Gabriele. All rights reserved.
//
#pragma once
#include <optional>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"

namespace Square
{
	class Variant;
	class VariantRef;
	// Res Dec
	enum class ResourceType;
	class ResourceObject;
	// Types
	enum VariantType
	{
		VR_NONE,
		VR_BOOL,
		VR_CHAR,
		VR_SHORT,
		VR_INT,
		VR_LONG,
		VR_LONGLONG,

		VR_UCHAR,
		VR_USHORT,
		VR_UINT,
		VR_ULONG,
		VR_ULONGLONG,

		VR_FLOAT,
		VR_DOUBLE,
		VR_LONG_DOUBLE,

		VR_VEC2,
		VR_VEC3,
		VR_VEC4,

		VR_IVEC2,
		VR_IVEC3,
		VR_IVEC4,

		VR_DVEC2,
		VR_DVEC3,
		VR_DVEC4,

		VR_QUAT,
		VR_DQUAT,

		//HEAP
		VR_FLOAT_MATRIX,
		VR_DOUBLE_MATRIX,

		VR_STD_VECTOR_CHAR,
		VR_STD_VECTOR_SHORT,
		VR_STD_VECTOR_INT,
		VR_STD_VECTOR_LONG,
		VR_STD_VECTOR_LONGLONG,

		VR_STD_VECTOR_UCHAR,
		VR_STD_VECTOR_USHORT,
		VR_STD_VECTOR_UINT,
		VR_STD_VECTOR_ULONG,
		VR_STD_VECTOR_ULONGLONG,

		VR_STD_VECTOR_FLOAT,
		VR_STD_VECTOR_DOUBLE,
		VR_STD_VECTOR_LONG_DOUBLE,

		VR_STD_VECTOR_FLOAT_MATRIX,
		VR_STD_VECTOR_DOUBLE_MATRIX,

		VR_STD_VECTOR_VEC2,
		VR_STD_VECTOR_VEC3,
		VR_STD_VECTOR_VEC4,

		VR_STD_VECTOR_IVEC2,
		VR_STD_VECTOR_IVEC3,
		VR_STD_VECTOR_IVEC4,

		VR_STD_VECTOR_DVEC2,
		VR_STD_VECTOR_DVEC3,
		VR_STD_VECTOR_DVEC4,

		VR_C_STRING,
		VR_STD_STRING,
		VR_STD_VECTOR_STRING,

		//Square type
		VR_RESOURCE,

		//PTR
		VR_PTR
	};
	// Get type
	template < class T, typename Enable = void > 
	inline typename std::enable_if< !std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, VariantType >::type variant_traits()
	{
		return VariantType::VR_NONE;
	};
	template < class T >
	inline typename std::enable_if< std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, VariantType >::type variant_traits()
	{
		return VariantType::VR_RESOURCE;
	};
	//template specialization 
	template <> inline VariantType variant_traits<bool>() { return VariantType::VR_BOOL; };
	template <> inline VariantType variant_traits<char>() { return VariantType::VR_CHAR; };
	template <> inline VariantType variant_traits<short>() { return VariantType::VR_SHORT; };
	template <> inline VariantType variant_traits<int>() { return VariantType::VR_INT; };
	template <> inline VariantType variant_traits<long>() { return VariantType::VR_LONG; };
	template <> inline VariantType variant_traits<long long>() { return VariantType::VR_LONGLONG; };

	template <> inline VariantType variant_traits<unsigned char>() { return VariantType::VR_UCHAR; };
	template <> inline VariantType variant_traits<unsigned short>() { return VariantType::VR_USHORT; };
	template <> inline VariantType variant_traits<unsigned int>() { return VariantType::VR_UINT; };
	template <> inline VariantType variant_traits<unsigned long>() { return VariantType::VR_ULONG; };
	template <> inline VariantType variant_traits<unsigned long long>() { return VariantType::VR_ULONGLONG; };

	template <> inline VariantType variant_traits<float>() { return VariantType::VR_FLOAT; };
	template <> inline VariantType variant_traits<double>() { return VariantType::VR_DOUBLE; };
	template <> inline VariantType variant_traits<long double>() { return VariantType::VR_LONG_DOUBLE; };

	template <> inline VariantType variant_traits<Vec2>() { return VariantType::VR_VEC2; };
	template <> inline VariantType variant_traits<Vec3>() { return VariantType::VR_VEC3; };
	template <> inline VariantType variant_traits<Vec4>() { return VariantType::VR_VEC4; };

	template <> inline VariantType variant_traits<IVec2>() { return VariantType::VR_IVEC2; };
	template <> inline VariantType variant_traits<IVec3>() { return VariantType::VR_IVEC3; };
	template <> inline VariantType variant_traits<IVec4>() { return VariantType::VR_IVEC4; };

	template <> inline VariantType variant_traits<DVec2>() { return VariantType::VR_DVEC2; };
	template <> inline VariantType variant_traits<DVec3>() { return VariantType::VR_DVEC3; };
	template <> inline VariantType variant_traits<DVec4>() { return VariantType::VR_DVEC4; };

	template <> inline VariantType variant_traits<Quat>()  { return VariantType::VR_QUAT; };
	template <> inline VariantType variant_traits<DQuat>() { return VariantType::VR_DQUAT; };

	template <> inline VariantType variant_traits<Mat4>() { return VariantType::VR_FLOAT_MATRIX; };
	template <> inline VariantType variant_traits<DMat4>() { return VariantType::VR_DOUBLE_MATRIX; };

	template <> inline VariantType variant_traits< std::vector< char > >() { return VariantType::VR_STD_VECTOR_CHAR; };
	template <> inline VariantType variant_traits< std::vector< short > >() { return VariantType::VR_STD_VECTOR_SHORT; };
	template <> inline VariantType variant_traits< std::vector< int > >() { return VariantType::VR_STD_VECTOR_INT; };
	template <> inline VariantType variant_traits< std::vector< long > >() { return VariantType::VR_STD_VECTOR_LONG; };
	template <> inline VariantType variant_traits< std::vector< long long > >() { return VariantType::VR_STD_VECTOR_LONGLONG; };

	template <> inline VariantType variant_traits< std::vector< unsigned char > >() { return VariantType::VR_STD_VECTOR_UCHAR; };
	template <> inline VariantType variant_traits< std::vector< unsigned short > >() { return VariantType::VR_STD_VECTOR_USHORT; };
	template <> inline VariantType variant_traits< std::vector< unsigned int > >() { return VariantType::VR_STD_VECTOR_UINT; };
	template <> inline VariantType variant_traits< std::vector< unsigned long > >() { return VariantType::VR_STD_VECTOR_ULONG; };
	template <> inline VariantType variant_traits< std::vector< unsigned long long > >() { return VariantType::VR_STD_VECTOR_ULONGLONG; };

	template <> inline VariantType variant_traits< std::vector< float > >() { return VariantType::VR_STD_VECTOR_FLOAT; };
	template <> inline VariantType variant_traits< std::vector< double > >() { return VariantType::VR_STD_VECTOR_DOUBLE; };
	template <> inline VariantType variant_traits< std::vector< long double > >() { return VariantType::VR_STD_VECTOR_LONG_DOUBLE; };
	template <> inline VariantType variant_traits< std::vector< Mat4 > >() { return VariantType::VR_STD_VECTOR_FLOAT_MATRIX; };
	template <> inline VariantType variant_traits< std::vector< DMat4 > >() { return VariantType::VR_STD_VECTOR_DOUBLE_MATRIX; };

	template <> inline VariantType variant_traits< std::vector< Vec2 > >() { return VariantType::VR_STD_VECTOR_VEC2; };
	template <> inline VariantType variant_traits< std::vector< Vec3 > >() { return VariantType::VR_STD_VECTOR_VEC3; };
	template <> inline VariantType variant_traits< std::vector< Vec4 > >() { return VariantType::VR_STD_VECTOR_VEC4; };

	template <> inline VariantType variant_traits< std::vector< IVec2 > >() { return VariantType::VR_STD_VECTOR_IVEC2; };
	template <> inline VariantType variant_traits< std::vector< IVec3 > >() { return VariantType::VR_STD_VECTOR_IVEC3; };
	template <> inline VariantType variant_traits< std::vector< IVec4 > >() { return VariantType::VR_STD_VECTOR_IVEC4; };

	template <> inline VariantType variant_traits< std::vector< DVec2 > >() { return VariantType::VR_STD_VECTOR_DVEC2; };
	template <> inline VariantType variant_traits< std::vector< DVec3 > >() { return VariantType::VR_STD_VECTOR_DVEC3; };
	template <> inline VariantType variant_traits< std::vector< DVec4 > >() { return VariantType::VR_STD_VECTOR_DVEC4; };

	template <> inline VariantType variant_traits<const char*>() { return VariantType::VR_C_STRING; };
	template <> inline VariantType variant_traits<std::string>() { return VariantType::VR_STD_STRING; };
	template <> inline VariantType variant_traits< std::vector< std::string > >() { return VariantType::VR_STD_VECTOR_STRING; };

	template <> inline VariantType variant_traits<void*>() { return VariantType::VR_PTR; };
	//
	class SQUARE_API Variant
	{
	public:

		Variant();

		~Variant();

		Variant(VariantType type);

		Variant(const Variant& in);

		Variant(Variant&& in) noexcept;

		Variant(bool b);

		Variant(char c);

		Variant(short s);

		Variant(int i);

		Variant(long l);

		Variant(long long l);

		Variant(unsigned char uc);

		Variant(unsigned short us);

		Variant(unsigned int ui);

		Variant(unsigned long ul);

		Variant(unsigned long long ull);

		Variant(float f);

		Variant(double d);

		Variant(long double ld);

		Variant(const Vec2& v2);

		Variant(const Vec3& v3);

		Variant(const Vec4& v4);

		Variant(const IVec2& iv2);

		Variant(const IVec3& iv3);

		Variant(const IVec4& iv4);

		Variant(const DVec2& dv2);

		Variant(const DVec3& dv3);

		Variant(const DVec4& dv4);

		Variant(const Quat& quat);

		Variant(const DQuat& dquat);

		Variant(const Mat4& fm);

		Variant(const DMat4& dm);
		
		Variant(const std::vector< char >& v_c);

		Variant(const std::vector< short >& v_s);

		Variant(const std::vector< int >& v_i);

		Variant(const std::vector< long >& v_l);

		Variant(const std::vector< long long >& v_ll);
		
		Variant(const std::vector< unsigned char >& v_uc);

		Variant(const std::vector< unsigned short >& v_us);

		Variant(const std::vector< unsigned int >& v_ui);

		Variant(const std::vector< unsigned long >& v_ul);

		Variant(const std::vector< unsigned long long >& v_ull);

		Variant(const std::vector< float >& v_f);

		Variant(const std::vector< double >& v_d);

		Variant(const std::vector< long double >& v_ld);

		Variant(const std::vector< Mat4 >& v_m4);

		Variant(const std::vector< DMat4 >& v_dm4);

		Variant(const std::vector< Vec2 >& v_v2);

		Variant(const std::vector< Vec3 >& v_v3);

		Variant(const std::vector< Vec4 >& v_v4);

		Variant(const std::vector< IVec2 >& v_iv2);

		Variant(const std::vector< IVec3 >& v_iv3);

		Variant(const std::vector< IVec4 >& v_iv4);

		Variant(const std::vector< DVec2 >& v_dv2);

		Variant(const std::vector< DVec3 >& v_dv3);

		Variant(const std::vector< DVec4 >& v_dv4);

		Variant(const char* c_str);

		Variant(const std::string& str);

		Variant(const std::vector< std::string >& v_str);

		Variant(const Shared< ResourceObject >& resouce);

		template < typename T >
		Variant(const Shared< T >& resouce)
		: Variant(DynamicPointerCast<ResourceObject>(resouce))
		{
		}

		Variant(void* ptr);

		Variant(const VariantRef& ref);

		//cast objects
		template < class T >
		T& get()
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				return *((T*)&m_ptr);
		}
		template < class T >
		const T& get() const
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				return *((T*)&m_ptr);
		}
		template < class T >
		explicit  operator const T& () const
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				 return *((T*)&m_ptr);
		}
        
        //assignament op
		Variant& operator = (const Variant& value);
        //move op
		Variant& operator = (Variant&& in) noexcept;
        
        //to variant ref
        VariantRef as_variant_ref() const;

		//alloc_cast
		template < class T >
		T& get_alloc()
		{
			if (variant_traits<T>() != m_type)
			{
				if (!is_heap_value())
				{
					std::memset(this, 0, sizeof(Variant));
					m_type = VariantType::VR_NONE;
				}
				set_type(variant_traits<T>());
			}
			return *((T*)&m_ptr);
		}
		
		//type
		VariantType get_type() const;

		//query
		bool is_none() const;

		bool is_null() const;

		bool is_heap_value() const;

		//get reference pointer
		void* get_ref() const;

        //move from other variant
		void move_from(Variant&& in);
        
		//copy from other variant
		void copy_from(const Variant& in);

	private:

		//native type
		union alignas(16)
		{
			bool       m_b;
			char       m_c;
			short      m_s;
			int        m_i;
			long       m_l;
			long long  m_ll;

			unsigned char       m_uc;
			unsigned short      m_us;
			unsigned int        m_ui;
			unsigned long       m_ul;
			unsigned long long  m_ull;

			float  m_f;
			double m_d;
			long double m_ld;

			Vec2 m_v2;
			Vec3 m_v3;
			Vec4 m_v4;

			IVec2 m_iv2;
			IVec3 m_iv3;
			IVec4 m_iv4;

			DVec2 m_dv2;
			DVec3 m_dv3;
			DVec4 m_dv4;

			Quat  m_quat;
			DQuat m_dquat;

			void* m_ptr;
		};
		//save type
		VariantType m_type{ VariantType::VR_NONE };
		//set type
		void set_type(VariantType type);
	};

	class SQUARE_API VariantRef
	{
	public:

		VariantRef();
        
		VariantRef(VariantType type, void* ptr);
		
		VariantRef(const bool& b);

		VariantRef(const char& c);

		VariantRef(const short& s);

		VariantRef(const int& i);

		VariantRef(const long& l);

		VariantRef(const long long& l);

		VariantRef(const unsigned char& uc);

		VariantRef(const unsigned short& us);

		VariantRef(const unsigned int& ui);

		VariantRef(const unsigned long& ul);

		VariantRef(const unsigned long long& ull);

		VariantRef(const float& f);

		VariantRef(const double& d);

		VariantRef(const long double& ld);

		VariantRef(const Vec2& v2);

		VariantRef(const Vec3& v3);

		VariantRef(const Vec4& v4);

		VariantRef(const IVec2& iv2);

		VariantRef(const IVec3& iv3);

		VariantRef(const IVec4& iv4);
		
		VariantRef(const DVec2& dv2);

		VariantRef(const DVec3& dv3);

		VariantRef(const DVec4& dv4);

		VariantRef(const Quat& quat);

		VariantRef(const DQuat& dquat);

		VariantRef(const Mat4& fm);

		VariantRef(const DMat4& dm);

		VariantRef(const std::vector< char >& v_c);

		VariantRef(const std::vector< short >& v_s);

		VariantRef(const std::vector< int >& v_i);

		VariantRef(const std::vector< long >& v_l);

		VariantRef(const std::vector< long long >& v_ll);

		VariantRef(const std::vector< unsigned char >& v_uc);

		VariantRef(const std::vector< unsigned short >& v_us);

		VariantRef(const std::vector< unsigned int >& v_ui);

		VariantRef(const std::vector< unsigned long >& v_ul);

		VariantRef(const std::vector< unsigned long long >& v_ull);

		VariantRef(const std::vector< float >& v_f);

		VariantRef(const std::vector< double >& v_d);

		VariantRef(const std::vector< long double >& v_ld);

		VariantRef(const std::vector< Mat4 >& v_fm);

		VariantRef(const std::vector< DMat4 >& v_dm);

		VariantRef(const std::vector< Vec2 >& v_v2);

		VariantRef(const std::vector< Vec3 >& v_v3);

		VariantRef(const std::vector< Vec4 >& v_v4);

		VariantRef(const std::vector< IVec2 >& v_iv2);

		VariantRef(const std::vector< IVec3 >& v_iv3);

		VariantRef(const std::vector< IVec4 >& v_iv4);

		VariantRef(const std::vector< DVec2 >& v_dv2);

		VariantRef(const std::vector< DVec3 >& v_dv3);

		VariantRef(const std::vector< DVec4 >& v_dv4);

		VariantRef(const char* c_str);

		VariantRef(const std::string& str);

		VariantRef(const std::vector< std::string >& v_str);
		
		template < typename T >
		VariantRef(Shared< T >& resouce)
		{
			m_ptr = &DynamicPointerCast<ResourceObject>(resouce);
			m_type = VariantType::VR_RESOURCE;
		}

		VariantRef(const void* ptr);

		//only if is explicit (shadowing the others constructors)
		explicit VariantRef(const Variant& var);

		template < class T >
		T& get()
		{
			return *((T*)m_ptr);
		}

		template < class T >
		const T& get() const
		{
			return *((T*)m_ptr);
		}
		
        template < class T >
        explicit  operator const T& () const
        {
            return *((T*)m_ptr);
        }
        
        template < class T >
        explicit  operator T& ()
        {
            return *((T*)m_ptr);
        }

		VariantType get_type() const;

		template < class T >
		T* get_ptr()
		{
			return (T*)m_ptr;
		}

		template < class T >
		const T* get_ptr() const
		{
			return (T*)m_ptr;
		}

		template < class T >
		std::optional<T*> ptr()
		{
			if (get_type() == variant_traits<T>())
			{
				return get_ptr<T>();
			}
			return {};
		}

		template < class T >
		std::optional<const T*> ptr() const
		{
			if (get_type() == variant_traits<T>())
			{
				return get_ptr<T>();
			}
			return {};
		}
        
		void copy_from(const VariantRef& ref);

	private:

		void*		 m_ptr { nullptr };
		VariantType  m_type{ VariantType::VR_NONE };

	};
}
