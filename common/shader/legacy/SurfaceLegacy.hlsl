//
//  Surface.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
struct SurfaceData
{
	Vec4   m_position;    // world
	Vec3   m_albedo;      // diffuse color
	Vec3   m_specular;    // specular color
	Vec3   m_normal;      // normal in world space
	float  m_alpha;       // alpha for transparencies
	float  m_shininess;   // shininess
	half   m_occlusion;   // occlusion (default 1)
};

SurfaceData DefaultSurfaceData()
{
	SurfaceData output;
	output.m_position = Vec4(0.0, 0.0, 0.0, 1.0);
	output.m_albedo = Vec3(1.0, 1.0, 1.0);
	output.m_specular = Vec3(1.0, 1.0, 1.0);
	output.m_normal = Vec3(0.0, 0.0, 0.0);
	output.m_alpha = 1.0;
	output.m_shininess = 16.0;
	output.m_occlusion = 1.0;
	return output;
}

#if defined( FORWARD_RENDERING )
#include <SurfaceForwardLegacy>
#elif  defined( DEFERRED_RENDERING )

#else
#error define type of rendering (FORWARD_RENDERING|DEFERRED_RENDERING)
#endif

#define surface SurfaceOutput fragment
#define surface_return return compute_surface_output