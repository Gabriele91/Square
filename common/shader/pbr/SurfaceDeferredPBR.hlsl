//
//  SurfaceDeferredPBR.hlsl
//  Square
//
//  Deferred branch of the PBR surface shader.
//  Included by <SurfacePBR> when DEFERRED_RENDERING is defined, right after the
//  SurfaceData struct is declared. It provides:
//   - the G-Buffer MRT layout (SurfaceOutput),
//   - encode_gbuffer()  : used by the geometry pass to fill the G-Buffer,
//   - decode_gbuffer()  : used by the light passes to rebuild a SurfaceData,
//   - compute_surface_output() : the geometry-pass entry that packs SurfaceData.
//
//  G-Buffer layout (all float targets, values kept in LINEAR space):
//   GT0 : world position (rgb) | 1                (RGBA32F)
//   GT1 : world normal   (rgb) | roughness (a)    (RGBA16F)
//   GT2 : albedo         (rgb) | metallic  (a)    (RGBA8)
//   GT3 : emissive       (rgb) | occlusion (a)    (RGBA16F)
//   + hardware depth buffer
//
#pragma once
#include <SurfaceDataPBR>

// Target indices, kept in sync with the GBuffer build list in DrawerPassDeferred.
#define GBUFFER_POSITION 0
#define GBUFFER_NORMAL   1
#define GBUFFER_ALBEDO   2
#define GBUFFER_EMISSIVE 3

// Shading model ID, written in position.w by the geometry pass and used by the
// light passes to pick the right lighting model per pixel.
// The geometry clear writes 0 (background) in position.w.
// The meaning of some channels depends on the model:
//              PBR (1)                  LEGACY (2)
//  GT1.w       roughness [0..1]         shininess (raw Blinn-Phong exponent)
//  GT2.w       metallic                 unused (0)
//  GT3.rgb     emissive                 specular color
#define GBUFFER_MODEL_BACKGROUND 0.0
#define GBUFFER_MODEL_PBR        1.0
#define GBUFFER_MODEL_LEGACY     2.0

// Geometry-pass output: the four MRT targets.
struct SurfaceOutput
{
	Vec4 m_position : SV_TARGET0;
	Vec4 m_normal   : SV_TARGET1;
	Vec4 m_albedo   : SV_TARGET2;
	Vec4 m_emissive : SV_TARGET3;
};

// Pack a SurfaceData into the G-Buffer.
SurfaceOutput encode_gbuffer(in SurfaceData data)
{
	SurfaceOutput output;
	output.m_position = Vec4(data.m_position.xyz, GBUFFER_MODEL_PBR);
	output.m_normal   = Vec4(normalize(data.m_normal), data.m_roughness);
	output.m_albedo   = Vec4(data.m_albedo, data.m_metallic);
	output.m_emissive = Vec4(data.m_emmisive, data.m_occlusion);
	return output;
}

// Unpack a SurfaceData from four G-Buffer samples (used by the light passes).
SurfaceData decode_gbuffer(in Vec4 g_position,
                           in Vec4 g_normal,
                           in Vec4 g_albedo,
                           in Vec4 g_emissive)
{
	SurfaceData data = DefaultSurfaceData();
	data.m_position  = Vec4(g_position.xyz, 1.0);
	data.m_normal    = normalize(g_normal.xyz);
	data.m_roughness = g_normal.a;
	data.m_albedo    = g_albedo.rgb;
	data.m_metallic  = g_albedo.a;
	data.m_emmisive  = g_emissive.rgb;
	data.m_occlusion = g_emissive.a;
	data.m_alpha     = 1.0;
	return data;
}

// Geometry-pass surface entry: just pack the material into the G-Buffer.
SurfaceOutput compute_surface_output(in SurfaceData data)
{
	return encode_gbuffer(data);
}
