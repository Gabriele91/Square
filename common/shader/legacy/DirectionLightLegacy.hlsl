//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
//the light (its buffer, its shadow): the same of every surface
#include <DirectionLight>

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
    // Apply shadow
    float shadow_factor = direction_light_apply_shadow(fposition, light_dir, normal);
    // Combine results
    LightResult result;
    const Vec3 tint = direction_light_shadow_tint(fposition);
    result.m_diffuse  = light.m_diffuse  * diff * shadow_factor * tint;
    result.m_specular = light.m_specular * spec * shadow_factor * tint;
    //return
    return result;
}

