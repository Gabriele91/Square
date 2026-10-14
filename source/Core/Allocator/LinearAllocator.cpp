//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include <cstdint>
#include "Square/Core/Allocator/LinearAllocator.h"

namespace Square
{
	LinearAllocator::LinearAllocator(Allocator* parent, size_t chunk_size)
	: m_parent(parent)
	, m_chunk_size(chunk_size)
	{
		square_assert(m_parent);
	}

	LinearAllocator::~LinearAllocator()
	{
		for (Chunk& chunk : m_chunks)
		{
			m_parent->free(chunk.m_memory, chunk.m_size);
		}
	}

	void* LinearAllocator::bump(size_t size, size_t align, size_t align_offset)
	{
		square_assert_debug(align && (align & (align - 1)) == 0);
		while (true)
		{
			if (m_chunk < m_chunks.size())
			{
				//in the chunk in use: its byte at the offset aligned
				const Chunk& chunk = m_chunks[m_chunk];
				const uintptr_t start = uintptr_t(chunk.m_memory) + m_offset;
				const uintptr_t block = ((start + align_offset + align - 1) & ~uintptr_t(align - 1)) - align_offset;
				const size_t end = size_t(block - uintptr_t(chunk.m_memory)) + size;
				if (end <= chunk.m_size)
				{
					m_offset = end;
					return reinterpret_cast<void*>(block);
				}
				//the next chunk (the rest of this one unused until the reset)
				++m_chunk;
				m_offset = 0;
				continue;
			}
			//a new chunk (a bigger block: a chunk of its size)
			const size_t chunk_size = std::max(m_chunk_size, size + align + align_offset);
			char* memory = static_cast<char*>(m_parent->alloc(chunk_size, "Square::LinearAllocator::Chunk", ALCT_DEFAULT));
			if (!memory) return nullptr;
			m_chunks.push_back(Chunk{ memory, chunk_size });
		}
	}

	void* LinearAllocator::alloc(size_t size, const char*, uint8)
	{
		return bump(size, default_align, 0);
	}

	void* LinearAllocator::alloc(size_t size, const char*, uint8, uint16 align, uint16 align_offset)
	{
		return bump(size, std::max<size_t>(align, 1), align_offset);
	}

	void LinearAllocator::free(void*, size_t)
	{
	}

	void LinearAllocator::free(void*)
	{
	}

	void LinearAllocator::reset()
	{
		m_chunk = 0;
		m_offset = 0;
	}

	void LinearAllocator::rewind(const Marker& marker)
	{
		square_assert_debug(marker.m_chunk < m_chunk || (marker.m_chunk == m_chunk && marker.m_offset <= m_offset));
		m_chunk = marker.m_chunk;
		m_offset = marker.m_offset;
	}

	void LinearAllocator::trim()
	{
		const size_t keep = m_chunk + (m_offset ? 1 : 0);
		while (m_chunks.size() > keep)
		{
			m_parent->free(m_chunks.back().m_memory, m_chunks.back().m_size);
			m_chunks.pop_back();
		}
	}

	size_t LinearAllocator::used() const
	{
		size_t used = m_offset;
		for (size_t i = 0; i < m_chunk && i < m_chunks.size(); ++i) used += m_chunks[i].m_size;
		return used;
	}

	size_t LinearAllocator::capacity() const
	{
		size_t capacity = 0;
		for (const Chunk& chunk : m_chunks) capacity += chunk.m_size;
		return capacity;
	}
}
