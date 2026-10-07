////////////////
//  SnowCoverPBR: snow laid on what faces up (the world normal), its edge broken by the brightness
//  of the surface under it (the snow in the hollows and on the ledges first, the cracks out), the
//  bumps of the surface smoothed under it; the snow a texture tiled from above
////////////////
#pragma once
#include <MaterialPBR>

Sampler2D(snow_cover_map);
Vec4 snow_cover; // normal y where it starts, softness of its edge, 1 / size of its texture (world), its roughness

// the snow laid on a surface
void apply_snow_cover(inout SurfaceData data, VertexShaderOutput input)
{
	Vec3  snow_albedo = to_rgb_space(texture2D(snow_cover_map, input.m_world_position.xz * snow_cover.z)).rgb;
	float bright = dot(data.m_albedo, Vec3(0.333, 0.333, 0.333));
	float up = data.m_normal.y + (bright - 0.25) * 0.5;
	float cover = smoothstep(snow_cover.x - snow_cover.y, snow_cover.x + snow_cover.y, up);
	data.m_albedo = lerp(data.m_albedo, snow_albedo, cover);
	data.m_normal = normalize(lerp(data.m_normal, normalize(input.m_normal), cover * 0.7));
	data.m_roughness = lerp(data.m_roughness, snow_cover.w, cover);
	data.m_metallic *= 1.0 - cover;
}
