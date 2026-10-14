/*
 *  Square
 *
 *  Created by Gabriele Di Bari on 14/10/2026.
 *  Copyright (c) 2026 Gabriele Di Bari. All rights reserved.
 *
 *  The standard library of FreeType (the fonts of RmlUi; FT_CONFIG_STANDARD_LIBRARY_H, in place of
 *  freetype/config/ftstdlib.h): the one of FreeType, its memory on the heap of the engine
 *  (Square/Core/Allocator/MemoryC.h)
 */
#ifndef SQUARE_UI_FREETYPECONFIG_H
#define SQUARE_UI_FREETYPECONFIG_H
#include <freetype/config/ftstdlib.h>
#include "Square/Core/Allocator/MemoryC.h"

#undef ft_scalloc
#undef ft_sfree
#undef ft_smalloc
#undef ft_srealloc
#define ft_scalloc   square_memory_calloc
#define ft_sfree     square_memory_free
#define ft_smalloc   square_memory_malloc
#define ft_srealloc  square_memory_realloc

#endif
