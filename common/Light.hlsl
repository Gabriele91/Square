//
//  Light.hlsl.h
//  Square
//
//  Created by Gabriele Di Bari on 31/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
//output
struct LightResult
{
    Vec3 m_diffuse;
    Vec3 m_specular;
};
//include the required light type
#if defined(RENDERING_DIRECTION_LIGHT)
#include <DirectionLight>
#elif defined(RENDERING_POINT_LIGHT)
#include <PointLight>
#elif defined(RENDERING_SPOT_LIGHT)
#include <SpotLight>
#endif
