//
//  MemoryProfiler.h
//  Square
//
//  The blocks of the heap of the engine, frame by frame (not in the Retail builds): every block
//  of a window of frames recorded by its call stack, the call stacks by their blocks per frame.
//  The blocks of a frame at the steady state are the work of the heap (and of the system, when
//  they are out of the bins or need a new slab) that a game can make zero.
//
//  The Application owns it: SQUARE_HEAP_PROFILE=N (environment) records N frames (600 by
//  default) after the first 300 (the loading), its report in the log (info, -v).
//
//  The stacks of the blocks only on Windows (CaptureStackBackTrace, their names by DbgHelp);
//  elsewhere the counters only.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/BinnedAllocator.h"
#if !defined(SQUARE_RETAIL)

namespace Square
{
	class SQUARE_API MemoryProfiler
	{
	public:
		//the blocks of a call stack, in the window
		struct Site
		{
			double         m_blocks{ 0 };    //per frame
			double         m_bytes{ 0 };     //per frame
			size_t         m_average{ 0 };   //bytes of a block
			size_t         m_max{ 0 };       //bytes of the biggest one
			size_t         m_large{ 0 };     //its blocks out of the bins (malloc)
			Vector<String> m_stack;          //the callers (the frames of the allocators not)
		};
		//the window
		struct Stats
		{
			unsigned int m_frames{ 0 };
			double       m_blocks{ 0 };       //per frame
			double       m_bytes{ 0 };        //per frame
			double       m_large{ 0 };        //per frame: out of the bins (malloc)
			double       m_large_bytes{ 0 };  //per frame
			size_t       m_slabs{ 0 };        //slabs committed in the window (VirtualAlloc/mprotect)
			size_t       m_committed{ 0 };    //the heap at the end (bytes)
			size_t       m_used{ 0 };         //the blocks of the bins in use at the end (bytes)
			size_t       m_lost{ 0 };         //blocks of a stack out of the table (full)
		};

		MemoryProfiler(BinnedAllocator& heap);
		~MemoryProfiler();

		//no copy (the heap calls one profiler)
		MemoryProfiler(const MemoryProfiler&) = delete;
		MemoryProfiler& operator=(const MemoryProfiler&) = delete;

		//a window of frames, after some frames: false when another profiler is recording
		bool start(unsigned int frames, unsigned int after = 0);
		bool recording() const;

		//a frame (the Application, at its start): true when it has published a window
		bool frame();

		//the last window published
		const Stats& stats() const;
		const Vector<Site>& sites() const;
		//the last window as text (its sites first by their blocks)
		String report(size_t sites = 40) const;

		//the blocks recorded (MemoryProfiler.cpp: of malloc, filled by the heap)
		struct Table;

	protected:
		BinnedAllocator& m_heap;
		Table*           m_table{ nullptr };
		unsigned int     m_frames{ 0 };      //of the window
		unsigned int     m_wait{ 0 };        //frames before it
		unsigned int     m_frame{ 0 };       //recorded
		bool             m_recording{ false };
		BinnedAllocator::Stats m_start;
		//published
		Stats            m_stats;
		Vector<Site>     m_sites;

		void attach();
		void publish();
	};
}

#endif
