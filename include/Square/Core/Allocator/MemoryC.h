/*
 *  Square
 *
 *  Created by Gabriele Di Bari on 14/10/2026.
 *  Copyright (c) 2026 Gabriele Di Bari. All rights reserved.
 *
 *  The heap of the engine (Memory.h) for the libraries in C inside the library (FreeType: the
 *  configuration of its standard library, Square/UI/FreeTypeConfig.h): malloc, calloc, realloc,
 *  free on it. zlib on it too, by its own functions (zcalloc/zcfree, MY_ZCALLOC).
 */
#ifndef SQUARE_CORE_ALLOCATOR_MEMORYC_H
#define SQUARE_CORE_ALLOCATOR_MEMORYC_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* square_memory_malloc(size_t size);
void* square_memory_calloc(size_t count, size_t size);
void* square_memory_realloc(void* ptr, size_t size);
void  square_memory_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif
