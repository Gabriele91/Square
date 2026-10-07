////////////////
//  PBRSnow: a material of glTF (MaterialPBR), the trails of the game pressed in it (TrailsPBR)
//  only where its albedo alpha is (1: the snow; ice, rock: 0): its alpha not a transparency
////////////////
#pragma once
#include <MaterialPBR>
#include <TrailsPBR>

surface(VertexShaderOutput input)
{
	float snow;
	SurfaceData data = material_standard(input, snow);
	data.m_alpha = color.a;
	apply_trails(data, input, snow);
	//return
	surface_return(data);
}
