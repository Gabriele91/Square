//
//  Square
//
//  Created by Gabriele Di Bari on 15/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Allocator/MemoryProfiler.h"
#if !defined(SQUARE_RETAIL)
#include <new>
#include <atomic>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#if defined(_WIN32)
	#include <windows.h>
	#include <dbghelp.h>
	#pragma comment(lib, "dbghelp.lib")
#endif

namespace Square
{
namespace AuxMemoryProfiler
{
	static constexpr size_t s_depth = 24;
	static constexpr size_t s_sites_count = size_t(1) << 15;
	static constexpr size_t s_probes = 64;
	//the frames of a site shown
	static constexpr size_t s_shown_frames = 12;

	//a frame of the allocators, of the heap, of the runtime: not of a caller
	static bool is_plumbing(const char* name)
	{
		static const char* const s_names[] =
		{
			"Square::Memory::", "Square::MemoryOperators::", "Square::BinnedAllocator::",
			"Square::DebugAllocator::", "Square::STLHeapAllocator<", "Square::Render::AllocationVK::",
			"Square::AuxMemoryProfiler::", "Square::MemoryProfiler::", "operator new", "square_memory_", "zcalloc", "malloc", "calloc", "realloc"
		};
		for (const char* plumbing : s_names) if (std::strstr(name, plumbing)) return true;
		return false;
	}

	static const char* file_name(const char* path)
	{
		const char* slash = std::max(std::strrchr(path, '\\'), std::strrchr(path, '/'));
		return slash ? slash + 1 : path;
	}
}

	//the blocks recorded: of malloc (the observer of the heap: no block of the heap, no recursion)
	struct MemoryProfiler::Table
	{
		struct Entry
		{
			uint32 m_hash;
			uint16 m_depth;
			size_t m_count;
			size_t m_bytes;
			size_t m_large;
			size_t m_max;
			void*  m_stack[AuxMemoryProfiler::s_depth];
		};

		std::atomic<bool> m_lock{ false };
		size_t m_count{ 0 };
		size_t m_bytes{ 0 };
		size_t m_large{ 0 };
		size_t m_large_bytes{ 0 };
		size_t m_lost{ 0 };
		Entry  m_entries[AuxMemoryProfiler::s_sites_count];

		void lock()   { while (m_lock.exchange(true, std::memory_order_acquire)) std::this_thread::yield(); }
		void unlock() { m_lock.store(false, std::memory_order_release); }

		static Table* make()
		{
			void* memory = std::calloc(1, sizeof(Table));
			return memory ? new (memory) Table() : nullptr;
		}
		static void destroy(Table* table)
		{
			if (!table) return;
			table->~Table();
			std::free(table);
		}
	};

namespace AuxMemoryProfiler
{
	//the table recording (one profiler at once: the heap calls a function, not an object)
	static std::atomic<MemoryProfiler::Table*> s_table{ nullptr };
	//the blocks being recorded: the table read (or freed) when none is
	static std::atomic<unsigned int> s_recording{ 0 };
	static thread_local bool s_inside = false;

	//the table given back by the heap: after the blocks being recorded with it
	static void detach(BinnedAllocator& heap)
	{
		heap.observe(nullptr);
		s_table.store(nullptr);
		while (s_recording.load()) std::this_thread::yield();
	}

	static void record_with(MemoryProfiler::Table* table, size_t size, size_t align);

	static void record(size_t size, size_t align)
	{
		if (s_inside) return;
		s_recording.fetch_add(1);
		if (MemoryProfiler::Table* table = s_table.load())
		{
			s_inside = true;
			record_with(table, size, align);
			s_inside = false;
		}
		s_recording.fetch_sub(1);
	}

	static void record_with(MemoryProfiler::Table* table, size_t size, size_t align)
	{
		void* stack[s_depth];
		unsigned long hash = 0;
		uint16 depth = 0;
	#if defined(_WIN32)
		depth = uint16(CaptureStackBackTrace(1, DWORD(s_depth), stack, &hash));
	#endif
		const bool large = size > BinnedAllocator::max_bin_size || align > BinnedAllocator::default_align;
		table->lock();
		++table->m_count;
		table->m_bytes += size;
		if (large)
		{
			++table->m_large;
			table->m_large_bytes += size;
		}
		if (depth)
		{
			MemoryProfiler::Table::Entry* entry = nullptr;
			for (size_t i = 0; i < s_probes && !entry; ++i)
			{
				MemoryProfiler::Table::Entry& candidate = table->m_entries[(hash + i) & (s_sites_count - 1)];
				if (!candidate.m_count)
				{
					candidate.m_hash = uint32(hash);
					candidate.m_depth = depth;
					std::memcpy(candidate.m_stack, stack, depth * sizeof(void*));
					entry = &candidate;
				}
				else if (candidate.m_hash == uint32(hash) && candidate.m_depth == depth && std::memcmp(candidate.m_stack, stack, depth * sizeof(void*)) == 0)
				{
					entry = &candidate;
				}
			}
			if (entry)
			{
				++entry->m_count;
				entry->m_bytes += size;
				if (large) ++entry->m_large;
				entry->m_max = std::max(entry->m_max, size);
			}
			else
			{
				++table->m_lost;
			}
		}
		table->unlock();
	}

	//the callers of a stack, by their names
	static Vector<String> callers(const MemoryProfiler::Table::Entry& entry)
	{
		Vector<String> frames;
	#if defined(_WIN32)
		HANDLE process = GetCurrentProcess();
		alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + 1024];
		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
		bool caller = false;
		for (uint16 f = 0; f < entry.m_depth && frames.size() < s_shown_frames; ++f)
		{
			const DWORD64 address = DWORD64(entry.m_stack[f]);
			std::memset(buffer, 0, sizeof(buffer));
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = 1023;
			DWORD64 displacement = 0;
			const char* name = SymFromAddr(process, address, &displacement, symbol) ? symbol->Name : "?";
			//the frames of the allocators before the caller: not shown
			if (!caller && is_plumbing(name)) continue;
			caller = true;
			char module[MAX_PATH] = "?";
			HMODULE handle = nullptr;
			if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(address), &handle))
			{
				GetModuleFileNameA(handle, module, MAX_PATH);
			}
			String frame = String(file_name(module)) + "!" + name;
			IMAGEHLP_LINE64 line{};
			line.SizeOfStruct = sizeof(line);
			DWORD line_displacement = 0;
			if (SymGetLineFromAddr64(process, address, &line_displacement, &line))
			{
				frame += String("  ") + file_name(line.FileName) + ":" + std::to_string(line.LineNumber).c_str();
			}
			frames.push_back(std::move(frame));
		}
	#else
		(void)entry;
	#endif
		return frames;
	}
}

	MemoryProfiler::MemoryProfiler(BinnedAllocator& heap)
	: m_heap(heap)
	{
	}

	MemoryProfiler::~MemoryProfiler()
	{
		if (m_recording) AuxMemoryProfiler::detach(m_heap);
		Table::destroy(m_table);
	}

	bool MemoryProfiler::start(unsigned int frames, unsigned int after)
	{
		if (m_recording || m_wait || AuxMemoryProfiler::s_table.load(std::memory_order_acquire)) return false;
		m_frames = std::max(frames, 1u);
		m_wait = after;
		if (!m_wait) attach();
		return true;
	}

	bool MemoryProfiler::recording() const
	{
		return m_recording;
	}

	void MemoryProfiler::attach()
	{
		Table::destroy(m_table);
		m_table = Table::make();
		if (!m_table) return;
		MemoryProfiler::Table* none = nullptr;
		if (!AuxMemoryProfiler::s_table.compare_exchange_strong(none, m_table)) return;
		m_start = m_heap.stats();
		m_frame = 0;
		m_recording = true;
		m_heap.observe(&AuxMemoryProfiler::record);
	}

	bool MemoryProfiler::frame()
	{
		if (m_wait)
		{
			if (--m_wait == 0) attach();
			return false;
		}
		if (!m_recording || ++m_frame < m_frames) return false;
		//the window ended: the blocks being recorded (in other threads) end before the table is read
		AuxMemoryProfiler::detach(m_heap);
		m_recording = false;
		publish();
		Table::destroy(m_table);
		m_table = nullptr;
		return true;
	}

	void MemoryProfiler::publish()
	{
		const BinnedAllocator::Stats end = m_heap.stats();
		const double frames = double(m_frames);
		m_stats = Stats();
		m_stats.m_frames = m_frames;
		m_stats.m_blocks = double(m_table->m_count) / frames;
		m_stats.m_bytes = double(m_table->m_bytes) / frames;
		m_stats.m_large = double(m_table->m_large) / frames;
		m_stats.m_large_bytes = double(m_table->m_large_bytes) / frames;
		m_stats.m_slabs = (end.m_committed - m_start.m_committed) / BinnedAllocator::slab_size;
		m_stats.m_committed = end.m_committed;
		m_stats.m_used = end.m_used;
		m_stats.m_lost = m_table->m_lost;
		//the sites by their blocks
		Vector<const Table::Entry*> entries;
		for (const Table::Entry& entry : m_table->m_entries) if (entry.m_count) entries.push_back(&entry);
		std::sort(entries.begin(), entries.end(), [](const Table::Entry* a, const Table::Entry* b) { return a->m_count > b->m_count; });
		m_sites.clear();
	#if defined(_WIN32)
		HANDLE process = GetCurrentProcess();
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
		const bool symbols = SymInitialize(process, nullptr, TRUE) != FALSE;
	#endif
		for (const Table::Entry* entry : entries)
		{
			Site site;
			site.m_blocks = double(entry->m_count) / frames;
			site.m_bytes = double(entry->m_bytes) / frames;
			site.m_average = entry->m_bytes / entry->m_count;
			site.m_max = entry->m_max;
			site.m_large = entry->m_large;
			m_sites.push_back(std::move(site));
		}
		//the names of the stacks shown by report (the others: their counters only)
		for (size_t i = 0; i < entries.size() && i < 100; ++i)
		{
			m_sites[i].m_stack = AuxMemoryProfiler::callers(*entries[i]);
		}
	#if defined(_WIN32)
		if (symbols) SymCleanup(process);
	#endif
	}

	const MemoryProfiler::Stats& MemoryProfiler::stats() const
	{
		return m_stats;
	}

	const Vector<MemoryProfiler::Site>& MemoryProfiler::sites() const
	{
		return m_sites;
	}

	String MemoryProfiler::report(size_t sites) const
	{
		char line[512];
		String text;
		std::snprintf(line, sizeof(line), "Heap profile: %u frames\n", m_stats.m_frames);
		text += line;
		std::snprintf(line, sizeof(line), "  blocks/frame        %10.1f\n", m_stats.m_blocks);
		text += line;
		std::snprintf(line, sizeof(line), "  bytes/frame         %10.0f\n", m_stats.m_bytes);
		text += line;
		std::snprintf(line, sizeof(line), "  large blocks/frame  %10.1f   (> %zu bytes or aligned: malloc)\n", m_stats.m_large, BinnedAllocator::max_bin_size);
		text += line;
		std::snprintf(line, sizeof(line), "  large bytes/frame   %10.0f\n", m_stats.m_large_bytes);
		text += line;
		std::snprintf(line, sizeof(line), "  slabs committed     %10zu   (of %zu KiB, in the window)\n", m_stats.m_slabs, BinnedAllocator::slab_size / 1024);
		text += line;
		std::snprintf(line, sizeof(line), "  heap committed      %10zu KiB, used %zu KiB\n", m_stats.m_committed / 1024, m_stats.m_used / 1024);
		text += line;
		std::snprintf(line, sizeof(line), "  stacks lost         %10zu\n", m_stats.m_lost);
		text += line;
		for (size_t i = 0; i < m_sites.size() && i < sites; ++i)
		{
			const Site& site = m_sites[i];
			std::snprintf
			(
				line, sizeof(line), "#%zu  %.2f blocks/frame, %.0f bytes/frame, %zu bytes on average, %zu the biggest, %zu large\n",
				i + 1, site.m_blocks, site.m_bytes, site.m_average, site.m_max, site.m_large
			);
			text += line;
			for (const String& frame : site.m_stack) text += "      " + frame + "\n";
		}
		return text;
	}
}
#endif
