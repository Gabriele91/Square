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
    //clear
    void Collection::clear()
    {
        m_cameras.clear();
        m_lights.clear();
        m_renderables.clear();
    }
    
    //compute distance
	static inline float compute_camera_depth(const Geometry::Frustum& f_camera, const Shared<Transform>& transform)
	{
		return f_camera.distance_from_near_plane(transform->position(true));
	}
	static inline float compute_camera_depth(const Geometry::Sphere& in_sphere, const Shared<Transform>& transform)
	{
		return distance(in_sphere.get_center(), transform->position(true));
	}
	static inline float compute_camera_depth(const Vec3& position, const Shared<Transform>& transform)
	{
		return distance(position, transform->position(true));
	}
    
    //Query Lights
    void CollectionQuery::lights(const Collection& collection, PoolQueues& queues,const Geometry::Frustum& view_frustum)
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
        for (auto weak_light : collection.m_lights)
        {
			//light
			auto light = weak_light.lock();
			//visiable?
			if (!light->visible()) continue;
			//direction light
			if (light->type() == LightType::DIRECTION)
			{
				rqueue_direction.push_front_to_back(weak_light, 0);
				continue;
			}
			//transform
			auto transform = light->transform().lock();
			//get distance //force to  compute transform
			float depth = compute_camera_depth(view_frustum, transform);
			//test
            if (Intersection::check(view_frustum, light->bounding_sphere()) != Intersection::OUTSIDE)
            {
				//add by type
                switch (light->type())
                {
                    case LightType::SPOT:      rqueue_spot.push_front_to_back(weak_light,depth);      break;
                    case LightType::POINT:     rqueue_point.push_front_to_back(weak_light,depth);     break;
                    default: break;
                };
                
            }
        }
        
    }
    
    void CollectionQuery::lights(const Collection& collection, PoolQueues& queues,const Geometry::Sphere& in_sphere)
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
        for (auto weak_light : collection.m_lights)
        {
            auto light = weak_light.lock();
			//visiable?
			if (!light->visible()) continue;
			//direction light
			if (light->type() == LightType::DIRECTION)
			{
				rqueue_direction.push_front_to_back(weak_light, 0);
				continue;
			}
			//gate distance
			auto transform = light->transform().lock();
			//get distance //force to compute transform
			float depth = compute_camera_depth(in_sphere, transform);
			//test
			if (Intersection::check(in_sphere, light->bounding_sphere()) != Intersection::OUTSIDE)
            {
                //add by type
                switch (light->type())
                {
                    case LightType::SPOT:      rqueue_spot.push_front_to_back(weak_light,depth);      break;
                    case LightType::POINT:     rqueue_point.push_front_to_back(weak_light,depth);     break;
                    default: break;
                };
                
            }
        }
    }
    
    void CollectionQuery::lights(const Collection& collection, PoolQueues& queues, const Camera&  in_camera)
    {
        lights(collection, queues, in_camera.frustum());
    }


    //Query renderables
    void CollectionQuery::renderables(const Collection& collection,PoolQueues& queues, const Vec3& position)
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
        for (Weak<Renderable> weak_renderable : collection.m_renderables)
        {
            auto renderable = weak_renderable.lock();
            if (renderable->can_draw() && !renderable->support_culling())
            {
                //gate distance
                auto transform = renderable->transform().lock();
                auto material = renderable->material().lock();
                ///queue
                EffectQueueType queue = material->queue();
                //distance
                switch (queue.m_type)
                {
                    case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(position, transform)); break;
                    case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_camera_depth(position, transform)); break;
                    default: queues[queue.m_type].push_back_to_front(weak_renderable, queue.m_order); break;
                }
            }
        }
    }
    
    void CollectionQuery::renderables(const Collection& collection,PoolQueues& queues, const Geometry::Frustum& view_frustum)
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
        for (Weak<Renderable> weak_renderable : collection.m_renderables)
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
    
    void CollectionQuery::renderables(const Collection& collection, PoolQueues& queues, const Geometry::Sphere& in_sphere)
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
		for (Weak<Renderable> weak_renderable : collection.m_renderables)
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
				case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(in_sphere, transform)); break;
				case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_camera_depth(in_sphere, transform)); break;
				default: queues[queue.m_type].push_back_to_front(weak_renderable, queue.m_order); break;
				}
			}
		}
	}
    
    void CollectionQuery::renderables(const Collection& collection, PoolQueues& queues, const Camera&  in_camera)
    {
        renderables(collection, queues, in_camera.frustum());
    }
    
	void CollectionQuery::opaque_renderables(const Collection& collection,PoolQueues& queues, const Vec3& position)
    {
        //clear
        queues[RQ_OPAQUE].clear();
        //alias
        auto& rqueue_opaque = queues.m_queues[RQ_OPAQUE];
        //using
        using namespace Geometry;
        //build queue opaque
        for (Weak<Renderable> weak_renderable : collection.m_renderables)
        {
            auto renderable = weak_renderable.lock();
            if (renderable->can_draw() /* && !renderable->support_culling() */)
            {
                //gate distance
                auto transform = renderable->transform().lock();
                auto material = renderable->material().lock();
                ///queue
                EffectQueueType queue = material->queue();
                //distance
                if(queue.m_type == RQ_OPAQUE)
					rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(position, transform));
            }
        }
    }
    
    void CollectionQuery::opaque_renderables(const Collection& collection,PoolQueues& queues, const Geometry::Frustum& view_frustum)
    {
		//clea
		queues[RQ_OPAQUE].clear();
		//alias
		auto& rqueue_opaque = queues.m_queues[RQ_OPAQUE];
		//using
		using namespace Geometry;
        //build queue opaque
        for (Weak<Renderable> weak_renderable : collection.m_renderables)
        {
            auto renderable = weak_renderable.lock();
			if (renderable->can_draw())
			{
				//gate distance
				auto transform = renderable->transform().lock();
				auto material = renderable->material().lock();
				///queue
				EffectQueueType queue = material->queue();
				//culling
				if (queue.m_type == RQ_OPAQUE)
				if (!renderable->support_culling() || Intersection::check(view_frustum, renderable->bounding_box()) != Intersection::OUTSIDE)
						rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(view_frustum, transform));
			}
        }
    }
    
    void CollectionQuery::opaque_renderables(const Collection& collection, PoolQueues& queues, const Geometry::Sphere& in_sphere)
	{
		//clear
		queues[RQ_OPAQUE].clear();
		//alias
		auto& rqueue_opaque = queues.m_queues[RQ_OPAQUE];
		//using
		using namespace Geometry;
		//build queue opaque
		for (Weak<Renderable> weak_renderable : collection.m_renderables)
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
				//culling
				if (queue.m_type == RQ_OPAQUE)
					if (!renderable->support_culling() || Intersection::check(renderable->bounding_box(), in_sphere) != Intersection::OUTSIDE)
						rqueue_opaque.push_front_to_back(weak_renderable, compute_camera_depth(in_sphere, transform));
			}
		}
	}

    void CollectionQuery::renderables(const Collection& collection, PoolQueues& queues, const Light& in_light)
    {
        switch (in_light.type())
        {
            case LightType::SPOT:
				opaque_renderables(collection, queues, in_light.frustum());
            break;
            case LightType::POINT:
				opaque_renderables(collection, queues, in_light.bounding_sphere());
            break;
            case LightType::DIRECTION:
                if(auto transform = in_light.transform().lock())
                {
					opaque_renderables(collection, queues, transform->position());
                }
            break;
        }
        
    }
}
}
