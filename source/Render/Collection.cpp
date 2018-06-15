//
//  RenderCore.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Math/Linear.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/Intersection.h"
#include "Square/Render/Light.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Collection.h"
#include "Square/Render/Material.h"

namespace Square
{
namespace Render
{
    //compute distance
    static inline float compute_camera_depth(const Geometry::Frustum& f_camera, const Shared<Transform>& transform)
    {
        return f_camera.distance_from_near_plane(transform->position());
    }
    //by frustum
    void Collection::compute_lights_queues(PoolQueues& queues,const Geometry::Frustum& view_frustum)
    {
        //alias
        auto& rqueue_spot        = queues.m_queues[RQ_SPOT_LIGHT];
        auto& rqueue_point       = queues.m_queues[RQ_POINT_LIGHT];
        auto& rqueue_direction   = queues.m_queues[RQ_DIRECTION_LIGHT];
        //clear
        rqueue_spot.clear();
        rqueue_point.clear();
        rqueue_direction.clear();
        //using
		using namespace Geometry;
        //build queue lights
        for (auto weak_light : m_lights)
        {
			auto light = weak_light.lock();
            if (light->visible() && Intersection::check(view_frustum, light->bounding_sphere()) != Intersection::OUTSIDE)
            {
				//gate distance
				auto transform = light->transform().lock();
                float depth = compute_camera_depth(view_frustum, transform);
				//add by type
                switch (light->type())
                {
                    case LightType::SPOT:      rqueue_spot.push_front_to_back(weak_light,depth);      break;
                    case LightType::POINT:     rqueue_point.push_front_to_back(weak_light,depth);     break;
                    case LightType::DIRECTION: rqueue_direction.push_front_to_back(weak_light,depth); break;
                    default: break;
                };
                
            }
        }
        
    }

    void Collection::compute_no_lights_queues(PoolQueues& queues, const Geometry::Frustum& view_frustum)
    {
		//clear
		queues[RQ_BACKGROUND].clear();
		queues[RQ_OPAQUE].clear();
		queues[RQ_TRANSLUCENT].clear();
		queues[RQ_UI].clear();
		//alias
		auto& rqueue_opaque = queues.m_queues[RQ_OPAQUE];
		auto& rqueue_translucent = queues.m_queues[RQ_TRANSLUCENT];
		//using
		using namespace Geometry;
        //build queue opaque
        for (Weak<Renderable> weak_renderable : m_renderables)
        {
            auto renderable = weak_renderable.lock();
			if (renderable->can_draw())
			if (!renderable->support_culling() || Intersection::check(view_frustum, renderable->bounding_box()) != Intersection::OUTSIDE)
			{
				//gate distance
				auto transform = renderable->transform().lock();
				auto material = renderable->material().lock();
				///queue
				EffectQueueType queue = material->queue();
				//distance
				switch (queue.m_type)
				{
				case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(view_frustum, transform)); break;
				case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_camera_depth(view_frustum, transform)); break;
				default: queues[queue.m_type].push_back_to_front(weak_renderable, queue.m_order); break;
				}
			}
        }
    }
    
    //by sphere
	void Collection::compute_no_lights_queues(PoolQueues& queues, const Geometry::Sphere& in_sphere)
	{
		//clear
		queues[RQ_BACKGROUND].clear();
		queues[RQ_OPAQUE].clear();
		queues[RQ_TRANSLUCENT].clear();
		queues[RQ_UI].clear();
		//alias
		auto& rqueue_opaque = queues.m_queues[RQ_OPAQUE];
		auto& rqueue_translucent = queues.m_queues[RQ_TRANSLUCENT];
		//using
		using namespace Geometry;
		//build queue opaque
		for (Weak<Renderable> weak_renderable : m_renderables)
		{
			auto renderable = weak_renderable.lock();
			if (renderable->can_draw())
			if (!renderable->support_culling() || Intersection::check(renderable->bounding_box(), in_sphere) != Intersection::OUTSIDE)
			{
				//gate distance
				auto transform = renderable->transform().lock();
				auto material = renderable->material().lock();
				///queue
				EffectQueueType queue = material->queue();
				//distance
				switch (queue.m_type)
				{
				case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, distance(transform->position(), in_sphere.get_position())); break;
				case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, distance(transform->position(), in_sphere.get_position())); break;
				default: queues[queue.m_type].push_back_to_front(weak_renderable, queue.m_order); break;
				}
			}
		}
	}
}
}
