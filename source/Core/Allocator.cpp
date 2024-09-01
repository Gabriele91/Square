//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#include <stdlib.h>
#include <string>
#include <algorithm>
#include "Square/Core/Allocator.h"


namespace Square
{
	static const size_t s_default_align = 16;
	
#if _WIN32
	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		void* alloc_memory_ptr = _aligned_malloc(size, s_default_align);;
		return alloc_memory_ptr;
	}

	void* DefaultAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) 
	{
		void* alloc_memory_ptr = _aligned_offset_malloc(size, align, align_offset);
		return alloc_memory_ptr;
	}

	void  DefaultAllocator::free(void* ptr, size_t size)
	{
		_aligned_free(ptr);
	}
	
	void  DefaultAllocator::free(void* ptr)
	{
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

	DebugAllocator::DebugAllocator(Allocator* allocator, Logger* logger) : m_allocator(allocator), m_logger(logger) {}

	void* DebugAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		void* alloc_memory_ptr = m_allocator->alloc(size, name, flag);
		// Map allocation
		if (alloc_memory_ptr)
		{
			m_debug_map[alloc_memory_ptr] = name;
			m_logger->debug("Alloc: " + std::string(name) + " ptr: " + std::to_string(size_t(alloc_memory_ptr)));
		}
		else
		{
			m_logger->debug("Faild allocation: " + std::string(name));
		}
		return alloc_memory_ptr;
	}

	void* DebugAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		void* alloc_memory_ptr = m_allocator->alloc(size, name, flag, align, align_offset);
		// Map allocation
		if (alloc_memory_ptr)
		{
			m_debug_map[alloc_memory_ptr] = name;
			m_logger->debug("Alloc: " + std::string(name) + " ptr: " + std::to_string(size_t(alloc_memory_ptr)));
		}
		else
		{
			m_logger->debug("Faild allocation: " + std::string(name));
		}
		return alloc_memory_ptr;
	}

	void  DebugAllocator::free(void* ptr, size_t size)
	{
		auto ptr_name_it = m_debug_map.find(ptr);
		if (ptr_name_it != m_debug_map.end())
			m_logger->debug("Free: " + ptr_name_it->second +  " ptr: " + std::to_string(size_t(ptr)));
		else
			m_logger->debug("Free: UNKNOWN ptr: " + std::to_string(size_t(ptr)));
		m_allocator->free(ptr,size);
	}
	
	void  DebugAllocator::free(void* ptr)
	{
		auto ptr_name_it = m_debug_map.find(ptr);
		if (ptr_name_it != m_debug_map.end())
			m_logger->debug("Free: " + ptr_name_it->second + " ptr: " + std::to_string(size_t(ptr)));
		else
			m_logger->debug("Free: UNKNOWN ptr: " + std::to_string(size_t(ptr)));
		m_allocator->free(ptr);
	}
}