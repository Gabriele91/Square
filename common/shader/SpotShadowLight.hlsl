#pragma once
#define PCF_SHADOW
#include <ShadowCamera>
Sampler2D(spot_shadow_map)

#ifdef PCF_SHADOW
float spot_light_shadow(in Vec3 proj_coords, const float bias)
{
	//depth of current pos
	float current_depth = proj_coords.z;
	//start shadow
	float shadow = 0.0;
	//size
	Vec2 tex_size = 1.0 / textureSize2D(spot_shadow_map, 0);
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
			float pcf_depth = shadow2D(spot_shadow_map, coord).r;
			shadow += (current_depth - bias) <= pcf_depth ? 1.0 : 0.0;
		}
	}
	shadow /= (kernel_size*kernel_size);
	//return
	return shadow;
}
#else
float spot_light_shadow(in Vec3 proj_coords, const float bias)
{
	// depth of shadow map
	float closest_depth = shadow2D(spot_shadow_map, proj_coords.xy).r;
	// depth of current pos
	float current_depth = proj_coords.z;
	// check whether current frag pos is in shadow
	float shadow = (current_depth + bias) <= closest_depth ? 1.0 : 0.0;
	// shadow
	return shadow;
}
#endif


float spot_light_compute_shadow(in Vec4 fposition, const float bias)
{
	// compute pos 
	Vec4 fposition_light_space = mul_spot_light_view_projection(fposition);
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
	// return
	return spot_light_shadow(proj_coords, bias);
}

void spot_light_apply_shadow(inout LightResult result, in Vec4 fposition)
{
	//const bias
	const float bias = 0.0000001;
	//factor
	float shadow_factor = spot_light_compute_shadow(fposition, bias);
	//add shadow
	result.m_diffuse *= shadow_factor;
	result.m_specular *= shadow_factor;
}
