////////////////
//  MaterialPBR: what the PBR effects share (PBR, PBRSnowCover, PBRTerrain, PBRWater...; a game can
//  build its own on it, e.g. the trails of Rush): the output of the vertex shader, the vertex
//  shader, the textures and the values of a material of glTF, the standard surface read from them.
//  An effect includes it (and the modules of its surface: TerrainPBR, SnowCoverPBR, WaterPBR),
//  its surface() made of them.
////////////////
#pragma once
#include <Camera>
#include <AnimatedUV>
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
float translucency;

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

#ifdef SQ_INSTANCED
// instanced (the "_instanced" techniques: Scene::InstancedMesh): the matrix of the instance first
#include <Instances>
VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input, uint instance_id : SV_InstanceID)
{
	VertexShaderOutput output;
	output.m_world_position = mul_instance_model(input.m_position, instance_id);
	output.m_position = mul_view_projection(output.m_world_position.xyz);
	output.m_uv = animated_uv(input.m_uv);

	Mat3 normal3x3    = (Mat3)transform.m_inv_model;
	output.m_normal   = mul(normal3x3, mul_instance_direction(input.m_normal, instance_id));
	output.m_tangent  = mul(normal3x3, mul_instance_direction(input.m_tangent, instance_id));
	output.m_binomial = mul(normal3x3, mul_instance_direction(input.m_binomial, instance_id));

	return output;
}
#elif defined(SQ_SKINNED)
// skinned (Scene::SkinnedMesh): moved by its joints, already in the world
#include <Skin>
VertexShaderOutput vertex(Position3DNormalTangetBinomialUVSkin input)
{
	VertexShaderOutput output;
	const Mat4 skin = skin_matrix(input.m_joints, input.m_weights);
	output.m_world_position = mul_skin(input.m_position, skin);
	output.m_position = mul_view_projection(output.m_world_position.xyz);
	output.m_uv = animated_uv(input.m_uv);

	output.m_normal   = mul_skin_direction(input.m_normal, skin);
	output.m_tangent  = mul_skin_direction(input.m_tangent, skin);
	output.m_binomial = mul_skin_direction(input.m_binomial, skin);

	return output;
}
#else
VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_world_position = mul_model(input.m_position);
	output.m_position = mul_view_projection(output.m_world_position.xyz);
	output.m_uv = animated_uv(input.m_uv);

	Mat3 normal3x3    = (Mat3)transform.m_inv_model;
	output.m_normal   = mul(normal3x3, input.m_normal);
	output.m_tangent  = mul(normal3x3, input.m_tangent);
	output.m_binomial = mul(normal3x3, input.m_binomial);

	return output;
}
#endif

// the frame of the normal map (tangent, binormal, normal)
Mat3 material_tbn(VertexShaderOutput input)
{
	return Mat3(input.m_tangent, input.m_binomial, input.m_normal);
}

// the surface of a material of glTF: its maps by their uv (the pixels under the mask dropped,
// the dithered opacity); albedo_alpha: the alpha of its albedo (a mask of the surface for some
// effects: the snow of PBRSnow)
SurfaceData material_standard(VertexShaderOutput input, out float albedo_alpha)
{
	// Note, in glTF
	// RGB (metallic, norma maps)
	// SRGB (emmisive, albedo)
	SurfaceData data = DefaultSurfaceData();
#ifdef SQ_CLIP
	// Its level of detail fading in or out (the clip variant: else no discard, early-z)
	lod_fade_clip(input.m_position.xy);
#endif
	// World position
	data.m_position = input.m_world_position;
	// Diffuse/albedo
	Vec4 albedo_color = to_rgb_space(texture2D(albedo_map, input.m_uv));
#ifdef SQ_CLIP
	if (albedo_color.a <= mask) discard;
#endif
	albedo_alpha = albedo_color.a;
	data.m_albedo = albedo_color.rgb * color.rgb;
	// Alpha
	data.m_alpha = albedo_color.a * color.a;
	// Dithered opacity: drop the pixels of the pattern above the alpha
#ifdef SQ_CLIP
	if (dither > 0.5)
	{
		if (data.m_alpha <= dither_threshold(input.m_position.xy)) discard;
		data.m_alpha = 1.0;
	}
#endif
	// Light of the sun through it (thin: leaves)
	data.m_translucency = translucency;
	// Emmisive
	data.m_emmisive = to_rgb_space(texture2D(emmisive_map, input.m_uv).rgb) * emmisive;
	// Normal
	Vec4 normal_color = texture2D(normal_map, input.m_uv);
	data.m_normal = normalize(mul(normal_from_texture(normal_color), material_tbn(input)));
	// AO
	data.m_occlusion = texture2D(occlusion_map, input.m_uv).r;
	// Metallic
	data.m_metallic = texture2D(metallic_map, input.m_uv).b * metallic; // glTF uses B channel for the metallic
	// Roughness
	data.m_roughness = texture2D(roughness_map, input.m_uv).g * roughness; // glTF uses G channel for the roughness
	return data;
}
