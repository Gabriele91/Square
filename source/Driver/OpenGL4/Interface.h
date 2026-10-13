//
//  Square
//
//  Created by Gabriele on 29/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Render.h"

extern "C"
{
	//the build of the driver (SQUARE_BUILD_ID: the library loads the drivers of its build only)
	DLL_EXPORT const char* square_render_build();
	DLL_EXPORT Square::Render::RenderDriver square_render_get_type();
	DLL_EXPORT Square::Render::Context* square_render_create_context(Square::Allocator*, Square::Logger*);
	DLL_EXPORT void square_render_delete_context(Square::Render::Context*&);
}