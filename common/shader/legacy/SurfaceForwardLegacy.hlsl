#pragma once
#include <LightLegacy>
#include <Camera>
#include <GammaCorrection>

//shader output
struct SurfaceOutput
{
	Vec4 m_color : SV_TARGET0;
};
//function shader
SurfaceOutput compute_surface_output(in SurfaceData data)
{
	//view dir
	Vec3 view_dir = normalize(camera.m_position - data.m_position.xyz);

	// Then calculate lighting as usual
	LightResult light_results = compute_light(
		  data.m_position
		, view_dir
		, data.m_normal
		, data.m_occlusion
		, data.m_shininess
	);

	//output
	SurfaceOutput output;
	// Noramly the input is in linear RGB, while the target buffer is in sRGB
	output.m_color = Vec4(
		  data.m_albedo * (light_results.m_diffuse + light_results.m_specular * data.m_specular)
		, data.m_alpha
	);
#if !defined(ENABLE_TARGET_SRGB)
	output.m_color = to_srgb_space(output.m_color);
#endif
	return output;
}