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
#include <mutex>
#include <new>
#include "Square/Driver/Render.h"
#include "Square/Core/Environment.h"
#include "Square/Render/Light.h"
#include "Square/Render/Material.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Pipeline/DrawList.h"

namespace Square
{
namespace Render
{
	//the lists of the lists gone (their memory kept: the next lists without a block)
	struct DrawList::Pool
	{
		std::mutex          m_mutex;
		Vector<Vector<Draw>> m_lists;

		Pool()
		{
			m_lists.reserve(pool_lists);
		}

		//never destroyed (the lists of the static objects given back after the end of main)
		static Pool& instance()
		{
			alignas(Pool) static unsigned char s_storage[sizeof(Pool)];
			static Pool* s_pool = new (s_storage) Pool();
			return *s_pool;
		}

		Vector<Draw> take()
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (!m_lists.empty())
				{
					Vector<Draw> list = std::move(m_lists.back());
					m_lists.pop_back();
					return list;
				}
			}
			Vector<Draw> list;
			list.reserve(pool_capacity);
			return list;
		}

		void give(Vector<Draw>&& list)
		{
			if (!list.capacity())
			{
				return;
			}
			list.clear();
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_lists.size() < pool_lists)
			{
				m_lists.push_back(std::move(list));
			}
		}
	};

	DrawList::~DrawList()
	{
		Pool::instance().give(std::move(m_draws));
	}

	DrawList::DrawList(DrawList&& other) noexcept
	: m_draws(std::move(other.m_draws))
	, m_drawn(other.m_drawn)
	, m_batches(other.m_batches)
	, m_peak(other.m_peak)
	, m_window(other.m_window)
#if defined(SQUARE_DEBUG_TOOLS)
	, m_test(other.m_test)
#endif
	{
	}

	DrawList& DrawList::operator=(DrawList&& other) noexcept
	{
		if (this != &other)
		{
			Pool::instance().give(std::move(m_draws));
			m_draws = std::move(other.m_draws);
			m_drawn = other.m_drawn;
			m_batches = other.m_batches;
			m_peak = other.m_peak;
			m_window = other.m_window;
		#if defined(SQUARE_DEBUG_TOOLS)
			m_test = other.m_test;
		#endif
		}
		return *this;
	}

	void DrawList::reserve()
	{
		if (!m_draws.capacity())
		{
			m_draws = Pool::instance().take();
		}
	}

	void DrawList::trim(size_t draws)
	{
		m_peak = std::max(m_peak, draws);
		if (++m_window < trim_window)
		{
			return;
		}
		//a quarter of it used at most in the window: halved (to the capacity of the pool at least)
		const size_t capacity = m_draws.capacity();
		if (capacity > pool_capacity && m_peak * 4 < capacity && m_draws.empty())
		{
			Vector<Draw> list;
			list.reserve(std::max(capacity / 2, pool_capacity));
			m_draws.swap(list);
		}
		m_peak = 0;
		m_window = 0;
	}

	void DrawList::clear()
	{
		m_draws.clear();
	}

	void DrawList::add
	(
		  const Shared<Renderable>& renderable
		, const String& technique_name
		, uint32 layers
		, bool settle
	)
	{
		reserve();
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
		reserve();
		m_draws.push_back(Draw{ &pass, pass_index, parameters, light, renderable, material_id, draw_id, MULTI_PASS_ALL_LAYERS, renderable->lod_fade(), m_draws.size() });
	}

#if defined(SQUARE_DEBUG_TOOLS)
	void DrawList::test(const char* environment_variable)
	{
		m_test = Test::NONE;
		if (Environment::is(environment_variable, "ordered"))
		{
			m_test = Test::ORDERED;
		}
		else if (Environment::is(environment_variable, "off"))
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
		trim(m_drawn);
	}
}
}
