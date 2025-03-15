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
    static ProjectionInformation fill_projection_information(const Mat4& projection)
    {
        ProjectionInformation projection_info;

        // Base values
        const float tan_half_val = 1.0 / projection[1][1];
        const float tan_half_val_time_aspect = 1.0 / projection[0][0];

        // FOV
        projection_info.fov = atan(tan_half_val) * 2.0f;

        // ASPECT
        projection_info.aspect = tan_half_val_time_aspect / tan_half_val;

        // FAR and NEAR
        #if 0 // -1,+1,  RIGHT HAND
            const float m22 = -projection[2][2];
            const float m32 = -projection[3][2];
            projection_info.far  = (2.0f * m32) / (2.0f * m22 - 2.0f);
            projection_info.near = (((m22 - 1.0f) * projection_info.far) / (m22 + 1.0)) * 2.0f;
        #else // 0,+1,  LEFT HAND
            const float m22 = projection[2][2];
            const float m32 = projection[3][2];
            projection_info.near = -m32 / m22; 
            projection_info.far = (m22 * projection_info.near) / (m22 - 1.0f);
        #endif

        return projection_info;
    }

    void Viewport::viewport(const IVec4& viewport)
    {
        m_viewport = viewport;
    }

    void Viewport::projection(const Mat4& projection)
    {
        m_projection = projection;
        m_projection_inv = inverse(m_projection);
        m_projection_info = fill_projection_information(m_projection);
    }

    void Viewport::perspective(float fov, float aspect, float near, float far)
    {
        m_projection      = Square::perspective(fov, aspect, near, far);
        m_projection_inv  = inverse(m_projection);
        m_projection_info = { fov, aspect, near, far };
    }

    void Viewport::ortogonal(float left, float right, float top, float bottom, float near, float far)
    {
        m_projection      = Square::ortho(left,right,top,bottom,near,far);
        m_projection_inv  = inverse(m_projection);
        m_projection_info = { left, right, top, bottom, near, far };
    }
    
    const Vec2 Viewport::near_and_far() const
    {
        return Vec2{ m_projection_info.near, m_projection_info.far };
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

    // Prospective values
    const float Viewport::fov() const
    {
        return m_projection_info.fov;
    }

    const float Viewport::aspect() const
    {
        return m_projection_info.aspect;
    }

    const float Viewport::left() const
    {
        return m_projection_info.left;
    }

    const float Viewport::right() const
    {
        return m_projection_info.right;
    }

    const float Viewport::top() const
    {
        return m_projection_info.top;
    }

    const float Viewport::bottom() const
    {
        return m_projection_info.bottom;
    }

    const float Viewport::near() const
    {
        return m_projection_info.near;
    }

    const float Viewport::far() const
    {
        return m_projection_info.far;
    }

    Viewport::operator const IVec2& () const
    {
        return viewport_size();
    }
    
    Viewport::operator const IVec4& () const
    {
        return viewport();
    }
    
    Viewport::operator const Mat4& () const
    {
        return projection();
    }

}
}
