////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
////////////////
struct VertexShaderOutput
{
	Vec4 m_position : SV_POSITION;  // vertex position (system value)
	Vec2 m_uv : TEXCOORD0;    // interpolated uv map
};

//global uniform
float mask;

//texture
Sampler2D(diffuse_map);

//draw
VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_position = mul_model_spot_light_view_projection(input.m_position);
	output.m_uv = input.m_uv;
	return output;
}

void fragment(VertexShaderOutput input)
{
	//diffuse/albedo
	Vec4 diffuse_color = texture2D(diffuse_map, input.m_uv);
	if (diffuse_color.a <= mask) discard;
}