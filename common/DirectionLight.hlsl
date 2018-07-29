//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

struct DirectionLight
{
    Vec3 m_diffuse;
    Vec3 m_specular;
};

Vec3 light_compute_diffuse
(
     in Vec4  fposition,
     in Vec3  vdir,
     in Vec3  normal,
     in float shininess
);

Vec3 light_compute_specular
(
     in Vec4  fposition,
     in Vec3  vdir,
     in Vec3  normal,
     in float shininess
);
