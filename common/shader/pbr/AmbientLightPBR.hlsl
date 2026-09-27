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
	// Emission, once: the ambient pass is the only one for every pixel (forward and deferred)
	result.m_radiance += data.m_emmisive;
	//return
	return result;
}
