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
#include "Square/Render/ConstantBuffer.h"

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
    CBStruct UniformBufferCamera
    {
        //viewport info
        Vec4 m_viewport;
        Mat4 m_projection;
        //view/model
        Mat4 m_view;
        Mat4 m_model;
        //position
        CBAlignas Vec3 m_position;
        //the time of the world: x its seconds, y the seconds of the frame (the animations of the
        //materials: AnimatedUV.hlsl)
        CBAlignas Vec4 m_time{ 0.0f };
    };
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
        virtual const Mat4& projection() const = 0;
        virtual const Render::Viewport& viewport() const = 0;
        virtual const Geometry::Frustum& frustum() const = 0;
        
        //set values to constant buffer
        virtual void set(UniformBufferCamera* gpubuffer) const = 0;
        
        //enable/diasable
        void enable(bool enable){ m_enable = enable; }
        bool enable() const { return m_enable; }

        //the time of the world it draws (x: seconds, y: the seconds of the frame), set by the
        //drawer before it draws; its buffer has it (UniformBufferCamera::m_time)
        void time(const Vec2& time) { m_time = time; }
        const Vec2& time() const { return m_time; }

    private:
        
        bool m_enable{true};
        Vec2 m_time{ 0.0f, 0.0f };
    };
}
}
