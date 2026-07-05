//
//  DeferredAmbientLight.hlsl
//  Square
//
//  Deferred ambient light pass (full-screen). Also the place where the emissive
//  term is added once, since it is view-independent.
//
//  NOTE: <SurfacePBR> is NOT included on purpose: it would expand the forward
//  branch too (the include preprocessor does not evaluate #if) and swallow
//  <LightPBR>/<GammaCorrection> inside a discarded branch. The needed pieces
//  are included explicitly instead.
//
#define RENDERING_AMBIENT_LIGHT
#include <Camera>
#include <Vertex>
#include <GammaCorrection>
#include <NDF>
#include <UtilsPBR>
#include <SurfaceDataPBR>
#include <LightPBR>
#include <SurfaceDeferredPBR>
#include <DeferredFullscreen>
#include <DeferredLightCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = deferred_screen_uv(input.m_position);
	Vec4 g_pos = texture2D(g_position, uv);
	// Background: no geometry here, the G-Buffer holds the clear color in every
	// target; show it as-is (like the forward renderer clear does).
	if (g_pos.w < 0.5)
	{
		return Vec4(texture2D(g_albedo, uv).rgb, 1.0);
	}
	Vec4 color = deferred_shade(uv);
	// Emissive is view-independent: add it once, here in the ambient pass.
	// PBR pixels only: for legacy pixels GT3.rgb holds the specular color.
	if (g_pos.w < 1.5)
	{
		color.rgb += texture2D(g_emissive, uv).rgb;
	}
	return color;
}
