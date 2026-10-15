//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

//the light (its buffer, its shadow): the same of every surface
#include <SpotLight>

LightResult compute_light
(
     in Vec4  fposition,
     in Vec3  view_dir,
     in Vec3  normal,
	 in float occlusion,
     in float shininess
)
{
    //value return
    LightResult result;
	// Light relative
	Vec3 light_relative = light.m_position - fposition.xyz;
    // Attenuation
    float attenuation = spot_light_compute_attenuation(light_relative);
    // Exit case
    if (attenuation <= 0.0)
    {
        result.m_diffuse  = Vec3(0.0,0.0,0.0);
        result.m_specular = Vec3(0.0,0.0,0.0);
        return result;
    }
    // Light dir
    Vec3 light_dir = normalize(light_relative);
    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // Specular shading
    Vec3  halfway_dir  = normalize(light_dir + view_dir);
    float spec         = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    // Spotlight intensity
    float intensity = spot_light_compute_penumbra(light_dir);
	// Apply shadow
	float shadow_factor = spot_light_apply_shadow(fposition);
    // Combine results
    result.m_diffuse  = light.m_diffuse  * diff * attenuation * intensity * shadow_factor;
    result.m_specular = light.m_specular * spec * attenuation * intensity * shadow_factor; 
    //return
    return result;
}
