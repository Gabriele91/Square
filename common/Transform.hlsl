//
//  Transform.hlsl.metal
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

// transform struct
struct Transform
{
    Mat4 m_model;
    Vec3 m_position;
    Vec3 m_scale;
    Mat3 m_rotation;
};
//Const buffer
ConstantBuffer<Transform> transform;


