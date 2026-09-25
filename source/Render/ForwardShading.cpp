//
//  ForwardShading.cpp
//  Square
//
//  See ForwardShading.h
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
#include "Square/Render/ForwardShading.h"

namespace Square
{
namespace Render
{
	void draw_forward
	(
	   Render::Context&             render
	 , const char*                  technique_name
	 , const Camera&                camera
	 , const Vec4&                  ambient_light
	 , const PoolQueues&            queues
	 , const std::vector<QueueType>& queue_types
	 , const ForwardShadingBuffers& buffers
	)
	{
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
			  buffers.m_camera
			, buffers.m_transform
			//light
			, ambient_light
			, buffers.m_direction_light
			, buffers.m_point_light
			, buffers.m_spot_light
			//shadow
			, nullptr
			, buffers.m_direction_shadow_light
			, buffers.m_point_shadow_light
			, buffers.m_spot_shadow_light
		};
        //update camera
        camera.set(&ucamera);
        render.update_steam_CB(buffers.m_camera, (const unsigned char*)&ucamera, sizeof(ucamera));
        //for each element of the selected queues
		for(auto randerable : RenderableQuery(queues, queue_types))
        if (randerable)
        {
            //jump?
            if(!randerable->can_draw()) continue;
			//update transform
			if (auto transform = randerable->transform().lock())
			{
				transform->set(&utransform);
				render.update_steam_CB(buffers.m_transform, (const unsigned char*)&utransform, sizeof(utransform));
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
				auto technique = effect->technique(technique_name);
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
						for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
							randerable->draw(render, material_id, inputs, pass, draw_id);
						break;
						//update constant buffer
					case EffectPass::LT_DIRECTION:
						for (auto weak_light : queues[RQ_DIRECTION_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							if (!light->visible()) continue;
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
							light->set(&udirection_light);
							//update buffer
							Render::update_constant_buffer(&render, buffers.m_direction_light, &udirection_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&udirection_shadow_light, &camera, false);
								//update buffer
								Render::update_constant_buffer(&render, buffers.m_direction_shadow_light, &udirection_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
								randerable->draw(render, material_id, inputs, pass, draw_id);
						}
						break;
						//update constant buffer
					case EffectPass::LT_POINT:
						for (auto weak_light : queues[RQ_POINT_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							if (!light->visible()) continue;
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
;							light->set(&upoint_light);
							//update buffer
							Render::update_constant_buffer(&render, buffers.m_point_light, &upoint_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&upoint_shadow_light, false);
								//update buffer
								Render::update_constant_buffer(&render, buffers.m_point_shadow_light, &upoint_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
								randerable->draw(render, material_id, inputs, pass, draw_id);
						}
						break;
						//update constant buffer
					case EffectPass::LT_SPOT:
						for (auto weak_light : queues[RQ_SPOT_LIGHT])
						if (auto light = weak_light->lock< Render::Light >())
						{
							if (!light->visible()) continue;
							//is a shadow light
							if (light->shadow() != shadow) break;
							//get buffer
							light->set(&uspot_light);
							//update buffer
							Render::update_constant_buffer(&render, buffers.m_spot_light, &uspot_light);
							//shadow
							if (shadow)
							{
								//get buffer
								light->set(&uspot_shadow_light, false);
								//update buffer
								Render::update_constant_buffer(&render, buffers.m_spot_shadow_light, &uspot_shadow_light);
								//shadow map
								inputs.m_shadow_map = light->shadow_buffer().texture();
							}
							//draw
							for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
								randerable->draw(render, material_id, inputs, pass, draw_id);
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
