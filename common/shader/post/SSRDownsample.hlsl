//
//  SSRDownsample.hlsl
//  Square
//
//  The blur levels of the screen space reflections (see PostEffectSSR): a level of the chain
//  from the one before (twice its size), 13 samples as in the bloom (Jimenez). The levels are
//  premultiplied by the confidence (rgb * a, a): a pixel without reflection does not darken
//  the others. The first level reads the trace (ssr_premultiply 1: not premultiplied yet).
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Vec2  ssr_size;        //pixels of the target
float ssr_premultiply; //1: g_source is the trace (color, confidence)

//a sample of g_source, premultiplied (a NaN/Inf pixel counts as nothing)
Vec4 ssr_sample(Vec2 uv)
{
	Vec4 value = texture2DLod(g_source, uv, 0.0);
	if (any(isnan(value)) || any(isinf(value))) return Vec4(0.0, 0.0, 0.0, 0.0);
	if (ssr_premultiply > 0.5) value.rgb *= value.a;
	return value;
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssr_size;
	Vec2 texel = 1.0 / textureSize2D(g_source, 0);
	Vec4 a = ssr_sample(uv + texel * Vec2(-2.0, -2.0));
	Vec4 b = ssr_sample(uv + texel * Vec2( 0.0, -2.0));
	Vec4 c = ssr_sample(uv + texel * Vec2( 2.0, -2.0));
	Vec4 d = ssr_sample(uv + texel * Vec2(-2.0,  0.0));
	Vec4 e = ssr_sample(uv);
	Vec4 f = ssr_sample(uv + texel * Vec2( 2.0,  0.0));
	Vec4 g = ssr_sample(uv + texel * Vec2(-2.0,  2.0));
	Vec4 h = ssr_sample(uv + texel * Vec2( 0.0,  2.0));
	Vec4 i = ssr_sample(uv + texel * Vec2( 2.0,  2.0));
	Vec4 j = ssr_sample(uv + texel * Vec2(-1.0, -1.0));
	Vec4 k = ssr_sample(uv + texel * Vec2( 1.0, -1.0));
	Vec4 l = ssr_sample(uv + texel * Vec2(-1.0,  1.0));
	Vec4 m = ssr_sample(uv + texel * Vec2( 1.0,  1.0));
	//five boxes of four: the center one 0.5, the corner ones 0.125 each
	Vec4 box_center = (j + k + l + m) * 0.25;
	Vec4 box_0 = (a + b + d + e) * 0.25;
	Vec4 box_1 = (b + c + e + f) * 0.25;
	Vec4 box_2 = (d + e + g + h) * 0.25;
	Vec4 box_3 = (e + f + h + i) * 0.25;
	return box_center * 0.5 + (box_0 + box_1 + box_2 + box_3) * 0.125;
}
