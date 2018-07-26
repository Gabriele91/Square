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

		virtual void draw(Render::Context& render) = 0;

		virtual bool support_culling() const = 0;
        
		virtual const Geometry::OBoundingBox& bounding_box() = 0;

		virtual Weak<Material> material() const = 0;

		virtual Weak<Transform> transform() const = 0;

        virtual Layout::InputLayoutId layout_id() const { return m_layout_input; };
        
		bool visible() const { return m_visible; }

		void visible(bool enable)  { m_visible = enable; }

		bool can_draw() const { return visible() && m_layout_input != ~(size_t(0)) && material().lock() && transform().lock(); }
		
    protected:
        
        virtual void layout_id(Layout::InputLayoutId id) {  m_layout_input = id; };
        
        void layout_id_from_type(unsigned long type)
        {
            layout_id(Layout::Collection::index_by_type(type));
        };
        
        void layout_id_from_object_id(::Square::uint64 id)
        {
            layout_id(Layout::Collection::index_by_object_id(id));
        };
        
        void layout_id_from_object_name(const std::string& name)
        {
            layout_id(Layout::Collection::index_by_object_name(name));
        };

	private:
        
        Layout::InputLayoutId m_layout_input;
		bool m_visible{ true };
	};
}
}
