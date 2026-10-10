//
//  TexturePreviews.h
//  Square
//
//  The thumbnails of the tab Textures of the debug panel: a few slots, each a small RGBA8 render
//  target where a texture of the driver (its registry: RenderInspector, TEXTURE_INTROSPECTION) is
//  drawn as it is (a 2D texture; the layers of an array, the faces of a cube side by side), in
//  the frame before the UI (UISystem::draw_overlay); a depth, a texture of one channel between
//  its smallest and its largest value, in colors (its value first drawn alone, then reduced to
//  them: DebugMinMax, 16 x 9 then 1 x 1). RmlUi shows a slot as an image
//  ("square-preview:<slot>": the backend gives it the texture of the slot).
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/RenderInspector.h"

namespace Square
{
	class Context;
namespace Resource
{
	class Shader;
}
namespace Render
{
	class Mesh;
}
namespace UI
{
	class TexturePreviews
	{
	public:
		//the slots, the size of each one (pixels)
		static constexpr size_t slots = 8;
		static constexpr int    width = 256;
		static constexpr int    height = 144;

		//a texture to show: it, how it was made, attached to a render target
		struct Item
		{
			Render::Texture*    m_texture{ nullptr };
			Render::TextureInfo m_info;
			bool                m_target{ false };
		};

		TexturePreviews(Square::Context& context);
		~TexturePreviews();

		//the source of the image of a slot; the slot of a source (false: not one of them)
		static std::string source(size_t slot);
		static bool slot_of(const std::string& source, size_t& slot);

		//the texture of a slot (made the first time)
		Render::Texture* texture(size_t slot);
		//a texture of its own (a slot, a buffer of the smallest and largest values): not shown
		bool owns(const Render::Texture* texture) const;

		//the textures of the slots (fewer: the others empty); drawn at the next draw
		void items(const std::vector<Item>& items);
		//drawn (false: not drawn, the slots as they are)
		void active(bool active) { m_active = active; }

		//each slot its texture (in the frame, before the UI)
		void draw(Render::Context& render);

	private:

		bool create(Render::Context& render);
		void draw_slot(Render::Context& render, size_t slot);
		//a texture drawn by its shader into the target bound (rect: where, in NDC)
		void draw_texture(Resource::Shader& shader, const Item& item, const Vec4& rect, float flip, bool normalized);
		//the smallest and the largest value of a texture of one value (into m_minmax)
		void min_max(Render::Context& render, Resource::Shader& shader, const Item& item);
		//a target of its own (a texture, its target), its size
		struct Buffer
		{
			Render::Texture* m_texture{ nullptr };
			Render::Target*  m_target{ nullptr };
		};
		static Buffer buffer(Render::Context& render, Render::TextureFormat format, int width, int height);
		static void release(Render::Context& render, Buffer& buffer);

		Square::Context&                 m_context;
		std::vector<Render::Texture*>    m_textures;
		std::vector<Render::Target*>     m_targets;
		std::vector<Item>                m_items;
		Shared<Resource::Shader>         m_shader_2D;
		Shared<Resource::Shader>         m_shader_array;
		Shared<Resource::Shader>         m_shader_cube;
		Shared<Resource::Shader>         m_shader_min_max;
		Buffer                           m_raw;    //a value alone (R32F, the size of a slot)
		Buffer                           m_reduce; //the smallest, the largest of 16 x 16 blocks (RG32F, 16 x 9)
		Buffer                           m_minmax; //of all of them (RG32F, 1 x 1)
		Shared<Render::Mesh>             m_quad;
		bool                             m_active{ false };
	};
}
}
