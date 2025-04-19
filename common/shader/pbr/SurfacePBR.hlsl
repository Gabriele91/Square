#pragma once
struct SurfaceData
{
	Vec4   m_position;    // world
	Vec3   m_albedo;      // diffuse color
	Vec3   m_emmisive;    // light emission
	Vec3   m_normal;      // normal in world space
	half   m_occlusion;   // occlusion (default 1)
	half   m_metallic;    // from 0=non-metal, to 1=metal
	half   m_roughness;   // from 0=smooth, to 1=rough
	float  m_alpha;       // alpha for transparencies
};

SurfaceData DefaultSurfaceData()
{
	SurfaceData output;
	output.m_position = Vec4(0.0, 0.0, 0.0, 1.0);
	output.m_albedo = Vec3(1.0, 1.0, 1.0);
	output.m_emmisive = Vec3(0.0, 0.0, 0.0);
	output.m_normal = Vec3(0.0, 0.0, 0.0);
	output.m_occlusion = 1.0;
	output.m_metallic = 0.0;
	output.m_roughness = 0.0;
	output.m_alpha = 1.0;
	return output;
}

#if defined( FORWARD_RENDERING )
#include <SurfaceForwardPBR>
#elif  defined( DEFERRED_RENDERING )

#else
#error define type of rendering (FORWARD_RENDERING|DEFERRED_RENDERING)
#endif

#define surface SurfaceOutput fragment
#define surface_return return compute_surface_output