//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <new>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include "Square/Core/Allocator/BinnedAllocator.h"
#if defined(_WIN32)
	#include <windows.h>
	#include <malloc.h>
#else
	#include <sys/mman.h>
#endif
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
	#include <immintrin.h>
	#define SQUARE_CPU_PAUSE() _mm_pause()
#else
	#define SQUARE_CPU_PAUSE() std::this_thread::yield()
#endif

namespace Square
{
namespace AuxBinnedAllocator
{
	static constexpr size_t s_slab_size = BinnedAllocator::slab_size;
	static constexpr size_t s_max_bin_size = BinnedAllocator::max_bin_size;
	static constexpr size_t s_bins_count = BinnedAllocator::bins_count;

	//the block of each bin
	static constexpr size_t s_bin_size[s_bins_count] =
	{
		16, 32, 48, 64, 80, 96, 112, 128,
		160, 192, 224, 256,
		320, 384, 448, 512,
		640, 768, 896, 1024,
		1280, 1536, 1792, 2048,
		2560, 3072, 3584, 4096
	};
	static_assert(s_bin_size[s_bins_count - 1] == s_max_bin_size, "the last bin: the biggest block");

	//the bin of a size, by (size + 15) / 16
	struct BinTable
	{
		uint8 m_bin[s_max_bin_size / 16 + 1];

		constexpr BinTable() : m_bin{}
		{
			size_t bin = 0;
			for (size_t i = 0; i <= s_max_bin_size / 16; ++i)
			{
				while (s_bin_size[bin] < i * 16) ++bin;
				m_bin[i] = uint8(bin);
			}
		}
	};
	static constexpr BinTable s_bin_of{};

	static inline size_t bin_of(size_t size)
	{
		return s_bin_of.m_bin[(size + 15) >> 4];
	}

	//a lock of a few instructions (a bin: held for a few instructions)
	class SpinLock
	{
	public:
		explicit SpinLock(std::atomic<bool>& lock) : m_lock(lock)
		{
			unsigned int spins = 0;
			while (m_lock.exchange(true, std::memory_order_acquire))
			{
				while (m_lock.load(std::memory_order_relaxed))
				{
					if (++spins < 64) SQUARE_CPU_PAUSE();
					else              std::this_thread::yield();
				}
			}
		}
		~SpinLock()
		{
			m_lock.store(false, std::memory_order_release);
		}
	private:
		std::atomic<bool>& m_lock;
	};

	//the addresses of the slabs (aligned to a slab), their pages later
	static char* reserve_addresses(size_t size)
	{
	#if defined(_WIN32)
		//aligned to 64 KiB (the granularity of the allocations of Windows)
		static_assert(s_slab_size == 64 * 1024, "VirtualAlloc: a slab aligned to its 64 KiB");
		return static_cast<char*>(VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS));
	#else
		//a slab more, its start rounded up to a slab (the rest given back)
		int flags = MAP_PRIVATE | MAP_ANONYMOUS;
		#if defined(MAP_NORESERVE)
		flags |= MAP_NORESERVE;
		#endif
		void* ptr = mmap(nullptr, size + s_slab_size, PROT_NONE, flags, -1, 0);
		if (ptr == MAP_FAILED) return nullptr;
		const uintptr_t start = uintptr_t(ptr);
		const uintptr_t aligned = (start + s_slab_size - 1) & ~uintptr_t(s_slab_size - 1);
		if (aligned > start) munmap(ptr, aligned - start);
		const uintptr_t tail = (start + size + s_slab_size) - (aligned + size);
		if (tail) munmap(reinterpret_cast<void*>(aligned + size), tail);
		return reinterpret_cast<char*>(aligned);
	#endif
	}

	static bool commit_pages(char* ptr, size_t size)
	{
	#if defined(_WIN32)
		return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
	#else
		return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
	#endif
	}

	static void release_addresses(char* ptr, size_t size)
	{
	#if defined(_WIN32)
		VirtualFree(ptr, 0, MEM_RELEASE);
	#else
		munmap(ptr, size);
	#endif
	}

	//the aligned malloc of the platform
	static void* aligned_malloc(size_t size, size_t align)
	{
	#if defined(_WIN32)
		return _aligned_malloc(size ? size : 1, align);
	#else
		void* ptr = nullptr;
		return posix_memalign(&ptr, std::max(align, sizeof(void*)), size ? size : 1) == 0 ? ptr : nullptr;
	#endif
	}

	static void aligned_free(void* ptr)
	{
	#if defined(_WIN32)
		_aligned_free(ptr);
	#else
		std::free(ptr);
	#endif
	}

	//the memory of malloc for the containers of the heap (not of the heap: no recursion)
	template < class T >
	struct MallocAllocator
	{
		using value_type = T;
		MallocAllocator() = default;
		template < class U > MallocAllocator(const MallocAllocator<U>&) noexcept {}
		T* allocate(size_t n)
		{
			if (void* ptr = std::malloc(n * sizeof(T))) return static_cast<T*>(ptr);
			throw std::bad_alloc();
		}
		void deallocate(T* ptr, size_t) noexcept { std::free(ptr); }
		template < class U > bool operator == (const MallocAllocator<U>&) const noexcept { return true; }
		template < class U > bool operator != (const MallocAllocator<U>&) const noexcept { return false; }
	};
}

	//the blocks with an offset of their alignment: their block of malloc
	struct BinnedAllocator::Registry
	{
		using Blocks = std::unordered_map
		<
			  void*, void*
			, std::hash<void*>
			, std::equal_to<void*>
			, AuxBinnedAllocator::MallocAllocator<std::pair<void* const, void*>>
		>;
		std::atomic<bool> m_lock{ false };
		Blocks            m_blocks;
	};

	BinnedAllocator::BinnedAllocator(size_t reserve)
	{
		using namespace AuxBinnedAllocator;
		for (size_t bin = 0; bin < bins_count; ++bin)
		{
			m_bins[bin].m_size = s_bin_size[bin];
		}
		if (void* registry = std::malloc(sizeof(Registry)))
		{
			m_registry = new (registry) Registry();
		}
		//the range (halved until the system gives one, at least 64 MiB; none: malloc)
		reserve &= ~(slab_size - 1);
		while (reserve >= (size_t(64) << 20) && !(m_base = reserve_addresses(reserve)))
		{
			reserve /= 2;
		}
		if (m_base)
		{
			//its first slabs: the bin of each slab (a byte)
			const size_t slabs = reserve / slab_size;
			const size_t first_slab = (slabs + slab_size - 1) / slab_size;
			if (commit_pages(m_base, first_slab * slab_size))
			{
				m_reserved = reserve;
				m_slabs = slabs;
				m_first_slab = first_slab;
				m_slab_bin = reinterpret_cast<uint8*>(m_base);
				m_next_slab.store(first_slab);
			}
			else
			{
				release_addresses(m_base, reserve);
				m_base = nullptr;
			}
		}
	}

	BinnedAllocator::~BinnedAllocator()
	{
		if (m_registry)
		{
			for (auto& block : m_registry->m_blocks) std::free(block.second);
			m_registry->~Registry();
			std::free(m_registry);
		}
		if (m_base)
		{
			AuxBinnedAllocator::release_addresses(m_base, m_reserved);
		}
	}

	char* BinnedAllocator::new_slab(size_t bin)
	{
		//the range used up: the counter stays past its end
		if (m_next_slab.load(std::memory_order_relaxed) >= m_slabs) return nullptr;
		const size_t slab = m_next_slab.fetch_add(1, std::memory_order_relaxed);
		if (slab >= m_slabs) return nullptr;
		char* ptr = m_base + slab * slab_size;
		if (!AuxBinnedAllocator::commit_pages(ptr, slab_size)) return nullptr;
		m_slab_bin[slab] = uint8(bin + 1);
		return ptr;
	}

	void* BinnedAllocator::bin_allocate(size_t index)
	{
		Bin& bin = m_bins[index];
		void* ptr = nullptr;
		{
			AuxBinnedAllocator::SpinLock lock(bin.m_lock);
			if ((ptr = bin.m_free))
			{
				bin.m_free = *static_cast<void**>(ptr);
			}
			else
			{
				if (size_t(bin.m_end - bin.m_bump) < bin.m_size)
				{
					char* slab = new_slab(index);
					if (!slab) return nullptr;
					bin.m_bump = slab;
					bin.m_end = slab + slab_size;
				}
				ptr = bin.m_bump;
				bin.m_bump += bin.m_size;
			}
			++bin.m_used;
		}
	#if defined(SQUARE_DEBUG)
		std::memset(ptr, 0xCD, bin.m_size);
	#endif
		return ptr;
	}

	void* BinnedAllocator::aligned_bin_allocate(size_t size, size_t align)
	{
		//a bin whose blocks are all aligned: its block a multiple of the alignment (its slab
		//aligned to its size); the last bin, 4 KiB, of every alignment up to it
		if (align > max_bin_size || size > max_bin_size) return nullptr;
		const size_t aligned_size = std::max((size + align - 1) & ~(align - 1), align);
		for (size_t bin = AuxBinnedAllocator::bin_of(aligned_size); bin < bins_count; ++bin)
		{
			if (m_bins[bin].m_size % align == 0) return bin_allocate(bin);
		}
		return nullptr;
	}

	void BinnedAllocator::bin_deallocate(void* ptr)
	{
		const size_t slab = (uintptr_t(ptr) - uintptr_t(m_base)) / slab_size;
		const size_t index = size_t(m_slab_bin[slab]) - 1;
		square_assert_debug(slab >= m_first_slab && index < bins_count);
		Bin& bin = m_bins[index];
		square_assert_debug((uintptr_t(ptr) - uintptr_t(m_base + slab * slab_size)) % bin.m_size == 0);
	#if defined(SQUARE_DEBUG)
		std::memset(ptr, 0xDD, bin.m_size);
	#endif
		AuxBinnedAllocator::SpinLock lock(bin.m_lock);
		*static_cast<void**>(ptr) = bin.m_free;
		bin.m_free = ptr;
		--bin.m_used;
	}

	void* BinnedAllocator::allocate(size_t size, size_t align)
	{
		square_assert_debug((align & (align - 1)) == 0);
#if !defined(SQUARE_RETAIL)
		if (Observer observer = m_observer.load(std::memory_order_relaxed))
		{
			observer(size, align);
		}
#endif

		if (align <= default_align)
		{
			if (size <= max_bin_size)
			{
				if (void* ptr = bin_allocate(AuxBinnedAllocator::bin_of(size))) return ptr;
			}
			m_large.fetch_add(1, std::memory_order_relaxed);
			return std::malloc(size ? size : 1);
		}
		if (void* ptr = aligned_bin_allocate(size, align)) return ptr;
		m_large.fetch_add(1, std::memory_order_relaxed);
		return AuxBinnedAllocator::aligned_malloc(size, align);
	}

	void BinnedAllocator::deallocate(void* ptr)
	{
		if (owns(ptr))
		{
			bin_deallocate(ptr);
		}
		else if (ptr)
		{
			if (m_registered.load(std::memory_order_relaxed) && registered_deallocate(ptr)) return;
			std::free(ptr);
		}
	}

	void BinnedAllocator::deallocate_aligned(void* ptr)
	{
		if (owns(ptr))
		{
			bin_deallocate(ptr);
		}
		else if (ptr)
		{
			if (m_registered.load(std::memory_order_relaxed) && registered_deallocate(ptr)) return;
			AuxBinnedAllocator::aligned_free(ptr);
		}
	}

	void* BinnedAllocator::reallocate(void* ptr, size_t size)
	{
		if (!ptr) return allocate(size);
		if (owns(ptr))
		{
			//its block big enough: the same one
			const size_t old_size = block_size(ptr);
			if (size <= old_size) return ptr;
			void* out = allocate(size);
			if (!out) return nullptr;
			std::memcpy(out, ptr, old_size);
			bin_deallocate(ptr);
			return out;
		}
		//a block of malloc (not one with an offset of its alignment: alloc)
		square_assert_debug(!m_registered.load(std::memory_order_relaxed) || m_registry->m_blocks.find(ptr) == m_registry->m_blocks.end());
#if !defined(SQUARE_RETAIL)
		if (Observer observer = m_observer.load(std::memory_order_relaxed))
		{
			observer(size, default_align);
		}
#endif
		return std::realloc(ptr, size ? size : 1);
	}

	void* BinnedAllocator::registered_allocate(size_t size, size_t align, size_t align_offset)
	{
		if (!m_registry) return nullptr;
#if !defined(SQUARE_RETAIL)
		if (Observer observer = m_observer.load(std::memory_order_relaxed))
		{
			observer(size + align_offset, align);
		}
#endif
		align = std::max(align, default_align);
		char* raw = static_cast<char*>(std::malloc(size + align + align_offset));
		if (!raw) return nullptr;
		//the block: its byte at the offset aligned
		char* ptr = reinterpret_cast<char*>(((uintptr_t(raw) + align_offset + align - 1) & ~uintptr_t(align - 1)) - align_offset);
		try
		{
			AuxBinnedAllocator::SpinLock lock(m_registry->m_lock);
			m_registry->m_blocks.emplace(ptr, raw);
		}
		catch (...)
		{
			std::free(raw);
			return nullptr;
		}
		m_registered.fetch_add(1, std::memory_order_relaxed);
		m_large.fetch_add(1, std::memory_order_relaxed);
		return ptr;
	}

	bool BinnedAllocator::registered_deallocate(void* ptr)
	{
		void* raw = nullptr;
		{
			AuxBinnedAllocator::SpinLock lock(m_registry->m_lock);
			auto block = m_registry->m_blocks.find(ptr);
			if (block == m_registry->m_blocks.end()) return false;
			raw = block->second;
			m_registry->m_blocks.erase(block);
		}
		m_registered.fetch_sub(1, std::memory_order_relaxed);
		std::free(raw);
		return true;
	}

	void* BinnedAllocator::alloc(size_t size, const char*, uint8)
	{
		return allocate(size, default_align);
	}

	void* BinnedAllocator::alloc(size_t size, const char*, uint8, uint16 align, uint16 align_offset)
	{
		//free has no alignment: out of the bins, the blocks aligned are in the registry
		if (align_offset == 0)
		{
			if (align <= default_align) return allocate(size, default_align);
			if (void* ptr = aligned_bin_allocate(size, align)) return ptr;
		}
		return registered_allocate(size, align, align_offset);
	}

	void BinnedAllocator::free(void* ptr, size_t)
	{
		deallocate(ptr);
	}

	void BinnedAllocator::free(void* ptr)
	{
		deallocate(ptr);
	}

	size_t BinnedAllocator::block_size(const void* ptr) const
	{
		if (!owns(ptr)) return 0;
		const size_t slab = (uintptr_t(ptr) - uintptr_t(m_base)) / slab_size;
		const size_t index = size_t(m_slab_bin[slab]);
		return index ? m_bins[index - 1].m_size : 0;
	}

	BinnedAllocator::Stats BinnedAllocator::stats() const
	{
		Stats stats;
		stats.m_reserved = m_reserved;
		stats.m_committed = std::min(m_next_slab.load(std::memory_order_relaxed), m_slabs) * slab_size;
		for (const Bin& bin : m_bins)
		{
			stats.m_blocks += bin.m_used;
			stats.m_used += bin.m_used * bin.m_size;
		}
		stats.m_large = m_large.load(std::memory_order_relaxed);
		return stats;
	}
}
