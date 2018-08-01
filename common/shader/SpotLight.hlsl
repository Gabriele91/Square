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

//attenuation
float spot_light_compute_attenuation(in Vec3  frag_position)
{
    float light_distance = length(light.m_position - frag_position);
    return pow(smoothstep(light.m_radius, light.m_inside_radius, light_distance), light.m_constant);
}

LightResult compute_light
(
     in Vec3  fposition,
     in Vec3  vdir,
     in Vec3  normal,
     in float shininess
)
{
    //value return
    LightResult result;
    // Attenuation
    float attenuation = spot_light_compute_attenuation(fposition);
    // Exit case
    if (attenuation <= 0.0)
    {
        result.m_diffuse  = Vec3(0.0,0.0,0.0);
        result.m_specular = Vec3(0.0,0.0,0.0);
        return result;
    }
    // Light dir
    Vec3 light_dir = normalize(light.m_position - fposition);
    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // Specular shading
    Vec3  halfway_dir  = normalize(light_dir + vdir);
    float spec         = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    // Spotlight intensity
    float theta = dot(light_dir, normalize(-light.m_direction));
    float epsilon = light.m_inner_cut_off - light.m_outer_cut_off;
    float intensity = clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);
    // Combine results
    result.m_diffuse  = light.m_diffuse  * diff * attenuation * intensity;
    result.m_specular = light.m_specular * spec * attenuation * intensity;
    //return
    return result;
}
