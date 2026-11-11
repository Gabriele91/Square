//
//  DeferredDirectionShadowBakedLight.hlsl
//  Square
//
//  As <DeferredDirectionShadowLight>, the light with a baked map (Light::baked_shadow_buffer):
//  its shadow its cascades (the dynamic casters) by its baked map (the static ones).
//
#define SQ_SHADOW_BAKED
#include <DeferredDirectionShadowLight>
