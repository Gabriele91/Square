//
//  LevelOfDetail.h
//  Square
//
//  A group of the levels of detail of an object (Scene::LodGroup): the Drawer asks it its level
//  for each camera, before the queues of the camera (its shadows and its render take that level),
//  with the settings of its world (RenderInstance::levels_of_detail).
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
	//how the levels of detail of a world are chosen
	struct LevelOfDetailSettings
	{
		//the seconds of a cross-fade between two levels (0: a level at once)
		float m_fade_duration{ 0.5f };
		//a level for all of them, wherever the camera is (a photo: 0, the most detailed; a group
		//with fewer levels: its last one); -1: by the camera
		int   m_force_level{ -1 };
	};

	class SQUARE_API LevelOfDetail : public BaseObject
	{
	public:

		SQUARE_OBJECT(LevelOfDetail)

		virtual ~LevelOfDetail() {}

		//its level for a camera: the renderables of that level drawn, the others not
		virtual void select(const Camera& camera, const LevelOfDetailSettings& settings) = 0;
	};
}
}
