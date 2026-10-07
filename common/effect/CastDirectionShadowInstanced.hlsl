////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <Matrix>
#include <ShadowCamera>
#include <MultiPassInfo>
////////////////
// Instanced directional CSM shadow (texture array) for backends WITH instanced draw
// AND the ability to write SV_RenderTargetArrayIndex from the vertex shader (Metal,
// OpenGL with GL_ARB_shader_viewport_layer_array). A single instanced draw of N
// cascades renders the whole array: SV_InstanceID selects both the view-projection
// AND the destination array layer, so the render target is bound once and only one
// draw call is issued (vs. the N draws of the multi-pass variant).
////////////////
//global uniform
float mask_shadow;
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
	// a cascade not of the light or of the caster: a point (the triangle has no area)
	const bool of_light  = instance_id < uint(direction_shadow_camera.m_options.y);
	const bool of_caster = MULTI_PASS_HAS_LAYER(multi_pass.m_mask, instance_id);
	if (!of_light || !of_caster) output.m_position = Vec4(0.0, 0.0, 0.0, 1.0);
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
	if (albedo_color.a <= mask_shadow) discard;
	//its level of detail fading in or out
	lod_fade_clip(input.m_position.xy);
}
