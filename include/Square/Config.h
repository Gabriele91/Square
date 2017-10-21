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

