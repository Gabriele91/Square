//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/BinnedAllocator.h"
#include "Square/Core/Allocator/MemoryHooks.h"

namespace Square
{
	//The heap of the process (every new/delete of the library, of its drivers, of its games: see
	//MemoryOperators.h), made at its first block, never destroyed (the blocks freed at the end of
	//the process, after the destructors of the library). SQUARE_HEAP=system (environment): no
	//bins, malloc (e.g. for the tools of the memory of the system, or to compare them)
	namespace Memory
	{
		SQUARE_API BinnedAllocator& heap();
		SQUARE_API void* allocate(size_t size, size_t align);
		SQUARE_API void  deallocate(void* ptr);
		SQUARE_API void  deallocate_aligned(void* ptr);
		//the hooks of the heap, for the modules out of the library (see MemoryHooks.h)
		SQUARE_API const MemoryHooks& hooks();
		//the heap now
		SQUARE_API BinnedAllocator::Stats stats();
	}
}
