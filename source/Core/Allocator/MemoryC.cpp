//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <cstring>
#include "Square/Core/Allocator/Memory.h"
#include "Square/Core/Allocator/MemoryC.h"

extern "C"
{
	//the libraries in C (see MemoryC.h)
	void* square_memory_malloc(size_t size)
	{
		return Square::Memory::allocate(size, Square::Allocator::default_align);
	}

	void* square_memory_calloc(size_t count, size_t size)
	{
		const size_t bytes = count * size;
		if (size && bytes / size != count) return nullptr;
		void* ptr = Square::Memory::allocate(bytes, Square::Allocator::default_align);
		if (ptr) std::memset(ptr, 0, bytes);
		return ptr;
	}

	void* square_memory_realloc(void* ptr, size_t size)
	{
		return Square::Memory::reallocate(ptr, size);
	}

	void square_memory_free(void* ptr)
	{
		Square::Memory::deallocate(ptr);
	}

	//zlib (built with MY_ZCALLOC: its streams without their own zalloc/zfree on the heap)
	void* zcalloc(void* opaque, unsigned items, unsigned size)
	{
		(void)opaque;
		return square_memory_calloc(items, size);
	}

	void zcfree(void* opaque, void* ptr)
	{
		(void)opaque;
		square_memory_free(ptr);
	}
}
