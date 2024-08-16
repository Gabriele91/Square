//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#include <stdlib.h>
#include <algorithm>
#include "Square/Core/Allocator.h"

#if defined(_DEBUG) || defined(DEBUG)
	#include <unordered_map>
	#include <iostream>
	static std::unordered_map<void*, std::string> s_debug_map;
	#define MEMORY_DEBUG(...) __VA_ARGS__
#else
	#define MEMORY_DEBUG(...)
#endif

namespace Square
{
	static const size_t s_default_align = 16;

#if _WIN32
	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		void* alloc_memory_ptr = _aligned_malloc(size, s_default_align);;
		MEMORY_DEBUG(s_debug_map[alloc_memory_ptr] = name;)
		return alloc_memory_ptr;
	}

	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) 
	{
		void* alloc_memory_ptr = _aligned_offset_malloc(size, align, align_offset);
		MEMORY_DEBUG(s_debug_map[alloc_memory_ptr] = name;)
		return alloc_memory_ptr;
	}

	void  DefaultAllocator::free(void* ptr, size_t size)
	{
		MEMORY_DEBUG(
			auto ptr_name_it = s_debug_map.find(ptr);
			if (ptr_name_it != s_debug_map.end())
				std::cout << "Free: " << ptr_name_it->second << " ptr:" << size_t(ptr) << std::endl;
			else
				std::cout << "Free: UNKNOWN! ptr:" << size_t(ptr) << std::endl;
		)
		_aligned_free(ptr);
	}
	
	void  DefaultAllocator::free(void* ptr)
	{
		MEMORY_DEBUG(
			auto ptr_name_it = s_debug_map.find(ptr);
			if (ptr_name_it != s_debug_map.end())
				std::cout << "Free: " << ptr_name_it->second << " ptr:" << size_t(ptr) << std::endl;
			else
				std::cout << "Free: UNKNOWN! ptr:" << size_t(ptr) << std::endl;
		)
		_aligned_free(ptr);
	}
#else
	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		return aligned_alloc(size, s_default_align);
	}

	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		return aligned_alloc(size, align, align_offset);
	}

	void  DefaultAllocator::free(void* ptr, size_t size)
	{
		free(ptr);
	}

	void  DefaultAllocator::free(void* ptr)
	{
		free(ptr);
	}
#endif
}