#pragma once
#include <Camera>
#include <GammaCorrection>
#include <NDF>
#include <UtilsPBR>
#include <LightPBR>

// Shader output
struct SurfaceOutput
{
	Vec4 m_color : SV_TARGET0;
};

// Function shader
SurfaceOutput compute_surface_output(in SurfaceData data)
{
	// View direction
	Vec3 view_direction = normalize(camera.m_position - data.m_position.xyz);

	// Then calculate lighting as usual
	LightResult light_results = compute_light(view_direction, data);

	//output
	SurfaceOutput output;
	output.m_color = to_srgb_space(Vec4(light_results.m_radiance, data.m_alpha));

	return output;
}