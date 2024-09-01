//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
// Windows
#define NOMINMAX
/////////////////////////////////////////////////////////////////////////////
// DEFAULT INCLUDE
#include <string>
#include <sstream>
#include <map>
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <limits>
#include <stdexcept> 
#include <functional>
#include <unordered_map>
/////////////////////////////////////////////////////////////////////////////
// MACRO
#if defined( _MSC_VER )
	#define PACKED( __Declaration__ )\
		__pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
	#define ALIGNED( size, __Declaration__ )\
		__declspec(align(size)) __Declaration__
#else
	#define PACKED( __Declaration__ )\
		__Declaration__ __attribute__((__packed__))
	#define ALIGNED( size, __Declaration__ )\
		__Declaration__ __attribute__ ((aligned(size)))
#endif

/////////////////////////////////////////////////////////////////////////////
// EXPORT/IMPOT
#ifdef _WIN32
	#define DLL_EXPORT __declspec(dllexport)
	#define DLL_IMPORT __declspec(dllimport)
#else 
	#define DLL_EXPORT 
	#define DLL_IMPORT 
#endif
/////////////////////////////////////////////////////////////////////////////
// TYPE OF LIB
#if defined(SQUARE_LIBRARY_EXPORT)
	// inside DLL
	#define SQUARE_API   DLL_EXPORT
#elif defined(SQUARE_LIBRARY_IMPORT)
	// outside DLL
	#define SQUARE_API   DLL_IMPORT
#else 
	// static
	#define SQUARE_API
#endif
/////////////////////////////////////////////////////////////////////////////
// Macro help
#define ConstantBufferStruct  struct alignas(16)
/////////////////////////////////////////////////////////////////////////////
// Export STD libs
#if defined( _WIN32 ) && defined( SQUARE_API )
// Disable warning
#pragma warning(disable: 4251)
// Import commonest class/struct
#include <string>
#include <vector>
// Export string
template class SQUARE_API std::allocator<char>;
template struct SQUARE_API std::char_traits<char>;
template class SQUARE_API std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
// Export vector
template class SQUARE_API std::allocator<int>;
template class SQUARE_API std::vector<int>;
template class SQUARE_API std::allocator<unsigned int>;
template class SQUARE_API std::vector<unsigned int>;
template class SQUARE_API std::allocator< std::string >;
template class SQUARE_API std::vector< std::string >;
#endif
/////////////////////////////////////////////////////////////////////////////
// debug
#include <assert.h>
#if defined(_DEBUG) || defined(DEBUG)
#define square_assert_debug(x) assert(x)
#define square_assert_or_release(x) assert(x)
#define square_assert(x) assert(x)
#else 
#define square_assert_debug(x)
#define square_assert_or_release(x) x
#define square_assert(x) assert(x)
#endif
/////////////////////////////////////////////////////////////////////////////
// TYPES
namespace Square
{
	using int64  = signed long long;
	using int32  = signed int;
	using int16  = signed short;
	using int8   = signed char;
	using uint64 = unsigned long long;
	using uint32 = unsigned int;
	using uint16 = unsigned short;
	using uint8  = unsigned char;
}

namespace Square 
{
	namespace TemplateHelp
	{
		template<typename T>
		struct is_shared_ptr : std::false_type {};

		template<typename T>
		struct is_unique_ptr : std::false_type {};

		template<typename T>
		struct is_weak_ptr : std::false_type {};

		template<typename T>
		struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

		template<typename T>
		struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

		template<typename T>
		struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};
	}

	template <typename T>
	using is_shared_ptr = typename TemplateHelp::is_shared_ptr<typename std::decay<T>::type>::type;

	template <typename T>
	using is_unique_ptr = typename TemplateHelp::is_unique_ptr<typename std::decay<T>::type>::type;

	template <typename T>
	using is_weak_ptr = typename TemplateHelp::is_weak_ptr<typename std::decay<T>::type>::type;

	template <class T>
	class const_reverse_wrapper
	{
		const T& m_ref;
	public:
		const_reverse_wrapper(const T& ref) : m_ref(ref) {}
		auto begin() const -> decltype(this->m_ref.rbegin()){ return m_ref.rbegin(); }
		auto end()   const -> decltype(this->m_ref.rend())  { return m_ref.rend();   }
	};
	  
	template <class T>
	inline const_reverse_wrapper<T> reverse(const T &x)
	{
		return const_reverse_wrapper<T>(x);
	}

	template<typename T, typename... Args>
	inline void init_only_movable_t_vector(std::vector<T>& vector, size_t size, Args&&... args)
	{
		vector.reserve(size);
		for (size_t i = 0; i < size; ++i) vector.emplace_back(std::forward<Args>(args)...);
	}
}
