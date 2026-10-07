//
//  DeferredDirectionShadowLight.hlsl
//  Square
//
//  Deferred directional light pass with shadow mapping (full-screen).
//  Same as <DeferredDirectionLight> plus SQ_SHADOW: the shared
//  <DirectionShadowLight> (CSM) code is pulled in by <LightPBR>.
//  <Transform> is required by <ShadowCamera> (mul_model_* helpers).
//
#define SQ_LIGHT_DIRECTION
#define SQ_SHADOW
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
