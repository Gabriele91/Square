//
//  Drawer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 24/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <set>
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Render
{
    //Init
    Drawer::Drawer(Square::Context& context):m_context(context){}
    
    //context
    Square::Context& Drawer::context(){ return m_context; }
    const Square::Context& Drawer::context() const { return m_context; }
    
    Render::Context& Drawer::render(){ return *context().render(); }
    const Render::Context& Drawer::render() const { return *context().render(); }
    
    //draw
    void Drawer::draw(const Collection& collection)
    {
        draw(Constants::identity<Vec4>(), Constants::identity<Vec4>(), collection);
    }
    void Drawer::draw(const Vec4& clear_color,const Vec4& ambient_color, const Collection& collection)
    {
        //objects queues
        std::vector<PoolQueues> queues;
        init_only_movable_t_vector(queues, collection.m_cameras.size(), context().allocator());
        //shadow rendered
        std::set<Light*> m_shadows_draw;
        //filter
        auto can_draw_shadow = [&](Light* light) -> bool
        {  return light->shadow() && m_shadows_draw.find(light) == m_shadows_draw.end(); };
        //build queues
        int camera_index { 0 };
        for(auto weak_camera : collection.m_cameras)
        if (auto camera = weak_camera.lock())
        {
            CollectionQuery::lights(collection, queues[camera_index], *camera);
            CollectionQuery::renderables(collection, queues[camera_index], *camera);
            ++camera_index;
        }
        //compute scene size
        auto scene_size = [&](PoolQueues& queues) -> Geometry::AABoundingBox
        {
            //compute scene size
            Geometry::AABoundingBox scene_aabb
            (
                Vec3{ std::numeric_limits<float>::max() },
                Vec3{ std::numeric_limits<float>::lowest() }
            );
            //for each elements of opaque and translucent queues
            for (QueueType qtype : {RQ_OPAQUE, RQ_TRANSLUCENT})
            for (const QueueElement* e_randerable : queues[qtype])
            if (auto randerable = e_randerable->lock< Render::Renderable >())
            {
                //jump?
                if (!randerable->can_draw()) continue;
                //get box
                scene_aabb = scene_aabb.merge(randerable->bounding_box().to_aabb());
            }
            return scene_aabb;
        };
        //shadow caster
        camera_index = 0;
        for(auto weak_camera : collection.m_cameras)
        if (auto camera = weak_camera.lock())
        {
            //pass counter
            int pass_shadow_count { 0 };
            //for each pass
            for(auto& pass : m_rendering_pass[RPT_SHADOW])
            {
                //draw a shadow map
                auto draw_shadow = [&](Shared<Light> light, QueueType queue_type)
                {
                    if (light)
                    if (can_draw_shadow(light.get()))
                    {
						//opaque queue
						PoolQueues queues(context().allocator());
						//compute queue
						CollectionQuery::renderables(collection, queues, *light);
						//render target
						render().enable_render_target(light->shadow_buffer().target());
                        //set scene size
                        if(queue_type == RQ_DIRECTION_LIGHT)
                            light->set_scene_size(scene_size(queues));
                        //draw
                        pass->draw(  *this
                                    , pass_shadow_count++
                                    , clear_color
                                    , ambient_color
                                    , *camera
                                    , *light
                                    , collection
                                    , queues
                                    );
						//disable
						render().disable_render_target(light->shadow_buffer().target());
						//drawed
                        m_shadows_draw.insert(light.get());
                    }
                };
                //for spot light objects
                for(auto e_light : queues[camera_index][RQ_SPOT_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_SPOT_LIGHT);
                for(auto e_light : queues[camera_index][RQ_POINT_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_POINT_LIGHT);
                for(auto e_light : queues[camera_index][RQ_DIRECTION_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_DIRECTION_LIGHT);
            }
            
        }
        //for each cameras
        for(auto weak_camera : collection.m_cameras)
        if (auto camera = weak_camera.lock())
        {
            //pass counter
            int pass_render_count { 0 };
            //draw all
            for(auto& pass : m_rendering_pass[RPT_RENDER])
            {
                pass->draw(  *this
                           , pass_render_count++
                           , clear_color
                           , ambient_color
                           , *camera
                           , collection
                           , queues[camera_index]
                           );
            }
            //pass counter
            int pass_ui_count { 0 };
            //draw all
            for(auto& pass : m_rendering_pass[RPT_UI])
            {
                pass->draw(  *this
                           , pass_ui_count++
                           , clear_color
                           , ambient_color
                           , *camera
                           , collection
                           , queues[camera_index]
                           );
            }
        }
    }
    
    //add a pass
    void Drawer::add(Shared<Square::Render::DrawerPass> pass)
    {
        m_rendering_pass[pass->type()].push_back(pass);
    }
}
}
