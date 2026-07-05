//
//  DeferredDirectionLight.hlsl
//  Square
//
//  Deferred directional light pass (full-screen).
//  NOTE: <SurfacePBR> is not included: the include preprocessor does not
//  evaluate #if, its forward branch would swallow <LightPBR>/<GammaCorrection>.
//
#define RENDERING_DIRECTION_LIGHT
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
	return deferred_shade(uv);
}
