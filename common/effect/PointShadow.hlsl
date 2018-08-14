////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
////////////////
//global uniform
float mask;
//texture
Sampler2D(diffuse_map);
////////////////
// REFERENCES:
// DirectX http://developers-club.com/posts/259679/
// OpenGL https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
////////////////
struct VertexShaderOutput
{
	Vec4 m_position : TEXCOORD0;  // vertex position
	Vec2 m_uv       : TEXCOORD1;    // interpolated uv map
};


//draw
VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
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
	Vec2 m_uv             : TEXCOORD1;    // interpolated uv map
	uint m_RTIndex        : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void geometry(triangle VertexShaderOutput input[3]
			, inout TriangleStream<GeometryShaderOutput> output)
{
	GeometryShaderOutput outvertex = (GeometryShaderOutput)0;
	//for each cube faces
	[unroll]
	for (uint face = 0; face < 6; ++face)
	{
		// set cubeface
		outvertex.m_RTIndex = face;
		// for each triangle's vertices
		[unroll]
		for (int i = 0; i < 3; ++i)
		{
			outvertex.m_world_position = input[i].m_position;
			outvertex.m_position = mul_point_light_view_projection(input[i].m_position, face);
			outvertex.m_uv = input[i].m_uv;
			output.Append(outvertex);
		}
		//end primitive
		output.RestartStrip();
	}
}

struct FragmentShaderinput
{
	Vec4 m_position       : SV_POSITION;  // vertex position (system value)
	Vec4 m_world_position : POSITION1;    // vertex position in world space
	Vec2 m_uv             : TEXCOORD1;    // interpolated uv map
#ifdef HLSL_BACKEND
	uint m_RTIndex        : SV_RenderTargetArrayIndex;
#endif
};

#if 1
float fragment(FragmentShaderinput input) : SV_Depth
{
	//diffuse/albedo
	//Vec4 diffuse_color = texture2D(diffuse_map, input.m_uv);
	//if (diffuse_color.a <= mask) discard; 
	//compute distance between wolrd and light source
	float light_distance = length(point_shadow_camera.m_position - input.m_world_position.xyz);
	//[0,1] range
	light_distance = light_distance / point_shadow_camera.m_radius;
	//write as depth
	return light_distance;
}
#endif