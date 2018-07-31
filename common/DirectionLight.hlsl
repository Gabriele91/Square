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

cbuffer Light
{
    DirectionLight light;
}

LightResult compute_light
(
     in Vec3  fposition,
     in Vec3  vdir,
     in Vec3  normal,
     in float shininess
)
{
    // Light dir
    Vec3 light_dir = normalize(light.m_direction);
    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // Specular shading
    Vec3  halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    // Combine results
    LightResult result;
    result.m_diffuse  = light.m_diffuse  * diff;
    result.m_specular = light.m_specular * spec;
    //return
    return result;
}
