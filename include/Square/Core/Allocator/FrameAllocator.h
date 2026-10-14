//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/LinearAllocator.h"
#include <mutex>

namespace Square
{
	//The memory of a frame: two linear allocators, one for each frame in turn; a block lives until
	//the end of the next frame (next_frame, called once at the start of each frame by the
	//application: the allocator of two frames ago freed). For the lists of a frame, its temporary
	//strings: a block in a few instructions, nothing to free. Thread safe.
	class SQUARE_API FrameAllocator : public Allocator
	{
	public:
		explicit FrameAllocator(Allocator* parent, size_t chunk_size = 256 * 1024);
		virtual ~FrameAllocator();

		//Allocator (free: nothing)
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;

		//a new frame: the blocks of the frame before the last one freed
		void next_frame();
		//the frames since the start
		uint64 frame() const { return m_frame; }
		//the bytes of the blocks of this frame
		size_t used() const;

	private:
		mutable std::mutex m_mutex;
		LinearAllocator    m_frames[2];
		size_t             m_current{ 0 };
		uint64             m_frame{ 0 };
	};
}
