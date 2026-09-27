//
//  SSAOApply.hlsl
//  Square
//
//  Second pass of the SSAO (see PostEffectSSAO): 4x4 blur of the raw occlusion (it removes
//  the 4x4 rotation pattern of the samples), weighted by the distance of the world
//  positions (it does not bleed over the edges). Output Vec4(1, 1, 1, ao), drawn with a
//  (ZERO, SRC_COLOR) blend on the G-Buffer occlusion target: rgb kept, alpha *= ao.
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_ao);
Sampler2D(g_position);
Vec2  ssao_size; //pixels of the frame
float ssao_blur;  //1: blur, 0: raw
float ssao_debug; //1: debug view, the occlusion in grey (drawn without blending)

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssao_size;
	Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
	//background: unchanged
	if (g_pos.w < 0.5) return Vec4(1.0, 1.0, 1.0, 1.0);
	float ao = texture2DLod(g_ao, uv, 0.0).r;
	if (ssao_blur > 0.5)
	{
		float sum = 0.0;
		float weights = 0.0;
		for (int y = -2; y < 2; ++y)
		for (int x = -2; x < 2; ++x)
		{
			Vec2  sample_uv = uv + Vec2(float(x), float(y)) / ssao_size;
			Vec4  sample_pos = texture2DLod(g_position, sample_uv, 0.0); //in a loop: no derivatives
			//the samples of other surfaces count less (edges kept), background not at all
			float weight = sample_pos.w < 0.5 ? 0.0 : 1.0 / (1.0 + 4.0 * length(sample_pos.xyz - g_pos.xyz));
			sum += texture2DLod(g_ao, sample_uv, 0.0).r * weight;
			weights += weight;
		}
		ao = weights > 0.0 ? sum / weights : ao;
	}
	if (ssao_debug > 0.5) return Vec4(ao, ao, ao, 1.0);
	return Vec4(1.0, 1.0, 1.0, ao);
}
