//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Core/Allocator/PoolAllocator.h"

namespace Square
{
	PoolAllocator::PoolAllocator(Allocator* parent, size_t block_size, size_t block_align, size_t blocks_per_chunk)
	: m_parent(parent)
	, m_block_align(std::max(block_align, alignof(void*)))
	, m_blocks_per_chunk(std::max<size_t>(blocks_per_chunk, 1))
	{
		square_assert(m_parent);
		square_assert((m_block_align & (m_block_align - 1)) == 0);
		//a block: a pointer at least (the list of the free ones), a multiple of its alignment
		m_block_size = std::max(block_size, sizeof(void*));
		m_block_size = (m_block_size + m_block_align - 1) & ~(m_block_align - 1);
	}

	PoolAllocator::~PoolAllocator()
	{
		square_assert_debug(m_used == 0);
		for (void* chunk : m_chunks)
		{
			m_parent->free(chunk, m_block_size * m_blocks_per_chunk);
		}
	}

	bool PoolAllocator::new_chunk()
	{
		const size_t size = m_block_size * m_blocks_per_chunk;
		char* chunk = static_cast<char*>
		(
			m_block_align <= default_align
			? m_parent->alloc(size, "Square::PoolAllocator::Chunk", ALCT_DEFAULT)
			: m_parent->alloc(size, "Square::PoolAllocator::Chunk", ALCT_DEFAULT, uint16(m_block_align), 0)
		);
		if (!chunk) return false;
		m_chunks.push_back(chunk);
		//its blocks in the list, the first one at its head
		for (size_t i = m_blocks_per_chunk; i-- > 0;)
		{
			void* block = chunk + i * m_block_size;
			*static_cast<void**>(block) = m_free;
			m_free = block;
		}
		m_capacity += m_blocks_per_chunk;
		return true;
	}

	void* PoolAllocator::alloc(size_t size, const char*, uint8)
	{
		if (size > m_block_size) return nullptr;
		if (!m_free && !new_chunk()) return nullptr;
		void* block = m_free;
		m_free = *static_cast<void**>(block);
		++m_used;
		return block;
	}

	void* PoolAllocator::alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset)
	{
		if (align > m_block_align || align_offset != 0) return nullptr;
		return alloc(size, name, flag);
	}

	void PoolAllocator::free(void* ptr, size_t)
	{
		free(ptr);
	}

	void PoolAllocator::free(void* ptr)
	{
		if (!ptr) return;
		*static_cast<void**>(ptr) = m_free;
		m_free = ptr;
		--m_used;
	}
}
