//
//  Light.hlsl.h
//  Square
//
//  Created by Gabriele Di Bari on 31/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
//output
struct LightResult
{
    Vec3 m_diffuse;
    Vec3 m_specular;
};
//include the required light type
#if defined(SQ_LIGHT_COLOR)
LightResult compute_light
(
	in Vec4  fposition,
	in Vec3  view_dir,
	in Vec3  normal,
	in float occlusion,
	in float shininess
)
{
	LightResult result;
	result.m_diffuse = Vec4(1.0, 1.0, 1.0, 1.0);
	result.m_specular = Vec4(0.0, 0.0, 0.0, 0.0);
	return result;
}
#elif defined(SQ_LIGHT_AMBIENT)
#include <AmbientLightLegacy>
#elif defined(SQ_LIGHT_DIRECTION)
#include <DirectionLightLegacy>
#elif defined(SQ_LIGHT_POINT)
#include <PointLightLegacy>
#elif defined(SQ_LIGHT_SPOT)
#include <SpotLightLegacy>
#endif
