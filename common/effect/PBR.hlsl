////////////////
#pragma once
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <Matrix>
// NOTE: GammaCorrection must be included BEFORE SurfacePBR: the include
// preprocessor expands #include inside dead #if branches too, and marks the
// files as already-included (pragma once); including it afterwards would leave
// its code only inside the discarded rendering branch.
#include <GammaCorrection>
#include <SurfacePBR>
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
float dither;

// Dithered opacity: 4x4 ordered (Bayer) threshold in (0,1) of a screen pixel.
// A pixel is kept when its alpha is above the threshold, so the share of kept
// pixels follows the alpha and the surface stays opaque (deferred, shadows...).
float dither_threshold(Vec2 pixel)
{
	static const float bayer[16] =
	{
		 0.0,  8.0,  2.0, 10.0,
		12.0,  4.0, 14.0,  6.0,
		 3.0, 11.0,  1.0,  9.0,
		15.0,  7.0, 13.0,  5.0
	};
	uint2 p = uint2(pixel) & 3;
	return (bayer[p.y * 4 + p.x] + 0.5) / 16.0;
}

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
    // SRGB (emmisive, albedo)
	SurfaceData data = DefaultSurfaceData();
	// World position
	data.m_position = input.m_world_position;
	// Diffuse/albedo
	Vec4 albedo_color = to_rgb_space(texture2D(albedo_map, input.m_uv));
	if (albedo_color.a <= mask) discard;
	data.m_albedo = albedo_color.rgb * color.rgb;
	// Alpha
	data.m_alpha = albedo_color.a * color.a;
	// Dithered opacity: drop the pixels of the pattern above the alpha
	if (dither > 0.5)
	{
		if (data.m_alpha <= dither_threshold(input.m_position.xy)) discard;
		data.m_alpha = 1.0;
	}
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

