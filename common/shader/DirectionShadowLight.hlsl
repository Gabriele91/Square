#pragma once
#define PCF_SHADOW
#include <ShadowCamera>
Sampler2DArray(direction_shadow_map)

uint find_csm_layer(in float depth)
{
	for (uint i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
	{
		if (depth < abs(direction_shadow_camera.m_depths[i]))
			return i;
	}
	return DIRECTION_SHADOW_CSM_NUMBER_OF_FACES - 1;
}

float bias_depth_driven(in Vec3  view_dir, in Vec3  normal, in float max_bias, in uint id)
{
	// calculate bias (based on depth map resolution and slope)
	float bias = max(0.0005 * (1.0 - dot(normal, view_dir)), max_bias);
	//bias *= 1 / (direction_shadow_camera.m_depths[id] * 0.001f);
	return bias;
}

#ifdef PCF_SHADOW
float direction_light_shadow(in Vec3 proj_coords, uint id, const float bias)
{
	//depth of current pos
	float current_depth = proj_coords.z;
	//start shadow
	float shadow = 0.0;
	//size
	Vec2 tex_size = Vec2(1.0, 1.0) / textureSize2DArray(direction_shadow_map, 0).xy;
	//size kernel
	const int kernel_size = 5;
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
	float current_depth = proj_coords.z;
	// check whether current frag pos is in shadow
	float shadow = (current_depth - bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}
#endif

#ifdef DEBUG
static const Vec3 csm_colors[5] = {
	Vec3(1.0f,0.0f,0.0f),
	Vec3(0.0f,1.0f,0.0f),
	Vec3(0.0f,1.0f,0.0f),
	Vec3(1.0f,1.0f,0.0f),
	Vec3(0.0f,1.0f,1.0f)
};
#endif

Vec4 direction_light_compute_shadow(in Vec4 fposition, in Vec3 view_dir, in Vec3 normal, in float max_bias)
{
	// Get cascade id
	Vec4 view_fposition = mul(fposition, camera.m_view);
	uint cascade_id = find_csm_layer(abs(view_fposition.z));
	// compute pos
	Vec4 fposition_light_space = mul_direction_light_view_projection(fposition, cascade_id);
	// perform perspective divide (homogenize position)
	Vec3 proj_coords = fposition_light_space.xyz / fposition_light_space.w;
	//(-1,1)->(0,1)
	proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
	//clamp
	if (proj_coords.x <= 0.0f || proj_coords.x >= 1.0) return 1.0;
	if (proj_coords.y <= 0.0f || proj_coords.y >= 1.0) return 1.0;
	if (proj_coords.z <= 0.0f || proj_coords.z >= 1.0) return 1.0;
	// DirectX y is inv
	proj_coords = invY(proj_coords);
	// Compute bias
	float bias = bias_depth_driven(view_dir, normal, max_bias, cascade_id);
	// Shadow
#ifdef DEBUG
	float shadow = direction_light_shadow(proj_coords, cascade_id, bias);
	Vec3  shadow_color = csm_colors[cascade_id] * 0.9 + Vec3(shadow, shadow, shadow) * 0.1f;
#else
	float shadow = direction_light_shadow(proj_coords, cascade_id, bias);
	Vec3  shadow_color = Vec3(shadow, shadow, shadow);
#endif
	// return
	return Vec4(shadow_color, 1.0f);
}

void direction_light_apply_shadow(inout LightResult result, in Vec4 fposition, in Vec3  view_dir, in Vec3  normal)
{
	//const bias
	const float max_bias = 0.000001;
	//factor
	Vec4 shadow_factor = direction_light_compute_shadow(fposition, view_dir, normal, max_bias);
	//add shadow
	result.m_diffuse *= shadow_factor;
	result.m_specular *= shadow_factor;
}