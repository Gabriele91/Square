//
//  DeferredSpotLight.hlsl
//  Square
//
//  Deferred spot light pass. Renders a cone volume oriented along the light
//  direction and scaled to the light radius (model matrix uploaded in the
//  LightVolume constant buffer).
//
//  NOTE: <SurfacePBR> is NOT included on purpose: it would expand the forward
//  branch too (the include preprocessor does not evaluate #if) and swallow
//  <LightPBR>/<GammaCorrection> inside a discarded branch. The needed pieces
//  are included explicitly instead.
//
#define RENDERING_SPOT_LIGHT
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
