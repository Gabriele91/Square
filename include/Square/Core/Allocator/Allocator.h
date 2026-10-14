//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include <new>
#include <cstddef>
#include <typeinfo>

namespace Square
{
	enum AllocType : uint8
	{
		ALCT_DEFAULT,
		ALCT_PERSISTENT,
		ALCT_SHORT
	};

	//The interface of the allocators of the engine. Its implementations (Core/Allocator/):
	// - SystemAllocator:  the allocator of the C runtime (malloc/free)
	// - BinnedAllocator:  the heap of the engine (its bins by size, its slabs): every new/delete of
	//                     the library and of its games (Memory.h, MemoryOperators.h)
	// - LinearAllocator:  a bump pointer, its memory freed all at once (reset)
	// - StackAllocator:   a linear one freed back to a marker (LIFO scopes)
	// - FrameAllocator:   two linear ones, the memory of a frame lives until the end of the next one
	// - PoolAllocator:    the blocks of one size, in O(1) (ObjectPool<T>: the objects of a type)
	// - DebugAllocator:   the allocations of another one, their leaks in the log
	// STLAllocator<T>: one of them for the containers of the STL
	class SQUARE_API Allocator
	{
	public:
		//the alignment of alloc without alignment (the one of malloc on the 64 bit platforms)
		static constexpr size_t default_align = 16;

		virtual ~Allocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) = 0;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) = 0;
		virtual void  free(void* ptr, size_t size) = 0;
		virtual void  free(void* ptr) = 0;
	};

	#define SQ_NEW(allocator, T, ...) new ((allocator)->alloc(sizeof(T), typeid(T).name(), __VA_ARGS__))
	#define SQ_DELETE(allocator, T, ptr) { (ptr)->~T(); (allocator)->free(ptr, sizeof(T)); }
	#define SQ_DELETE_NAMESPACE(allocator, NAMESPACE, T, ptr) { (ptr)->~T(); (allocator)->free(ptr, sizeof(NAMESPACE::T)); }
}
