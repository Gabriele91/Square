#pragma once
//ambient light
Vec4 light;

LightResult compute_light
(
	in Vec3  fposition,
	in Vec3  vdir,
	in Vec3  normal,
	in float shininess
)
{
	// Combine results
	LightResult result;
	result.m_diffuse = light;
	result.m_specular = Vec4(1,1,1,1);
	//return
	return result;
}
