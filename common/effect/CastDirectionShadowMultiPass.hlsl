////////////////
#pragma once
#include <Vertex>
#include <Transform>
#include <MeshVertex>
#include <Matrix>
#include <ShadowCamera>
#include <MultiPassInfo>
////////////////
// Multi-pass directional CSM shadow (texture array) for backends WITHOUT geometry
// shaders (Metal / GLES / Vulkan-without-GS). The host draws the casters once per
// cascade; multi_pass.m_id selects both the view-projection AND the destination
// array layer (SV_RenderTargetArrayIndex), so the render target is bound once.
// SQ_LAYER_BY_TARGET (CastDirectionShadowLayerTarget.hlsl): the host binds the layer of each
// cascade as the target, no layer written (a driver that can not route a draw to a layer).
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
#ifndef SQ_LAYER_BY_TARGET
	uint m_layer    : SV_RenderTargetArrayIndex; // destination array layer (cascade)
#endif
};

//draw
VertexShaderOutput vertex(in MeshVertex input)
{
	VertexShaderOutput output;
	Vec4 world_position = mesh_world_position(input);
	output.m_position = direction_light_clip_as_layered(mul_direction_light_view_projection(world_position, multi_pass.m_id));
	output.m_uv       = input.m_uv;
#ifndef SQ_LAYER_BY_TARGET
	output.m_layer    = multi_pass.m_id;
#endif
	return output;
}

struct FragmentShaderInput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
#if defined(SQ_BACKEND_HLSL) && !defined(SQ_LAYER_BY_TARGET)
	uint m_layer    : SV_RenderTargetArrayIndex;
#endif
};

void fragment(in FragmentShaderInput input)
{
	//albedo
	Vec4 albedo_color = texture2D(albedo_map, input.m_uv);
#ifdef SQ_CLIP
	//(the clip variant: its mask of the shadow, its level of detail fading)
	if (albedo_color.a <= mask_shadow) discard;
	lod_fade_clip(input.m_position.xy);
#endif
}
