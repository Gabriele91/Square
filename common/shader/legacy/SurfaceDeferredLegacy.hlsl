//
//  SurfaceDeferredLegacy.hlsl
//  Square
//
//  Deferred branch of the Legacy surface shader.
//  The deferred G-Buffer stores a shading model ID in position.w (see
//  <SurfaceDeferredPBR> for the layout); legacy pixels are lit with the same
//  Blinn-Phong math as the forward renderer, so a legacy material looks the
//  same in both pipelines:
//   GT0 : world position | GBUFFER_MODEL_LEGACY
//   GT1 : world normal   | shininess (raw Blinn-Phong exponent)
//   GT2 : albedo         | 0
//   GT3 : specular color | occlusion
//
#pragma once

// Target indices, kept in sync with the GBuffer build list in DrawerPassDeferred.
#define GBUFFER_POSITION 0
#define GBUFFER_NORMAL   1
#define GBUFFER_ALBEDO   2
#define GBUFFER_EMISSIVE 3

// Shading model IDs (kept in sync with <SurfaceDeferredPBR>)
#define GBUFFER_MODEL_BACKGROUND 0.0
#define GBUFFER_MODEL_PBR        1.0
#define GBUFFER_MODEL_LEGACY     2.0

// Geometry-pass output: the four MRT targets (same layout as <SurfaceDeferredPBR>).
struct SurfaceOutput
{
	Vec4 m_position : SV_TARGET0;
	Vec4 m_normal   : SV_TARGET1;
	Vec4 m_albedo   : SV_TARGET2;
	Vec4 m_emissive : SV_TARGET3;
};

// Geometry-pass surface entry: pack the legacy material into the G-Buffer.
SurfaceOutput compute_surface_output(in SurfaceData data)
{
	SurfaceOutput output;
	output.m_position = Vec4(data.m_position.xyz, GBUFFER_MODEL_LEGACY);
	output.m_normal   = Vec4(normalize(data.m_normal), data.m_shininess);
	output.m_albedo   = Vec4(data.m_albedo, 0.0);
	output.m_emissive = Vec4(data.m_specular, data.m_occlusion);
	return output;
}
