//
//  SpotLight.hlsl
//  Square
//
//  The spot light of a pass (SQ_LIGHT_SPOT): its buffer, its attenuation, its cone, its shadow
//  (SQ_SHADOW: SpotShadowLight, else none). The surfaces lit by it: SpotLightPBR,
//  SpotLightLegacy, Sprite.
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

#ifdef SQ_SHADOW
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

//its cone: 1 inside, 0 out of it, its penumbra between (light_direction: to the light)
float spot_light_compute_penumbra(in Vec3 light_direction)
{
    float theta = dot(light_direction, normalize(-light.m_direction));
    float epsilon = light.m_inner_cut_off - light.m_outer_cut_off;
    return clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);
}
