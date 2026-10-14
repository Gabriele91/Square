//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"
#include <atomic>
#include <cstdint>

namespace Square
{
	//The heap of the engine: the small blocks (up to 4 KiB) in bins by their size, each bin with
	//its slabs of 64 KiB (a slab: the blocks of one bin), its free blocks in a list (O(1), no
	//header in a block); the bigger blocks from malloc. Its slabs are in a range of addresses
	//reserved at once (their pages committed when a bin needs them, never given back to the
	//system: the memory of a bin is reused by its next blocks), so the bin of a block is found by
	//its address alone, and a pointer out of the range is one of malloc: free takes both (the
	//blocks of the modules out of the heap, of malloc, freed here; their free calls of malloc
	//never get a block of the heap: see MemoryOperators.h). Thread safe (a lock for each bin).
	class SQUARE_API BinnedAllocator : public Allocator
	{
	public:
		//a slab: its blocks of a bin
		static constexpr size_t slab_size = 64 * 1024;
		//the biggest block of the bins (bigger: malloc)
		static constexpr size_t max_bin_size = 4096;
		//the bins: 16..128 (16 apart), ..256 (32), ..512 (64), ..1024 (128), ..2048 (256), ..4096 (512)
		static constexpr size_t bins_count = 28;
		//the addresses reserved by default (not memory: only the pages of the slabs in use are)
		static constexpr size_t default_reserve = sizeof(void*) >= 8 ? size_t(16) << 30 : size_t(256) << 20;

		struct Stats
		{
			size_t m_reserved{ 0 };    //the range of the slabs (bytes)
			size_t m_committed{ 0 };   //the slabs made (bytes)
			size_t m_used{ 0 };        //the blocks in use in the bins (bytes)
			size_t m_blocks{ 0 };      //the blocks in use in the bins
			size_t m_large{ 0 };       //the allocations of malloc, since the start (the big blocks)
		};

		//reserve: the range of the slabs (0: no bins, every block of malloc)
		explicit BinnedAllocator(size_t reserve = default_reserve);
		//its range given back to the system: all of its blocks
		virtual ~BinnedAllocator();

		//no copy
		BinnedAllocator(const BinnedAllocator&) = delete;
		BinnedAllocator& operator=(const BinnedAllocator&) = delete;

		//a block (nullptr when there is no memory): align a power of two; up to 16, malloc's
		void* allocate(size_t size, size_t align = default_align);
		//a block of allocate (align <= 16) or of malloc
		void  deallocate(void* ptr);
		//a block of allocate (align > 16) or of the aligned malloc of the platform
		void  deallocate_aligned(void* ptr);

		//Allocator
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;

		//a block of the bins
		bool owns(const void* ptr) const
		{
			return (uintptr_t(ptr) - uintptr_t(m_base)) < m_reserved;
		}
		//the size of a block of the bins (0: not one of them)
		size_t block_size(const void* ptr) const;
		//its memory now (the fields of a bin read without its lock: about)
		Stats stats() const;

	private:
		struct alignas(64) Bin
		{
			std::atomic<bool> m_lock{ false };
			void*  m_free{ nullptr };     //its free blocks (each one: the next one in its first bytes)
			char*  m_bump{ nullptr };     //its slab being split
			char*  m_end{ nullptr };
			size_t m_size{ 0 };           //its block
			size_t m_used{ 0 };           //its blocks in use
		};
		struct Registry;

		void* bin_allocate(size_t bin);
		void* aligned_bin_allocate(size_t size, size_t align);
		void  bin_deallocate(void* ptr);
		char* new_slab(size_t bin);
		void* registered_allocate(size_t size, size_t align, size_t align_offset);
		bool  registered_deallocate(void* ptr);

		char*               m_base{ nullptr };      //the range of the slabs
		size_t              m_reserved{ 0 };
		size_t              m_slabs{ 0 };           //its slabs
		size_t              m_first_slab{ 0 };      //the first one of the bins (before: m_slab_bin)
		uint8*              m_slab_bin{ nullptr };  //the bin of each slab, + 1 (0: none)
		std::atomic<size_t> m_next_slab{ 0 };
		std::atomic<size_t> m_large{ 0 };
		Bin                 m_bins[bins_count];
		//the blocks with an offset of their alignment (alloc), out of the bins: their block of malloc
		Registry*           m_registry{ nullptr };
		std::atomic<size_t> m_registered{ 0 };
	};
}
