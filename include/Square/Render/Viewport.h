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
    struct SQUARE_API ProjectionInformation
    {
        // Prospective
        float fov{ 0.0f };
        float aspect{ 0.0f };

        // Ortogonal
        float left{ 0.0f };
        float right{ 0.0f };
        float top{ 0.0f };
        float bottom{ 0.0f };

        // Both
        float near{ 0.0f };
        float far{ 0.0f };

        ProjectionInformation() = default;

        ProjectionInformation(float fov, float aspect, float near, float far)
        : fov(fov), aspect(aspect), left(0.0f), right(0.0f), top(0.0f), bottom(0.0f), near(near), far(far)
        {}
        ProjectionInformation(float left, float right, float top, float bottom, float near, float far)
        : fov(0.0f), aspect(0.0f), left(left), right(right), top(top), bottom(bottom), near(near), far(far)
        {}
    };

    class SQUARE_API Viewport : public BaseObject
    {
    public:
		//Viewport
        SQUARE_OBJECT(Viewport)

        // Default contructor
        Viewport() = default;

        //update
        void viewport(const IVec4& viewport);
        void projection(const Mat4& projection);
        void perspective(float fov, float aspect, float near, float far);
        void ortogonal(float left, float right, float top, float bottom, float near, float far);

        //info
        const Vec2  near_and_far() const;
        const IVec2  viewport_size() const;
        const IVec4& viewport() const;
        const Mat4&  projection() const;
        const Mat4&  projection_inv() const;

        // Prospective values
        const float fov() const;
        const float aspect() const;

        // Ortogonal values
        const float left() const;
        const float right() const;
        const float top() const;
        const float bottom() const;
        
        // Values
        const float near() const;
        const float far() const;

        //cast
        explicit operator const IVec2& () const;
        explicit operator const IVec4& () const;
        explicit operator const Mat4& () const;

    protected:
        //Attrs
        IVec4                    m_viewport;
        Mat4                     m_projection;
        Mat4                     m_projection_inv;
        ProjectionInformation    m_projection_info;
    };
}
}
