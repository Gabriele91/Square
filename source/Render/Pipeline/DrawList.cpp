//
//  DrawList.cpp
//  Square
//
//  See DrawList.h for the high level description.
//
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include "Square/Driver/Render.h"
#include "Square/Render/Light.h"
#include "Square/Render/Material.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Pipeline/DrawList.h"

namespace Square
{
namespace Render
{
	void DrawList::clear()
	{
		m_draws.clear();
	}

	void DrawList::add
	(
		  const Shared<Renderable>& renderable
		, const std::string& technique_name
		, uint32 layers
		, bool settle
	)
	{
		//its fade (settled: whole); the clip variant: by its material, or its fade
		const float fade = settle ? 1.0f : renderable->lod_fade();
		const bool fading = fade < 1.0f;
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
					//(a multi-pass one: a draw for each of its layers)
					const bool multi_pass = pass.m_draw_count > 1;
					for (int draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
					{
						if (!multi_pass || MULTI_PASS_HAS_LAYER(layers, draw_id))
						{
							m_draws.push_back(Draw{ &pass, pass_index, material->parameters(), nullptr, renderable, material_id, draw_id, layers, fade, m_draws.size() });
						}
					}
					++pass_index;
				}
			}
		}
	}

	void DrawList::add
	(
		  const Shared<Renderable>& renderable
		, size_t material_id
		, EffectPass& pass
		, size_t pass_index
		, EffectParameters* parameters
		, int draw_id
		, Light* light
	)
	{
		m_draws.push_back(Draw{ &pass, pass_index, parameters, light, renderable, material_id, draw_id, MULTI_PASS_ALL_LAYERS, renderable->lod_fade(), m_draws.size() });
	}

#if defined(SQUARE_DEBUG_TOOLS)
	void DrawList::test(const char* environment_variable)
	{
		const char* value = std::getenv(environment_variable);
		m_test = Test::NONE;
		if (value && std::strcmp(value, "ordered") == 0)
		{
			m_test = Test::ORDERED;
		}
		else if (value && std::strcmp(value, "off") == 0)
		{
			m_test = Test::OFF;
		}
	}
#endif

	void DrawList::draw(Render::Context& render, EffectPassInputs& inputs, bool sorted, const LightSetup& light_setup)
	{
#if defined(SQUARE_DEBUG_TOOLS)
		//(a test: not sorted, or no batch)
		sorted = sorted && m_test == Test::NONE;
		const bool batching = m_test != Test::OFF;
#else
		const bool batching = true;
#endif
		//sorted: by the index of its pass, its light, its batch (its pass, the parameters of its
		//material), in a batch as they were added
		if (sorted)
		{
			std::sort(m_draws.begin(), m_draws.end(), [](const Draw& left, const Draw& right)
			{
				if (left.m_pass_index != right.m_pass_index)
				{
					return left.m_pass_index < right.m_pass_index;
				}
				if (left.m_light != right.m_light)
				{
					return std::less<const Light*>()(left.m_light, right.m_light);
				}
				if (left.m_pass != right.m_pass)
				{
					return std::less<const EffectPass*>()(left.m_pass, right.m_pass);
				}
				if (left.m_parameters != right.m_parameters)
				{
					return std::less<const EffectParameters*>()(left.m_parameters, right.m_parameters);
				}
				return left.m_order < right.m_order;
			});
		}
		UniformBufferTransform utransform;
		const EffectPass*       bound_pass = nullptr;
		const EffectParameters* bound_parameters = nullptr;
		const Light*            bound_light = nullptr;
		const Light*            light_set = nullptr;
		const Renderable*       transformed = nullptr;
		const uint32            layers = inputs.m_layer_mask;
		m_batches = 0;
		for (Draw& draw : m_draws)
		{
			//its transform (and its fade), once for its draws in a row
			if (draw.m_renderable.get() != transformed && inputs.m_transform)
			{
				if (auto transform = draw.m_renderable->transform().lock())
				{
					transform->set(&utransform);
					utransform.m_lod_fade = draw.m_fade;
					render.update_steam_CB(inputs.m_transform, (const unsigned char*)&utransform, sizeof(utransform));
				}
				transformed = draw.m_renderable.get();
			}
			//its light (its buffers, its shadow map: the pass bound again with it)
			if (draw.m_light && draw.m_light != light_set && light_setup)
			{
				light_setup(*draw.m_light);
				light_set = draw.m_light;
			}
			//its layers (the cascades of a caster)
			inputs.m_layer_mask = draw.m_layers;
			//in its batch, bound if it is not (its draw id, its layers: bound with it, else for it)
			bool drawn = false;
			if (batching)
			{
				const bool bound = bound_pass == draw.m_pass && bound_parameters == draw.m_parameters && bound_light == draw.m_light;
				if (!bound)
				{
					if (bound_pass)
					{
						bound_pass->unbind();
					}
					draw.m_pass->bind(render, inputs, draw.m_parameters, draw.m_draw_id);
					bound_pass = draw.m_pass;
					bound_parameters = draw.m_parameters;
					bound_light = draw.m_light;
					++m_batches;
				}
				else if (draw.m_pass->multi_pass())
				{
					draw.m_pass->bind_multi_pass(render, inputs, draw.m_draw_id);
				}
				drawn = draw.m_renderable->draw_bound(render, draw.m_material_id, inputs, *draw.m_pass, draw.m_draw_id);
			}
			//else by its own draw (it binds its pass)
			if (!drawn)
			{
				if (bound_pass)
				{
					bound_pass->unbind();
					bound_pass = nullptr;
					bound_parameters = nullptr;
					bound_light = nullptr;
				}
				draw.m_renderable->draw(render, draw.m_material_id, inputs, *draw.m_pass, draw.m_draw_id);
				++m_batches;
			}
		}
		if (bound_pass)
		{
			bound_pass->unbind();
		}
		inputs.m_layer_mask = layers;
		m_drawn = m_draws.size();
		m_draws.clear();
	}
}
}
