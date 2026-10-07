////////////////
//  TerrainPBR: five grounds blended over a terrain by a map of their weights (the albedo_map,
//  over the uv of the terrain: R, G, B, A the layers 1 to 4, what is left the layer 0), each
//  ground tiled in world x/z: its albedo (its roughness in its alpha), its normal (OpenGL). The
//  weights sharpened by the height of each ground (the brightness of its albedo: the sand in the
//  cracks of the rock, the grass over the dirt), its tiling broken by a far, slow variation
////////////////
#pragma once
#include <MaterialPBR>

Sampler2D(layer0_map);
Sampler2D(layer0_normal);
Sampler2D(layer1_map);
Sampler2D(layer1_normal);
Sampler2D(layer2_map);
Sampler2D(layer2_normal);
Sampler2D(layer3_map);
Sampler2D(layer3_normal);
Sampler2D(layer4_map);
Sampler2D(layer4_normal);
Vec4 terrain_tiling; // x: 1 / the size of a tile of the grounds (world), y: the sharpness of the edges, z: the variation far

// the surface of the terrain; layers: how much each of the layers 1 to 4 shows (after the height
// blend, [0, 1]: the layer 0 what is left)
SurfaceData material_terrain(VertexShaderOutput input, out Vec4 layers)
{
	SurfaceData data = DefaultSurfaceData();
	data.m_position = input.m_world_position;
	// the weights (the layer 0 what is left)
	Vec4  splat = texture2D(albedo_map, input.m_uv);
	float weights[5] = { saturate(1.0 - splat.r - splat.g - splat.b - splat.a), splat.r, splat.g, splat.b, splat.a };
	// the grounds tiled in world x/z (v along -z: the normal maps of OpenGL), far a slower tile
	Vec2  uv = Vec2(input.m_world_position.x, -input.m_world_position.z) * terrain_tiling.x;
	Vec4  albedos[5];
	Vec3  normals[5];
	albedos[0] = to_rgb_space(texture2D(layer0_map, uv)); normals[0] = normal_from_texture(texture2D(layer0_normal, uv));
	albedos[1] = to_rgb_space(texture2D(layer1_map, uv)); normals[1] = normal_from_texture(texture2D(layer1_normal, uv));
	albedos[2] = to_rgb_space(texture2D(layer2_map, uv)); normals[2] = normal_from_texture(texture2D(layer2_normal, uv));
	albedos[3] = to_rgb_space(texture2D(layer3_map, uv)); normals[3] = normal_from_texture(texture2D(layer3_normal, uv));
	albedos[4] = to_rgb_space(texture2D(layer4_map, uv)); normals[4] = normal_from_texture(texture2D(layer4_normal, uv));
	// height blend: a ground shows where its weight plus its height is near the highest
	// (written out for the five grounds: no loops)
	float t0 = weights[0] * (1.0 + dot(albedos[0].rgb, Vec3(0.333, 0.333, 0.333)) * terrain_tiling.y);
	float t1 = weights[1] * (1.0 + dot(albedos[1].rgb, Vec3(0.333, 0.333, 0.333)) * terrain_tiling.y);
	float t2 = weights[2] * (1.0 + dot(albedos[2].rgb, Vec3(0.333, 0.333, 0.333)) * terrain_tiling.y);
	float t3 = weights[3] * (1.0 + dot(albedos[3].rgb, Vec3(0.333, 0.333, 0.333)) * terrain_tiling.y);
	float t4 = weights[4] * (1.0 + dot(albedos[4].rgb, Vec3(0.333, 0.333, 0.333)) * terrain_tiling.y);
	float highest = max(max(max(t0, t1), max(t2, t3)), t4);
	float b0 = max(t0 - highest + 0.2, 0.0) * step(0.001, weights[0]);
	float b1 = max(t1 - highest + 0.2, 0.0) * step(0.001, weights[1]);
	float b2 = max(t2 - highest + 0.2, 0.0) * step(0.001, weights[2]);
	float b3 = max(t3 - highest + 0.2, 0.0) * step(0.001, weights[3]);
	float b4 = max(t4 - highest + 0.2, 0.0) * step(0.001, weights[4]);
	Vec3  albedo = albedos[0].rgb * b0 + albedos[1].rgb * b1 + albedos[2].rgb * b2 + albedos[3].rgb * b3 + albedos[4].rgb * b4;
	Vec3  bump = normals[0] * b0 + normals[1] * b1 + normals[2] * b2 + normals[3] * b3 + normals[4] * b4;
	float rough = albedos[0].a * b0 + albedos[1].a * b1 + albedos[2].a * b2 + albedos[3].a * b3 + albedos[4].a * b4;
	float total = b0 + b1 + b2 + b3 + b4;
	total = max(total, 1e-4);
	layers = Vec4(b1, b2, b3, b4) / total;
	// the tiling broken: a slow variation of the light of the grounds
	float far = dot(to_rgb_space(texture2D(layer0_map, uv * terrain_tiling.z)).rgb, Vec3(0.333, 0.333, 0.333));
	data.m_albedo = albedo / total * lerp(0.82, 1.18, saturate(far * 2.0));
	// the normal: the one of the grounds in the frame of the terrain (world x, -z)
	Vec3 n = normalize(input.m_normal);
	Vec3 t = normalize(Vec3(1.0, 0.0, 0.0) - n * n.x);
	//(OpenGL maps flipped to DX by Model, back by normal_from_texture: y up the texture, -z)
	Vec3 bi = normalize(Vec3(0.0, 0.0, -1.0) + n * n.z);
	Vec3 bent = bump / total;
	data.m_normal = normalize(t * bent.x + bi * bent.y + n * max(bent.z, 0.2));
	data.m_roughness = rough / total;
	data.m_metallic = 0.0;
	data.m_occlusion = 1.0;
	data.m_alpha = 1.0;
	data.m_emmisive = Vec3(0.0, 0.0, 0.0);
	return data;
}
