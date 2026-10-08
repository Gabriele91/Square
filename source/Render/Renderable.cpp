//
//  Renderable.cpp
//  Square
//
//  See Renderable.h: static by its transform (the caches of the shadows).
//
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"

namespace Square
{
namespace Render
{
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
