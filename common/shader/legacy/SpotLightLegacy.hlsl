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
    float theta = dot(light_dir, normalize(-light.m_direction));
    float epsilon = light.m_inner_cut_off - light.m_outer_cut_off;
    float intensity = clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);
	// Apply shadow
	float shadow_factor = spot_light_apply_shadow(fposition);
    // Combine results
    result.m_diffuse  = light.m_diffuse  * diff * attenuation * intensity * shadow_factor;
    result.m_specular = light.m_specular * spec * attenuation * intensity * shadow_factor; 
    //return
    return result;
}
