//
//  LevelOfDetail.h
//  Square
//
//  A group of the levels of detail of an object (Scene::LodGroup): the Drawer asks it its level
//  for each camera, before the queues of the camera (its shadows and its render take that level).
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
namespace Render
{
	class Camera;
}
}

namespace Square
{
namespace Render
{
	class SQUARE_API LevelOfDetail : public BaseObject
	{
	public:

		SQUARE_OBJECT(LevelOfDetail)

		virtual ~LevelOfDetail() {}

		//its level for a camera: the renderables of that level drawn, the others not
		virtual void select(const Camera& camera) = 0;
	};
}
}
