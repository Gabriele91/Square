//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

struct SpotLightStruct
{
	Vec3  m_position;
	Vec3  m_direction;
    Vec3  m_diffuse;
    Vec3  m_specular;
    float m_constant;
    float m_radius;
    float m_inside_radius;
    float m_inner_cut_off;
    float m_outer_cut_off;
};

cbuffer Light
{
    SpotLightStruct light;
}

#ifdef RENDERING_SHADOW_ENABLE
#include <SpotShadowLight>
#else
float spot_light_apply_shadow(in Vec4 fposition)
{
    return 1.0;
}
#endif

//attenuation
float spot_light_compute_attenuation(in Vec3 light_relative)
{
	float light_distance = length(light_relative);
	return pow(smoothstep(light.m_radius, light.m_inside_radius, light_distance), light.m_constant);
}

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
    float theta = dot(light_direction, -light.m_direction);
    float epsilon = light.m_inner_cut_off - light.m_outer_cut_off;
    float penumbra_intensity = clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);

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
