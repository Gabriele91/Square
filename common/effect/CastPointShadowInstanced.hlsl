////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
////////////////
// Instanced point-light shadow (cube map) for backends WITH instanced draw AND the
// ability to write SV_RenderTargetArrayIndex from the vertex shader (Metal, OpenGL
// with GL_ARB_shader_viewport_layer_array). A single instanced draw of 6 instances
// renders the whole cube: SV_InstanceID selects both the view-projection AND the
// destination cube face, so the render target is bound once and only one draw call
// is issued (vs. the 6 draws of the multi-pass variant).
////////////////
//global uniform
float shadow_mask;
//texture
Sampler2D(albedo_map);
////////////////
struct VertexShaderOutput
{
	Vec4 m_position       : SV_POSITION;             // clip-space position for the face
	Vec4 m_world_position : POSITION1;               // world-space position
	Vec2 m_uv             : TEXCOORD0;               // interpolated uv map
	uint m_layer          : SV_RenderTargetArrayIndex; // destination cube face
};

//draw
VertexShaderOutput vertex(in Position3DNormalTangetBinomialUV input, uint instance_id : SV_InstanceID)
{
	VertexShaderOutput output;
	Vec4 world_position = mul(Vec4(input.m_position, 1.0), transform.m_model);
	output.m_world_position = world_position;
	output.m_position       = mul_point_light_view_projection(world_position, instance_id);
	output.m_uv             = input.m_uv;
	output.m_layer          = instance_id;
	return output;
}

struct FragmentShaderInput
{
	Vec4 m_position       : SV_POSITION;
	Vec4 m_world_position : POSITION1;
	Vec2 m_uv             : TEXCOORD0;
#ifdef HLSL_BACKEND
	uint m_layer          : SV_RenderTargetArrayIndex;
#endif
};

float fragment(in FragmentShaderInput input) : SV_Depth
{
	//albedo
	Vec4 albedo_color = texture2D(albedo_map, input.m_uv);
	if (albedo_color.a <= shadow_mask) discard;
	//compute distance between world and light source
	float light_distance = length(point_shadow_camera.m_position - input.m_world_position.xyz);
	//[0,1] range
	light_distance = light_distance / point_shadow_camera.m_radius;
	//write as depth
	return light_distance;
}
