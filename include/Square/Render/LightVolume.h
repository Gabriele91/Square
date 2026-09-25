//
//  LightVolume.h
//  Square
//
//  Meshes and model matrices of the volumes that bound point and spot lights.
//  Shared by the deferred light pass (which rasterizes them) and the debug pass
//  (which draws them in wireframe), so what is debugged is what is lit.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/Light.h"

namespace Square
{

class Context;

namespace Render
{

class Mesh;

namespace LightVolume
{
	//unit sphere centered at the origin
	SQUARE_API Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings = 12, unsigned int sectors = 24);
	//cone: apex at origin, base circle of radius 1 at z=+1
	SQUARE_API Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors = 24);

	//sphere at the light position, scaled to the radius (+10% margin)
	SQUARE_API Mat4 point_light_model(const UniformPointLight& upoint_light);
	//cone at the light position, +z along the light direction, sized by radius and outer cut off
	SQUARE_API Mat4 spot_light_model(const UniformSpotLight& uspot_light);
}
}
}
