//
//  Renderable .h
//  Square
//
//  Created by Gabriele Di Bari on 08/06/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
	namespace Render
	{
		class Context;
		class Material;
		class Transform;
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

		virtual void draw(Render::Context* render) = 0;

		virtual bool support_culling() const = 0;
		
		virtual const Geometry::OBoundingBox& bounding_box() = 0;

		virtual Weak<Material> material() const = 0;

		virtual Weak<Transform> transform() const = 0;

		bool visible() const { return m_visible; }

		void visible(bool enable)  { m_visible = enable; }

		bool can_draw() const { return visible() && material().lock() && transform().lock(); }
		
	private:

		bool m_visible{ true };
	};
}
}