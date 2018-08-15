//
//  DrawerPassForward.cpp
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/DrawerPassShadow.h"
#include "Square/Resource/Effect.h"

namespace Square
{
namespace Render
{
	DrawerPassShadow::DrawerPassShadow(Square::Context& context)
    : DrawerPass(RPT_SHADOW)
    , m_context(context)
    {
		m_shadow = DynamicPointerCast<Render::Effect>(context.resource<Resource::Effect>("Shadow"));

		m_cb_camera = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());

		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionShadowLight>(&render());
		m_cb_point_light = Render::stream_constant_buffer<Render::UniformPointShadowLight>(&render());
		m_cb_spot_light = Render::stream_constant_buffer<Render::UniformSpotShadowLight>(&render());
	}
    //context
    Square::Context& DrawerPassShadow::context(){ return m_context; }
    const Square::Context& DrawerPassShadow::context() const { return m_context; }
    //render
    Render::Context& DrawerPassShadow::render(){ return *context().render(); }
    const Render::Context& DrawerPassShadow::render() const { return *context().render(); }
    //draw
    void DrawerPassShadow::draw
    (
       Drawer&           drawer
     , int               num_of_pass
     , const Vec4&       clear_color
     , const Vec4&       ambient_light
	 , const Light&      light
     , const Collection& collection
     , const PoolQueues& queues
    )
    {
		//effect loaded
		if (!m_shadow) 
		{
			context().add_wrong("Warning: can't draw shadow map");
			return;
		}
        //transfor
		Render::UniformBufferTransform utransform;
		Render::EffectTechnique* technique = nullptr;
		//parameters
		EffectPassInputs inputs
		{
			//render
			  m_cb_camera.get()
			, m_cb_transform.get()
			//light
			, ambient_light
			, nullptr
			, nullptr
			, nullptr
			, nullptr
			, m_cb_direction_light.get()
			, m_cb_point_light.get()
			, m_cb_spot_light.get()
		};
		//light
		switch (light.type())
		{
		case LightType::DIRECTION:
		{
			Render::UniformDirectionShadowLight udirectionshadow;
			//update camera
			light.set(&udirectionshadow);
			render().update_steam_CB(m_cb_direction_light.get(), (const unsigned char*)&udirectionshadow, sizeof(udirectionshadow));
			//load currect tequnique
			technique = m_shadow->technique("DirectionShadow");
		}
		break;
		case LightType::POINT:
		{
			Render::UniformPointShadowLight upointshadow;
			//update camera
			light.set(&upointshadow);
			render().update_steam_CB(m_cb_point_light.get(), (const unsigned char*)&upointshadow, sizeof(upointshadow));
			//load currect tequnique
			technique = m_shadow->technique("PointShadow");
			//set viewport
			render().set_viewport_state(light.shadow_viewport());
		}
		break;
		case LightType::SPOT:
		{
			Render::UniformSpotShadowLight uspotshadow;
			//update camera
			light.set(&uspotshadow);
			render().update_steam_CB(m_cb_spot_light.get(), (const unsigned char*)&uspotshadow, sizeof(uspotshadow));
			//load currect tequnique
			technique = m_shadow->technique("SpotShadow");
			//set viewport
			render().set_viewport_state(light.shadow_viewport());
		}
		break;
		default: return;
		}
		//not implemented, return
		if (!technique) return;
		//clear
		render().clear(Render::CLEAR_DEPTH);
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
				//draw for each pass
				for (auto& pass : *technique)
				{
					randerable->draw(render(), material_id, inputs, pass);
				}
			}
        }
    }
}
}