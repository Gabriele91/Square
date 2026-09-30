//
//  BloomPrefilter.hlsl
//  Square
//
//  First pass of the bloom (see PostEffectBloom): the frame to the first level of the chain
//  (half size), 13 samples with the average of Karis, then the soft threshold: the light
//  over the threshold, a knee under it fading in.
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
#include <BloomCommon>

Vec2 bloom_size;      //pixels of the target
Vec4 bloom_threshold; //threshold, knee, clamp, -

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / bloom_size;
	Vec2 texel = 1.0 / textureSize2D(g_source, 0);
	Vec3 color = bloom_downsample(uv, texel, 1.0, bloom_threshold.z);
	//soft threshold: brightness over threshold kept, a quadratic knee under it
	float threshold = bloom_threshold.x;
	float knee = bloom_threshold.y;
	float brightness = max(color.r, max(color.g, color.b));
	float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
	soft = soft * soft / (4.0 * knee + 0.0001);
	float contribution = max(soft, brightness - threshold) / max(brightness, 0.0001);
	return Vec4(color * contribution, 1.0);
}
