//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"
#include <vector>

namespace Square
{
	//The blocks of one size in the chunks of another allocator, its free blocks in a list: a block
	//in O(1), its memory together (the objects of a type, their nodes: see ObjectPool). Its chunks
	//given back when it is destroyed. Not thread safe.
	class SQUARE_API PoolAllocator : public Allocator
	{
	public:
		//block_size: its blocks (at least a pointer), block_align: their alignment (a power of two)
		PoolAllocator(Allocator* parent, size_t block_size, size_t block_align = default_align, size_t blocks_per_chunk = 64);
		virtual ~PoolAllocator();

		//no copy
		PoolAllocator(const PoolAllocator&) = delete;
		PoolAllocator& operator=(const PoolAllocator&) = delete;

		//Allocator (a block of its size or less, of its alignment or less: else nullptr)
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;

		//its blocks: their size, how many in use, in its chunks
		size_t block_size() const { return m_block_size; }
		size_t used() const { return m_used; }
		size_t capacity() const { return m_capacity; }

	private:
		bool new_chunk();

		Allocator*         m_parent{ nullptr };
		size_t             m_block_size{ 0 };
		size_t             m_block_align{ 0 };
		size_t             m_blocks_per_chunk{ 0 };
		void*              m_free{ nullptr };   //its free blocks (each one: the next one in its first bytes)
		std::vector<void*> m_chunks;
		size_t             m_used{ 0 };
		size_t             m_capacity{ 0 };
	};
}
