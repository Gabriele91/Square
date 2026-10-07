////////////////
//  PBRTerrainTrails: the grounds of a terrain (TerrainPBR), the trails of the game pressed in the
//  soft ones (TrailsPBR: the sand, the snow; trail_layers: how soft each of the layers 1 to 4)
////////////////
#pragma once
#include <MaterialPBR>
#include <TerrainPBR>
#include <TrailsPBR>

Vec4 trail_layers; // how much the layers 1 to 4 are pressed ([0, 1]; the layer 0 never)

surface(VertexShaderOutput input)
{
	Vec4 layers;
	SurfaceData data = material_terrain(input, layers);
	apply_trails(data, input, saturate(dot(layers, trail_layers)));
	//return
	surface_return(data);
}
