//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#include <stdlib.h>
#include <algorithm>
#include "Square/Core/Allocator/SystemAllocator.h"
#if defined(_WIN32)
	#include <malloc.h>
#endif

namespace Square
{
	SystemAllocator::~SystemAllocator() {}

#if _WIN32
	void* SystemAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		return _aligned_malloc(size, default_align);
	}

	void* SystemAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		return _aligned_offset_malloc(size, std::max<size_t>(align, default_align), align_offset);
	}

	void  SystemAllocator::free(void* ptr, size_t size)
	{
		_aligned_free(ptr);
	}

	void  SystemAllocator::free(void* ptr)
	{
		_aligned_free(ptr);
	}
#else
	void* SystemAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		void* ptr = nullptr;
		return posix_memalign(&ptr, default_align, size ? size : 1) == 0 ? ptr : nullptr;
	}

	void* SystemAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		//(the offset of the alignment: not on this platform)
		void* ptr = nullptr;
		return posix_memalign(&ptr, std::max<size_t>(align, default_align), size ? size : 1) == 0 ? ptr : nullptr;
	}

	void  SystemAllocator::free(void* ptr, size_t size)
	{
		::free(ptr);
	}

	void  SystemAllocator::free(void* ptr)
	{
		::free(ptr);
	}
#endif
}
