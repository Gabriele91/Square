////////////////
//  TrailsPBR: a map over the ground (from above, world x/z) of how deep it is pressed (R, [0, 1]:
//  a groove), drawn by the game (SnowTrails): the pressed ground darker, smoother, its normal
//  following the sides of the groove, seen into it (parallax). Where it can be pressed: a mask
//  of the effect (the snow of PBRSnow: its albedo alpha; the soft layers of PBRTerrainTrails)
////////////////
#pragma once
#include <MaterialPBR>

Sampler2D(trail_map);
Vec4 trail_area;  // x, z of its corner (world); 1 / its size along x, z
Vec4 trail_style; // darkening, slope of the sides (normal), roughness kept, depth (world units)

// the depth of the trails at a world point x/z, [0, 1]
float trail_depth(Vec2 world_xz)
{
	return texture2DLod(trail_map, (world_xz - trail_area.xy) * trail_area.zw, 0.0).r;
}

// parallax into the grooves: from the surface the view ray goes down (layers of the depth of a
// groove) until it is under the pressed ground; the point (x/z) seen there. On the flat ground
// (no depth) it stops at once
Vec2 trail_parallax(Vec3 world)
{
	const int steps = 14;
	Vec3  view = normalize(world - camera.m_position);
	// x/z of the ray per layer (grazing views clamped: no long smears)
	Vec2  step_xz = view.xz / max(-view.y, 0.2) * (trail_style.w / float(steps));
	float layer = 0.0;
	Vec2  p = world.xz;
	float depth = trail_depth(p);
	Vec2  previous_p = p;
	float previous_gap = depth;
	for (int i = 0; i < steps; ++i)
	{
		if (layer >= depth) break;
		previous_p = p;
		previous_gap = depth - layer;
		p += step_xz;
		layer += 1.0 / float(steps);
		depth = trail_depth(p);
	}
	// between the last two layers: where the ray met the ground
	float gap = layer - depth;
	float t = previous_gap + gap > 0.0001 ? previous_gap / (previous_gap + gap) : 1.0;
	return lerp(previous_p, p, saturate(t));
}

// the trails pressed in a surface; where: how much it can be pressed there ([0, 1])
void apply_trails(inout SurfaceData data, VertexShaderOutput input, float where)
{
	// the point of the groove seen (parallax), its depth, its slope from the texels around
	Vec2  trail_seen = where > 0.0 ? trail_parallax(input.m_world_position.xyz) : input.m_world_position.xz;
	Vec2  trail_uv = (trail_seen - trail_area.xy) * trail_area.zw;
	Vec2  trail_texel = 1.0 / textureSize2D(trail_map, 0);
	float trail = texture2DLod(trail_map, trail_uv, 0.0).r * where;
	float trail_dx = texture2DLod(trail_map, trail_uv + Vec2(trail_texel.x, 0.0), 0.0).r - texture2DLod(trail_map, trail_uv - Vec2(trail_texel.x, 0.0), 0.0).r;
	float trail_dz = texture2DLod(trail_map, trail_uv + Vec2(0.0, trail_texel.y), 0.0).r - texture2DLod(trail_map, trail_uv - Vec2(0.0, trail_texel.y), 0.0).r;
	// the ground lower where it is deeper: the normal leans toward the deeper side
	data.m_normal = normalize(data.m_normal + Vec3(trail_dx, 0.0, trail_dz) * (trail_style.y * where));
	data.m_albedo *= 1.0 - trail * trail_style.x;
	data.m_roughness = lerp(data.m_roughness, data.m_roughness * trail_style.z, trail);
}
