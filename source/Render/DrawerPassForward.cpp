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
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/DrawerPassForward.h"
#include "Square/Render/ForwardShading.h"

namespace Square
{
namespace Render
{
    DrawerPassForward::DrawerPassForward(Square::Context& context)
    : DrawerPass(context.allocator(), RPT_RENDER)
    , m_context(context)
    {
        m_cb_camera    = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());

		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionLight>(&render());
		m_cb_point_light = Render::stream_constant_buffer<Render::UniformPointLight>(&render());
		m_cb_spot_light = Render::stream_constant_buffer<Render::UniformSpotLight>(&render());
		
		m_cb_direction_shadow_light = Render::stream_constant_buffer<Render::UniformDirectionShadowLight>(&render());
		m_cb_point_shadow_light = Render::stream_constant_buffer<Render::UniformPointShadowLight>(&render());
		m_cb_spot_shadow_light = Render::stream_constant_buffer<Render::UniformSpotShadowLight>(&render());
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
     , const Vec4&       ambient_light
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
        //draw opaque and translucent renderables with their "forward" technique
        draw_forward
        (
              render()
            , "forward"
            , camera
            , ambient_light
            , queues
            , { RQ_OPAQUE, RQ_TRANSLUCENT }
            , ForwardShadingBuffers
              {
                  m_cb_camera.get()
                , m_cb_transform.get()
                , m_cb_direction_light.get()
                , m_cb_point_light.get()
                , m_cb_spot_light.get()
                , m_cb_direction_shadow_light.get()
                , m_cb_point_shadow_light.get()
                , m_cb_spot_shadow_light.get()
              }
        );
    }
}
}
