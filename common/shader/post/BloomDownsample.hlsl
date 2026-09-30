//
//  BloomDownsample.hlsl
//  Square
//
//  Second pass of the bloom (see PostEffectBloom): a level of the chain from the one before
//  (twice its size), 13 samples.
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
#include <BloomCommon>

Vec2 bloom_size; //pixels of the target

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / bloom_size;
	Vec2 texel = 1.0 / textureSize2D(g_source, 0);
	return Vec4(bloom_downsample(uv, texel, 0.0, 65000.0), 1.0);
}
