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
#if defined(RENDERING_COLOR)
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
#elif defined(RENDERING_AMBIENT_LIGHT)
#include <AmbientLightPBR>
#elif defined(RENDERING_DIRECTION_LIGHT)
#include <DirectionLightPBR>
#elif defined(RENDERING_POINT_LIGHT)
#include <PointLightPBR>
#elif defined(RENDERING_SPOT_LIGHT)
#include <SpotLightPBR>
#endif
