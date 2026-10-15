//
//  PointLight.hlsl
//  Square
//
//  The point light of a pass (SQ_LIGHT_POINT): its buffer, its attenuation, its shadow
//  (SQ_SHADOW: PointShadowLight, else none). The surfaces lit by it: PointLightPBR,
//  PointLightLegacy, Sprite.
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

#ifdef SQ_SHADOW
#include <PointShadowLight>
#else
float point_light_apply_shadow(in Vec4 fposition)
{
    return 1.0;
}
#endif
