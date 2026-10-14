//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"
#include "Square/Core/Logger.h"
#include <mutex>
#include <unordered_map>

namespace Square
{
	//The allocations of another allocator by name: the ones not freed (leaks) in the log, when it
	//is destroyed (DEBUG_ALLOCATOR == 1: every allocation, every free in the log)
	class SQUARE_API DebugAllocator : public Allocator
	{
		Logger* m_logger;
		Allocator* m_allocator;
		std::unordered_map<void*, std::string> m_debug_map;
		std::mutex m_mutex;
	public:
		DebugAllocator(Allocator* , Logger*);
		virtual ~DebugAllocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};
}
