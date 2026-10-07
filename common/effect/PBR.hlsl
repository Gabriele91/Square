////////////////
//  PBR (PBR, PBRTranslucent): a material of glTF (MaterialPBR)
////////////////
#pragma once
#include <MaterialPBR>

surface(VertexShaderOutput input)
{
	float albedo_alpha;
	SurfaceData data = material_standard(input, albedo_alpha);
	//return
	surface_return(data);
}
