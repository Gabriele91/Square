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
	Vec3 m_direction;
    Vec3 m_diffuse;
    Vec3 m_specular;
};

cbuffer Light
{
    DirectionLight light;
}

#ifdef RENDERING_SHADOW_ENABLE
#include <DirectionShadowLight>
#else
void direction_light_apply_shadow(inout LightResult result, in Vec4 fposition, in Vec3  view_dir, in Vec3  normal)
{
}
#endif


LightResult compute_light
(
     in Vec4  fposition,
     in Vec3  view_dir,
     in Vec3  normal,
	 in float occlusion,
     in float shininess
)
{
    // Light dir
	Vec3 light_dir = normalize(-light.m_direction);
    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // Specular shading
    Vec3  halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    // Combine results
    LightResult result;
    result.m_diffuse  = light.m_diffuse  * diff;
    result.m_specular = light.m_specular * spec;
    //apply shadow
    direction_light_apply_shadow(result, fposition, light_dir, normal);
    //return
    return result;
}

