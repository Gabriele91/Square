#pragma once
// Ambient light color
Vec4 light;

LightResult compute_light
(
 	in Vec3 view_direction,
	in SurfaceData data
)
{
	LightResult result;
	// Combine results, IBL * texture color * AO
	result.m_radiance = light.rgb * data.m_albedo * data.m_occlusion;
	//return
	return result;
}
