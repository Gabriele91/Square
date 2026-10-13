//
//  FXAA.hlsl
//  Square
//
//  FXAA (see PostEffectFXAA, after Lottes' FXAA 3 console): the luma of the pixel and of its four
//  corners; under the threshold of contrast the pixel as it is; else the direction of the edge
//  (across the gradient of the luma), two blends along it (near, far), the far one kept if its
//  luma stays within the range of the neighbors (it did not cross the edge).
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Vec2 fxaa_size;    //pixels of the frame
Vec4 fxaa_params;  //edge threshold (relative), edge threshold (absolute), span max (pixels)

float fxaa_luma(in Vec3 color)
{
	return dot(color, Vec3(0.299, 0.587, 0.114));
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 texel = 1.0 / fxaa_size;
	Vec2 uv = input.m_position.xy * texel;
	Vec3 rgb_m  = texture2DLod(g_source, uv, 0.0).rgb;
	Vec3 rgb_nw = texture2DLod(g_source, uv + Vec2(-1.0, -1.0) * texel, 0.0).rgb;
	Vec3 rgb_ne = texture2DLod(g_source, uv + Vec2( 1.0, -1.0) * texel, 0.0).rgb;
	Vec3 rgb_sw = texture2DLod(g_source, uv + Vec2(-1.0,  1.0) * texel, 0.0).rgb;
	Vec3 rgb_se = texture2DLod(g_source, uv + Vec2( 1.0,  1.0) * texel, 0.0).rgb;
	float luma_m  = fxaa_luma(rgb_m);
	float luma_nw = fxaa_luma(rgb_nw);
	float luma_ne = fxaa_luma(rgb_ne);
	float luma_sw = fxaa_luma(rgb_sw);
	float luma_se = fxaa_luma(rgb_se);
	float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
	float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));
	//no edge: as it is
	if (luma_max - luma_min < max(fxaa_params.y, luma_max * fxaa_params.x)) return Vec4(rgb_m, 1.0);
	//the direction along the edge
	Vec2 dir;
	dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
	dir.y =  ((luma_nw + luma_sw) - (luma_ne + luma_se));
	float reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * 0.25 * 0.125, 1.0 / 128.0);
	float scale = 1.0 / (min(abs(dir.x), abs(dir.y)) + reduce);
	dir = clamp(dir * scale, Vec2(-fxaa_params.z, -fxaa_params.z), Vec2(fxaa_params.z, fxaa_params.z)) * texel;
	//two blends along it: near, far
	Vec3 rgb_a = 0.5 * (texture2DLod(g_source, uv + dir * (1.0 / 3.0 - 0.5), 0.0).rgb +
	                    texture2DLod(g_source, uv + dir * (2.0 / 3.0 - 0.5), 0.0).rgb);
	Vec3 rgb_b = rgb_a * 0.5 + 0.25 * (texture2DLod(g_source, uv + dir * -0.5, 0.0).rgb +
	                                   texture2DLod(g_source, uv + dir *  0.5, 0.0).rgb);
	float luma_b = fxaa_luma(rgb_b);
	if (luma_b < luma_min || luma_b > luma_max) return Vec4(rgb_a, 1.0);
	return Vec4(rgb_b, 1.0);
}
