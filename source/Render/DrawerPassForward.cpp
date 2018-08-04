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
		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionLight>(&render());
		m_cb_point_light = Render::stream_constant_buffer<Render::UniformPointLight>(&render());
		m_cb_spot_light = Render::stream_constant_buffer<Render::UniformSpotLight>(&render());
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
		Render::UniformDirectionLight udirection_light;
		Render::UniformPointLight upoint_light;
		Render::UniformSpotLight uspot_light;
		//parameters
		EffectPass::Buffers cbuffers
		{
			  ~size_t(0)
			, m_cb_camera.get()
			, m_cb_transform.get()
			, m_cb_direction_light.get()
			, m_cb_point_light.get()
			, m_cb_spot_light.get()
		};
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
			//set id
			cbuffers.m_layout_id = randerable->layout_id();
            //draw for each pass
            for(auto& pass : *technique)
            {
				//set parameters
				pass.bind
				(
					&render()
					, ambient_color
					, cbuffers
					, material->parameters()
				);
				//bind
				switch (pass.m_support_light)
				{
				//not costant buffer
				case EffectPass::LT_NONE:
				case EffectPass::LT_AMBIENT:
					pass.bind
					(
						&render()
						, ambient_color
						, cbuffers
						, material->parameters()
					);
					//draw
					randerable->draw(render());
				break;
				//update constant buffer
				case EffectPass::LT_DIRECTION:
					for (auto weak_light : queues[RQ_DIRECTION_LIGHT]) 
					if  (auto light = weak_light->lock< Render::Light >())
					{
						//get buffer
						light->set(&udirection_light);
						//update buffer
						Render::update_constant_buffer(&render(), m_cb_direction_light.get(), &udirection_light);
						//draw
						randerable->draw(render());
					}
				break;
				//update constant buffer
				case EffectPass::LT_POINT:
					for (auto weak_light : queues[RQ_POINT_LIGHT]) 
					if  (auto light = weak_light->lock< Render::Light >())
					{
						//get buffer
						light->set(&upoint_light);
						//update buffer
						Render::update_constant_buffer(&render(), m_cb_point_light.get(), &upoint_light);
						//draw
						randerable->draw(render());
					}
				break;
				//update constant buffer
				case EffectPass::LT_SPOT:
					for (auto weak_light : queues[RQ_SPOT_LIGHT]) 
					if  (auto light = weak_light->lock< Render::Light >())
					{
						//get buffer
						light->set(&uspot_light);
						//update buffer
						Render::update_constant_buffer(&render(), m_cb_spot_light.get(), &uspot_light);
						//draw
						randerable->draw(render());
					}
				break;
				/* not support */
				default: continue;
				}
            }
        }
    }
}
}
