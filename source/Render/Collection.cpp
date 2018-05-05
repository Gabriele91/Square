//
//  RenderCore.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Render/Collection.h"

namespace Square
{
namespace Render
{
#if 0
    //pool
    void Collection::remove(Weak<Object> e)
    {
        //search light and remove
        for (auto
             it_light  = m_lights.begin();
             it_light != m_lights.end();
             ++it_light)
        {
            if (it_light->lock() == e.lock())
            {
                m_lights.erase(it_light);
                break;
            }
        }
        //search renderable object and remove
        for (auto
             it_renderable  = m_renderable.begin();
             it_renderable != m_renderable.end();
             ++it_renderable)
        {
            if (it_renderable->lock() == e.lock())
            {
                m_renderable.erase(it_renderable);
                break;
            }
        }
    }
    
    void Collection::push(Weak<Object> e)
    {
        if (e->has_component<light>())      m_lights.push_back( e );
        if (e->has_component<renderable>()) m_renderable.push_back( e );
    }
    
    void Collection::reserve(size_t size)
    {
        m_lights.reserve(size);
        m_renderable.reserve(size);
    }
    
    //clear all
    void Collection::clear()
    {
        m_lights.clear();
        m_renderable.clear();
    }
    
    //get iterator
    QueueElement* CollectionObjects::get_first(render_scene_queue_type queue) const
    {
        return m_queues[queue].get_first();
    }
    //clear
    void CollectionObjects::clear()
    {
        m_pool.clear();
        for(auto& queue:m_queues) queue.clear();
    }
#endif
    
#if 0
    //compute distance
    static inline float compute_camera_depth(const Geometry::Frustum& f_camera, const transform_ptr& t_entity)
    {
        return f_camera.distance_from_near_plane(t_entity->get_global_position());
    }
    //by frustum
    void Collection::compute_lights_queues(PoolQueues& queues,const Geometry::Frustum& view_frustum)
    {
        //ref
        auto& rqueue_spot        = queues.m_queues[RQ_SPOT_LIGHT];
        auto& rqueue_point       = queues.m_queues[RQ_POINT_LIGHT];
        auto& rqueue_direction   = queues.m_queues[RQ_DIRECTION_LIGHT];
        //clear
        rqueue_spot.clear();
        rqueue_point.clear();
        rqueue_direction.clear();
        
        //build queue lights
        for (entity::wptr weak_entity: m_pool.m_lights)
        {
            auto entity   = weak_entity.lock();
            auto t_entity = entity->get_component<transform>();
            auto l_entity = entity->get_component<light>();
            
            if (l_entity->is_enabled() &&
                intersection::check(view_frustum, l_entity->get_sphere()) != intersection::OUTSIDE)
            {
                float depth = compute_camera_depth(view_frustum, t_entity);
                switch (l_entity->get_type())
                {
                    case light::SPOT_LIGHT:      rqueue_spot.push_front_to_back(weak_entity,depth);      break;
                    case light::POINT_LIGHT:     rqueue_point.push_front_to_back(weak_entity,depth);     break;
                    case light::DIRECTION_LIGHT: rqueue_direction.push_front_to_back(weak_entity,depth); break;
                    default: break;
                };
                
            }
        }
        
    }
    
    void CollectionObjects::compute_no_lights_queues(const Geometry::Frustum& view_frustum)
    {
        //clear
        m_queues[RQ_BACKGROUND].clear();
        m_queues[RQ_OPAQUE].clear();
        m_queues[RQ_TRANSLUCENT].clear();
        m_queues[RQ_UI].clear();
        //build queue opaque
        for (Weak<Object> weak_entity : m_pool.m_renderable)
        {
            auto entity     = weak_entity.lock();
            auto r_entity   = entity->get_component<renderable>();
            
            if (r_entity->is_enabled())
                if (material_ptr       r_material = r_entity->get_material())
                    if (effect::ptr        r_effect   = r_material->get_effect())
                        if (transform_ptr      t_entity   = entity->get_component<transform>())
                            if (!r_entity->has_support_culling() ||
                                intersection::check(view_frustum, r_entity->get_bounding_box()) != intersection::OUTSIDE
                                )
                            {
                                ///queue
                                const effect::parameter_queue& queue = r_effect->get_queue();
                                //distance
                                switch (queue.m_type)
                                {
                                    case RQ_TRANSLUCENT:
                                        m_queues[RQ_TRANSLUCENT].push_back_to_front(weak_entity, compute_camera_depth(view_frustum, t_entity));
                                        break;
                                    case RQ_OPAQUE:
                                        m_queues[RQ_OPAQUE].push_front_to_back(weak_entity, compute_camera_depth(view_frustum, t_entity));
                                        break;
                                    default:
                                        m_queues[queue.m_type].push_back_to_front(weak_entity, queue.m_order);
                                        break;
                                }
                            }
        }
    }
    
    //by sphere
    void CollectionObjects::compute_no_lights_queues(const Geometry::Sphere& in_sphere)
    {
        //clear
        m_queues[RQ_BACKGROUND].clear();
        m_queues[RQ_OPAQUE].clear();
        m_queues[RQ_TRANSLUCENT].clear();
        m_queues[RQ_UI].clear();
        //build queue opaque
        for (Weak<Object> weak_entity : m_pool.m_renderable)
        {
            auto entity     = weak_entity.lock();
            auto r_entity   = entity->get_component<renderable>();
            
            if (r_entity->is_enabled())
                if (material_ptr       r_material = r_entity->get_material())
                    if (effect::ptr        r_effect   = r_material->get_effect())
                        if (transform_ptr      t_entity   = entity->get_component<transform>())
                            if (!r_entity->has_support_culling() ||
                                intersection::check(r_entity->get_bounding_box(), in_sphere))
                            {
                                ///queue
                                const effect::parameter_queue& queue = r_effect->get_queue();
                                //distance
                                switch (queue.m_type)
                                {
                                    case RQ_TRANSLUCENT:
                                        m_queues[RQ_TRANSLUCENT].push_back_to_front(weak_entity, distance(t_entity->get_global_position(), in_sphere.get_position()));
                                        break;
                                    case RQ_OPAQUE:
                                        m_queues[RQ_OPAQUE].push_front_to_back(weak_entity, distance(t_entity->get_global_position(), in_sphere.get_position()));
                                        break;
                                    default:
                                        m_queues[queue.m_type].push_back_to_front(weak_entity, queue.m_order);
                                        break;
                                }
                            }
        }
    }
#endif
}
}
