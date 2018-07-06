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
    SQUARE_API OBoundingBox obounding_box_from_sequenzial_triangles(const std::vector< Vec3 >& points);

    SQUARE_API OBoundingBox obounding_box_from_sequenzial_triangles(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);

    SQUARE_API OBoundingBox obounding_box_from_triangles(const std::vector< Vec3 > &points, const std::vector<unsigned int>& triangles);

    SQUARE_API OBoundingBox obounding_box_from_triangles(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points, const unsigned int* triangles, size_t size);

    SQUARE_API OBoundingBox obounding_box_from_points(const std::vector< Vec3 >& points);

    SQUARE_API OBoundingBox obounding_box_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);
    
    SQUARE_API AABoundingBox aabounding_from_points(const std::vector< Vec3 >& points);
    
    SQUARE_API AABoundingBox aabounding_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);

    SQUARE_API Sphere sphere_from_points(const std::vector< Vec3 >& points);

    SQUARE_API Sphere sphere_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points);
    
    //cast
    SQUARE_API OBoundingBox to_obounding(const AABoundingBox& aabb);
    SQUARE_API OBoundingBox to_obounding(const Sphere& sphere);
    
    SQUARE_API AABoundingBox to_aabounding(const OBoundingBox& obb);
    SQUARE_API AABoundingBox to_aabounding(const Sphere& sphere);
    
    SQUARE_API Sphere to_sphere(const OBoundingBox& obb);
    SQUARE_API Sphere to_sphere(const AABoundingBox& aabb);

}
}
