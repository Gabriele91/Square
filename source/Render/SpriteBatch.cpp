//
//  SpriteBatch.cpp
//  Square
//
//  See SpriteBatch.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/ConstantBuffer.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/BasicMesh.h"
#include "Square/Render/Effect.h"
#include "Square/Resource/Shader.h"
#include "Square/Render/SpriteBatch.h"

namespace Square
{
namespace Render
{
	SpriteBatch::SpriteBatch(Square::Context& context)
	: m_context(context)
	{
	}

	void SpriteBatch::draw
	(
		  Render::Context& render
		, EffectPass& pass
		, EffectPassInputs& inputs
		, EffectParameters* parameters
		, int draw_id
		, const SpriteInstance* sprites
		, size_t count
	)
	{
		//its quad, its buffer (made once: the size of Sprite.hlsl)
		if (!m_quad)
		{
			m_quad = BasicMesh::build_quad(m_context);
		}
		if (!m_buffer)
		{
			m_buffer = Render::stream_constant_buffer(&render, sizeof(SpriteInstance) * sprites_max);
		}
		if (m_quad && m_buffer && sprites && count)
		{
			//in batches: their data, a draw each
			for (size_t first = 0; first < count; first += sprites_max)
			{
				const size_t batch = std::min(sprites_max, count - first);
				pass.bind(render, inputs, parameters, draw_id);
				if (auto uniform = pass.m_shader->constant_buffer("Sprites"))
				{
					render.update_steam_CB(m_buffer.get(), (const unsigned char*)(sprites + first), sizeof(SpriteInstance) * batch);
					uniform->bind(m_buffer.get());
					m_quad->draw(render, (unsigned int)batch);
				}
				pass.unbind();
			}
		}
	}
}
}
