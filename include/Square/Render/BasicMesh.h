//
//  BasicMesh.h
//  Square
//
//  The basic meshes (Position3D, triangles): one function per shape, for the passes of the
//  engine and for the games, so that none of them declares them again.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{

class Context;

namespace Render
{

class Mesh;

namespace BasicMesh
{
	//quad in NDC: x, y in [-1, 1], z 0, wound clockwise (engine front face): the full-screen passes
	SQUARE_API Shared<Mesh> build_quad(Square::Context& context);
	//box: x, y, z in [-size, size], two triangles per face
	SQUARE_API Shared<Mesh> build_box(Square::Context& context, float size = 1.0f);
	//box of the clip space volume: x, y in [-1, 1], z in [0, 1] (GLM_FORCE_DEPTH_ZERO_TO_ONE);
	//through inverse(projection * view) it is exactly the frustum
	SQUARE_API Shared<Mesh> build_frustum_box(Square::Context& context);
	//sphere of radius 1 at the origin (rings from pole to pole, sectors around y); circumscribed:
	//the vertices are pushed out so that the flat faces enclose the sphere (a volume that must
	//never cut it, e.g. the light of a point light), else the vertices are on it
	SQUARE_API Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings = 12, unsigned int sectors = 24, bool circumscribed = false);
	//cone: apex at the origin, base circle of radius 1 at z = +1 (with its cap); circumscribed: the
	//base polygon encloses the circle, else its vertices are on it
	SQUARE_API Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors = 24, bool circumscribed = false);
}
}
}
