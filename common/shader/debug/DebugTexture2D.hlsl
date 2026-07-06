//
//  DebugTexture2D.hlsl
//  Square
//
//  Debug texture panel: draws a 2D texture inside a screen-space rect.
//  rect.xy = bottom-left corner in NDC, rect.zw = size in NDC;
//  params.x = grayscale (depth textures), params.y = flip V.
//
#include <Vertex>

Sampler2D(g_texture);

Vec4 rect;
Vec4 params;

struct DebugTextureVSOutput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
};

DebugTextureVSOutput vertex(Position3D input)
{
	DebugTextureVSOutput output;
	Vec2 uv = input.m_position.xy * 0.5 + 0.5;
	output.m_position = Vec4(rect.xy + uv * rect.zw, 0.0, 1.0);
	output.m_uv = uv;
	return output;
}

Vec4 fragment(DebugTextureVSOutput input) : SV_TARGET0
{
	Vec2 uv = Vec2(input.m_uv.x, params.y > 0.5 ? 1.0 - input.m_uv.y : input.m_uv.y);
	Vec4 color = texture2D(g_texture, uv);
	if (params.x > 0.5) color = Vec4(color.rrr, 1.0);
	color.a = 1.0;
	return color;
}
