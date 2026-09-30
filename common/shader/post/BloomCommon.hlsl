//
//  BloomCommon.hlsl
//  Square
//
//  The filters of the bloom (see PostEffectBloom): the 13 samples downsample (a smooth half
//  size, Jimenez, Call of Duty: Advanced Warfare) and the 3x3 tent upsample.
//  The downsample reads g_source: declare it (Sampler2D(g_source)) before the include. The
//  textures are globals, not function parameters (the HLSL front end would need the SPIR-V
//  optimizer to inline them).
//
#pragma once

//3x3 tent of a texture around uv (texel: of that texture)
#define BLOOM_TENT(name, uv, texel)                                                                        \
	( ( texture2DLod(name, (uv), 0.0).rgb * 4.0                                                            \
	  + ( texture2DLod(name, (uv) + (texel) * Vec2(-1.0,  0.0), 0.0).rgb                                    \
	    + texture2DLod(name, (uv) + (texel) * Vec2( 1.0,  0.0), 0.0).rgb                                    \
	    + texture2DLod(name, (uv) + (texel) * Vec2( 0.0, -1.0), 0.0).rgb                                    \
	    + texture2DLod(name, (uv) + (texel) * Vec2( 0.0,  1.0), 0.0).rgb ) * 2.0                            \
	  + texture2DLod(name, (uv) + (texel) * Vec2(-1.0, -1.0), 0.0).rgb                                      \
	  + texture2DLod(name, (uv) + (texel) * Vec2( 1.0, -1.0), 0.0).rgb                                      \
	  + texture2DLod(name, (uv) + (texel) * Vec2(-1.0,  1.0), 0.0).rgb                                      \
	  + texture2DLod(name, (uv) + (texel) * Vec2( 1.0,  1.0), 0.0).rgb ) / 16.0 )

float bloom_luma(Vec3 color)
{
	return dot(color, Vec3(0.2126, 0.7152, 0.0722));
}

//a sample of g_source in [0, max_value]: a NaN/Inf pixel of the frame (invisible alone) would
//spread through the blurs over the whole screen, so it counts as black
Vec3 bloom_sample(Vec2 uv, float max_value)
{
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	if (any(isnan(color)) || any(isinf(color))) return Vec3(0.0, 0.0, 0.0);
	return clamp(color, Vec3(0.0, 0.0, 0.0), Vec3(max_value, max_value, max_value));
}

//13 samples of g_source around uv (texel: of g_source, half the one of the target), in five
//boxes of four: the center one weights 0.5, the four corner ones 0.125 each. karis 1: each box
//weighted by 1 / (1 + luma) too, a lone very bright pixel does not blow up (first downsample)
Vec3 bloom_downsample(Vec2 uv, Vec2 texel, float karis, float max_value)
{
	Vec3 a = bloom_sample(uv + texel * Vec2(-2.0, -2.0), max_value);
	Vec3 b = bloom_sample(uv + texel * Vec2( 0.0, -2.0), max_value);
	Vec3 c = bloom_sample(uv + texel * Vec2( 2.0, -2.0), max_value);
	Vec3 d = bloom_sample(uv + texel * Vec2(-2.0,  0.0), max_value);
	Vec3 e = bloom_sample(uv,                            max_value);
	Vec3 f = bloom_sample(uv + texel * Vec2( 2.0,  0.0), max_value);
	Vec3 g = bloom_sample(uv + texel * Vec2(-2.0,  2.0), max_value);
	Vec3 h = bloom_sample(uv + texel * Vec2( 0.0,  2.0), max_value);
	Vec3 i = bloom_sample(uv + texel * Vec2( 2.0,  2.0), max_value);
	Vec3 j = bloom_sample(uv + texel * Vec2(-1.0, -1.0), max_value);
	Vec3 k = bloom_sample(uv + texel * Vec2( 1.0, -1.0), max_value);
	Vec3 l = bloom_sample(uv + texel * Vec2(-1.0,  1.0), max_value);
	Vec3 m = bloom_sample(uv + texel * Vec2( 1.0,  1.0), max_value);
	//the five boxes
	Vec3 box_center = (j + k + l + m) * 0.25;
	Vec3 box_0 = (a + b + d + e) * 0.25;
	Vec3 box_1 = (b + c + e + f) * 0.25;
	Vec3 box_2 = (d + e + g + h) * 0.25;
	Vec3 box_3 = (e + f + h + i) * 0.25;
	//weights: 0.5 the center, 0.125 the corners (by 1 / (1 + luma) with karis)
	float w_center = 0.5   / (1.0 + karis * bloom_luma(box_center));
	float w_0      = 0.125 / (1.0 + karis * bloom_luma(box_0));
	float w_1      = 0.125 / (1.0 + karis * bloom_luma(box_1));
	float w_2      = 0.125 / (1.0 + karis * bloom_luma(box_2));
	float w_3      = 0.125 / (1.0 + karis * bloom_luma(box_3));
	return (box_center * w_center + box_0 * w_0 + box_1 * w_1 + box_2 * w_2 + box_3 * w_3)
	     / (w_center + w_0 + w_1 + w_2 + w_3);
}
