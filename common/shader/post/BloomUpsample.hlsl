//
//  BloomUpsample.hlsl
//  Square
//
//  Third pass of the bloom (see PostEffectBloom): a level of the chain (g_source, its
//  downsample) mixed with the level under it, upsampled with a 3x3 tent (g_lower): by
//  bloom_scatter, the share of the wider levels.
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_lower);
#include <BloomCommon>

Vec2  bloom_size;    //pixels of the target
float bloom_scatter; //share of the level under it

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / bloom_size;
	Vec2 texel_lower = 1.0 / textureSize2D(g_lower, 0);
	Vec3 high = texture2DLod(g_source, uv, 0.0).rgb;
	Vec3 low  = BLOOM_TENT(g_lower, uv, texel_lower);
	return Vec4(lerp(high, low, bloom_scatter), 1.0);
}
