//
//  DeferredPointLight.hlsl
//  Square
//
//  Deferred point light pass. Renders a sphere volume scaled to the light radius
//  (model matrix uploaded in the LightVolume constant buffer).
//  NOTE: <SurfacePBR> is not included: the include preprocessor does not
//  evaluate #if, its forward branch would swallow <LightPBR>/<GammaCorrection>.
//
#define RENDERING_POINT_LIGHT
#include <Camera>
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
