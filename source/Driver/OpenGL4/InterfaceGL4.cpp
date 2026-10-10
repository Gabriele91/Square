#include "Interface.h"
#include "RenderGL4.h"

const char* square_render_build()
{
	return SQUARE_BUILD_ID;
}
Square::Render::RenderDriver square_render_get_type()
{
	return Square::Render::DR_OPENGL;
}
Square::Render::Context* square_render_create_context(Square::Allocator* allocator, Square::Logger* logger)
{
	return new Square::Render::ContextGL4(allocator, logger);
}
void square_render_delete_context(Square::Render::Context*& ctx)
{
	delete (Square::Render::ContextGL4*)ctx;
	ctx = nullptr;
}
