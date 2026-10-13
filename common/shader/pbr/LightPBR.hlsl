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
    Vec3 m_radiance;
};
//include the required light type
#if defined(SQ_LIGHT_COLOR)
LightResult compute_light
(
 	in Vec3 view_direction,
	in SurfaceData data
)
{
	LightResult result;
	result.m_radiance = data.m_albedo;
	return result;
}
#elif defined(SQ_LIGHT_AMBIENT)
#include <AmbientLightPBR>
#elif defined(SQ_LIGHT_DIRECTION)
#include <DirectionLightPBR>
#elif defined(SQ_LIGHT_POINT)
#include <PointLightPBR>
#elif defined(SQ_LIGHT_SPOT)
#include <SpotLightPBR>
#endif
