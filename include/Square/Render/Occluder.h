//
//  Occluder.h
//  Square
//
//  Something that hides what is behind it (the software occlusion of a camera:
//  Pipeline/SoftwareOcclusion.h): its triangles in world space, few (a simple shape inside the
//  thing it stands for: what it hides must really be hidden), and their box.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Geometry/AABoundingBox.h"
#include <vector>

namespace Square
{
namespace Render
{
	class SQUARE_API Occluder
	{
	public:

		virtual ~Occluder() = default;

		//its triangles in world space (3 points each)
		virtual const std::vector<Vec3>& occluder_triangles() = 0;
		//the box of its triangles in world space
		virtual const Geometry::AABoundingBox& occluder_box() = 0;
		//it hides now (false: not drawn into the occlusion)
		virtual bool can_occlude() const = 0;
	};
}
}
