//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include  <algorithm>
#include "Square/Config.h"
#include "Square/Math/Transformation.h"
#include "Square/Geometry/CreateBounding.h"

namespace Square
{
namespace Geometry
{
	namespace OBBAux
	{
		#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
		// Calculate mean of a set of 3D points
		Vec3 calculate_mean(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
		{
			Vec3 mean(0.0f);
			for (size_t i = 0; i < n_points; ++i)
			{
				mean += att_vertex(i);
			}
			return mean / static_cast<float>(n_points);
		}

		// Calculate covariance matrix for 3D points
		Mat3 calculate_covariance_matrix(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points, const Vec3& mean)
		{
			Mat3 cov(0.0f);
			for (size_t i = 0; i < n_points; ++i)
			{
				Vec3 diff = att_vertex(i) - mean;

				// Outer product: diff * diff^T to build covariance matrix
				cov[0][0] += diff.x * diff.x;
				cov[0][1] += diff.x * diff.y;
				cov[0][2] += diff.x * diff.z;
				cov[1][0] += diff.y * diff.x;
				cov[1][1] += diff.y * diff.y;
				cov[1][2] += diff.y * diff.z;
				cov[2][0] += diff.z * diff.x;
				cov[2][1] += diff.z * diff.y;
				cov[2][2] += diff.z * diff.z;
			}
			return cov / float(n_points);
		}

		// Calculate eigenvectors using power iteration method
		Mat3 calculate_eigenvectors(const Mat3& cov)
		{
			std::vector<Vec3> eigenbasis;
			eigenbasis.reserve(3);

			// Find dominant eigenvector
			Vec3 v1(1.0f);
			for (int iter = 0; iter < 10; ++iter) 
			{
				v1 = normalize(cov * v1);
			}
			eigenbasis.push_back(v1);

			// Find second eigenvector in the plane perpendicular to first
			Vec3 v2;
			if (std::abs(eigenbasis[0].x) > std::abs(eigenbasis[0].y)) 
			{
				v2 = Vec3(0.0f, 1.0f, 0.0f);
			}
			else 
			{
				v2 = Vec3(1.0f, 0.0f, 0.0f);
			}
			v2 = normalize(v2 - glm::dot(v2, eigenbasis[0]) * eigenbasis[0]);

			// Refine second eigenvector
			Mat3 proj = Mat3(1.0f) - glm::outerProduct(eigenbasis[0], eigenbasis[0]);
			for (int iter = 0; iter < 10; ++iter) 
			{
				v2 = normalize(proj * (cov * v2));
			}
			eigenbasis.push_back(v2);

			// Third eigenvector is cross product of first two
			Vec3 v3 = cross(eigenbasis[0], eigenbasis[1]);
			eigenbasis.push_back(normalize(v3));

			// Build rotation matrix from eigenvectors
			Mat3 eigenvectors(1.0f);
			eigenvectors[0] = eigenbasis[0];
			eigenvectors[1] = eigenbasis[1];
			eigenvectors[2] = eigenbasis[2];

			return eigenvectors;
		}

		// Rotate points by a rotation matrix
		std::vector<Vec3> rotate_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points, const Mat3& rot_matrix)
		{
			std::vector<Vec3> rotated_points;
			rotated_points.reserve(n_points);
			for (size_t i = 0; i < n_points; ++i)
			{
				rotated_points.push_back(rot_matrix * att_vertex(i));
			}
			return rotated_points;
		}

		// Find min and max bounds of a set of 3D points
		std::pair<Vec3, Vec3> find_bounds(const std::vector<Vec3>& points) 
		{
			Vec3 min_bound = points[0];
			Vec3 max_bound = points[0];

			for (const auto& p : points) 
			{
				min_bound = (min)(min_bound, p);
				max_bound = (max)(max_bound, p);
			}

			return { min_bound, max_bound };
		}
		#undef att_vertex
	}

	OBoundingBox obounding_box_from_points(const std::vector< Vec3 >& points)
	{
		return obounding_box_from_points((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size());
	}

	OBoundingBox obounding_box_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
	{
		// Calculate mean point
		Vec3 mean = OBBAux::calculate_mean(points, pos_offset, vertex_size, n_points);

		// Calculate covariance matrix
		Mat3 cov = OBBAux::calculate_covariance_matrix(points, pos_offset, vertex_size, n_points, mean);

		// Calculate eigenvectors
		Mat3 eigenvectors = OBBAux::calculate_eigenvectors(cov);
		Mat3 rot_matrix = transpose(eigenvectors);

		// Rotate points to align with principal axes
		std::vector<Vec3> rotated_points = OBBAux::rotate_points(points, pos_offset, vertex_size, n_points, rot_matrix);

		// Find min and max along rotated axes
		auto [min_bound, max_bound] = OBBAux::find_bounds(rotated_points);

		// Rotation
		Mat3 world_rotation = transpose(rot_matrix);

		// Calculate half-sizes and center
		Vec3 half_size = (max_bound - min_bound) * 0.5f;
		Vec3 center = min_bound + half_size;

		// Transform center back to world space
		Vec3 world_center = world_rotation * center;

		// Create and return the OBoundingBox
		return OBoundingBox{ world_rotation, world_center, half_size };
	}

    AABoundingBox aabounding_from_points(const std::vector< Vec3 >& points)
    {
        return aabounding_from_points((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size());
    }
    
    AABoundingBox aabounding_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
    {
        //none
        if(!n_points) return AABoundingBox();
#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
        //min max for aabb
        Vec3  vertex_min = att_vertex(0);
        Vec3  vertex_max = att_vertex(0);
        //compute min/max
        for (size_t i = 1; i < (int)n_points; i++)
        {
            vertex_min.x = std::min(vertex_min.x, att_vertex(i).x);
            vertex_min.y = std::min(vertex_min.y, att_vertex(i).y);
            vertex_min.z = std::min(vertex_min.z, att_vertex(i).z);
            
            vertex_max.x = std::min(vertex_max.x, att_vertex(i).x);
            vertex_max.y = std::min(vertex_max.y, att_vertex(i).y);
            vertex_max.z = std::min(vertex_max.z, att_vertex(i).z);
        }
#undef att_vertex
        return AABoundingBox(vertex_min, vertex_max);
        
    }
    
	Sphere sphere_from_points(const std::vector< Vec3 >& points)
	{
		return sphere_from_points((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size());
	}

	Sphere sphere_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
	{
		Vec3  sphere_position;
		float sphere_radius = 0.0;
		#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
		//compute center
		for (int i = 0; i < (int)n_points; i++) sphere_position += att_vertex(i);
		sphere_position /= n_points;
		//compute radius
		for (int i = 0; i < (int)n_points; i++)
		{
			float d = distance(att_vertex(i), sphere_position);
			sphere_radius  = d > sphere_radius ? d : sphere_radius;
		}
		#undef att_vertex
		return Sphere(sphere_position, sphere_radius);
	}

    //cast
    OBoundingBox to_obounding(const AABoundingBox& aabb)
    {
        return OBoundingBox(Mat3(1), aabb.get_center(), aabb.get_extension());
    }
    OBoundingBox to_obounding(const Sphere& sphere)
    {
        return OBoundingBox(Mat3(1), sphere.get_position(), Vec3(sphere.get_radius()));
    }
    
    AABoundingBox to_aabounding(const OBoundingBox& obb)
    {
        return aabounding_from_points(obb.get_bounding_box()); //too slow
    }
    AABoundingBox to_aabounding(const Sphere& sphere)
    {
        return AABoundingBox(sphere.get_position() - Vec3(sphere.get_radius()),
                             sphere.get_position() + Vec3(sphere.get_radius()));
    }
    
    Sphere to_sphere(const OBoundingBox& obb)
    {
        return sphere_from_points(obb.get_bounding_box()); //too slow
    }
    Sphere to_sphere(const AABoundingBox& aabb)
    {
        auto extension = aabb.get_extension();
        auto radius    = std::max(std::max(extension.x,extension.y), extension.z);
        return Sphere(aabb.get_center(), radius);
    }
    
}
}
