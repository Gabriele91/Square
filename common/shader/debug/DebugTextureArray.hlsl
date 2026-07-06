//
//  DebugTextureArray.hlsl
//  Square
//
//  Debug texture panel: draws the layers of a 2D texture array side by side
//  (e.g. CSM shadow maps) inside a screen-space rect.
//  rect.xy = bottom-left corner in NDC, rect.zw = size in NDC;
//  params.x = grayscale (depth textures), params.y = flip V, params.z = layers.
//
#include <Vertex>

Sampler2DArray(g_texture);

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
	//layer strip: uv.x in [0,1] -> params.z columns
	float layers    = max(params.z, 1.0);
	float layer_pos = min(uv.x * layers, layers - 0.0001);
	float layer     = floor(layer_pos);
	Vec2  st        = Vec2(layer_pos - layer, uv.y);
	Vec4  color     = texture2DArray(g_texture, Vec3(st, layer));
	if (params.x > 0.5) color = Vec4(color.rrr, 1.0);
	color.a = 1.0;
	return color;
}
