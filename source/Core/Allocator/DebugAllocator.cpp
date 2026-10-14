//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#include <string>
#include "Square/Core/Allocator/DebugAllocator.h"
// Enable verbose allocator in debug mode
#if defined(_DEBUG) && (DEBUG_ALLOCATOR == 1)
	#define DEBUG_ALLOCATOR_SUPER_VERBOSE 1
#endif

namespace Square
{
	DebugAllocator::DebugAllocator(Allocator* allocator, Logger* logger) : m_logger(logger), m_allocator(allocator) {}
	DebugAllocator::~DebugAllocator()
	{
		for (auto& it : m_debug_map)
		{
			m_logger->debug("Memory leak: " + it.second + " ptr: " + to_string(size_t(it.first)));
		}
	}

	void* DebugAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		std::lock_guard _lock(m_mutex);
		void* alloc_memory_ptr = m_allocator->alloc(size, name, flag);
		// Map allocation
		if (alloc_memory_ptr)
		{
			m_debug_map[alloc_memory_ptr] = name;
#if DEBUG_ALLOCATOR_SUPER_VERBOSE
			m_logger->debug("Alloc: " + String(name) + " ptr: " + to_string(size_t(alloc_memory_ptr)));
#endif
		}
		else
		{
			m_logger->debug("Faild allocation: " + String(name));
		}
		return alloc_memory_ptr;
	}

	void* DebugAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		std::lock_guard _lock(m_mutex);
		void* alloc_memory_ptr = m_allocator->alloc(size, name, flag, align, align_offset);
		// Map allocation
		if (alloc_memory_ptr)
		{
			m_debug_map[alloc_memory_ptr] = name;
#if DEBUG_ALLOCATOR_SUPER_VERBOSE
			m_logger->debug("Alloc: " + String(name) + " ptr: " + to_string(size_t(alloc_memory_ptr)));
#endif
		}
		else
		{
			m_logger->debug("Faild allocation: " + String(name));
		}
		return alloc_memory_ptr;
	}

	void  DebugAllocator::free(void* ptr, size_t size)
	{
		std::lock_guard _lock(m_mutex);
		auto ptr_name_it = m_debug_map.find(ptr);
		if (ptr_name_it != m_debug_map.end())
		{
#if DEBUG_ALLOCATOR_SUPER_VERBOSE
			m_logger->debug("Free: " + ptr_name_it->second + " ptr: " + to_string(size_t(ptr)));
#endif
			m_debug_map.erase(ptr_name_it);
		}
		else
			m_logger->debug("Free: UNKNOWN ptr: " + to_string(size_t(ptr)));
		m_allocator->free(ptr,size);
	}

	void  DebugAllocator::free(void* ptr)
	{
		std::lock_guard _lock(m_mutex);
		auto ptr_name_it = m_debug_map.find(ptr);
		if (ptr_name_it != m_debug_map.end())
		{
#if DEBUG_ALLOCATOR_SUPER_VERBOSE
			m_logger->debug("Free: " + ptr_name_it->second + " ptr: " + to_string(size_t(ptr)));
#endif
			m_debug_map.erase(ptr_name_it);
		}
		else
			m_logger->debug("Free: UNKNOWN ptr: " + to_string(size_t(ptr)));
		m_allocator->free(ptr);
	}
}
