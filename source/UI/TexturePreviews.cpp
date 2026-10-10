//
//  TexturePreviews.cpp
//  Square
//
//  See TexturePreviews.h for the high level description.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
#include "Square/Resource/Shader.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/BasicMesh.h"
#include "Square/Math/Linear.h"
#include "TexturePreviews.h"

namespace Square
{
namespace UI
{
	namespace AuxTexturePreviews
	{
		//the render device (none: no render system)
		static Render::Context* render_of(Square::Context& context)
		{
			auto* render_system = System::get<RenderSystem>(context);
			return render_system ? render_system->render() : nullptr;
		}

		//the prefix of the source of a slot (RmlUi may join it to the path of its document)
		static const char* s_prefix = "square-preview:";
	}

	TexturePreviews::TexturePreviews(Square::Context& context)
	: m_context(context)
	{
	}

	TexturePreviews::~TexturePreviews()
	{
		if (Render::Context* render = AuxTexturePreviews::render_of(m_context))
		{
			for (Render::Target*& target : m_targets) if (target) render->delete_render_target(target);
			for (Render::Texture*& texture : m_textures) if (texture) render->delete_texture(texture);
			release(*render, m_raw);
			release(*render, m_reduce);
			release(*render, m_minmax);
		}
	}

	TexturePreviews::Buffer TexturePreviews::buffer(Render::Context& render, Render::TextureFormat format, int width, int height)
	{
		Buffer made;
		const Render::TextureType type = format == Render::TF_R32F ? Render::TT_R : Render::TT_RG;
		made.m_texture = render.create_texture
		(
			{ format, (unsigned int)width, (unsigned int)height, nullptr, type, Render::TTF_FLOAT, false },
			{ Render::TMIN_NEAREST, Render::TMAG_NEAREST, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP }
		);
		made.m_target = made.m_texture ? render.create_render_target({ Render::TargetField{ made.m_texture, Render::RT_COLOR } }) : nullptr;
		return made;
	}

	void TexturePreviews::release(Render::Context& render, Buffer& buffer)
	{
		if (buffer.m_target) render.delete_render_target(buffer.m_target);
		if (buffer.m_texture) render.delete_texture(buffer.m_texture);
	}

	std::string TexturePreviews::source(size_t slot)
	{
		return AuxTexturePreviews::s_prefix + std::to_string(slot);
	}

	bool TexturePreviews::slot_of(const std::string& source, size_t& slot)
	{
		const std::string prefix = AuxTexturePreviews::s_prefix;
		const size_t at = source.rfind(prefix);
		bool found = at != std::string::npos && at + prefix.size() < source.size();
		if (found)
		{
			slot = size_t(std::atoi(source.c_str() + at + prefix.size()));
			found = slot < slots;
		}
		return found;
	}

	bool TexturePreviews::create(Render::Context& render)
	{
		if (m_textures.empty())
		{
			//dark, until something is drawn in them
			const std::vector<unsigned char> dark(size_t(width) * size_t(height) * 4, 20);
			for (size_t i = 0; i != slots; ++i)
			{
				Render::Texture* texture = render.create_texture
				(
					{ Render::TF_RGBA8, (unsigned int)width, (unsigned int)height, dark.data(), Render::TT_RGBA, Render::TTF_UNSIGNED_BYTE, false },
					{ Render::TMIN_LINEAR, Render::TMAG_LINEAR, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP }
				);
				m_textures.push_back(texture);
				m_targets.push_back(texture ? render.create_render_target({ Render::TargetField{ texture, Render::RT_COLOR } }) : nullptr);
			}
			m_shader_2D = m_context.resource<Resource::Shader>("DebugTexture2D");
			m_shader_array = m_context.resource<Resource::Shader>("DebugTextureArray");
			m_shader_cube = m_context.resource<Resource::Shader>("DebugTextureCube");
			m_shader_min_max = m_context.resource<Resource::Shader>("DebugMinMax");
			m_raw = buffer(render, Render::TF_R32F, width, height);
			m_reduce = buffer(render, Render::TF_RG32F, 16, 9);
			m_minmax = buffer(render, Render::TF_RG32F, 1, 1);
			m_quad = Render::BasicMesh::build_quad(m_context);
		}
		return m_quad && m_shader_2D;
	}

	Render::Texture* TexturePreviews::texture(size_t slot)
	{
		Render::Texture* texture = nullptr;
		Render::Context* render = AuxTexturePreviews::render_of(m_context);
		if (render && create(*render) && slot < m_textures.size())
		{
			texture = m_textures[slot];
		}
		return texture;
	}

	bool TexturePreviews::owns(const Render::Texture* texture) const
	{
		const bool slot = std::find(m_textures.begin(), m_textures.end(), texture) != m_textures.end();
		return slot || texture == m_raw.m_texture || texture == m_reduce.m_texture || texture == m_minmax.m_texture;
	}

	void TexturePreviews::items(const std::vector<Item>& items)
	{
		m_items = items;
	}

	void TexturePreviews::draw(Render::Context& render)
	{
		if (m_active && create(render))
		{
			//the state of the engine, back after
			const Render::ViewportState viewport = render.get_viewport_state();
			const Render::DepthBufferState depth = render.get_depth_buffer_state();
			const Render::BlendState blend = render.get_blend_state();
			const Render::CullfaceState cullface = render.get_cullface_state();
			const Render::ClearColorState clear = render.get_clear_color_state();
			render.set_depth_buffer_state({ Render::DM_DISABLE });
			render.set_blend_state({});
			render.set_cullface_state({ Render::CF_DISABLE });
			render.set_clear_color_state({ Vec4(0.08f, 0.08f, 0.1f, 1.0f) });
			for (size_t slot = 0; slot != m_targets.size(); ++slot)
			{
				draw_slot(render, slot);
			}
			render.set_viewport_state(viewport);
			render.set_depth_buffer_state(depth);
			render.set_blend_state(blend);
			render.set_cullface_state(cullface);
			render.set_clear_color_state(clear);
		}
	}

	void TexturePreviews::draw_texture(Resource::Shader& shader, const Item& item, const Vec4& rect, float flip, bool normalized)
	{
		Render::Context* render = AuxTexturePreviews::render_of(m_context);
		if (render && m_quad)
		{
			shader.bind();
			if (auto uniform = shader.uniform("g_texture")) uniform->set(item.m_texture);
			//(always one: a texture not bound is an error on some drivers)
			if (auto uniform = shader.uniform("g_minmax")) uniform->set(m_minmax.m_texture ? m_minmax.m_texture : item.m_texture);
			if (auto uniform = shader.uniform("rect")) uniform->set(rect);
			if (auto uniform = shader.uniform("params")) uniform->set(Vec4(item.m_info.is_depth() || normalized ? 1.0f : 0.0f, flip, float(item.m_info.m_layers), normalized ? 1.0f : 0.0f));
			m_quad->draw(*render);
			shader.unbind();
		}
	}

	void TexturePreviews::min_max(Render::Context& render, Resource::Shader& shader, const Item& item)
	{
		//its value alone, the whole buffer (no border: it would count)
		render.enable_render_target(m_raw.m_target);
		render.set_viewport_state({ Vec4(0.0f, 0.0f, float(width), float(height)) });
		draw_texture(shader, item, Vec4(-1.0f, -1.0f, 2.0f, 2.0f), 0.0f, false);
		render.disable_render_target(m_raw.m_target);
		//reduced: blocks of 16 x 16, then all of them
		Resource::Shader& reduce = *m_shader_min_max;
		auto pass = [&](const Buffer& source, const Buffer& target, const IVec2& source_size, const IVec2& target_size, bool one_value)
		{
			render.enable_render_target(target.m_target);
			render.set_viewport_state({ Vec4(0.0f, 0.0f, float(target_size.x), float(target_size.y)) });
			reduce.bind();
			if (auto uniform = reduce.uniform("g_texture")) uniform->set(source.m_texture);
			if (auto uniform = reduce.uniform("reduce")) uniform->set(Vec4(16.0f, 16.0f, float(source_size.x), float(source_size.y)));
			if (auto uniform = reduce.uniform("params")) uniform->set(Vec4(one_value ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f));
			m_quad->draw(render);
			reduce.unbind();
			render.disable_render_target(target.m_target);
		};
		pass(m_raw, m_reduce, IVec2(width, height), IVec2(16, 9), true);
		pass(m_reduce, m_minmax, IVec2(16, 9), IVec2(1, 1), false);
	}

	void TexturePreviews::draw_slot(Render::Context& render, size_t slot)
	{
		Render::Target* target = m_targets[slot];
		if (target)
		{
			render.enable_render_target(target);
			render.set_viewport_state({ Vec4(0.0f, 0.0f, float(width), float(height)) });
			render.clear(Render::CLEAR_COLOR);
			//its texture: still alive (the registry of the driver), by its shape
			const Item* item = slot < m_items.size() ? &m_items[slot] : nullptr;
			Render::RenderInspector* inspector = render.inspector();
			const bool alive = item && item->m_texture && inspector && inspector->texture_info(item->m_texture);
			Resource::Shader* shader = nullptr;
			if (alive)
			{
				switch (item->m_info.m_shape)
				{
				case Render::TS_TEXTURE_2D:    shader = m_shader_2D.get(); break;
				case Render::TS_TEXTURE_ARRAY: shader = m_shader_array.get(); break;
				case Render::TS_TEXTURE_CUBE:  shader = m_shader_cube.get(); break;
				default: break;
				}
			}
			if (shader && shader->base_shader())
			{
				//its aspect kept (the layers, the faces side by side), in the middle of the slot
				const Render::TextureInfo& info = item->m_info;
				const int columns = info.m_shape == Render::TS_TEXTURE_2D ? 1 : std::max(1, info.m_layers);
				const float aspect = info.m_width ? float(info.m_height) / float(info.m_width * columns) : 1.0f;
				const float slot_aspect = float(height) / float(width);
				const Vec2 size = aspect > slot_aspect ? Vec2(slot_aspect / aspect, 1.0f) : Vec2(1.0f, aspect / slot_aspect);
				const Vec4 rect(-size.x, -size.y, size.x * 2.0f, size.y * 2.0f);
				//its rows: the slot is shown as an image (row 0 on top); on GL a render target has
				//row 0 at the bottom, this slot too
				const Render::RenderDriver driver = render.get_render_driver();
				const bool gl = driver == Render::DR_OPENGL || driver == Render::DR_OPENGL_ES;
				const float flip = gl ? (item->m_target ? 1.0f : 0.0f) : 1.0f;
				//one value (a depth, a channel): between its smallest and its largest, in colors
				const bool one_value = info.is_depth() || info.m_format == Render::TF_R8 || info.m_format == Render::TF_R16F || info.m_format == Render::TF_R32F;
				const bool normalized = one_value && m_shader_min_max && m_shader_min_max->base_shader() && m_raw.m_target && m_reduce.m_target && m_minmax.m_target;
				if (normalized)
				{
					render.disable_render_target(target);
					min_max(render, *shader, *item);
					render.enable_render_target(target);
					render.set_viewport_state({ Vec4(0.0f, 0.0f, float(width), float(height)) });
				}
				draw_texture(*shader, *item, rect, flip, normalized);
			}
			render.disable_render_target(target);
		}
	}
}
}
