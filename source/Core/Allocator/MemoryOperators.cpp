//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
//  The new/delete of a module on the heap of the engine (see MemoryOperators.h): built in the
//  library, in each module that links it (CMake adds it to them), and with SQUARE_MEMORY_HOOKED in
//  the modules that do not link it (the drivers of the render, HLSL2ALL)
//
#include <new>
#include <atomic>
#include <cstdlib>
#include "Square/Core/Allocator/MemoryOperators.h"
#if defined(SQUARE_MEMORY_HOOKED)
	#if defined(_WIN32)
		#include <malloc.h>
	#endif
#else
	#include "Square/Core/Allocator/Memory.h"
#endif

#if defined(SQUARE_MEMORY_HOOKED)
//the hooks of the heap, installed by the library (malloc/free before: the free of the heap takes
//their blocks too, so the blocks of before are freed by it after)
static std::atomic<const Square::MemoryHooks*> s_memory_hooks{ nullptr };

void square_memory_install(const Square::MemoryHooks* hooks)
{
	s_memory_hooks.store(hooks, std::memory_order_release);
}
#endif

//the heap of the module (its containers: STLHeapAllocator; its new/delete)
namespace Square
{
namespace MemoryOperators
{
#if defined(SQUARE_MEMORY_HOOKED)
	void* allocate(std::size_t size, std::size_t align)
	{
		if (const MemoryHooks* hooks = s_memory_hooks.load(std::memory_order_acquire))
		{
			return hooks->m_allocate(size, align);
		}
	#if defined(_WIN32)
		return align <= 16 ? std::malloc(size ? size : 1) : _aligned_malloc(size ? size : 1, align);
	#else
		void* ptr = nullptr;
		return posix_memalign(&ptr, align < sizeof(void*) ? sizeof(void*) : align, size ? size : 1) == 0 ? ptr : nullptr;
	#endif
	}

	void deallocate(void* ptr)
	{
		if (const MemoryHooks* hooks = s_memory_hooks.load(std::memory_order_acquire))
		{
			hooks->m_deallocate(ptr);
		}
		else
		{
			std::free(ptr);
		}
	}

	void deallocate_aligned(void* ptr)
	{
		if (const MemoryHooks* hooks = s_memory_hooks.load(std::memory_order_acquire))
		{
			hooks->m_deallocate_aligned(ptr);
		}
		else
		{
		#if defined(_WIN32)
			_aligned_free(ptr);
		#else
			std::free(ptr);
		#endif
		}
	}
#else
	void* allocate(std::size_t size, std::size_t align)
	{
		return Memory::allocate(size, align);
	}

	void deallocate(void* ptr)
	{
		Memory::deallocate(ptr);
	}

	void deallocate_aligned(void* ptr)
	{
		Memory::deallocate_aligned(ptr);
	}
#endif
}
}

//on Linux and macOS the new/delete of the library are those of the process: none in a hooked module
#if !defined(SQUARE_MEMORY_HOOKED) || defined(_WIN32)
void* operator new(std::size_t size)
{
	if (void* ptr = Square::MemoryOperators::allocate(size, 16)) return ptr;
	throw std::bad_alloc();
}

void* operator new[](std::size_t size)
{
	if (void* ptr = Square::MemoryOperators::allocate(size, 16)) return ptr;
	throw std::bad_alloc();
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
	return Square::MemoryOperators::allocate(size, 16);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
	return Square::MemoryOperators::allocate(size, 16);
}

void* operator new(std::size_t size, std::align_val_t align)
{
	if (void* ptr = Square::MemoryOperators::allocate(size, std::size_t(align))) return ptr;
	throw std::bad_alloc();
}

void* operator new[](std::size_t size, std::align_val_t align)
{
	if (void* ptr = Square::MemoryOperators::allocate(size, std::size_t(align))) return ptr;
	throw std::bad_alloc();
}

void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return Square::MemoryOperators::allocate(size, std::size_t(align));
}

void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return Square::MemoryOperators::allocate(size, std::size_t(align));
}

void operator delete(void* ptr) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete[](void* ptr) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
	Square::MemoryOperators::deallocate(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}

void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
	Square::MemoryOperators::deallocate_aligned(ptr);
}
#endif
