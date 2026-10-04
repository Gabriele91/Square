//
//  DeferredDirectionShadowLightPCSS.hlsl
//  Square
//
//  Deferred directional light pass with soft shadows (PCSS, full-screen): as
//  <DeferredDirectionShadowLight>, the shadow filter of <DirectionShadowLight> is PCSS (the
//  penumbra wider far from the caster) instead of the fixed PCF. Chosen by the light
//  (Render::Light::shadow_filter).
//
#define RENDERING_DIRECTION_LIGHT
#define RENDERING_SHADOW_ENABLE
#define PCSS_SHADOW
#include <Camera>
#include <Transform>
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
	return deferred_shade(uv);
}
