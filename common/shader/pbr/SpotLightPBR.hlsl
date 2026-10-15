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
 	in Vec3 view_direction,
	in SurfaceData data
)
{
    //value return
    LightResult result;
	// Light relative
	Vec3 light_relative = light.m_position - data.m_position.xyz;
    // Attenuation
    float attenuation = spot_light_compute_attenuation(light_relative);
    // Exit case
    if (attenuation <= 0.0)
    {
        result.m_radiance  = Vec3(0.0,0.0,0.0);
        return result;
    }
    
    // Light dir
	Vec3 light_direction = normalize(light_relative);

    // Spotlight penumbra intensity
    float penumbra_intensity = spot_light_compute_penumbra(light_direction);

    // Shadow
    float shadow_factor = spot_light_apply_shadow(data.m_position);

    // Spotlight Radiance
    Vec3 light_color = light.m_diffuse * (attenuation * penumbra_intensity * shadow_factor);

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
