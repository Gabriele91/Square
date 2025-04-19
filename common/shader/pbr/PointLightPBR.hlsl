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
float point_light_apply_shadow(in Vec4 fposition)
{
    return 1.0;
}
#endif

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

    Vec3 light_color = light.m_diffuse * radiance;
    result.m_radiance = calculate_PBR(data.m_albedo, 
                                      data.m_metallic,
                                      data.m_roughness, 
                                      data.m_normal, 
                                      view_direction, 
                                      light_relative, 
                                      light_color);
    // Shadow
    float shadow_factor = point_light_apply_shadow(data.m_position);
    // Shadow
    result.m_radiance *= shadow_factor;
    //return
    return result;
}
