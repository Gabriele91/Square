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
float mask_shadow;

//texture
Sampler2D(albedo_map);

//draw
#ifdef SQ_INSTANCED
// instanced (the instanced variant of SpotShadow: Scene::InstancedMesh): the matrix of the instance first
#include <Instances>
VertexShaderOutput vertex(in Position3DNormalTangetBinomialUV input, uint instance_id : SV_InstanceID)
{
	VertexShaderOutput output;
	output.m_position = mul_spot_light_view_projection(mul_instance_model(input.m_position, instance_id));
	output.m_uv = input.m_uv;
	return output;
}
#else
VertexShaderOutput vertex(in Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_position = mul_model_spot_light_view_projection(input.m_position);
	output.m_uv = input.m_uv;
	return output;
}
#endif

void fragment(in VertexShaderOutput input)
{
	//albedo/albedo
	Vec4 albedo_color = texture2D(albedo_map, input.m_uv);
#ifdef SQ_CLIP
	//(the clip variant: its mask of the shadow, its level of detail fading)
	if (albedo_color.a <= mask_shadow) discard;
	lod_fade_clip(input.m_position.xy);
#endif
}
