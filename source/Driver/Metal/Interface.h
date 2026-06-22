//
//  Interface.h  —  Square Metal driver plugin entry points
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Render.h"

extern "C"
{
    DLL_EXPORT Square::Render::RenderDriver square_render_get_type();
    DLL_EXPORT Square::Render::Context* square_render_create_context(Square::Allocator*, Square::Logger*);
    DLL_EXPORT void square_render_delete_context(Square::Render::Context*&);
}
