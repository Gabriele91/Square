////////////////
#pragma once
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <Matrix>
#include <SurfacePBR>
#include <GammaCorrection>
////////////////
struct VertexShaderOutput
{
	Vec4 m_position       : SV_POSITION;  // vertex position (system value)
	Vec2 m_uv             : TEXCOORD0;    // interpolated uv map
	Vec4 m_world_position : TEXCOORD1;    // vertex position (wolrd space)
	//TBN
	Vec3 m_normal   : TEXCOORD3;    // normal
	Vec3 m_tangent  : TEXCOORD4;    // normal
	Vec3 m_binomial : TEXCOORD5;    // normal
};

//texture
Sampler2D(albedo_map);
Sampler2D(emmisive_map);
Sampler2D(metallic_map);
Sampler2D(roughness_map);
Sampler2D(occlusion_map);
Sampler2D(normal_map);
//global uniform
Vec4  color;
float metallic;
float roughness;
Vec3 emmisive;
float mask;

VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_world_position = mul_model(input.m_position);
	output.m_position = mul_view_projection(output.m_world_position.xyz);
	output.m_uv = input.m_uv;

	Mat3 normal3x3    = (Mat3)transform.m_inv_model;
	output.m_normal   = mul(normal3x3, input.m_normal);
	output.m_tangent  = mul(normal3x3, input.m_tangent);
	output.m_binomial = mul(normal3x3, input.m_binomial);

	return output;
}

surface(VertexShaderOutput input)
{
	SurfaceData data = DefaultSurfaceData();
	// World position
	data.m_position = input.m_world_position;
	// Diffuse/albedo
	Vec4 albedo_color = to_rgb_space(texture2D(albedo_map, input.m_uv));
	if (albedo_color.a <= mask) discard;
	data.m_albedo = albedo_color.rgb * color.rgb;
	// Alpha
	data.m_alpha = albedo_color.a * color.a;
	// Emmisive
	data.m_emmisive = to_rgb_space(texture2D(emmisive_map, input.m_uv).rgb) * emmisive;
	// Normal
	Vec4 normal_color = texture2D(normal_map, input.m_uv);
	Mat3 TBN = Mat3(input.m_tangent, input.m_binomial, input.m_normal);
	data.m_normal = normalize(mul(normal_from_texture(normal_color), TBN));
	// AO
	data.m_occlusion = texture2D(occlusion_map, input.m_uv).r;
	// Metallic
	data.m_metallic = texture2D(metallic_map, input.m_uv).b * metallic; // glTF uses B channel for the metallic
	// Roughness
	data.m_roughness = texture2D(roughness_map, input.m_uv).g * roughness; // glTF uses G channel for the roughness
	//return
	surface_return(data);
}

