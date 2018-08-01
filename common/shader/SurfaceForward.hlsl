//
//  SurfaceForward.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Light>
#include <Camera>
//shader output
struct SurfaceOutput
{
	Vec4 m_color : SV_TARGET0;
};
//function shader
SurfaceOutput compute_surface_output(in SurfaceData data)
{
	//todo: material
	float shininess = 16.0f;

	//view dir
	Vec3 view_dir = normalize(camera.m_position - data.m_position);

	// Then calculate lighting as usual
	LightResult light_results = compute_light(data.m_position, view_dir, data.m_normal, shininess);

	//output
	SurfaceOutput output;
	output.m_color =  Vec4(
		data.m_albedo * (light_results.m_diffuse + light_results.m_specular * data.m_specular), 
		data.m_alpha
	);
	return output;
}