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
        //objects queues;
        std::vector<PoolQueues> queues(collection.m_cameras.size());
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
        //shadow caster
        camera_index = 0;
        for(auto weak_camera : collection.m_cameras)
        if (auto camera = weak_camera.lock())
        {
            //pass counter
            int pass_shadow_count { 0 };
            //for each pass
            for(auto pass : m_rendering_pass[RPT_SHADOW])
            {
                //draw a shadow map
                auto draw_shadow = [&](Shared<Light> light)
                {
                    if (light)
                    if (can_draw_shadow(light.get()))
                    {
						//opaque queue
						PoolQueues queues;
						//compute queue
						CollectionQuery::renderables(collection, queues, *light);
						//render target
						render().enable_render_target(light->shadow_buffer().target());
                        //draw
                        pass->draw(  *this
                                    , pass_shadow_count++
                                    , clear_color
                                    , ambient_color
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
                for(auto e_light : queues[camera_index][RQ_SPOT_LIGHT]) draw_shadow(e_light->lock<Light>());
                for(auto e_light : queues[camera_index][RQ_POINT_LIGHT]) draw_shadow(e_light->lock<Light>());
                for(auto e_light : queues[camera_index][RQ_DIRECTION_LIGHT]) draw_shadow(e_light->lock<Light>());
            }
            
        }
        //for each cameras
        for(auto weak_camera : collection.m_cameras)
        if (auto camera = weak_camera.lock())
        {
            //pass counter
            int pass_render_count { 0 };
            //draw all
            for(auto pass : m_rendering_pass[RPT_RENDER])
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
            for(auto pass : m_rendering_pass[RPT_UI])
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
