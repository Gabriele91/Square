//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Square
{
namespace Geometry
{
    class SQUARE_API Frustum
    {
    public:
        
        //planes
        enum PlaneType
        {
            TOP = 0,
            BOTTOM,
            LEFT,
            RIGHT,
            NEARP,
            FARP,
            N_PLANES
        };
        
        //results
        enum TestingResult { OUTSIDE, INTERSECT, INSIDE };
        
        //update planes
        void update_frustum(const Mat4& projection);
        
        //distance
        float distance_from_plane(PlaneType plane, const Vec3& point) const;
        float distance_from_near_plane(const Vec3& point) const;
        
        //get plane
        const Vec4& plane(const PlaneType& type) const
        {
            return m_planes[type];
        }
        
    private:
        
        Vec4 m_planes[N_PLANES];
        
    };
}
}
