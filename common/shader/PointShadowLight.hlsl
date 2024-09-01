#pragma once
#define PCF_SHADOW
#include <ShadowCamera>
SamplerCube(point_shadow_map)

#ifdef PCF_SHADOW
float point_light_shadow(in Vec3 fposition_to_light, const float bias)
{
	static Vec3 sample_offset_directions[20] = 
	{
		Vec3(1, 1, 1),  Vec3(1, -1, 1),  Vec3(-1, -1, 1),  Vec3(-1, 1, 1),
		Vec3(1, 1, -1), Vec3(1, -1, -1), Vec3(-1, -1, -1), Vec3(-1, 1, -1),
		Vec3(1, 1, 0),  Vec3(1, -1, 0),  Vec3(-1, -1, 0),  Vec3(-1, 1, 0),
		Vec3(1, 0, 1),  Vec3(-1, 0, 1),  Vec3(1, 0, -1),   Vec3(-1, 0, -1),
		Vec3(0, 1, 1),  Vec3(0, -1, 1),  Vec3(0, -1, -1),  Vec3(0, 1, -1)
	};
	float shadow = 0.0;
	int   samples = 20;
	float current_depth = length(fposition_to_light);
	float disk_radius = 0.1;

	[unroll]
	for (int i = 0; i < samples; ++i)
	{
		Vec3 cube_dir = normalize(fposition_to_light + sample_offset_directions[i] * disk_radius);
		float closest_depth = shadowCube(point_shadow_map, cube_dir).r;
		//0-1 to 0 to dist
		closest_depth *= light.m_radius;
		//test
		if (current_depth + bias <= closest_depth) shadow += 1.0;
	}

	shadow /= float(samples);
	//return
	return shadow;
}
#else
float point_light_shadow(in Vec3 fposition_to_light, const float bias)
{
	Vec3 cube_dir = normalize(fposition_to_light);
	float closest_depth = shadowCube(point_shadow_map, cube_dir).r;
	// It is currently in linear range between [0,1]. Re-transform back to original value
	closest_depth *= light.m_radius;
	// Now get current linear depth as the length between the fragment and light position
	float current_depth = length(fposition_to_light);
	// check whether current frag pos is in shadow
	float shadow = (current_depth + bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}
#endif

float point_light_compute_shadow(in Vec4 fposition, const float bias)
{
	// Get vector between fragment position and light position
	Vec3 fposition_to_light = fposition.xyz - light.m_position.xyz;
	// return
	return point_light_shadow(fposition_to_light, bias);
}

void point_light_apply_shadow(inout LightResult result, in Vec4 fposition)
{
	//const bias
	const float bias = 0.001;
	//factor
	float shadow_factor = point_light_compute_shadow(fposition, bias);
	//add shadow
	result.m_diffuse *= shadow_factor;
	result.m_specular *= shadow_factor;
}