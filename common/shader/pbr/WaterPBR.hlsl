////////////////
//  WaterPBR: its texture flowing (a waterfall: water_flow.xy), two layers of ripples of the
//  normal map moving across each other, a Fresnel: clear where looked at from above, the sky
//  reflected where looked at grazing (a gradient: its horizon, its top), the lights (the glint of
//  the sun) as on any surface. water_time.x: the seconds of the game (set every frame)
////////////////
#pragma once
#include <MaterialPBR>

Vec4 water_time;  // x: seconds
Vec4 water_flow;  // xy: the speed of its uv (per second), z: the scale of the ripples (uv), w: their strength
Vec4 water_style; // x: alpha looking down, y: alpha grazing, z: the reflection, w: the Fresnel power
Vec4 water_sky;   // rgb: the sky at the horizon (linear)
Vec4 water_top;   // rgb: the sky at the top (linear)
// a waterfall (water_fall.x 1): its uv v from its top (0) to its foot (1), u across it (0 to 1);
// its albedo map a noise (streaks along v) scrolled down in two layers (water_flow.xy, the
// second faster): where they are bright the light water (color), else the deep one
// (water_deep), sharp bands (the threshold); foam at its sides and at its foot, its sides
// dissolving
Vec4 water_fall;  // x: 1 a waterfall, y: the threshold of the bands, z: the foam of the sides (u), w: the foam of the foot (v)
Vec4 water_deep;  // rgb: the deep water (linear), w: how many times the noise repeats down it
// the wakes of the hovercraft (drawn by the game, from above: world x/z, R how fresh): rings of
// waves along them, moving out, foam where they are fresh
Sampler2D(wake_map);
Vec4 wake_area;   // x, z of its corner (world); 1 / its size along x, z
Vec4 wake_style;  // x: the strength of its waves, y: their rings (per unit of the map), z: their speed, w: its foam

float wake_height(Vec2 world_xz, float t)
{
	float w = texture2DLod(wake_map, (world_xz - wake_area.xy) * wake_area.zw, 0.0).r;
	return w * sin(w * wake_style.y - t * wake_style.z);
}

// the uv of the water flowing (its texture moves along water_flow.xy)
Vec2 water_flowing_uv(Vec2 uv)
{
	return uv + water_flow.xy * water_time.x;
}

// the water over a surface (read at its flowing uv); water_uv: the uv of the surface (not
// flowing: the sides, the foot of a waterfall)
void apply_water(inout SurfaceData data, VertexShaderOutput input, Vec2 water_uv)
{
	// the ripples: two layers of the normal map, moving across each other
	float t = water_time.x;
	Vec2  uv = input.m_uv * water_flow.z;
	Vec3  n1 = normal_from_texture(texture2D(normal_map, uv + Vec2(0.11, 0.07) * t));
	Vec3  n2 = normal_from_texture(texture2D(normal_map, uv * 1.73 + Vec2(-0.08, 0.12) * t));
	Vec3  ripple = normalize(Vec3((n1.xy + n2.xy) * water_flow.w, 1.0));
	data.m_normal = normalize(mul(ripple, material_tbn(input)));
	// Fresnel: clear from above, the sky reflected grazing
	Vec3  view = normalize(camera.m_position - input.m_world_position.xyz);
	float facing = saturate(dot(data.m_normal, view));
	float fresnel = 0.02 + 0.98 * pow(1.0 - facing, water_style.w);
	Vec3  reflected = reflect(-view, data.m_normal);
	Vec3  sky = lerp(water_sky.rgb, water_top.rgb, saturate(reflected.y));
	// the wakes: the slope of their waves bends the normal, foam where they are fresh
	{
		Vec2  xz = input.m_world_position.xz;
		float e = 0.25;
		float dx = wake_height(xz + Vec2(e, 0.0), t) - wake_height(xz - Vec2(e, 0.0), t);
		float dz = wake_height(xz + Vec2(0.0, e), t) - wake_height(xz - Vec2(0.0, e), t);
		data.m_normal = normalize(data.m_normal - Vec3(dx, 0.0, dz) * wake_style.x);
		float fresh = texture2DLod(wake_map, (xz - wake_area.xy) * wake_area.zw, 0.0).r;
		float foam = smoothstep(0.55, 1.0, fresh) * wake_style.w;
		data.m_albedo = lerp(data.m_albedo, Vec3(0.9, 0.95, 0.95), foam);
		data.m_alpha = lerp(data.m_alpha, 1.0, foam);
		facing = saturate(dot(data.m_normal, view));
		fresnel = 0.02 + 0.98 * pow(1.0 - facing, water_style.w);
		reflected = reflect(-view, data.m_normal);
		sky = lerp(water_sky.rgb, water_top.rgb, saturate(reflected.y));
	}
	data.m_alpha *= lerp(water_style.x, water_style.y, fresnel);
	data.m_emmisive += sky * fresnel * water_style.z;
	data.m_albedo *= 1.0 - fresnel;
	if (water_fall.x > 0.5)
	{
		// the waterfall: two layers of the noise falling (the second faster, shifted)
		Vec2  fall = Vec2(water_uv.x, water_uv.y * water_deep.w);
		float a = to_rgb_space(texture2D(albedo_map, fall + water_flow.xy * t)).r;
		float b = to_rgb_space(texture2D(albedo_map, fall * Vec2(1.37, 0.8) + water_flow.xy * t * 1.6 + Vec2(0.37, 0.11))).r;
		float noise = (a + b) * 0.5;
		float band = smoothstep(water_fall.y - 0.04, water_fall.y + 0.04, noise);
		float across = min(water_uv.x, 1.0 - water_uv.x);
		float side = 1.0 - smoothstep(0.0, water_fall.z, across);
		float foot = smoothstep(1.0 - water_fall.w, 1.0, water_uv.y);
		float foam = saturate(band + side * smoothstep(0.25, 0.55, noise) + foot * smoothstep(0.15, 0.45, noise));
		data.m_albedo = lerp(water_deep.rgb, color.rgb, foam);
		data.m_emmisive += data.m_albedo * 0.35;
		// opaque where it foams, its sides dissolving into the noise
		data.m_alpha = lerp(0.72, 1.0, foam) * color.a * smoothstep(0.0, water_fall.z * 0.6, across + (noise - 0.5) * water_fall.z * 0.8);
		data.m_roughness = lerp(0.05, 0.5, foam);
	}
}
