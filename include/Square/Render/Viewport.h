//
//  Viewport.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Math/Linear.h"
#include "Square/Geometry/Frustum.h"

namespace Square
{
namespace Render
{
    class SQUARE_API Viewport
    {
    public:
        //update
        void viewport(const IVec4& viewport);
        void projection(const Mat4& projection);
        void perspective(float fov, float aspect, float near, float far);
        void ortogonal(float left, float right, float top, float bottom, float near, float far);

        //info
        const Vec2   near_and_far() const;
        const IVec2  viewport_size() const;
        const IVec4& viewport() const;
        const Mat4&  projection() const;
        const Mat4&  projection_inv() const;
        
        //cast
        operator Vec2 () const;
        operator Vec4 () const;
        explicit operator Mat4 () const;

    protected:
        //Attrs
        IVec4    m_viewport;
        Mat4     m_projection;
        Mat4     m_projection_inv;
    };
}
}
