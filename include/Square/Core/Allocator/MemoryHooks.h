//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <cstddef>

namespace Square
{
	//The heap of the library (Memory.h) for a module that does not link it (a driver of the
	//render, HLSL2ALL): its new/delete call these functions once the library has installed them
	//(the function square_memory_install the module exports, MemoryOperators.cpp built with
	//SQUARE_MEMORY_HOOKED), malloc/free before. Its free functions take any pointer of malloc too
	//(not of the heap)
	struct MemoryHooks
	{
		void* (*m_allocate)(size_t size, size_t align);   //nullptr when there is no memory
		void  (*m_deallocate)(void* ptr);                 //of m_allocate (align <= 16) or of malloc
		void  (*m_deallocate_aligned)(void* ptr);         //of m_allocate (align > 16) or of the aligned malloc
	};
	//the function a module exports to get them (by its name, square_memory_install)
	using MemoryHooksInstall = void (*)(const MemoryHooks* hooks);
}
