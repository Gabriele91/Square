//
//  Transform.hlsl.metal
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

// transform struct
cbuffer Transform
{
    Mat4 transform_model;
    Vec3 transform_position;
    Vec3 transform_scale;
    Mat3 transform_rotation;
};
