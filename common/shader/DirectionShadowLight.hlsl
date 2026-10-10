#pragma once
#define PCF_SHADOW 5
#define DEPTH 0
#define NORMAL_OFFSET_MIN 0.5   // texels of the cascade the point moves along its normal, light from above
#define NORMAL_OFFSET_MAX 2.0   // ... at grazing light
#define BIAS_TEXELS 1.0         // depth bias: texels of the cascade (the same in each cascade)
#define SLOPE_BIAS_TEXELS 1.5   // ... more by the slope (tan of the light angle)
#define SLOPE_BIAS_MAX_TAN 3.0  // slope term of the depth bias at most (tan of the light angle)
#define PCSS_LIGHT_SIZE 0.02    // PCSS: size of the light (tan of its angle), more is softer
#define PCSS_BLOCKER_TEXELS 8.0 // PCSS: texels of the search of the casters
#define PCSS_MAX_TEXELS 8.0    // PCSS: penumbra at most (texels)
#define PCSS_SAMPLES 16         // PCSS: samples of the search and of the filter
#define SHADOW_FADE_START 0.85  // the shadow fades from this fraction of the last cascade to its end
#define SHADOW_FADE_BORDER 0.03 // ... and toward the border of the map of a cascade (uv)
#define SHADOW_FADE_ON 1        // 1: the fades above on; 0: off (test: the shadow as it is)
#include <ShadowCamera>
Sampler2DArray(direction_shadow_map)
// Material option: 1 = lit by this light without its shadow (e.g. glows, light beams).
// A material that does not set it gets 0: shadows on.
#ifndef IGNORE_SHADOWS_UNIFORM
#define IGNORE_SHADOWS_UNIFORM
float ignore_shadows;
#endif

uint find_csm_layer(in float depth)
{
	// the cascades of the light
	uint cascades = uint(clamp(direction_shadow_camera.m_options.y, 1, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
	for (uint i = 0; i < cascades; ++i)
	{
		if (depth < abs(direction_shadow_camera.m_data[i][DEPTH]))
			return i;
	}
	return cascades - 1;
}

//world size of a texel of a cascade (its orthographic projection: 2 / width)
float csm_texel_world_size(uint id)
{
	float width = 2.0 / max(abs(direction_shadow_camera.m_projection[id][0][0]), 0.000001);
	return width / textureSize2DArray(direction_shadow_map, 0).x;
}

// normalized depth of the shadow map per world unit (orthographic: |m22|, z in -1..1 on GL)
float csm_depth_per_world(uint id)
{
	float scale = abs(direction_shadow_camera.m_projection[id][2][2]);
#ifdef SQ_BACKEND_GLSL
	scale *= 0.5;
#endif
	return max(scale, 0.000001);
}

//light_dir: to the light. The depth bias in texels of the cascade (world: its texel size), so
//the same in each cascade: no shadow lighter in the nearer ones
float bias_depth_driven(in Vec3 light_dir, in Vec3 normal, in uint id)
{
	// slope scale biasing: tan(acos(NoL)), clamped
	float NoL = saturate(dot(normalize(normal), normalize(light_dir)));
	float slope = min(sqrt(1.0 - NoL * NoL) / max(NoL, 0.0001), SLOPE_BIAS_MAX_TAN);
	float texels = BIAS_TEXELS + SLOPE_BIAS_TEXELS * slope;
	return texels * csm_texel_world_size(id) * csm_depth_per_world(id);
}

// the filters (direction_shadow_camera.m_options.x)
#define SHADOW_FILTER_NONE 0
#define SHADOW_FILTER_PCF 1
#define SHADOW_FILTER_PCSS 2

// PCF: a fixed kernel of PCF_SHADOW x PCF_SHADOW samples
float direction_light_shadow_pcf(in Vec3 proj_coords, uint id, const float bias)
{
	//depth of current pos
#ifdef SQ_BACKEND_GLSL
	float current_depth = proj_coords.z * 0.5 + 0.5;
#else
	float current_depth = proj_coords.z;
#endif
	//start shadow
	float shadow = 0.0;
	//size
	Vec2 tex_size = Vec2(1.0, 1.0) / textureSize2DArray(direction_shadow_map, 0);
	//size kernel
	const int kernel_size = PCF_SHADOW;
	const int kenrel_hsize = kernel_size / 2;
	//pcf
	[unroll]
	for (int x = -kenrel_hsize; x <= kenrel_hsize; ++x)
	{
		[unroll]
		for (int y = -kenrel_hsize; y <= kenrel_hsize; ++y)
		{
			Vec2 coord = proj_coords.xy + Vec2(x, y) * tex_size;
			float pcf_depth = shadow2DArray(direction_shadow_map, Vec3(coord, id)).r;
			shadow += (current_depth - bias) <= pcf_depth ? 1.0 : 0.0;
		}
	}
	shadow /= (kernel_size * kernel_size);
	//return
	return shadow;
}

// no filter: one sample (hard, aliased)
float direction_light_shadow_none(in Vec3 proj_coords, uint id, const float bias)
{
	// depth of shadow map
	float closest_depth = shadow2DArray(direction_shadow_map, Vec3(proj_coords.xy, id)).r;
	// depth of current pos
#ifdef SQ_BACKEND_GLSL
	float current_depth = proj_coords.z * 0.5 + 0.5;
#else
	float current_depth = proj_coords.z;
#endif
	// check whether current frag pos is in shadow
	float shadow = (current_depth - bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}

// Poisson disk, radius 1
static const Vec2 pcss_poisson[PCSS_SAMPLES] =
{
	Vec2(-0.94201624, -0.39906216), Vec2( 0.94558609, -0.76890725),
	Vec2(-0.09418410, -0.92938870), Vec2( 0.34495938,  0.29387760),
	Vec2(-0.91588581,  0.45771432), Vec2(-0.81544232, -0.87912464),
	Vec2(-0.38277543,  0.27676845), Vec2( 0.97484398,  0.75648379),
	Vec2( 0.44323325, -0.97511554), Vec2( 0.53742981, -0.47373420),
	Vec2(-0.26496911, -0.41893023), Vec2( 0.79197514,  0.19090188),
	Vec2(-0.24188840,  0.99706507), Vec2(-0.81409955,  0.91437590),
	Vec2( 0.19984126,  0.78641367), Vec2( 0.14383161, -0.14100790)
};

Vec2 pcss_rotate(in Vec2 v, in float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return Vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// PCSS: the casters around the point give the penumbra (wider far from them), then a PCF of it
float direction_light_shadow_pcss(in Vec3 proj_coords, uint id, const float bias)
{
#ifdef SQ_BACKEND_GLSL
	float current_depth = proj_coords.z * 0.5 + 0.5;
#else
	float current_depth = proj_coords.z;
#endif
	Vec2 texel = Vec2(1.0, 1.0) / textureSize2DArray(direction_shadow_map, 0);
	// the disk rotated per texel (noise, not bands)
	float angle = 6.2831853 * frac(52.9829189 * frac(dot(proj_coords.xy / texel, Vec2(0.06711056, 0.00583715))));
	// 1) the casters: their average depth
	float blocker_depth = 0.0;
	float blockers = 0.0;
	[unroll]
	for (int i = 0; i < PCSS_SAMPLES; ++i)
	{
		Vec2  coord = proj_coords.xy + pcss_rotate(pcss_poisson[i], angle) * texel * PCSS_BLOCKER_TEXELS;
		float depth = shadow2DArray(direction_shadow_map, Vec3(coord, id)).r;
		if (depth < current_depth - bias)
		{
			blocker_depth += depth;
			blockers += 1.0;
		}
	}
	if (blockers < 0.5) return 1.0;
	blocker_depth /= blockers;
	// 2) the penumbra: the distance caster - receiver (world) by the size of the light, in texels
	float world_distance = (current_depth - blocker_depth) / csm_depth_per_world(id);
	float radius = clamp(world_distance * PCSS_LIGHT_SIZE / csm_texel_world_size(id), 1.0, PCSS_MAX_TEXELS);
	// 3) PCF of the penumbra
	float shadow = 0.0;
	[unroll]
	for (int j = 0; j < PCSS_SAMPLES; ++j)
	{
		Vec2  coord = proj_coords.xy + pcss_rotate(pcss_poisson[j], angle) * texel * radius;
		float depth = shadow2DArray(direction_shadow_map, Vec3(coord, id)).r;
		shadow += (current_depth - bias) <= depth ? 1.0 : 0.0;
	}
	return shadow / float(PCSS_SAMPLES);
}

Vec4 rh_mul_direction_light_view_projection(in Vec4 position, uint id)
{
	// The view of the light
	Vec4 position_new = mul(position, direction_shadow_camera.m_view[id]);

	// The orthographic projection, right handed (if it is not done elsewhere)
	Mat4 rh_projection = direction_shadow_camera.m_projection[id];

	// The projection
	return mul(position_new, rh_projection);
}

//light_dir: to the light
Vec4 direction_light_compute_shadow(in Vec4 fposition, in Vec3 light_dir, in Vec3 normal)
{
	// Get cascade id
	Vec4 view_fposition = mul(fposition, camera.m_view);
	float view_depth = abs(view_fposition.z);
	uint cascade_id = find_csm_layer(view_depth);
	// Beyond the last cascade no shadow (not its border stretched), faded toward it
	uint  cascades = uint(clamp(direction_shadow_camera.m_options.y, 1, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
	float shadow_distance = abs(direction_shadow_camera.m_data[cascades - 1][DEPTH]);
	if (view_depth >= shadow_distance) return 1.0;
	float fade = saturate((view_depth - shadow_distance * SHADOW_FADE_START) / (shadow_distance * (1.0 - SHADOW_FADE_START)));
	// Normal offset: along the normal, by texels of the cascade (more at grazing light)
	Vec3  n = normalize(normal);
	float NoL = saturate(dot(n, normalize(light_dir)));
	float offset = csm_texel_world_size(cascade_id) * lerp(NORMAL_OFFSET_MIN, NORMAL_OFFSET_MAX, 1.0 - NoL);
	Vec4  offset_position = Vec4(fposition.xyz / fposition.w + n * offset, 1.0);
	// compute pos
	Vec4 fposition_light_space = mul_direction_light_view_projection(offset_position, cascade_id);
	// perform perspective divide (homogenize position)
	Vec3 proj_coords = fposition_light_space.xyz / fposition_light_space.w;
	//(-1,1)->(0,1)
	proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
	// Out of the map of the last cascade no shadow, faded toward its border (the inner ones: the
	// next cascade covers their border)
	if (cascade_id == cascades - 1)
	{
		float border = min(min(proj_coords.x, 1.0 - proj_coords.x), min(proj_coords.y, 1.0 - proj_coords.y));
		if (border <= 0.0) return 1.0;
		fade = max(fade, 1.0 - saturate(border / SHADOW_FADE_BORDER));
	}
	//clamp
#if 0 // defined in the Render\ShadowBuffer.cpp TBO description
	if (proj_coords.x <= 0.0f || proj_coords.x >= 1.0) return 1.0;
	if (proj_coords.y <= 0.0f || proj_coords.y >= 1.0) return 1.0;
	if (proj_coords.z <= 0.0f || proj_coords.z >= 1.0) return 1.0;
#endif
	// DirectX y is inv
	proj_coords = invY(proj_coords);
	// Compute bias
	float bias = bias_depth_driven(light_dir, normal, cascade_id);
	// Shadow: by the filter of the light
	float shadow = 1.0;
	switch (direction_shadow_camera.m_options.x)
	{
	case SHADOW_FILTER_NONE: shadow = direction_light_shadow_none(proj_coords, cascade_id, bias); break;
	case SHADOW_FILTER_PCSS: shadow = direction_light_shadow_pcss(proj_coords, cascade_id, bias); break;
	default:                 shadow = direction_light_shadow_pcf(proj_coords, cascade_id, bias); break;
	}
	// return
#if SHADOW_FADE_ON
	return lerp(shadow, 1.0, fade);
#else
	return shadow;
#endif
}

//the light tinted by the cascade of a point (debug: m_options.z, the cascades in colors; out of
//the shadow white), else white
Vec3 direction_light_shadow_tint(in Vec4 fposition)
{
	Vec3 tint = Vec3(1.0, 1.0, 1.0);
	if (direction_shadow_camera.m_options.z != 0)
	{
		const float view_depth = abs(mul(fposition, camera.m_view).z);
		const uint  cascades = uint(clamp(direction_shadow_camera.m_options.y, 1, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
		if (view_depth < abs(direction_shadow_camera.m_data[cascades - 1][DEPTH]))
		{
			switch (find_csm_layer(view_depth))
			{
			case 0:  tint = Vec3(1.0, 0.25, 0.25); break;
			case 1:  tint = Vec3(0.25, 1.0, 0.25); break;
			case 2:  tint = Vec3(0.3, 0.45, 1.0); break;
			case 3:  tint = Vec3(1.0, 1.0, 0.25); break;
			case 4:  tint = Vec3(1.0, 0.3, 1.0); break;
			case 5:  tint = Vec3(0.25, 1.0, 1.0); break;
			case 6:  tint = Vec3(1.0, 0.6, 0.2); break;
			default: tint = Vec3(0.6, 0.35, 1.0); break;
			}
		}
	}
	return tint;
}

//light_dir: to the light
float direction_light_apply_shadow(in Vec4 fposition, in Vec3 light_dir, in Vec3 normal)
{
	if (ignore_shadows > 0.5) return 1.0;
	//factor
	float shadow_factor = direction_light_compute_shadow(fposition, light_dir, normal);
	//add shadow
	return shadow_factor;
}