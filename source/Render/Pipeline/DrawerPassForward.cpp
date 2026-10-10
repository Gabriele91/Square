//
//  DrawerPassForward.cpp
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/Pipeline/DrawerPassForward.h"
#include "Square/Render/Profiler.h"
#include "Square/Render/BasicMesh.h"
#include "Square/Render/Pipeline/ForwardShading.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
    DrawerPassForward::DrawerPassForward(Square::Context& context)
    : DrawerPass(context.allocator(), RPT_RENDER)
    , m_context(context)
    , m_post_effects(context)
    {
        m_cb_camera    = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());

		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionLight>(&render());
		m_cb_point_light = Render::stream_constant_buffer<Render::UniformPointLight>(&render());
		m_cb_spot_light = Render::stream_constant_buffer<Render::UniformSpotLight>(&render());
		
		m_cb_direction_shadow_light = Render::stream_constant_buffer<Render::UniformDirectionShadowLight>(&render());
		m_cb_point_shadow_light = Render::stream_constant_buffer<Render::UniformPointShadowLight>(&render());
		m_cb_spot_shadow_light = Render::stream_constant_buffer<Render::UniformSpotShadowLight>(&render());
		//post effects: full-screen quad and the final copy
		m_quad = BasicMesh::build_quad(context);
		m_shader_copy = context.resource<Resource::Shader>("PostCopy");
    }
    //context
    Square::Context& DrawerPassForward::context(){ return m_context; }
    const Square::Context& DrawerPassForward::context() const { return m_context; }
    //render
    Render::Context& DrawerPassForward::render(){ return *System::get<RenderSystem>(context())->render(); }
    const Render::Context& DrawerPassForward::render() const { return *System::get<RenderSystem>(context())->render(); }
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
        //color post effects: the scene goes on the intermediate target
        const auto& post_effects = drawer.post_effects();
        const Vec4& viewport = camera.viewport().viewport();
        const IVec2 size((int)viewport.z, (int)viewport.w);
        const bool post = PostEffectChain::any(post_effects, PES_COLOR)
                       && m_shader_copy && m_shader_copy->base_shader()
                       && size.x > 0 && size.y > 0
                       && build_frame(size);
        if (post) render().enable_render_target(m_frame->target());
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
        {
        SQUARE_RENDER_SCOPE(render(), "Forward");
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
        //post effects, then the result on the screen
        if (post)
        {
            SQUARE_RENDER_SCOPE(render(), "Color effects");
            render().disable_render_target(m_frame->target());
            PostEffectFrame frame;
            frame.m_render        = &render();
            frame.m_camera        = &camera;
            frame.m_camera_buffer = m_cb_camera.get(); //updated by draw_forward
            frame.m_size          = size;
            frame.m_viewport      = viewport;
            frame.m_quad          = m_quad.get();
            //the forward shaders write linear colors on an sRGB framebuffer, encoded ones otherwise
            frame.m_linear        = render().is_srgb_framebuffer();
            Texture* result = m_post_effects.draw_color(post_effects, frame, m_frame->texture(0));
            //or the debug view of a post effect
            if (Texture* debug = PostEffectChain::debug_texture(post_effects)) result = debug;
            present(camera, result);
        }
    }

    bool DrawerPassForward::build_frame(const IVec2& size)
    {
        if (m_frame && m_frame->size() == size) return m_frame->target() != nullptr;
        m_frame = MakeShared<GBuffer>(context(), size, std::vector<GBuffer::BufferFormat>
        {
            GBuffer::BufferFormat(TF_RGBA16F, TT_RGBA, TTF_FLOAT, RT_COLOR),
            //same depth format as the screen: it is copied there after the post effects
            GBuffer::BufferFormat(TF_DEPTH24_STENCIL8, TT_DEPTH_STENCIL, TTF_UNSIGNED_INT_24_8, RT_DEPTH)
        });
        return m_frame->target() != nullptr;
    }

    void DrawerPassForward::present(const Camera& camera, Texture* frame)
    {
        //the frame on the screen (as it is: already in the space of the screen)
        render().set_viewport_state({ camera.viewport().viewport() });
        render().set_depth_buffer_state({ DM_DISABLE });
        render().set_blend_state({});
        render().set_cullface_state({ CF_BACK });
        m_shader_copy->bind();
        if (auto uniform_source = m_shader_copy->uniform("g_source")) uniform_source->set(frame);
        m_quad->draw(render());
        m_shader_copy->unbind();
        render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
        //the depth of the scene, for the passes that follow (debug...)
        const IVec4 area(0, 0, (int)m_frame->size().x, (int)m_frame->size().y);
        render().copy_target_to_target(area, m_frame->target(), area, nullptr, RT_DEPTH);
    }
}
}
