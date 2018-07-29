//
//  camera.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

// camera struct
struct Camera
{
    Vec4 m_viewport;
    Mat4 m_projection;
    Mat4 m_view;
    Mat4 m_model;
    Vec3 m_position;
};
//Const buffer
ConstantBuffer<Camera> camera;
