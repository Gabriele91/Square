//
//  camera.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//

// camera struct
cbuffer Camera
{
    Vec4 camera_viewport;
    Mat4 camera_projection;
    Mat4 camera_view;
    Mat4 camera_model;
    Vec3 camera_position;
};

