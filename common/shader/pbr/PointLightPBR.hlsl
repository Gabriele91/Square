//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

//the light (its buffer, its shadow): the same of every surface
#include <PointLight>

LightResult compute_light
(
 	in Vec3 view_direction,
	in SurfaceData data
)
{
    //value return
    LightResult result;

	// Light relative
	Vec3 light_relative = light.m_position - data.m_position.xyz;
    // Attenuation
    float radiance = point_light_compute_attenuation(light_relative);
    // Exit case
    if (radiance <= 0.0)
    {
        result.m_radiance = Vec3(0.0,0.0,0.0);
        return result;
    }
    // Shadow
    float shadow_factor = point_light_apply_shadow(data.m_position);

    // Lgiht final color
    Vec3 light_color = light.m_diffuse * (radiance * shadow_factor);

    // Compute PBR material
    result.m_radiance = calculate_PBR(data.m_albedo, 
                                      data.m_metallic,
                                      data.m_roughness, 
                                      data.m_normal, 
                                      view_direction, 
                                      light_relative, 
                                      light_color);
    //return
    return result;
}
