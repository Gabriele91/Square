////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
#include <MultiPassInfo>
////////////////
//global uniform
float mask_shadow;
//texture
Sampler2D(albedo_map);
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
	//for each cascade of the light and of the caster
	[unroll]
	for (uint id = 0; id < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++id)
	{
		const bool of_light  = id < uint(direction_shadow_camera.m_options.y);
		const bool of_caster = MULTI_PASS_HAS_LAYER(multi_pass.m_mask, id);
		if (!of_light || !of_caster) continue;
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
	// a cascade of the light and of the caster
	const bool of_light  = id < uint(direction_shadow_camera.m_options.y);
	const bool of_caster = MULTI_PASS_HAS_LAYER(multi_pass.m_mask, id);
	if (!of_light || !of_caster) return;
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
	//albedo/albedo
	Vec4 albedo_color = texture2D(albedo_map, input.m_uv);
    if (albedo_color.a <= mask_shadow) discard;
}
