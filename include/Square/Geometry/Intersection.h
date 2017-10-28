//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Geometry
{
namespace Intersection
{
    enum Result
    {
        OUTSIDE,
        INTERSECT,
        INSIDE
    };

    //frustum vs obb
    SQUARE_API Result check(const Frustum& in_frustum, const OBoundingBox& in_obb);
    //frustum vs aabb
    SQUARE_API Result check(const Frustum& in_frustum, const AABoundingBox& in_aabb);
    //frustum vs sphere
    SQUARE_API Result check(const Frustum& in_frustum, const Sphere& in_sphere);
    //frustum vs point
    SQUARE_API Result check(const Frustum& in_frustum, const Vec3& in_point);
    //obb vs sphere
    SQUARE_API Result check(const OBoundingBox& in_obb, const Sphere& in_sphere);
    //obb vs point
    SQUARE_API Result check(const OBoundingBox& in_obb, const Vec3& in_point);
    //obb vs obb
    SQUARE_API Result check(const OBoundingBox& in_obb1, const OBoundingBox& in_obb2);
    //sphere vs point
    SQUARE_API Result check(const Sphere& in_sphere, const Vec3& in_point);
    //sphere vs sphere
    SQUARE_API Result check(const Sphere& in_sphere1, const Sphere& in_sphere2);
}
}
}
