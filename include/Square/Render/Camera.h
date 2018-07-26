//
//  Viewport.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"

namespace Square
{
namespace Render
{
    class Viewport;
}
namespace Geometry
{
    class Frustum;
}
}

namespace Square
{
namespace Render
{
    //Camera uniform buffer
    PACKED(ConstantBufferStruct UniformBufferCamera
    {
        //viewport info
        Vec4 m_viewport;
        Mat4 m_projection;
        //view/model
        Mat4 m_view;
        Mat4 m_model;
        //position
        Vec3 m_position; float __PADDING0__; //alignas(16)
    });
    //Camera info
    class SQUARE_API Camera : public BaseObject
    {
    public:
		//Viewport
		SQUARE_OBJECT(Camera)

        //default
        Camera() = default;
        
        //get
        virtual const Mat4& model() const = 0;
        virtual const Mat4& view() const  = 0;
        virtual const Render::Viewport& viewport() const = 0;
        virtual const Geometry::Frustum& frustum() const= 0;
        
        //set values to constant buffer
        virtual void set(UniformBufferCamera* gpubuffer) const = 0;
        
        //enable/diasable
        void enable(bool enable){ m_enable = enable; }
        bool enable() const { return m_enable; }

    private:
        
        bool m_enable{true};
    };
}
}
