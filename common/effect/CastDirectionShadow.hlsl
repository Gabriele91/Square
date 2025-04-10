////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
////////////////
//global uniform
float shadow_mask;
//texture
Sampler2D(diffuse_map);
////////////////
// REFERENCES:
// DirectX http://developers-club.com/posts/259679/
// OpenGL https://learnopengl.com/Guest-Articles/2021/CSM
////////////////
struct VertexShaderOutput
{
	Vec4 m_position : TEXCOORD0;  // vertex position
	Vec2 m_uv       : TEXCOORD1;  // interpolated uv map
};

//draw
VertexShaderOutput vertex(in Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	Vec4 position = Vec4(input.m_position, 1.0);
	output.m_position = mul(position, transform.m_model);
	output.m_uv = input.m_uv;
	return output;
}

struct GeometryShaderOutput
{
	Vec4 m_position       : SV_POSITION;  // vertex position (system value)
	Vec4 m_world_position : POSITION1;    // vertex position in world space
	Vec2 m_uv             : TEXCOORD0;    // interpolated uv map
	uint m_RTIndex        : SV_RenderTargetArrayIndex;
};

#if 0
[maxvertexcount(3 * DIRECTION_SHADOW_CSM_NUMBER_OF_FACES)]
void geometry(triangle VertexShaderOutput input[3]
	        , inout TriangleStream<GeometryShaderOutput> output)
{
	GeometryShaderOutput outvertex = (GeometryShaderOutput)0;
	//for each cube faces
	[unroll]
	for (uint id = 0; id < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++id)
	{
		// Set index
		outvertex.m_RTIndex = id;
		// for each triangle's vertices
		[loop]
		for (int i = 0; i < 3; ++i)
		{
			outvertex.m_world_position = input[i].m_position;
			outvertex.m_position = mul_direction_light_view_projection(input[i].m_position, id);
			outvertex.m_uv = input[i].m_uv;
			output.Append(outvertex);
		}
		output.RestartStrip();
	}
}
#else
[maxvertexcount(3)]
[instance(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES)]
void geometry(triangle VertexShaderOutput input[3]
			, inout TriangleStream<GeometryShaderOutput> output
	        , uint id : SV_GSInstanceID)
{
	GeometryShaderOutput outvertex = (GeometryShaderOutput)0;
	// Set index
	outvertex.m_RTIndex = id;
	// for each triangle's vertices
	[loop]
	for (int i = 0; i < 3; ++i)
	{
		outvertex.m_world_position = input[i].m_position;
		outvertex.m_position = mul_direction_light_view_projection(input[i].m_position, id);
		outvertex.m_uv = input[i].m_uv;
		output.Append(outvertex);
	}
	output.RestartStrip();
}
#endif

struct FragmentShaderinput
{
	Vec4 m_position       : SV_POSITION;  // vertex position (system value)
	Vec4 m_world_position : POSITION1;    // vertex position in world space
	Vec2 m_uv             : TEXCOORD0;    // interpolated uv map
#ifdef HLSL_BACKEND
	uint m_RTIndex        : SV_RenderTargetArrayIndex;
#endif
};

void fragment(in FragmentShaderinput input)
{
	//diffuse/albedo
	Vec4 diffuse_color = texture2D(diffuse_map, input.m_uv);
    if (diffuse_color.a <= shadow_mask) discard;
}
