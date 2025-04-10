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
    SQUARE_API OBoundingBox obounding_box_from_points(const std::vector< Vec3 >& points);

    SQUARE_API OBoundingBox obounding_box_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);

    template< size_t N >
    inline OBoundingBox obounding_box_from_points(const std::array< Vec3, N >& points)
    {
        return obounding_box_from_points(reinterpret_cast<const unsigned char*>(points.data()), 0, sizeof(Vec3), N);
    }

    SQUARE_API AABoundingBox aabounding_from_points(const std::vector< Vec3 >& points);

    SQUARE_API AABoundingBox aabounding_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);

    template< size_t N >
    inline AABoundingBox aabounding_from_points(const std::array< Vec3, N >& points)
    {
        return aabounding_from_points(reinterpret_cast<const unsigned char*>(points.data()), 0, sizeof(Vec3), N);
    }

    SQUARE_API Sphere sphere_from_points(const std::vector< Vec3 >& points);

    SQUARE_API Sphere sphere_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);

    template< size_t N >
    inline Sphere sphere_from_points(const std::array< Vec3, N >& points)
    {
        return sphere_from_points(reinterpret_cast<const unsigned char*>(points.data()), 0, sizeof(Vec3), N);
    }

    //cast
    SQUARE_API OBoundingBox to_obounding(const AABoundingBox& aabb);
    SQUARE_API OBoundingBox to_obounding(const Sphere& sphere);
    
    SQUARE_API AABoundingBox to_aabounding(const OBoundingBox& obb);
    SQUARE_API AABoundingBox to_aabounding(const Sphere& sphere);
    
    SQUARE_API Sphere to_sphere(const OBoundingBox& obb);
    SQUARE_API Sphere to_sphere(const AABoundingBox& aabb);

}
}
