//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// DEFAULT INCLUDE
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <stdexcept> 

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
#if defined(SQUARE_LIBRARY_EXPORT)
	// inside DLL
	#define SQUARE_API   __declspec(dllexport)
	#define SQUARE_RENDER_API   __declspec(dllexport)
#elif defined(SQUARE_LIBRARY_IMPORT)
	// outside DLL
	#define SQUARE_API   __declspec(dllimport)
	#define SQUARE_RENDER_API   __declspec(dllimport)
#else 
	// static
	#define SQUARE_API
	#define SQUARE_RENDER_API
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

}