////////////////
//  PBRTerrain: five grounds blended by a map of their weights (TerrainPBR)
////////////////
#pragma once
#include <MaterialPBR>
#include <TerrainPBR>

surface(VertexShaderOutput input)
{
	Vec4 layers;
	SurfaceData data = material_terrain(input, layers);
	//return
	surface_return(data);
}
