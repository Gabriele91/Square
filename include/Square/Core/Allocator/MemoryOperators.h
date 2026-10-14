//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <cstddef>

//The replaceable operators new/delete of a module (an executable, a library) on the heap of the
//engine (Memory.h): every new/delete of its code, of its containers of the STL. They are in
//source/Core/Allocator/MemoryOperators.cpp, built in each module:
// - the library and each module that links it (CMake adds the file to them): Memory directly;
// - the modules that do not link it, built with SQUARE_MEMORY_HOOKED (the drivers of the render,
//   HLSL2ALL): the hooks that the library installs in them (MemoryHooks.h), malloc before.
//On Windows each module has its own new/delete, so every module that shares memory with the
//library (a std::string made in one, freed in the other) has them; on Linux and macOS the ones
//of the library and of the executable are those of the process (the hooked modules: nothing).
namespace Square
{
	namespace MemoryOperators
	{
		//the heap of the module (of the operators)
		void* allocate(std::size_t size, std::size_t align);
		void  deallocate(void* ptr);
		void  deallocate_aligned(void* ptr);
	}
}

#if defined(SQUARE_MEMORY_HOOKED)
#include "Square/Core/Allocator/MemoryHooks.h"
//the function of the hooks of a hooked module (called by the library)
#if defined(_WIN32)
extern "C" __declspec(dllexport) void square_memory_install(const Square::MemoryHooks* hooks);
#else
extern "C" __attribute__((visibility("default"))) void square_memory_install(const Square::MemoryHooks* hooks);
#endif
#endif
