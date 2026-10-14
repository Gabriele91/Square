//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Allocator/FrameAllocator.h"

namespace Square
{
	FrameAllocator::FrameAllocator(Allocator* parent, size_t chunk_size)
	: m_frames{ LinearAllocator(parent, chunk_size), LinearAllocator(parent, chunk_size) }
	{
	}

	FrameAllocator::~FrameAllocator()
	{
	}

	void* FrameAllocator::alloc(size_t size, const char* name, uint8 flag)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_frames[m_current].alloc(size, name, flag);
	}

	void* FrameAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_frames[m_current].alloc(size, name, flag, align, align_offset);
	}

	void FrameAllocator::free(void*, size_t)
	{
	}

	void FrameAllocator::free(void*)
	{
	}

	void FrameAllocator::next_frame()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_current ^= 1;
		m_frames[m_current].reset();
		++m_frame;
	}

	size_t FrameAllocator::used() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_frames[m_current].used();
	}
}
