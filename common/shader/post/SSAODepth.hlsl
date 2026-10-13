//
//  SSAODepth.hlsl
//  Square
//
//  First pass of the SSAO (see PostEffectSSAO): the view depth of the G-Buffer pixel of each
//  texel (half size: one pixel of each 2x2), 0 on the background. The other passes read it
//  (4 bytes a sample) instead of the world positions (16 bytes).
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_position);

#include <SSAOCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = (floor(input.m_position.xy) * ssao_scale + 0.5) / ssao_full_size;
	Vec4 g_pos = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	//background
	if (g_pos.w < 0.5) return Vec4(0.0, 0.0, 0.0, 1.0);
	return Vec4(max(ssao_view_depth(g_pos.xyz), 0.0001), 0.0, 0.0, 1.0);
}
