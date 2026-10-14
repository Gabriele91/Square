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
	//A bump pointer in the chunks of another allocator: a block in a few instructions, its free
	//does nothing, all of them freed at once (reset); its chunks kept for the next ones (its memory:
	//the most it had at once). Not thread safe.
	class SQUARE_API LinearAllocator : public Allocator
	{
	public:
		//chunk_size: its chunks (a bigger block: a chunk of its own)
		explicit LinearAllocator(Allocator* parent, size_t chunk_size = 64 * 1024);
		virtual ~LinearAllocator();

		//no copy
		LinearAllocator(const LinearAllocator&) = delete;
		LinearAllocator& operator=(const LinearAllocator&) = delete;

		//Allocator (free: nothing, see reset)
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;

		//all of its blocks freed (its chunks kept)
		void reset();
		//its chunks not in use given back to its allocator
		void trim();

		//the bytes of its blocks (with their alignments), of its chunks
		size_t used() const;
		size_t capacity() const;

	protected:
		//where it is: its chunk, the offset in it
		struct Marker
		{
			size_t m_chunk{ 0 };
			size_t m_offset{ 0 };
		};
		Marker marker() const { return Marker{ m_chunk, m_offset }; }
		void   rewind(const Marker& marker);

	private:
		struct Chunk
		{
			char*  m_memory{ nullptr };
			size_t m_size{ 0 };
		};
		void* bump(size_t size, size_t align, size_t align_offset);

		Allocator*         m_parent{ nullptr };
		size_t             m_chunk_size{ 0 };
		std::vector<Chunk> m_chunks;
		size_t             m_chunk{ 0 };     //the chunk in use
		size_t             m_offset{ 0 };    //its first free byte
	};
}
