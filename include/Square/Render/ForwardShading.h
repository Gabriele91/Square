//
//  ForwardShading.h
//  Square
//
//  Forward shading shared by the render passes: every renderable of the selected
//  queues is drawn with a technique of its effect, and each pass of the technique
//  is repeated for every light of the kind it supports (ambient, direction, point,
//  spot, with or without shadow).
//  Used by DrawerPassForward (everything) and by DrawerPassDeferred (translucent).
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Render/Drawer.h"
#include <vector>

namespace Square
{
namespace Render
{
	class ConstBuffer;

	//constant buffers the passes are bound to
	struct ForwardShadingBuffers
	{
		ConstBuffer* m_camera;
		ConstBuffer* m_transform;
		ConstBuffer* m_direction_light;
		ConstBuffer* m_point_light;
		ConstBuffer* m_spot_light;
		ConstBuffer* m_direction_shadow_light;
		ConstBuffer* m_point_shadow_light;
		ConstBuffer* m_spot_shadow_light;
	};

	//draw the renderables of queue_types (in queue order) with the technique technique_name
	//of their effects; renderables whose effect has no such technique are skipped
	SQUARE_API void draw_forward
	(
	   Render::Context&              render
	 , const char*                   technique_name
	 , const Camera&                 camera
	 , const Vec4&                   ambient_light
	 , const PoolQueues&             queues
	 , const std::vector<QueueType>& queue_types
	 , const ForwardShadingBuffers&  buffers
	);
}
}
