//
//  SSAOBlur.hlsl
//  Square
//
//  Blur of the raw occlusion (see PostEffectSSAO), at its size, weighted by the view depth
//  (it does not bleed over the edges):
//   mode 0: box of side radius, 2x2 or 4x4 (4x4: it removes the 4x4 rotation pattern of the
//           samples), one pass;
//   mode 1: gaussian along a direction (sigma: radius / 2), run horizontal then vertical.
//  Output: occlusion in r.
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_ao);
Sampler2D(g_depth);
Vec2  ssao_size;      //texels of the occlusion
Vec4  ssao_blur;      //mode, radius (box: side, 2 or 4; gaussian: texels, at most 6), direction
float ssao_sharpness; //how much the depth differences cut the blur

#include <SSAOCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2  uv = input.m_position.xy / ssao_size;
	float center = texture2DLod(g_depth, uv, 0.0).r;
	float ao = texture2DLod(g_ao, uv, 0.0).r;
	//background: unchanged
	if (center <= 0.0) return Vec4(ao, ao, ao, 1.0);
	float sum = 0.0;
	float weights = 0.0;
	if (ssao_blur.x < 0.5)
	{
		//offsets from -side / 2 to side / 2 - 1
		float first = -floor(ssao_blur.y * 0.5);
		float last = first + ssao_blur.y - 1.0;
		for (int y = -2; y < 2; ++y)
		for (int x = -2; x < 2; ++x)
		{
			if (float(x) < first || float(x) > last || float(y) < first || float(y) > last) continue;
			Vec2  sample_uv = uv + Vec2(float(x), float(y)) / ssao_size;
			float weight = ssao_depth_weight(texture2DLod(g_depth, sample_uv, 0.0).r, center, ssao_sharpness); //in a loop: no derivatives
			sum += texture2DLod(g_ao, sample_uv, 0.0).r * weight;
			weights += weight;
		}
	}
	else
	{
		float radius = ssao_blur.y;
		float inv_two_sigma2 = 1.0 / (2.0 * max(radius * 0.5, 0.5) * max(radius * 0.5, 0.5));
		Vec2  texel_step = ssao_blur.zw / ssao_size;
		for (int i = -6; i <= 6; ++i)
		{
			float offset = float(i);
			if (abs(offset) > radius) continue;
			Vec2  sample_uv = uv + texel_step * offset;
			float weight = exp(-offset * offset * inv_two_sigma2)
			             * ssao_depth_weight(texture2DLod(g_depth, sample_uv, 0.0).r, center, ssao_sharpness);
			sum += texture2DLod(g_ao, sample_uv, 0.0).r * weight;
			weights += weight;
		}
	}
	ao = weights > 0.0 ? sum / weights : ao;
	return Vec4(ao, ao, ao, 1.0);
}
