//
//  DrawerPassForward.h
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
//  Forward: the renderables are drawn on the screen with their "forward" technique.
//  With color post effects (see PostEffect.h) the scene is drawn on an intermediate target
//  (color HDR + depth), the post effects run on it, then it is copied to the screen (with its
//  depth, for the passes that follow). The G-Buffer post effects are not drawn (no G-Buffer).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/PostEffect.h"

namespace Square
{
namespace Resource
{
	class Shader;
}
namespace Render
{
    class SQUARE_API DrawerPassForward : public DrawerPass
    {
    public:
        //passo
        DrawerPassForward(Square::Context& context);
        //disegna
        virtual void draw
        (
          Drawer& drawer
        , int num_of_pass
        , const Vec4&  clear_color
        , const Vec4&  ambient_color
        , const Camera& camera
        , const Collection& collection
        , const PoolQueues& queues
        )
        override;
    
    protected:
        //the intermediate target of the post effects, of a size (rebuilt when it changes)
        bool build_frame(const IVec2& size);
        //the frame (post effects result) on the screen, and the depth of the scene
        void present(const Camera& camera, Texture* frame);
        //context
        Square::Context& context();
        const Square::Context& context() const;
        //render
        Render::Context& render();
        const Render::Context& render() const;
        //CPU DATA
        Square::Context& m_context;
        //GPU DATA
        Shared<Render::ConstBuffer> m_cb_camera;
		Shared<Render::ConstBuffer> m_cb_transform;

		Shared<Render::ConstBuffer> m_cb_direction_light;
		Shared<Render::ConstBuffer> m_cb_point_light;
		Shared<Render::ConstBuffer> m_cb_spot_light;

		Shared<Render::ConstBuffer> m_cb_direction_shadow_light;
		Shared<Render::ConstBuffer> m_cb_point_shadow_light;
		Shared<Render::ConstBuffer> m_cb_spot_shadow_light;
        //post effects: intermediate target (color + depth), color chain, final copy
        Shared<GBuffer>                m_frame;
        PostEffectChain                m_post_effects;
        Shared<Mesh>                   m_quad;
        Shared<Resource::Shader>       m_shader_copy;
    };
}
}
