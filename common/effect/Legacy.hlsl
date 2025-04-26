////////////////
#pragma once
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <Matrix>
#include <SurfaceLegacy>
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

//global uniform
float mask;
Vec4 color;
float shininess;

//texture
Sampler2D(albedo_map);
Sampler2D(normal_map);
Sampler2D(specular_map);
Sampler2D(occlusion_map);

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
	// Note, in glTF 
    // RGB (metallic, norma maps)
    // SRGB (albedo)
    // SRGB (specular)
	//Albedo
	Vec4 albedo_color = to_rgb_space(texture2D(albedo_map, input.m_uv)); // sRGB
	if (albedo_color.a <= mask) discard;
	//material info
	Vec4 normal_color = texture2D(normal_map, input.m_uv);
	Vec4 specular_color = to_rgb_space(texture2D(specular_map, input.m_uv)); // sRGB
	Vec4 occlusion_color = texture2D(occlusion_map, input.m_uv); // RGB
	//output data
	SurfaceData data = DefaultSurfaceData();
	data.m_position = input.m_world_position;
	//color
	data.m_albedo = color.rgb * albedo_color.rgb;
	data.m_alpha = color.a * albedo_color.a;
	//norm info
	Mat3 TBN = Mat3(input.m_tangent, input.m_binomial, input.m_normal);
	data.m_normal = normalize(mul(normal_from_texture(normal_color), TBN));
	//material info
	data.m_shininess = shininess;
	data.m_specular = specular_color.rgb;
	data.m_occlusion = occlusion_color.r;
	//return
	surface_return(data);
}
