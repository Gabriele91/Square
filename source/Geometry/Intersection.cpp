//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Geometry/Intersection.h"

namespace Square
{
namespace Geometry
{
namespace Intersection
{
    
    //plane distance
    static float plane_distance(const Vec4 &plane, const Vec3& point)
    {
        return plane.w + dot((Vec3)plane, point);
    }
    
    //aabb vertex
    static Vec3 get_aabb_positive(const AABoundingBox& in_aabb,const Vec3& plane_normal)
    {
        Vec3 positive = in_aabb.get_min();

        if (plane_normal.x >= 0)
            positive.x = in_aabb.get_max().x;
        if (plane_normal.y >= 0)
            positive.y = in_aabb.get_max().y;
        if (plane_normal.z >= 0)
            positive.z = in_aabb.get_max().z;

        return positive;
    }
    static Vec3 get_aabb_negative(const AABoundingBox& in_aabb, const Vec3& plane_normal)
    {
        Vec3 negative = in_aabb.get_max();

        if (plane_normal.x >= 0)
            negative.x = in_aabb.get_min().x;
        if (plane_normal.y >= 0)
            negative.y = in_aabb.get_min().y;
        if (plane_normal.z >= 0)
            negative.z = in_aabb.get_min().z;

        return negative;
    }
    
    //frustum vs obb
    #if 0
    Result check(const Frustum& in_frustum, const OBoundingBox& in_obb)
    {
        Result result = INSIDE;
        //refs
        const Mat3& rotation = transpose(in_obb.get_rotation_matrix());
        const Vec3& position = in_obb.get_position();
        //axis
        auto& axis_0 = rotation[0];//column(rotation, 0);
        auto& axis_1 = rotation[1];//column(rotation, 1);
        auto& axis_2 = rotation[2];//column(rotation, 2);

        for (int i = 0; i != Frustum::N_PLANES; ++i)
        {
            // plane normal
            const Vec3 norm = ((Vec3)in_frustum.plane((Frustum::plane_type)i));
            // get extension
            const Vec3& ext = in_obb.get_extension();
            // distance
            float dist = ext.x * fabs(dot(axis_0, norm)) +
                ext.y * fabs(dot(axis_1, norm)) +
                ext.z * fabs(dot(axis_2, norm));
            //compute distances
            float distance_box = plane_distance(in_frustum.plane((Frustum::plane_type)i), position);
            //like Sphere
            if (distance_box < -dist) return OUTSIDE;
            else if (distance_box <= dist) result = INTERSECT;
        }
        return result;
    }
    #else
    static float plane_classify(const OBoundingBox& obb, const Vec4& plane)
    {
        //ml plane
        Vec3 normal  = ((Vec3)plane) * obb.get_rotation_matrix();

        // maximum extent in direction of plane normal
        float r = fabsf(obb.get_extension().x * normal.x)
                + fabsf(obb.get_extension().y * normal.y)
                + fabsf(obb.get_extension().z * normal.z);

        // signed distance between box center and plane
        //float d = plane.Test(mCenter);
        float d = plane_distance(plane, obb.get_position());

        // return signed distance
             if (fabsf(d) < r) return 0.0f;
        else if (d < 0.0f)     return d + r;
        else                   return d - r;
    }
    
    Result check(const Frustum& in_frustum, const OBoundingBox& in_obb)
    {
        //result
        Result result = INSIDE;
        //
        for (int i = 0; i != Frustum::N_PLANES; ++i)
        {
            float side = plane_classify(in_obb, in_frustum.plane((Frustum::PlaneType)i));
            //cases
                 if (side <  0.0) return OUTSIDE;
            else if (side == 0.0) result = INTERSECT;
        }
        return result;
    }
    #endif
    //frustum vs aabb
    Result check(const Frustum& in_frustum, const AABoundingBox& in_aabb)
    {
        Result result_check = INSIDE;
        //test all planes
        for (int i = 0; i != Frustum::N_PLANES; ++i)
        {
            //plane
            Vec4 plane = in_frustum.plane((Frustum::PlaneType)i);
            Vec3 pnorm = (Vec3)(plane);

            if (plane_distance(plane, get_aabb_positive(in_aabb, pnorm)) < 0)
                return OUTSIDE;
            else if (result_check != INTERSECT && plane_distance(plane, get_aabb_negative(in_aabb, pnorm)) < 0)
                result_check = INTERSECT;
        }
        return result_check;
    }
    //frustum vs Sphere
    Result check(const Frustum& in_frustum, const Sphere& in_Sphere)
    {
        float center_distance;
        Result result = INSIDE;

        for (int i = 0; i != Frustum::N_PLANES; i++)
        {
            //calc distance
            center_distance = plane_distance(in_frustum.plane((Frustum::PlaneType)i), in_Sphere.get_position());
            //test
            if (center_distance < -in_Sphere.get_radius()) return OUTSIDE;
            else if (center_distance <= in_Sphere.get_radius()) result = INTERSECT;
        }

        return result;
    }
    //frustum vs point
    Result check(const Frustum& in_frustum, const Vec3& in_point)
    {
        for (int i = 0; i != Frustum::N_PLANES; i++)
        {
            if (plane_distance(in_frustum.plane((Frustum::PlaneType)i), in_point) < 0) return OUTSIDE;
        }
        return INSIDE;
    }
    //obb vs Sphere
    Result check(const OBoundingBox& in_obb, const Sphere& in_Sphere)
    {
        // Find the point on this AABB closest to the Sphere center.
        const Vec3 pt = in_obb.closest_point(in_Sphere.get_position());

        //squere distance
        const Vec3  diff = pt - in_Sphere.get_position();
        const float sq_dist = dot(diff, diff);
        //float squere radius
        const float s_radius = in_Sphere.get_radius() * in_Sphere.get_radius();
        //test sq distance
        if (sq_dist <  s_radius) return INSIDE;
        if (sq_dist == s_radius) return INTERSECT;
        return OUTSIDE;
    }
    //obb vs point
    Result check(const OBoundingBox& in_obb, const Vec3& in_point)
    {
        //inv rotation
        const Mat3& inv_rot = transpose(in_obb.get_rotation_matrix());
        //compute relative position
        Vec3 p = in_point * inv_rot - in_obb.get_position();
        //test if point is OUTSIDE with obb
        if (fabs(p.x) < in_obb.get_extension().x &&
            fabs(p.y) < in_obb.get_extension().y &&
            fabs(p.z) < in_obb.get_extension().z)
            return OUTSIDE;
        //test if point is INTERSECT with obb
        if (fabs(p.x) == in_obb.get_extension().x ||
            fabs(p.y) == in_obb.get_extension().y ||
            fabs(p.z) == in_obb.get_extension().z)
            return INTERSECT;
        //or is inside
        return INSIDE;
    }
    //obb vs obb
    Result check(const OBoundingBox& in_obb1, const OBoundingBox& in_obb2)
    {
        Vec3 sizeA = in_obb1.get_extension();
        Vec3 sizeB = in_obb2.get_extension();
        Mat3 rotA = transpose(in_obb1.get_rotation_matrix());
        Mat3 rotB = transpose(in_obb2.get_rotation_matrix());

        float R[3][3];  // Rotation from B to A
        float AR[3][3]; // absolute values of R matrix, to use with box extents
        float ExtentA, ExtentB, Separation;
        int i, k;

        // Calculate B to A rotation matrix
        for (i = 0; i < 3; i++)
        for (k = 0; k < 3; k++)
        {
            R[i][k]  = dot(rotA[i],rotB[k]);
            AR[i][k] = fabs(R[i][k]);
        }

        // Vector separating the centers of Box B and of Box A
        Vec3 vSepWS = in_obb2.get_position() - in_obb1.get_position();
        // Rotated into Box A's coordinates
        Vec3 vSepA(dot(vSepWS,rotA[0]), dot(vSepWS, rotA[1]), dot(vSepWS, rotA[2]));

        // Test if any of A's basis vectors separate the box
        for (i = 0; i < 3; i++)
        {
            ExtentA    = sizeA[i];
            ExtentB    = dot(sizeB,Vec3(AR[i][0], AR[i][1], AR[i][2]));
            Separation = fabs(vSepA[i]);
            if (Separation > ExtentA + ExtentB) return OUTSIDE;
        }

        // Test if any of B's basis vectors separate the box
        for (k = 0; k < 3; k++)
        {
            ExtentA    = dot(sizeA,Vec3(AR[0][k], AR[1][k], AR[2][k]));
            ExtentB    = sizeB[k];
            Separation = fabs(dot(vSepA,Vec3(R[0][k], R[1][k], R[2][k])));

            if (Separation > ExtentA + ExtentB) return OUTSIDE;
        }

        // Now test Cross Products of each basis vector combination ( A[i], B[k] )
        for (i = 0; i<3; i++)
        for (k = 0; k<3; k++)
        {
            int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
            int k1 = (k + 1) % 3, k2 = (k + 2) % 3;
            ExtentA    = sizeA[i1] * AR[i2][k] + sizeA[i2] * AR[i1][k];
            ExtentB    = sizeB[k1] * AR[i][k2] + sizeB[k2] * AR[i][k1];
            Separation = fabs(vSepA[i2] * R[i1][k] - vSepA[i1] * R[i2][k]);
            if (Separation > ExtentA + ExtentB) return OUTSIDE;
        }

        // No separating axis found, the boxes overlap
        return INTERSECT;
    }
    //Sphere vs Sphere
    Result check(const Sphere& in_Sphere1, const Sphere& in_Sphere2)
    {
        //float squere radius
        const float s_radius_1 = in_Sphere1.get_radius() * in_Sphere1.get_radius();
        const float s_radius_2 = in_Sphere2.get_radius() * in_Sphere2.get_radius();
        //radius
        const float sum_radius = s_radius_1 + s_radius_2;
        //squere distance
        const Vec3  diff    = in_Sphere1.get_position() - in_Sphere2.get_position();
        const float sq_dist = dot(diff, diff);
        //test sq distance
        if (sq_dist <= sum_radius)
        {
            if ((std::sqrt(s_radius_1) - std::sqrt(sq_dist)) > std::sqrt(s_radius_2))
            {
                return INSIDE;
            }
            return INTERSECT;
        }
        return OUTSIDE;
    }
    //Sphere vs point
    Result check(const Sphere& in_Sphere, const Vec3& in_point)
    {
        //float squere radius
        const float s_radius = in_Sphere.get_radius() * in_Sphere.get_radius();
        //squere distance
        const Vec3  diff = in_point - in_Sphere.get_position();
        const float sq_dist = dot(diff, diff);
        //test sq distance
        if (sq_dist <  s_radius) return INSIDE;
        if (sq_dist == s_radius) return INTERSECT;
        return OUTSIDE;
    }
}
}
}
