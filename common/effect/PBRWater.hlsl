////////////////
//  PBRWater: a material of glTF read at its flowing uv (MaterialPBR), the water over it (WaterPBR)
////////////////
#pragma once
#include <MaterialPBR>
#include <WaterPBR>

surface(VertexShaderOutput input)
{
	// the texture flows (the uv of the surface kept: the waterfall, its sides, its foot)
	Vec2 water_uv = input.m_uv;
	input.m_uv = water_flowing_uv(input.m_uv);
	float albedo_alpha;
	SurfaceData data = material_standard(input, albedo_alpha);
	apply_water(data, input, water_uv);
	//return
	surface_return(data);
}
