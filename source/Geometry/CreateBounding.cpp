//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include  <algorithm>
#include "Square/Config.h"
#include "Square/Geometry/CreateBounding.h"

namespace Square
{
namespace Geometry
{
	OBoundingBox obounding_box_from_covariance_matrix
	(
		Mat3 C,
		const unsigned char* points,
		size_t pos_offset,
		size_t vertex_size,
		size_t size
	)
	{
		Mat3 obounding_box_rotation;
		Vec3 obounding_box_position;
		Vec3 obounding_box_extension;
		// extract the eigenvalues and eigenvectors from C
		Mat3 eigvec;
		// Vec3 eigval =
        eigenvalues_jacobi(C, 100, eigvec);

		// find the right, up and forward vectors from the eigenvectors
		Vec3  r(eigvec[0][0], eigvec[1][0], eigvec[2][0]);
		Vec3  u(eigvec[0][1], eigvec[1][1], eigvec[2][1]);
		Vec3  f(eigvec[0][2], eigvec[1][2], eigvec[2][2]);

		normalize(r);
		normalize(u);
		normalize(f);

		// set the rotation matrix using the eigvenvectors
		obounding_box_rotation[0][0] = r.x; obounding_box_rotation[0][1] = u.x; obounding_box_rotation[0][2] = f.x;
		obounding_box_rotation[1][0] = r.y; obounding_box_rotation[1][1] = u.y; obounding_box_rotation[1][2] = f.y;
		obounding_box_rotation[2][0] = r.z; obounding_box_rotation[2][1] = u.z; obounding_box_rotation[2][2] = f.z;

		// now build the bounding box extents in the rotated frame
		Vec3 minim(1e10, 1e10, 1e10), maxim(-1e10, -1e10, -1e10);

		for (int i = 0; i < (int)size; i++)
		{
			#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
			Vec3 p_prime(dot(r, att_vertex(i)),
						 dot(u, att_vertex(i)),
						 dot(f, att_vertex(i)));
			#undef att_vertex

			minim = min(minim, p_prime);
			maxim = max(maxim, p_prime);
		}

		// set the center of the OBB to be the average of the 
		// minimum and maximum, and the extents be half of the
		// difference between the minimum and maximum
		Vec3 center = (maxim + minim) * 0.5f;
		obounding_box_position[0] = dot(obounding_box_rotation[0], center);
		obounding_box_position[1] = dot(obounding_box_rotation[1], center);
		obounding_box_position[2] = dot(obounding_box_rotation[2], center);
		obounding_box_extension = (maxim - minim) * 0.5f;

		return  OBoundingBox
				(
					obounding_box_rotation,
					obounding_box_position,
					obounding_box_extension
				);
	}

	OBoundingBox obounding_box_from_sequenzial_triangles(const std::vector< Vec3 >& points)
	{
		return obounding_box_from_sequenzial_triangles((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size());
	}

	OBoundingBox obounding_box_from_sequenzial_triangles( const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
	{
		float Ai = 0.0;
		float Am = 0.0;
		Vec3 mu(0.0f, 0.0f, 0.0f), mui;
		Mat3 C;
		float cxx = 0.0, cxy = 0.0, cxz = 0.0, cyy = 0.0, cyz = 0.0, czz = 0.0;

		// loop over the triangles this time to find the
		// mean location
		for (int i = 0; i < (int)n_points; i += 3)
		{
			#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
			const Vec3& p = att_vertex(i + 0);
			const Vec3& q = att_vertex(i + 1);
			const Vec3& r = att_vertex(i + 2);
			#undef att_vertex
			mui = (p + q + r) / 3.0f;

			//compute len( triangle  dir/norm )
			Ai = length(cross((q - p), (r - p))) / 2.0f;

			//inc
			mu += mui*Ai;
			Am += Ai;

			// these bits set the c terms to Am*E[xx], Am*E[xy], Am*E[xz]....
			cxx += (float)((9.0*mui.x*mui.x + p.x*p.x + q.x*q.x + r.x*r.x)*(Ai / 12.0));
			cxy += (float)((9.0*mui.x*mui.y + p.x*p.y + q.x*q.y + r.x*r.y)*(Ai / 12.0));
			cxz += (float)((9.0*mui.x*mui.z + p.x*p.z + q.x*q.z + r.x*r.z)*(Ai / 12.0));
			cyy += (float)((9.0*mui.y*mui.y + p.y*p.y + q.y*q.y + r.y*r.y)*(Ai / 12.0));
			cyz += (float)((9.0*mui.y*mui.z + p.y*p.z + q.y*q.z + r.y*r.z)*(Ai / 12.0));
		}
		// divide out the Am fraction from the average position and 
		// covariance terms
		mu /= Am;
		cxx /= Am; cxy /= Am; cxz /= Am; cyy /= Am; cyz /= Am; czz /= Am;

		// now subtract off the E[x]*E[x], E[x]*E[y], ... terms
		cxx -= mu.x*mu.x; cxy -= mu.x*mu.y; cxz -= mu.x*mu.z;
		cyy -= mu.y*mu.y; cyz -= mu.y*mu.z; czz -= mu.z*mu.z;

		// now build the covariance matrix
		C[0][0] = cxx; C[0][1] = cxy; C[0][2] = cxz;
		C[1][0] = cxy; C[1][1] = cyy; C[1][2] = cyz;
		C[2][0] = cxz; C[2][1] = cyz; C[2][2] = czz;


		// set the OBB parameters from the covariance matrix
		return obounding_box_from_covariance_matrix(C, points, pos_offset, vertex_size, n_points);
	}

	OBoundingBox obounding_box_from_triangles(const std::vector< Vec3 > &points, const std::vector<unsigned int>& triangles)
	{
		return obounding_box_from_triangles((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size(), triangles.data(), triangles.size());
	}

	OBoundingBox obounding_box_from_triangles(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points, const unsigned int* triangles, size_t size)
	{
		float Ai = 0.0;
		float Am = 0.0;
		Vec3 mu(0.0f, 0.0f, 0.0f), mui;
		Mat3 C;
		float cxx = 0.0, cxy = 0.0, cxz = 0.0, cyy = 0.0, cyz = 0.0, czz = 0.0;
		// access
		// loop over the triangles this time to find the
		// mean location
		for (int i = 0; i < (int)size; i += 3)
		{
			#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
			const Vec3& p = att_vertex(triangles[i + 0]);
			const Vec3& q = att_vertex(triangles[i + 1]);
			const Vec3& r = att_vertex(triangles[i + 2]);
			#undef att_vertex
			mui = (p + q + r) / 3.0f;

			//compute len( triangle  dir/norm )
			Ai = length(cross((q - p), (r - p))) / 2.0f;

			//inc
			mu += mui*Ai;
			Am += Ai;

			// these bits set the c terms to Am*E[xx], Am*E[xy], Am*E[xz]....
			cxx += (float)((9.0*mui.x*mui.x + p.x*p.x + q.x*q.x + r.x*r.x)*(Ai / 12.0));
			cxy += (float)((9.0*mui.x*mui.y + p.x*p.y + q.x*q.y + r.x*r.y)*(Ai / 12.0));
			cxz += (float)((9.0*mui.x*mui.z + p.x*p.z + q.x*q.z + r.x*r.z)*(Ai / 12.0));
			cyy += (float)((9.0*mui.y*mui.y + p.y*p.y + q.y*q.y + r.y*r.y)*(Ai / 12.0));
			cyz += (float)((9.0*mui.y*mui.z + p.y*p.z + q.y*q.z + r.y*r.z)*(Ai / 12.0));
		}
		// divide out the Am fraction from the average position and 
		// covariance terms
		mu /= Am;
		cxx /= Am; cxy /= Am; cxz /= Am; cyy /= Am; cyz /= Am; czz /= Am;

		// now subtract off the E[x]*E[x], E[x]*E[y], ... terms
		cxx -= mu.x*mu.x; cxy -= mu.x*mu.y; cxz -= mu.x*mu.z;
		cyy -= mu.y*mu.y; cyz -= mu.y*mu.z; czz -= mu.z*mu.z;

		// now build the covariance matrix
		C[0][0] = cxx; C[0][1] = cxy; C[0][2] = cxz;
		C[1][0] = cxy; C[1][1] = cyy; C[1][2] = cyz;
		C[2][0] = cxz; C[2][1] = cyz; C[2][2] = czz;


		// set the obb parameters from the covariance matrix
		return obounding_box_from_covariance_matrix(C, points, pos_offset, vertex_size, n_points);
	}

	OBoundingBox obounding_box_from_points(const std::vector< Vec3 >& points)
	{
		return obounding_box_from_points((const unsigned char*)points.data(), 0, sizeof(Vec3), points.size());
	}

	OBoundingBox obounding_box_from_points(const unsigned char* points, size_t pos_offset, size_t vertex_size, size_t n_points)
	{
		Vec3 mu(0.0, 0.0, 0.0);
		Mat3 C;

		#define att_vertex(i) (*(const Vec3*)(points+vertex_size*(i)+pos_offset))
		// loop over the points to find the mean point
		// location
		for (int i = 0; i < (int)n_points; i++)
		{
			mu += att_vertex(i) / float(n_points);
		}

		// loop over the points again to build the 
		// covariance matrix.  Note that we only have
		// to build terms for the upper trianglular 
		// portion since the matrix is symmetric
		float cxx = 0.0,
			cxy = 0.0,
			cxz = 0.0,
			cyy = 0.0,
			cyz = 0.0,
			czz = 0.0;

		for (int i = 0; i < (int)n_points; i++)
		{
			const Vec3& p = att_vertex(i);
			cxx += p.x*p.x - mu.x*mu.x;
			cxy += p.x*p.y - mu.x*mu.y;
			cxz += p.x*p.z - mu.x*mu.z;
			cyy += p.y*p.y - mu.y*mu.y;
			cyz += p.y*p.z - mu.y*mu.z;
			czz += p.z*p.z - mu.z*mu.z;
		}
		#undef att_vertex
		// now build the covariance matrix
		C[0][0] = cxx; C[0][1] = cxy; C[0][2] = cxz;
		C[1][0] = cxy; C[1][1] = cyy; C[1][2] = cyz;
		C[2][0] = cxz; C[2][1] = cyz; C[2][2] = czz;

		// set the OBB parameters from the covariance matrix
		return obounding_box_from_covariance_matrix(C, points, pos_offset, vertex_size, n_points);
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
