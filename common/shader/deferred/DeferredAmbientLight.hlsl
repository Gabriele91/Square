//
//  DeferredAmbientLight.hlsl
//  Square
//
//  Deferred ambient light pass (full-screen). Also adds the emissive term,
//  once, since it is view-independent.
//  NOTE: <SurfacePBR> is not included: the include preprocessor does not
//  evaluate #if, its forward branch would swallow <LightPBR>/<GammaCorrection>.
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
	Vec4 gbuffer_position = texture2D(g_position, uv);
	//background: the G-Buffer holds the clear color, show it as-is
	if (gbuffer_position.w < 0.5)
	{
		return Vec4(texture2D(g_albedo, uv).rgb, 1.0);
	}
	Vec4 color = deferred_shade(uv);
	//add emissive to PBR pixels only: for legacy pixels GT3.rgb is the specular color
	if (gbuffer_position.w < 1.5)
	{
		color.rgb += texture2D(g_emissive, uv).rgb;
	}
	return color;
}
