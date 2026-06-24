////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
////////////////
// Instanced directional CSM shadow (texture array) for backends WITH instanced draw
// AND the ability to write SV_RenderTargetArrayIndex from the vertex shader (Metal,
// OpenGL with GL_ARB_shader_viewport_layer_array). A single instanced draw of N
// cascades renders the whole array: SV_InstanceID selects both the view-projection
// AND the destination array layer, so the render target is bound once and only one
// draw call is issued (vs. the N draws of the multi-pass variant).
////////////////
//global uniform
float shadow_mask;
//texture
Sampler2D(albedo_map);
////////////////
struct VertexShaderOutput
{
	Vec4 m_position : SV_POSITION;               // clip-space position for the cascade
	Vec2 m_uv       : TEXCOORD0;                 // interpolated uv map
	uint m_layer    : SV_RenderTargetArrayIndex; // destination array layer (cascade)
};

//draw
VertexShaderOutput vertex(in Position3DNormalTangetBinomialUV input, uint instance_id : SV_InstanceID)
{
	VertexShaderOutput output;
	Vec4 world_position = mul(Vec4(input.m_position, 1.0), transform.m_model);
	output.m_position = mul_direction_light_view_projection(world_position, instance_id);
	output.m_uv       = input.m_uv;
	output.m_layer    = instance_id;
	return output;
}

struct FragmentShaderInput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
#ifdef HLSL_BACKEND
	uint m_layer    : SV_RenderTargetArrayIndex;
#endif
};

void fragment(in FragmentShaderInput input)
{
	//albedo
	Vec4 albedo_color = texture2D(albedo_map, input.m_uv);
	if (albedo_color.a <= shadow_mask) discard;
}
