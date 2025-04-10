//
//  Renderable .h
//  Square
//
//  Created by Gabriele Di Bari on 08/06/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
	namespace Render
	{
		class Context;
		class Material;
		class Transform;
		class EffectPass;
		struct EffectPassInputs;
	}
	namespace Geometry
	{
		class OBoundingBox;
	}
}

namespace Square
{
namespace Render
{
	class SQUARE_API Renderable : public BaseObject
	{
	public:

		SQUARE_OBJECT(Renderable)

		Renderable() {}

		virtual ~Renderable() {};

		virtual size_t materials_count() const = 0;

		virtual Weak<Material> material(size_t material_id = 0) const = 0;

		virtual void draw(
			  Render::Context& render
			, size_t material_id
			, EffectPassInputs& current_input
			, EffectPass& current_pass
		) = 0;

		virtual bool support_culling() const = 0;
        
		virtual const Geometry::OBoundingBox& bounding_box() = 0;

		virtual Weak<Transform> transform() const = 0;
        
		virtual bool visible() const { return m_visible; }

		virtual void visible(bool enable)  { m_visible = enable; }

		virtual bool can_draw() const { return visible() && material().lock() && transform().lock(); }

	private:
        
		bool m_visible{ true };
	};
}
}
