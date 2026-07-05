//
//  DeferredPointShadowLight.hlsl
//  Square
//
//  Deferred point light pass with shadow mapping (sphere volume).
//  Same as <DeferredPointLight> plus RENDERING_SHADOW_ENABLE: the shared
//  <PointShadowLight> (cube map) code is pulled in by <LightPBR>.
//  <Transform> is required by <ShadowCamera> (mul_model_* helpers).
//
#define RENDERING_POINT_LIGHT
#define RENDERING_SHADOW_ENABLE
#include <Camera>
#include <Transform>
#include <Vertex>
#include <GammaCorrection>
#include <NDF>
#include <UtilsPBR>
#include <SurfaceDataPBR>
#include <LightPBR>
#include <SurfaceDeferredPBR>
#include <DeferredVolume>
#include <DeferredLightCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = deferred_screen_uv(input.m_position);
	return deferred_shade(uv);
}
