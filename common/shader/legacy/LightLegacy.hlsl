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
#if defined(RENDERING_COLOR)
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
#elif defined(RENDERING_AMBIENT_LIGHT)
#include <AmbientLightLegacy>
#elif defined(RENDERING_DIRECTION_LIGHT)
#include <DirectionLightLegacy>
#elif defined(RENDERING_POINT_LIGHT)
#include <PointLightLegacy>
#elif defined(RENDERING_SPOT_LIGHT)
#include <SpotLightLegacy>
#endif
