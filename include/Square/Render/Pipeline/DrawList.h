//
//  DrawList.h
//  Square
//
//  The draws of a pass in batches: each draw (a renderable, a material of it, a pass of the
//  technique of that material, a light of that pass) by its pass and the parameters of its
//  material; a batch (the same pass, the same parameters, the same light) bound once, its
//  renderables drawn with it (Renderable::draw_bound: their transform, their own buffers, their
//  geometry), the ones that cannot by their own draw. The cost of a draw then is its transform
//  and its geometry, not the states, the shader, the textures and the uniforms of its material
//  (the same for the renderables that share it: the tiles of a terrain, the copies of a prop).
//
//  Sorted (the opaque ones): by the index of the pass in its technique (the first passes of all
//  of them before the second ones: a pass that writes the depth before the ones that add a
//  light), its light (its buffers written once), its batch; in a batch as they were added (front
//  to back: the queue). Not sorted (the translucent ones, back to front): in their order, a batch
//  of the draws in a row that share it. The worst case (no draw shares a batch): a binding each,
//  as without the list.
//
//  Its memory: a list of a pool of the lists (big: pool_capacity draws at least), taken at its
//  first draw, given back to the pool when the list goes (a list of a scope made at every frame
//  makes no block); a list bigger than it needs given back lazily (its peak under a quarter of
//  it for trim_window draws in a row: halved, to pool_capacity at least).
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/Effect.h"
#include "Square/Render/MultiPass.h"

namespace Square
{
namespace Render
{
	class Context;
	class Light;
	class Renderable;

	class SQUARE_API DrawList
	{
	public:
		//the buffers of a light written (its uniforms, its shadow map in the inputs): before the
		//draws with it
		using LightSetup = std::function<void(Light& light)>;
		//the draws of a list of the pool (at least), the lists kept by the pool
		static constexpr size_t pool_capacity = 1024;
		static constexpr size_t pool_lists = 32;
		//the draws (draw) of a list bigger than it needs before it is halved
		static constexpr size_t trim_window = 600;

		DrawList() = default;
		//its list given back to the pool
		~DrawList();
		//its list moved (no copy: a list of the pool each)
		DrawList(DrawList&& other) noexcept;
		DrawList& operator=(DrawList&& other) noexcept;
		DrawList(const DrawList&) = delete;
		DrawList& operator=(const DrawList&) = delete;

		//no draw (the memory kept for the next frame)
		void clear();
		//the draws of a renderable: each material of it, the technique of its effect (by name: the
		//variant of the renderable, the clip one if it fades), each pass of it; none if it has not.
		//layers: the ones it is drawn in (a multi-pass one: its draws of the others not added; the
		//shader: the mask of the cascades of a caster); settle: drawn whole (no fade: a cache)
		void add
		(
			  const Shared<Renderable>& renderable
			, const String& technique_name
			, uint32 layers = MULTI_PASS_ALL_LAYERS
			, bool settle = false
		);
		//a draw of a pass (the index of the pass in its technique) of a material of a renderable,
		//with a light (nullptr: none)
		void add
		(
			  const Shared<Renderable>& renderable
			, size_t material_id
			, EffectPass& pass
			, size_t pass_index
			, EffectParameters* parameters
			, int draw_id
			, Light* light
		);
		//all of them in batches (sorted, else in their order), the transform of each renderable
		//(and the fade of its level of detail) into the constant buffer of the inputs before its
		//draws, the buffers of each light by light_setup; then none (no renderable kept)
		void draw
		(
			  Render::Context& render
			, EffectPassInputs& inputs
			, bool sorted = true
			, const LightSetup& light_setup = LightSetup()
		);
		//the draws, the batches (the bindings) of the last draw
		size_t draws() const { return m_drawn; }
		size_t batches() const { return m_batches; }
#if defined(SQUARE_DEBUG_TOOLS)
		//a test of its gain: the batches as they are, not sorted (a batch of the draws in a row
		//that share it), off (each draw by its own draw, as without the list); by the variable
		//of the environment of a pass ("ordered", "off")
		enum class Test { NONE, ORDERED, OFF };
		void test(const char* environment_variable);
#endif

	private:
		//a draw: its pass (its index in its technique), the parameters of its material, its light
		//(its batch), its renderable, its material, its layers, its fade, its order (as it was added)
		struct Draw
		{
			EffectPass*          m_pass{ nullptr };
			size_t               m_pass_index{ 0 };
			EffectParameters*    m_parameters{ nullptr };
			Light*               m_light{ nullptr };
			Shared<Renderable>   m_renderable;
			size_t               m_material_id{ 0 };
			int                  m_draw_id{ 0 };
			uint32               m_layers{ MULTI_PASS_ALL_LAYERS };
			float                m_fade{ 1.0f };
			size_t               m_order{ 0 };
		};
		struct Pool;

		//its list: of the pool (before its first draw)
		void reserve();
		//its list halved, lazily (its peak in the window under a quarter of it): after a draw
		void trim(size_t draws);

		Vector<Draw> m_draws;
		size_t            m_drawn{ 0 };
		size_t            m_batches{ 0 };
		size_t            m_peak{ 0 };       //the draws, at most, in the window
		size_t            m_window{ 0 };     //its draws (draw) in the window
#if defined(SQUARE_DEBUG_TOOLS)
		Test              m_test{ Test::NONE };
#endif
	};
}
}
