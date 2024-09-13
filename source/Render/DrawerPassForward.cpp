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
        //buffers
        Render::UniformBufferCamera ucamera;
		Render::UniformBufferTransform utransform;

		Render::UniformDirectionLight udirection_light;
		Render::UniformPointLight upoint_light;
		Render::UniformSpotLight uspot_light;

		Render::UniformDirectionShadowLight udirection_shadow_light;
		Render::UniformPointShadowLight upoint_shadow_light;
		Render::UniformSpotShadowLight uspot_shadow_light;
		//parameters
		EffectPassInputs inputs
		{
			//render
			  m_cb_camera.get()
			, m_cb_transform.get()
			//light
			, ambient_light
			, m_cb_direction_light.get()
			, m_cb_point_light.get()
			, m_cb_spot_light.get()
			//shadow
			, nullptr
			, m_cb_direction_shadow_light.get()
			, m_cb_point_shadow_light.get()
			, m_cb_spot_shadow_light.get()
		};
        //update camera
        camera.set(&ucamera);
        render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
        //for each elements of opaque  and translucent queues
		for(QueueType qtype : {RQ_OPAQUE, RQ_TRANSLUCENT})
        for(const QueueElement* e_randerable : queues[qtype])
        if (auto randerable = e_randerable->lock< Render::Renderable >())
        {
            //jump?
            if(!randerable->can_draw()) continue;
			//update transform
			if (auto transform = randerable->transform().lock())
			{
				transform->set(&utransform);
				render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
			}
			//set id
			//for each materials
			for (size_t material_id = 0; material_id != randerable->materials_count(); ++material_id)
			{
				//material
				auto weak_material = randerable->material(material_id);
				auto material = weak_material.lock();
				if (!material) continue;
				//effect
				auto effect = material->effect();
				auto technique = effect->technique("forward");
				if (!technique) continue;
				//draw for each pass
				for (auto& pass : *technique)
				{
					//light only or light and shadow?
                    int support[]
                    {
                          static_cast<int>(pass.m_support_light)
                        , static_cast<int>(pass.m_support_shadow)
                    };
					//shadow?
					bool shadow = pass.m_support_shadow != EffectPass::LT_NONE;
					//bind
					switch (support[shadow])
					{
						//not costant buffer
					case EffectPass::LT_NONE:
					case EffectPass::LT_AMBIENT:
						//no shadow light
						if (shadow) break;
						//draw
						randerable->draw(render(), material_id, inputs, pass);
						break;
						//update constant buffer
					case EffectPass::LT_DIRECTION:
						for (auto weak_light : queues[RQ_DIRECTION_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
							light->set(&udirection_light);
							//update buffer
							Render::update_constant_buffer(&render(), m_cb_direction_light.get(), &udirection_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&udirection_shadow_light, &camera);
								//update buffer
								Render::update_constant_buffer(&render(), m_cb_direction_shadow_light.get(), &udirection_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							randerable->draw(render(), material_id, inputs, pass);
						}
						break;
						//update constant buffer
					case EffectPass::LT_POINT:
						for (auto weak_light : queues[RQ_POINT_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
;							light->set(&upoint_light);
							//update buffer
							Render::update_constant_buffer(&render(), m_cb_point_light.get(), &upoint_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&upoint_shadow_light);
								//update buffer
								Render::update_constant_buffer(&render(), m_cb_point_shadow_light.get(), &upoint_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							randerable->draw(render(), material_id, inputs, pass);
						}
						break;
						//update constant buffer
					case EffectPass::LT_SPOT:
						for (auto weak_light : queues[RQ_SPOT_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
							light->set(&uspot_light);
							//update buffer
							Render::update_constant_buffer(&render(), m_cb_spot_light.get(), &uspot_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&uspot_shadow_light);
								//update buffer
								Render::update_constant_buffer(&render(), m_cb_spot_shadow_light.get(), &uspot_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							randerable->draw(render(), material_id, inputs, pass);
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
}
