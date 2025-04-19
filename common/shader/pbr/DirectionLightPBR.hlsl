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
float direction_light_apply_shadow(in Vec4 fposition, in Vec3  view_dir, in Vec3  normal)
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
    // Combine results
    LightResult result;

    // Shadow
    float shadow_factor = direction_light_apply_shadow(data.m_position, view_direction, data.m_normal);

    // Lgiht final color
    Vec3 light_color = light.m_diffuse * shadow_factor;

    // Compute PBR material
    result.m_radiance = calculate_PBR(data.m_albedo, 
                                      data.m_metallic,
                                      data.m_roughness, 
                                      data.m_normal, 
                                      view_direction, 
                                      -light.m_direction, 
                                      light_color);
    //return
    return result;
}

