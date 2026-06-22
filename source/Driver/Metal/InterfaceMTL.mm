//
//  InterfaceMTL.mm  —  Square Metal driver plugin entry points
//
#include "Interface.h"
#include "RenderMTL.h"

using namespace Square;
using namespace Square::Render;

extern "C"
{
    DLL_EXPORT RenderDriver square_render_get_type()
    {
        return DR_METAL;
    }

    DLL_EXPORT Context* square_render_create_context(Allocator* alloc, Logger* log)
    {
        return new ContextMTL(alloc, log);
    }

    DLL_EXPORT void square_render_delete_context(Context*& ctx)
    {
        delete (Square::Render::ContextMTL*)ctx;
        ctx = nullptr;
    }
}
