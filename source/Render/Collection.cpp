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

	//point a renderable is sorted by: the center of its bounding box. The origin of the actor
	//can be far from the geometry (e.g. every object exported with an identity transform has
	//it at the scene origin), which breaks the back to front order of translucent objects.
	static inline Vec3 sort_point(const Shared<Renderable>& renderable, const Shared<Transform>& transform)
	{
		if (renderable->support_culling()) return renderable->bounding_box().get_position();
		return transform ? transform->position(true) : Vec3(0.0f);
	}
	static inline float compute_renderable_depth(const Geometry::Frustum& f_camera, const Shared<Renderable>& renderable, const Shared<Transform>& transform)
	{
		return f_camera.distance_from_near_plane(sort_point(renderable, transform));
	}
	static inline float compute_renderable_depth(const Geometry::Sphere& in_sphere, const Shared<Renderable>& renderable, const Shared<Transform>& transform)
	{
		return distance(in_sphere.get_center(), sort_point(renderable, transform));
	}
	static inline float compute_renderable_depth(const Vec3& position, const Shared<Renderable>& renderable, const Shared<Transform>& transform)
	{
		return distance(position, sort_point(renderable, transform));
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
                    case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(position, renderable, transform)); break;
                    case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_renderable_depth(position, renderable, transform)); break;
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
				case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(view_frustum, renderable, transform)); break;
				case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_renderable_depth(view_frustum, renderable, transform)); break;
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
				case RQ_OPAQUE:      rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(in_sphere, renderable, transform)); break;
				case RQ_TRANSLUCENT: rqueue_translucent.push_back_to_front(weak_renderable, compute_renderable_depth(in_sphere, renderable, transform)); break;
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
					rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(position, renderable, transform));
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
						rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(view_frustum, renderable, transform));
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
						rqueue_opaque.push_front_to_back(weak_renderable, compute_renderable_depth(in_sphere, renderable, transform));
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
            default: break;
        }
        
    }
}
}
