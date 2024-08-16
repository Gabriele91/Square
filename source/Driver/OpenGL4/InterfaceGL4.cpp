#include "Interface.h"
#include "RenderGL4.h"

Square::Render::RenderDriver square_render_get_type()
{
	return Square::Render::DR_OPENGL;
}
Square::Render::Context* square_render_create_context(Square::Allocator* allocator)
{
	return new Square::Render::ContextGL4(allocator);
}
void square_render_delete_context(Square::Render::Context*& ctx)
{
	delete (Square::Render::ContextGL4*)ctx;
	ctx = nullptr;
}
