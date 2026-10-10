//
//  Renderable.cpp
//  Square
//
//  See Renderable.h: static by its transform (the caches of the shadows).
//
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Effect.h"

namespace Square
{
namespace Render
{
	unsigned char Renderable::variant() const
	{
		unsigned char variant = EV_NONE;
		if (instanced())
		{
			variant |= EV_INSTANCED;
		}
		if (skinned())
		{
			variant |= EV_SKINNED;
		}
		return variant;
	}

	bool Renderable::is_static() const
	{
		bool is_static = false;
		if (auto owner = transform().lock())
		{
			is_static = owner->is_static();
		}
		return is_static;
	}
}
}
