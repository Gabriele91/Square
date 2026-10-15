//
//  DirectionLight.hlsl
//  Square
//
//  The direction light of a pass (SQ_LIGHT_DIRECTION): its buffer, its shadow (SQ_SHADOW:
//  DirectionShadowLight, else none). The surfaces lit by it: DirectionLightPBR,
//  DirectionLightLegacy, Sprite.
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

#ifdef SQ_SHADOW
#include <DirectionShadowLight>
#else
float direction_light_apply_shadow(in Vec4 fposition, in Vec3  view_dir, in Vec3  normal)
{
    return 1.0;
}
Vec3 direction_light_shadow_tint(in Vec4 fposition)
{
    return Vec3(1.0, 1.0, 1.0);
}
#endif
