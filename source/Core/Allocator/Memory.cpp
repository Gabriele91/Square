//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <new>
#include <cstdlib>
#include <cstring>
#include "Square/Core/Allocator/Memory.h"

namespace Square
{
namespace Memory
{
	//the heap: a pointer to it, read at every block (made once)
	static std::atomic<BinnedAllocator*> s_heap{ nullptr };

	//its range (SQUARE_HEAP=system: none, malloc)
	static size_t heap_reserve()
	{
		const char* heap = std::getenv("SQUARE_HEAP");
		return heap && std::strcmp(heap, "system") == 0 ? 0 : BinnedAllocator::default_reserve;
	}

	static BinnedAllocator* make_heap()
	{
		//never destroyed: its blocks freed until the end of the process
		alignas(BinnedAllocator) static unsigned char s_storage[sizeof(BinnedAllocator)];
		static BinnedAllocator* s_made = new (s_storage) BinnedAllocator(heap_reserve());
		s_heap.store(s_made, std::memory_order_release);
		return s_made;
	}

	static inline BinnedAllocator& the_heap()
	{
		BinnedAllocator* heap = s_heap.load(std::memory_order_acquire);
		return heap ? *heap : *make_heap();
	}

	BinnedAllocator& heap()
	{
		return the_heap();
	}

	void* allocate(size_t size, size_t align)
	{
		return the_heap().allocate(size, align);
	}

	void deallocate(void* ptr)
	{
		the_heap().deallocate(ptr);
	}

	void deallocate_aligned(void* ptr)
	{
		the_heap().deallocate_aligned(ptr);
	}

	void* reallocate(void* ptr, size_t size)
	{
		return the_heap().reallocate(ptr, size);
	}

	const MemoryHooks& hooks()
	{
		static const MemoryHooks s_hooks{ &allocate, &deallocate, &deallocate_aligned };
		return s_hooks;
	}

	BinnedAllocator::Stats stats()
	{
		return the_heap().stats();
	}
}
}

#if defined(_WIN32) && defined(HLSL2ALL_LIBRARY_IMPORT)
//HLSL2ALL (a library of its own): its new/delete on the heap, before the library calls it (its
//containers filled there, freed here, and the other way: MemoryOperators.cpp hooked in it)
extern "C" __declspec(dllimport) void square_memory_install(const Square::MemoryHooks* hooks);
namespace
{
	struct HLSL2ALLMemory
	{
		HLSL2ALLMemory() { square_memory_install(&Square::Memory::hooks()); }
	};
	static HLSL2ALLMemory s_hlsl2all_memory;
}
#endif
