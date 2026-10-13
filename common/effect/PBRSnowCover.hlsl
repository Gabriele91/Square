////////////////
//  PBRSnowCover: a material of glTF (MaterialPBR), snow laid on what faces up (SnowCoverPBR)
////////////////
#pragma once
#include <MaterialPBR>
#include <SnowCoverPBR>

surface(VertexShaderOutput input)
{
	float albedo_alpha;
	SurfaceData data = material_standard(input, albedo_alpha);
	apply_snow_cover(data, input);
	//return
	surface_return(data);
}
