#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Square
{
    //compute tangent per vertex
    template < class T, typename Scalar = float >
    void tangent_model_fast(std::vector< T >& vertices)
    {
        //all to 0
        for (unsigned int i = 0; i < vertices.size(); ++i)
        {
            vertices[i].m_tangent = { 0,0,0 };
            vertices[i].m_bitangent = { 0,0,0 };
        }
        //compute tangent per vertex
        for (unsigned int i = 0; i < vertices.size(); i += 3)
        {

            // Shortcuts for vertices
            TVec3<Scalar> & v0 = vertices[i + 0].m_position;
            TVec3<Scalar> & v1 = vertices[i + 1].m_position;
            TVec3<Scalar> & v2 = vertices[i + 2].m_position;

            // Shortcuts for UVs
            TVec2<Scalar> & uv0 = vertices[i + 0].m_uvmap;
            TVec2<Scalar> & uv1 = vertices[i + 1].m_uvmap;
            TVec2<Scalar> & uv2 = vertices[i + 2].m_uvmap;

            // Edges of the triangle : postion delta
            TVec3<Scalar> edge1 = v1 - v0;
            TVec3<Scalar> edge2 = v2 - v0;

            // UV delta
            TVec2<Scalar> delta_uv1 = uv1 - uv0;
            TVec2<Scalar> delta_uv2 = uv2 - uv0;

            //compute div
            Scalar div = (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);

            //can compute tangent?
            if (div != Scalar(0.0))
            {
                //compute tanget & bitangent
                Scalar r = Scalar(1.0) / div;
                TVec3<Scalar> tangent = (edge1 * delta_uv2.y - edge2 * delta_uv1.y)*r;
                TVec3<Scalar> bitangent = (edge2 * delta_uv1.x - edge1 * delta_uv2.x)*r;

                //add tangent
                vertices[i + 0].m_tangent += tangent;
                vertices[i + 1].m_tangent += tangent;
                vertices[i + 2].m_tangent += tangent;

                //add bitangent
                vertices[i + 0].m_bitangent += bitangent;
                vertices[i + 1].m_bitangent += bitangent;
                vertices[i + 2].m_bitangent += bitangent;
            }

        }

        // See "Going Further"
        for (unsigned int i = 0; i < vertices.size(); ++i)
        {
            TVec3<Scalar>& n = vertices[i].m_normal;
            TVec3<Scalar>& t = vertices[i].m_tangent;
            TVec3<Scalar>& b = vertices[i].m_bitangent;

            //normalize
            t = normalize(t);
            b = normalize(b);

            // Gram-Schmidt orthogonalize
            t = normalize(t - n * dot(n, t));

            // Calculate handedness
            if (dot(cross(n, t), b) < Scalar(0.0))
            {
                t = t * Scalar(-1.0);
            }
        }
    }
    //compute tangent per index
    template < class T, typename Scalar = float >
    void tangent_model_fast(const std::vector<unsigned int>& indexes, std::vector< T >& vertices)
    {
        for (unsigned int i = 0; i < indexes.size(); ++i)
        {
            vertices[indexes[i]].m_tangent = { 0,0,0 };
            vertices[indexes[i]].m_bitangent = { 0,0,0 };
        }
        //compute tangent per vertex
        for (unsigned int i = 0; i < indexes.size(); i += 3)
        {

            // Shortcuts for vertices
            TVec3<Scalar> & v0 = vertices[indexes[i + 0]].m_position;
            TVec3<Scalar> & v1 = vertices[indexes[i + 1]].m_position;
            TVec3<Scalar> & v2 = vertices[indexes[i + 2]].m_position;

            // Shortcuts for UVs
            TVec2<Scalar> & uv0 = vertices[indexes[i + 0]].m_uvmap;
            TVec2<Scalar> & uv1 = vertices[indexes[i + 1]].m_uvmap;
            TVec2<Scalar> & uv2 = vertices[indexes[i + 2]].m_uvmap;

            // Edges of the triangle : postion delta
            TVec3<Scalar> edge1 = v1 - v0;
            TVec3<Scalar> edge2 = v2 - v0;

            // UV delta
            TVec2<Scalar> delta_uv1 = uv1 - uv0;
            TVec2<Scalar> delta_uv2 = uv2 - uv0;

            //compute div
            Scalar div = (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);

            //can compute tangent?
            if (div != Scalar(0.0))
            {
                //compute tanget & bitangent
                Scalar r = Scalar(1.0) / div;
                TVec3<Scalar> tangent = (edge1 * delta_uv2.y - edge2 * delta_uv1.y)*r;
                TVec3<Scalar> bitangent = (edge2 * delta_uv1.x - edge1 * delta_uv2.x)*r;

                //add tangent
                vertices[indexes[i + 0]].m_tangent += tangent;
                vertices[indexes[i + 1]].m_tangent += tangent;
                vertices[indexes[i + 2]].m_tangent += tangent;

                //add bitangent
                vertices[indexes[i + 0]].m_bitangent += bitangent;
                vertices[indexes[i + 1]].m_bitangent += bitangent;
                vertices[indexes[i + 2]].m_bitangent += bitangent;
            }
        }

        // See "Going Further"
        for (unsigned int i = 0; i < vertices.size(); ++i)
        {
            TVec3<Scalar>& n = vertices[i].m_normal;
            TVec3<Scalar>& t = vertices[i].m_tangent;
            TVec3<Scalar>& b = vertices[i].m_bitangent;

            //normalize
            t = normalize(t);
            b = normalize(b);

            // Gram-Schmidt orthogonalize
            t = normalize(t - n * dot(n, t));

            // Calculate handedness
            if (dot(cross(n, t), b) < Scalar(0.0))
            {
                t = t * Scalar(-1.0);
            }
        }
    }
}
