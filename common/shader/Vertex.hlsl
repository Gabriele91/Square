//
//  Vertex.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

//position (2D)
struct Position2D
{
    Vec2 m_position : POSITION;
};
#define Pos2D Position2D

//position (2D) + uv
struct Position2DUV
{
    Vec2 m_position : POSITION;
    Vec2 m_uvmap    : TEXCOORD0;
};
#define Pos2DUV Position2DUV

//position (3D)
struct Position3D
{
    Vec3 m_position : POSITION;
};
#define Pos3D Position3D

//position (3D) + uv
struct Position3DUV
{
    Vec3 m_position : POSITION;
    Vec2 m_uv       : TEXCOORD0;
};
#define Pos3DUV Position3DUV

//position (3D) + normal + uv
struct Position3DNormalUV
{
    Vec3 m_position : POSITION;
    Vec3 m_normal   : NORMAL0;
    Vec2 m_uv       : TEXCOORD0;
};
#define Pos3DNormalUV Position3DNormalUV
#define Pos3DNorUV Position3DNormalUV

//position (3D) + normal + tangent + bitangent + uv
struct Position3DNormalTangetBinomialUV
{
    Vec3 m_position : POSITION;
    Vec3 m_normal   : NORMAL0;
    Vec3 m_tangent  : TANGENT0;
    Vec3 m_binomial : BINORMAL0;
    Vec2 m_uv       : TEXCOORD0;
};
#define Pos3DNormalTangetBinomialUV Position3DNormalTangetBinomialUV
#define Pos3DNorTanBinUV Position3DNormalTangetBinomialUV

