//
//  SpriteBatch.h
//  Square
//
//  Sprites drawn by a pass of a sprite effect (Sprite.sqfx, SpriteAdditive.sqfx: Sprite.hlsl):
//  a quad, an instance a sprite (its center in the space of its actor and its rotation, its size
//  and the frame of its flipbook, its color), at most sprites_max a draw call (more: more of
//  them). Scene::Sprite draws one, Scene::ParticleEmitter its particles.
//
#pragma once
#include <vector>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"
#include "Square/Render/Effect.h"

namespace Square
{
	class Context;
namespace Render
{
	class ConstBuffer;
	class Mesh;

	//a sprite: where (the space of its actor), its rotation (radians, around the view), its size,
	//the frame of its flipbook (< 0: by the time; w 1: z a share of the frames, the life of a
	//particle), its color
	struct SpriteInstance
	{
		Vec4 m_center_rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
		Vec4 m_size_frame{ 1.0f, 1.0f, -1.0f, 0.0f };
		Vec4 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	class SQUARE_API SpriteBatch
	{
	public:
		//the sprites of a draw call at most (Sprite.hlsl: SPRITES_MAX)
		static constexpr size_t sprites_max = 256;

		SpriteBatch(Square::Context& context);

		//the sprites (count of them, the first ones of an array of their owner) by a pass of a
		//sprite effect, its parameters (of a material)
		void draw
		(
			  Render::Context& render
			, EffectPass& pass
			, EffectPassInputs& inputs
			, EffectParameters* parameters
			, int draw_id
			, const SpriteInstance* sprites
			, size_t count
		);

	private:

		Square::Context&             m_context;
		Shared<Render::ConstBuffer>  m_buffer;
		Shared<Render::Mesh>         m_quad;
	};
}
}
