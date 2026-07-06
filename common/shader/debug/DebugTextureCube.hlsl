//
//  DebugTextureCube.hlsl
//  Square
//
//  Debug texture panel: draws the six faces of a cube texture side by side
//  (+X -X +Y -Y +Z -Z) inside a screen-space rect.
//  rect.xy = bottom-left corner in NDC, rect.zw = size in NDC;
//  params.x = grayscale (depth textures), params.y = flip V.
//
#include <Vertex>

SamplerCube(g_texture);

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
	//face strip: uv.x in [0,1] -> 6 columns
	float face_pos = min(uv.x * 6.0, 5.9999);
	int   face     = int(face_pos);
	Vec2  st       = Vec2(frac(face_pos), uv.y) * 2.0 - 1.0;
	Vec3  dir;
	     if (face == 0) dir = Vec3( 1.0,  st.y, -st.x); //+X
	else if (face == 1) dir = Vec3(-1.0,  st.y,  st.x); //-X
	else if (face == 2) dir = Vec3( st.x,  1.0,  st.y); //+Y
	else if (face == 3) dir = Vec3( st.x, -1.0, -st.y); //-Y
	else if (face == 4) dir = Vec3( st.x,  st.y,  1.0); //+Z
	else                dir = Vec3(-st.x,  st.y, -1.0); //-Z
	Vec4 color = textureCube(g_texture, dir);
	if (params.x > 0.5) color = Vec4(color.rrr, 1.0);
	color.a = 1.0;
	return color;
}
