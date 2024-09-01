#pragma once
#define PCF_SHADOW
#include <ShadowCamera>
Sampler2DArray(direction_shadow_map)

Vec4 direction_light_shadow_position(in Vec4 fposition, uint id)
{
	fposition = mul(fposition, direction_shadow_camera.m_view[id]);
	fposition = mul(fposition, direction_shadow_camera.m_projection[id]);
	return fposition;
}

// Find layer of csm
int find_csm_layer(float depth)
{
	for (int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES - 1; ++i)
	{
		if (depth < abs(direction_shadow_camera.m_depths[i])) 
			return i;
	}
	return DIRECTION_SHADOW_CSM_NUMBER_OF_FACES-1;
}

float cmp_bias(float bias, int cascade_id)
{
	return bias * (1.0 / direction_shadow_camera.m_depths[cascade_id]);
}

float direction_light_shadow(in Vec4 proj_coords, int cascade_id, float bias)
{
	// depth of shadow map
	float closest_depth = shadow2DArray(direction_shadow_map, Vec3(proj_coords.xy, cascade_id)).r;
	// depth of current pos
	float current_depth = proj_coords.z;
	// check whether current frag pos is in shadow
	float shadow = (current_depth + bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}

const Vec4 colord_id[] = {
	Vec4(1.0,0.0,0.0,1.0), // RED
	Vec4(0.0,1.0,0.0,1.0), // GREEN
	Vec4(0.0,0.0,1.0,1.0), // BLUE

	Vec4(1.0,1.0,0.0,1.0), //
	Vec4(1.0,0.0,1.0,1.0), //
	Vec4(0.0,1.0,1.0,1.0), //

	Vec4(1.0,1.0,1.0,1.0), //
	Vec4(0.0,0.0,0.0,1.0), //
};

Vec4 direction_light_compute_shadow(in Vec4 fposition, float bias)
{
	// compute position in view space
	Vec4 pointpos = mul(fposition, mul(camera.m_view, camera.m_projection));
	// get layer id
	int cascade_id = find_csm_layer(abs(pointpos.z));
	// get space
	Vec4 fposition_light_space = direction_light_shadow_position(fposition, cascade_id);
	// perform perspective divide (homogenize position)
	Vec3 proj_coords = fposition_light_space.xyz / fposition_light_space.w;
	//(-1,1)->(0,1)
	proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
	//clamp
	if (proj_coords.x <= 0.0f || proj_coords.x >= 1.0) return colord_id[7];
	if (proj_coords.y <= 0.0f || proj_coords.y >= 1.0) return colord_id[7];
	if (proj_coords.z <= 0.0f || proj_coords.z >= 1.0) return colord_id[7];
	// DirectX y is inv
	proj_coords = invY(proj_coords);
	// return
	return colord_id[cascade_id] * direction_light_shadow(Vec4(proj_coords, fposition_light_space.w), cascade_id, bias);
}

void direction_light_apply_shadow(inout LightResult result, in Vec4 fposition)
{
	//const bias
	const float bias = 0.01;
	//factor
	Vec4 shadow_factor = direction_light_compute_shadow(fposition, bias);
	//add shadow
	result.m_diffuse = Vec4(shadow_factor);
	result.m_specular = Vec4(shadow_factor);
}