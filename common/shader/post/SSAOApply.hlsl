//
//  SSAOApply.hlsl
//  Square
//
//  Last pass of the SSAO (see PostEffectSSAO), at the frame size: the (blurred) occlusion,
//  at half size up by the 4 nearest texels weighted bilinear and by the view depth (it does
//  not bleed over the edges). Output Vec4(1, 1, 1, ao), drawn with a (ZERO, SRC_COLOR) blend
//  on the G-Buffer occlusion target: rgb kept, alpha *= ao.
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_ao);
Sampler2D(g_depth);
Sampler2D(g_position);
Vec2  ssao_size;      //texels of the occlusion
float ssao_sharpness; //how much the depth differences cut the up sampling
float ssao_debug;     //1: debug view, the occlusion in grey (drawn without blending)

#include <SSAOCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssao_full_size;
	Vec4 g_pos = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	//background: unchanged
	if (g_pos.w < 0.5) return Vec4(1.0, 1.0, 1.0, 1.0);
	float ao;
	if (ssao_scale < 1.5)
	{
		ao = texture2DLod(g_ao, uv, 0.0).r;
	}
	else
	{
		float center = ssao_view_depth(g_pos.xyz);
		//texel coordinates of the pixel (texel t: pixel t * scale), the 4 around it
		Vec2  texel = (floor(input.m_position.xy) + 0.5) / ssao_scale - 0.5;
		Vec2  base = floor(texel);
		Vec2  f = texel - base;
		float sum = 0.0;
		float weights = 0.0;
		for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
		{
			Vec2  sample_uv = (base + Vec2(float(x), float(y)) + 0.5) / ssao_size;
			float bilinear = (x == 0 ? 1.0 - f.x : f.x) * (y == 0 ? 1.0 - f.y : f.y);
			//a little of the bilinear always: no surface of the 4 like this pixel, still smooth
			float weight = bilinear * (ssao_depth_weight(texture2DLod(g_depth, sample_uv, 0.0).r, center, ssao_sharpness) + 0.001);
			sum += texture2DLod(g_ao, sample_uv, 0.0).r * weight;
			weights += weight;
		}
		ao = weights > 0.0 ? sum / weights : 1.0;
	}
	if (ssao_debug > 0.5) return Vec4(ao, ao, ao, 1.0);
	return Vec4(1.0, 1.0, 1.0, ao);
}
