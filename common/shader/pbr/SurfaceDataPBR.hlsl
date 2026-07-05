//
//  SurfaceDataPBR.hlsl
//  Square
//
//  PBR surface description, shared between the surface shaders (forward and
//  deferred geometry) and the deferred light passes.
//
//  NOTE: kept in its own file (instead of inside <SurfacePBR>) because Square's
//  include preprocessor expands #include directives without evaluating #if
//  blocks: a file included only from a dead #if branch is still marked as
//  "already included" and would be skipped later. Standalone shaders that need
//  SurfaceData without the surface entry points (e.g. the deferred light
//  passes) include this file directly.
//
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
