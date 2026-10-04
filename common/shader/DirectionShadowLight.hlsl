#pragma once
#define PCF_SHADOW 5
#define DEPTH 0
#define BIAS 1
#define SLOPE_BIAS 2
//normal offset: the point moved along its normal by texels of its cascade before the lookup
//(less depth bias: no acne, no shadow detached from its caster), more at grazing light
#define NORMAL_OFFSET_MIN 0.5
#define NORMAL_OFFSET_MAX 2.0
//the slope term of the depth bias at most (tan of the angle: infinite at grazing light)
#define SLOPE_BIAS_MAX_TAN 10.0
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
	for (uint i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
	{
		if (depth < abs(direction_shadow_camera.m_data[i][DEPTH]))
			return i;
	}
	return DIRECTION_SHADOW_CSM_NUMBER_OF_FACES - 1;
}

//light_dir: to the light
float bias_depth_driven(in Vec3 light_dir, in Vec3 normal, in uint id)
{
	float bias = direction_shadow_camera.m_data[id][BIAS];
	float slope_bias = direction_shadow_camera.m_data[id][SLOPE_BIAS];
	// slope scale biasing: tan(acos(NoL)), clamped
	float NoL = saturate(dot(normalize(normal), normalize(light_dir)));
	float slope = min(sqrt(1.0 - NoL * NoL) / max(NoL, 0.0001), SLOPE_BIAS_MAX_TAN);
	return bias + slope_bias * slope;
}

//world size of a texel of a cascade (its orthographic projection: 2 / width)
float csm_texel_world_size(uint id)
{
	float width = 2.0 / max(abs(direction_shadow_camera.m_projection[id][0][0]), 0.000001);
	return width / textureSize2DArray(direction_shadow_map, 0).x;
}

#if defined(PCF_SHADOW) && PCF_SHADOW >= 1
float direction_light_shadow(in Vec3 proj_coords, uint id, const float bias)
{
	//depth of current pos
#ifdef GLSL_BACKEND
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
#else
float direction_light_shadow(in Vec3 proj_coords, uint id, const float bias)
{
	// depth of shadow map
	float closest_depth = shadow2DArray(direction_shadow_map, Vec3(proj_coords.xy, id)).r;
	// depth of current pos
#ifdef GLSL_BACKEND
	float current_depth = proj_coords.z * 0.5 + 0.5;
#else
	float current_depth = proj_coords.z;
#endif
	// check whether current frag pos is in shadow
	float shadow = (current_depth - bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}
#endif

Vec4 rh_mul_direction_light_view_projection(in Vec4 position, uint id)
{
	// Applicazione della vista
	Vec4 position_new = mul(position, direction_shadow_camera.m_view[id]);

	// Modifica la matrice ortografica per RH (se questo non � gi� fatto altrove)
	Mat4 rh_projection = direction_shadow_camera.m_projection[id];

	// Applicazione della proiezione
	return mul(position_new, rh_projection);
}

//light_dir: to the light
Vec4 direction_light_compute_shadow(in Vec4 fposition, in Vec3 light_dir, in Vec3 normal)
{
	// Get cascade id
	Vec4 view_fposition = mul(fposition, camera.m_view);
	uint cascade_id = find_csm_layer(abs(view_fposition.z));
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
	// Shadow
	float shadow = direction_light_shadow(proj_coords, cascade_id, bias);
	// return
	return shadow;
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