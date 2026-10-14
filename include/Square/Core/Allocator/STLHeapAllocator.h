//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <new>
#include <limits>
#include <cstddef>
#include <type_traits>
#include "Square/Core/Allocator/MemoryOperators.h"

namespace Square
{
	//The allocator of the containers of the engine (Core/Container: String, Vector, Map...): the heap
	//of the engine (Memory.h), by the functions of the module (MemoryOperators.h: also in the
	//drivers, which do not link the library). No state: a container as big as with std::allocator,
	//its memory moved and swapped with any other one of the same type.
	template < class T >
	class STLHeapAllocator
	{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using propagate_on_container_move_assignment = std::true_type;
		using is_always_equal = std::true_type;

		STLHeapAllocator() noexcept = default;
		template < class U >
		STLHeapAllocator(const STLHeapAllocator<U>&) noexcept {}

		T* allocate(std::size_t n)
		{
			if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
			void* ptr = MemoryOperators::allocate(n * sizeof(T), alignof(T) > 16 ? alignof(T) : 16);
			if (!ptr) throw std::bad_alloc();
			return static_cast<T*>(ptr);
		}

		void deallocate(T* ptr, std::size_t) noexcept
		{
			if constexpr (alignof(T) > 16) MemoryOperators::deallocate_aligned(ptr);
			else                           MemoryOperators::deallocate(ptr);
		}

		template < class U >
		bool operator == (const STLHeapAllocator<U>&) const noexcept { return true; }
		template < class U >
		bool operator != (const STLHeapAllocator<U>&) const noexcept { return false; }
	};
}
