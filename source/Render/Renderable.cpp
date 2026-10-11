//
//  Renderable.cpp
//  Square
//
//  See Renderable.h: static by its transform (the caches of the shadows).
//
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Material.h"

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

	bool Renderable::casts_shadow() const
	{
		bool casts = false;
		const size_t count = m_cast_shadow ? materials_count() : 0;
		for (size_t i = 0; i < count && !casts; ++i)
		{
			if (auto material = this->material(i).lock())
			{
				const auto* mask_shadow = material->parameter_by_name("mask_shadow");
				casts = !mask_shadow || mask_shadow->get_float() < 1.0f;
			}
		}
		return casts;
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
