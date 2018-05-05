//
//  Viewport.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Render/Viewport.h"
#include "Square/Math/Transformation.h"

namespace Square
{
namespace Render
{
    void Viewport::viewport(const IVec4& viewport)
    {
        m_viewport = viewport;
    }

    void Viewport::projection(const Mat4& projection)
    {
        m_projection = projection;
        m_projection_inv = inverse(m_projection);
    }

    void Viewport::perspective(float fov, float aspect, float in_near, float in_far)
    {
        m_projection     = Square::perspective(fov, aspect, in_near, in_far);
        m_projection_inv = inverse(m_projection);
    }
    void Viewport::ortogonal(float left, float right, float top, float bottom, float near, float far)
    {
        m_projection     = Square::ortho(left,right,top,bottom,near,far);
        m_projection_inv = inverse(m_projection);
    }
    
    const Vec2 Viewport::near_and_far() const
    {
        float m22 = -m_projection[2][2];
        float m32 = -m_projection[3][2];
        
        float prj_far = (2.0f*m32) / (2.0f*m22 - 2.0f);
        float prj_near = ((m22 - 1.0f)*prj_far) / (m22 + 1.0);
        
        return Vec2(prj_near, prj_far);
    }
    
    const IVec2  Viewport::viewport_size() const
    {
        return IVec2{ m_viewport.z - m_viewport.x, m_viewport.w - m_viewport.y };
    }
    
    const IVec4& Viewport::viewport() const
    {
        return m_viewport;
    }
    
    const Mat4& Viewport::projection() const
    {
        return m_projection;
    }
    
    const Mat4& Viewport::projection_inv() const
    {
        return m_projection_inv;
    }
    
    Viewport::operator Vec2 () const
    {
        return viewport_size();
    }
    
    Viewport::operator Vec4 () const
    {
        return viewport();
    }
    
    Viewport::operator Mat4 () const
    {
        return projection();
    }

}
}
