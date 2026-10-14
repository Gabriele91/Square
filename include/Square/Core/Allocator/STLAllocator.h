//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"

namespace Square
{
	//An allocator of the engine for the containers of the STL, e.g. a vector in the memory of the
	//frame: std::vector<T, STLAllocator<T>> list(STLAllocator<T>(context.frame_allocator()))
	template < class T >
	class STLAllocator
	{
	public:
		using value_type = T;

		STLAllocator(Allocator* allocator) noexcept : m_allocator(allocator) {}
		template < class U >
		STLAllocator(const STLAllocator<U>& other) noexcept : m_allocator(other.allocator()) {}

		T* allocate(size_t n)
		{
			void* ptr = alignof(T) <= Allocator::default_align
			          ? m_allocator->alloc(n * sizeof(T), typeid(T).name(), ALCT_DEFAULT)
			          : m_allocator->alloc(n * sizeof(T), typeid(T).name(), ALCT_DEFAULT, uint16(alignof(T)), 0);
			if (!ptr) throw std::bad_alloc();
			return static_cast<T*>(ptr);
		}

		void deallocate(T* ptr, size_t n) noexcept
		{
			m_allocator->free(ptr, n * sizeof(T));
		}

		Allocator* allocator() const noexcept { return m_allocator; }

		template < class U >
		bool operator == (const STLAllocator<U>& other) const noexcept { return m_allocator == other.allocator(); }
		template < class U >
		bool operator != (const STLAllocator<U>& other) const noexcept { return m_allocator != other.allocator(); }

	private:
		Allocator* m_allocator;
	};
}
