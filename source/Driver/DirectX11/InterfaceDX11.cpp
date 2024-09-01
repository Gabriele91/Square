#include "Interface.h"
#include "RenderDX11.h"

Square::Render::RenderDriver square_render_get_type()
{
	return Square::Render::DR_DIRECTX;
}
Square::Render::Context* square_render_create_context(Square::Allocator* allocator, Square::Logger* logger)
{
	return new Square::Render::ContextDX11(allocator, logger);
}
void square_render_delete_context(Square::Render::Context*& ctx)
{
	delete (Square::Render::ContextDX11*)ctx;
	ctx = nullptr;
}
