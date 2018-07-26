//
//  Transform.hlsl.metal
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//

// transform struct
cbuffer Transform
{
    Mat4 model_model;
    Vec3 model_position;
    Vec3 model_scale;
    Mat3 model_rotation;
};
