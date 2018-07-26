//
//  Vertex.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//

//base vertex (2D)
struct Vertex2D
{
    Vec2 m_position : POSITION;
};

//base vertex (2D) + uv
struct Vertex2DUV
{
    Vec2 m_position : POSITION;
    Vec2 m_uvmap    : TEXCOORD0;
};

//base vertex (3D)
struct Vertex3D
{
    Vec3 m_position : POSITION;
};

//base vertex (3D) + uv
struct Vertex3DUV
{
    Vec3 m_position : POSITION;
    Vec2 m_uvmap    : TEXCOORD0;
};

//vertex (3D) + normal + uv
struct Vertex3DNormalUV
{
    Vec3 m_position : POSITION;
    Vec3 m_normal   : NORMAL0;
    Vec2 m_uvmap    : TEXCOORD0;
};

//vertex (3D) + normal + tangent + bitangent + uv
struct Vertex3DNormalUVTangetBinomial
{
    Vec3 m_position : POSITION;
    Vec3 m_normal   : NORMAL0;
    Vec3 m_tangent  : TANGENT0;
    Vec3 m_binomial : BINORMAL0;
    Vec2 m_uvmap    : TEXCOORD0;
};

