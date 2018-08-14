//
//  SpotLight.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 28/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

struct PointLightStruct
{
	Vec3  m_position;
    Vec3  m_diffuse;
    Vec3  m_specular;
    float m_constant;
    float m_radius;
    float m_inside_radius;
};

cbuffer Light
{
    PointLightStruct light;
}

//attenuation
float point_light_compute_attenuation(in Vec3  light_relative)
{
    float light_distance = length(light_relative);
    return pow(smoothstep(light.m_radius, light.m_inside_radius, light_distance), light.m_constant);
}

#ifdef RENDERING_SHADOW_ENABLE
#include <PointShadowLight>
#else
void point_light_apply_shadow(inout LightResult result, in Vec4 fposition)
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
    //value return
    LightResult result;
	// Light relative
	Vec3 light_relative = light.m_position - fposition.xyz;
    // Attenuation
    float attenuation = point_light_compute_attenuation(light_relative);
    // Exit case
    if (attenuation <= 0.0)
    {
        result.m_diffuse = Vec3(0.0,0.0,0.0);
        result.m_specular = Vec3(0.0,0.0,0.0);
        return result;
    }
    // Light dir
    Vec3 light_dir = normalize(light_relative);
    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // Specular shading
    Vec3  halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    // Combine results
    result.m_diffuse  = light.m_diffuse * diff * attenuation;
    result.m_specular = light.m_specular * spec * attenuation;
	//apply shadow
	point_light_apply_shadow(result, fposition);
    // Return
    return result;
}
