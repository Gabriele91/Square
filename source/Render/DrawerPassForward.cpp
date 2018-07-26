//
//  DrawerPassForward.cpp
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/DrawerPassForward.h"

namespace Square
{
namespace Render
{
    DrawerPassForward::DrawerPassForward(Square::Context& context)
    : DrawerPass(RPT_RENDER)
    , m_context(context)
    {
        m_cb_camera    = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
        m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
    }
    //context
    Square::Context& DrawerPassForward::context(){ return m_context; }
    const Square::Context& DrawerPassForward::context() const { return m_context; }
    //render
    Render::Context& DrawerPassForward::render(){ return *context().render(); }
    const Render::Context& DrawerPassForward::render() const { return *context().render(); }
    //draw
    void DrawerPassForward::draw
    (
       Drawer&           drawer
     , int               num_of_pass
     , const Vec4&       clear_color
     , const Vec4&       ambient_color
     , const Camera&     camera
     , const Collection& collection
     , const PoolQueues& queues
    )
    {
        //start to draw
        if(num_of_pass == 0)
        {
            //set viewport (2D, On Screen)
            render().set_viewport_state({ camera.viewport().viewport() });
            //set color
            render().set_clear_color_state({ clear_color });
            //clear
            render().clear();
        }
        //transfor
        Render::UniformBufferCamera ucamera;
        Render::UniformBufferTransform utransform;
        //update camera
        camera.set(&ucamera);
        render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
        //get effect
        for(const QueueElement* e_randerable : queues[RQ_OPAQUE])
        if (auto randerable = e_randerable->lock< Render::Renderable >())
        {
            //jump?
            if(!randerable->can_draw()) continue;
            //material
            auto weak_material = randerable->material();
            auto material = weak_material.lock();
            if(!material) continue;
            //effect
            auto effect = material->effect();
            auto technique = effect->technique("forward");
            if(!technique) continue;
            //update transform
            if (auto transform = randerable->transform().lock())
            {
                transform->set(&utransform);
                render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
            }
            //draw for each pass
            for(auto& pass : *technique)
            {
                //bind effect
                pass.bind(  &render()
                          , randerable->layout_id()
                          , m_cb_camera.get()
                          , m_cb_transform.get()
                          , material->parameters()
                          );
                //draw
                randerable->draw(render());
            }
        }
    }
}
}
