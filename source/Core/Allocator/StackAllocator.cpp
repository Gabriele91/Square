//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Allocator/StackAllocator.h"

namespace Square
{
	StackAllocator::StackAllocator(Allocator* parent, size_t chunk_size)
	: LinearAllocator(parent, chunk_size)
	{
	}

	StackAllocator::~StackAllocator()
	{
	}

	StackAllocator::Marker StackAllocator::marker() const
	{
		return LinearAllocator::marker();
	}

	void StackAllocator::rewind(const Marker& marker)
	{
		LinearAllocator::rewind(marker);
	}

	StackAllocator::Scope::Scope(StackAllocator& stack)
	: m_stack(stack)
	, m_marker(stack.marker())
	{
	}

	StackAllocator::Scope::~Scope()
	{
		m_stack.rewind(m_marker);
	}
}
