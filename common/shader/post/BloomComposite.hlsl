//
//  BloomComposite.hlsl
//  Square
//
//  Last pass of the bloom (see PostEffectBloom): the frame plus the bloom (the first level
//  of the chain, upsampled with a 3x3 tent) times the intensity. Debug: only the bloom.
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_bloom);
#include <BloomCommon>

Vec2  bloom_size;      //pixels of the frame
float bloom_intensity;
float bloom_debug;     //1: only the bloom

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / bloom_size;
	Vec2 texel_bloom = 1.0 / textureSize2D(g_bloom, 0);
	Vec3 bloom = BLOOM_TENT(g_bloom, uv, texel_bloom);
	if (bloom_debug > 0.5) return Vec4(bloom, 1.0);
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	return Vec4(color + bloom * bloom_intensity, 1.0);
}
