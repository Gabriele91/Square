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
#include "Square/Render/Pipeline/ForwardShading.h"
#include "Square/Render/Light.h"
#include "Square/Render/Pipeline/DrawList.h"

namespace Square
{
namespace Render
{
	namespace AuxForwardShading
	{
		//the draws of a pass with each light of a queue (the visible ones, until one whose shadow is
		//not the one of the pass, as they are in the queue)
		static void add_lit
		(
			  DrawList& list
			, const Shared<Renderable>& renderable
			, size_t material_id
			, EffectPass& pass
			, size_t pass_index
			, EffectParameters* parameters
			, const Queue& lights
			, bool shadow
		)
		{
			for (auto element : lights)
			{
				auto light = element->lock<Light>();
				if (light && light->visible())
				{
					if (light->shadow() != shadow)
					{
						break;
					}
					for (int draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
					{
						list.add(renderable, material_id, pass, pass_index, parameters, draw_id, light.get());
					}
				}
			}
		}

		//the draws of a renderable: each material, the technique of its effect, each pass of it
		//with no light, the ambient one, or each light of its kind
		static void add
		(
			  DrawList& list
			, const Shared<Renderable>& renderable
			, const char* technique_name
			, const PoolQueues& queues
		)
		{
			//(the clip variant: by its material, or its level of detail is fading)
			const bool fading = renderable->lod_fade() < 1.0f;
			for (size_t material_id = 0; material_id != renderable->materials_count(); ++material_id)
			{
				auto material = renderable->material(material_id).lock();
				EffectTechnique* technique = nullptr;
				if (material && material->effect())
				{
					technique = material->effect()->technique(technique_name, renderable->variant(), *material->parameters(), fading);
				}
				if (technique)
				{
					size_t pass_index = 0;
					for (EffectPass& pass : *technique)
					{
						//light only or light and shadow?
						const bool shadow = pass.m_support_shadow != EffectPass::LT_NONE;
						const int support = static_cast<int>(shadow ? pass.m_support_shadow : pass.m_support_light);
						EffectParameters* parameters = material->parameters();
						switch (support)
						{
						case EffectPass::LT_NONE:
						case EffectPass::LT_AMBIENT:
							//(no shadow light)
							if (!shadow)
							{
								for (int draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
								{
									list.add(renderable, material_id, pass, pass_index, parameters, draw_id, nullptr);
								}
							}
							break;
						case EffectPass::LT_DIRECTION: add_lit(list, renderable, material_id, pass, pass_index, parameters, queues[RQ_DIRECTION_LIGHT], shadow); break;
						case EffectPass::LT_POINT:     add_lit(list, renderable, material_id, pass, pass_index, parameters, queues[RQ_POINT_LIGHT], shadow); break;
						case EffectPass::LT_SPOT:      add_lit(list, renderable, material_id, pass, pass_index, parameters, queues[RQ_SPOT_LIGHT], shadow); break;
						default: break;
						}
						++pass_index;
					}
				}
			}
		}
	}

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
		//what the camera sees: the parts of a renderable out of it not drawn
		inputs.m_frustum = &camera.frustum();
		//update camera
		Render::UniformBufferCamera ucamera;
		camera.set(&ucamera);
		render.update_steam_CB(buffers.m_camera, (const unsigned char*)&ucamera, sizeof(ucamera));
		//the buffers of a light (its shadow, its shadow map: a shadow light)
		auto light_setup = [&](Light& light)
		{
			switch (light.type())
			{
			case LightType::DIRECTION:
			{
				Render::UniformDirectionLight udirection_light;
				light.set(&udirection_light);
				Render::update_constant_buffer(&render, buffers.m_direction_light, &udirection_light);
				if (light.shadow())
				{
					Render::UniformDirectionShadowLight udirection_shadow_light;
					light.set(&udirection_shadow_light, &camera, false);
					Render::update_constant_buffer(&render, buffers.m_direction_shadow_light, &udirection_shadow_light);
					inputs.m_shadow_map = light.shadow_buffer().texture();
				}
			}
			break;
			case LightType::POINT:
			{
				Render::UniformPointLight upoint_light;
				light.set(&upoint_light);
				Render::update_constant_buffer(&render, buffers.m_point_light, &upoint_light);
				if (light.shadow())
				{
					Render::UniformPointShadowLight upoint_shadow_light;
					light.set(&upoint_shadow_light, false);
					Render::update_constant_buffer(&render, buffers.m_point_shadow_light, &upoint_shadow_light);
					inputs.m_shadow_map = light.shadow_buffer().texture();
				}
			}
			break;
			case LightType::SPOT:
			{
				Render::UniformSpotLight uspot_light;
				light.set(&uspot_light);
				Render::update_constant_buffer(&render, buffers.m_spot_light, &uspot_light);
				if (light.shadow())
				{
					Render::UniformSpotShadowLight uspot_shadow_light;
					light.set(&uspot_shadow_light, false);
					Render::update_constant_buffer(&render, buffers.m_spot_shadow_light, &uspot_shadow_light);
					inputs.m_shadow_map = light.shadow_buffer().texture();
				}
			}
			break;
			default: break;
			}
		};
		//each queue in its order: the opaque one in batches (sorted), the others as they are (the
		//translucent ones: back to front)
		DrawList list;
		for (QueueType queue_type : queue_types)
		{
			for (auto randerable : RenderableQuery(queues, { queue_type }))
			{
				if (randerable && randerable->can_draw())
				{
					AuxForwardShading::add(list, randerable, technique_name, queues);
				}
			}
			list.draw(render, inputs, queue_type == RQ_OPAQUE, light_setup);
		}
	}
}
}
