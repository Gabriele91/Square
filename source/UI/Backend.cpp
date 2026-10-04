//
//  Backend.cpp
//  Square
//
//  See Backend.h.
//
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Logger.h"
#include "Square/Core/Time.h"
#include "Square/Data/Image.h"
#include "Square/Resource/Shader.h"
#include "Square/System/RenderSystem.h"
#include "Square/Render/Profiler.h"
#include "Backend.h"

namespace Square
{
namespace UI
{
	//the shader of the UI
	static const char* s_shader_source =
	#include "UI.hlsl"
	;

	//a vertex of the geometry (the colour of RmlUi as floats)
	struct UIVertex
	{
		Vec2 m_position;
		Vec4 m_color;
		Vec2 m_uv;
	};

	Backend::Backend(Square::Context& context) : m_context(context)
	{
	}

	Backend::~Backend()
	{
		release();
	}

	Render::Context* Backend::render() const
	{
		auto* render_system = System::get<RenderSystem>(m_context);
		return render_system ? render_system->render() : nullptr;
	}

	bool Backend::initialize()
	{
		auto* render = this->render();
		if (!render) return false;
		m_start_time = Time::get_time();
		//shader and layout of the vertices
		m_shader = MakeShared<Resource::Shader>(m_context);
		m_shader->profile_name("UI");
		if (!m_shader->compile(s_shader_source, {}))
		{
			m_shader.reset();
			return false;
		}
		m_layout = render->create_IL(Render::AttributeList
		({
			{ Render::ATT_POSITION0, Render::AST_FLOAT2, offsetof(UIVertex, m_position) },
			{ Render::ATT_COLOR0,    Render::AST_FLOAT4, offsetof(UIVertex, m_color) },
			{ Render::ATT_TEXCOORD0, Render::AST_FLOAT2, offsetof(UIVertex, m_uv) }
		}));
		//the texture of the geometry without one
		const unsigned char white[4] = { 255, 255, 255, 255 };
		m_white = create_texture(white, 1, 1);
		return m_layout && m_white;
	}

	void Backend::release()
	{
		auto* render = this->render();
		if (render)
		{
			for (Geometry* geometry : m_geometries)
			{
				if (geometry->m_vertices) render->delete_VBO(geometry->m_vertices);
				if (geometry->m_indices) render->delete_IBO(geometry->m_indices);
			}
			for (Render::Texture* texture : m_textures) render->delete_texture(texture);
			if (m_white) render->delete_texture(m_white);
			if (m_layout) render->delete_IL(m_layout);
		}
		for (Geometry* geometry : m_geometries) delete geometry;
		m_geometries.clear();
		m_textures.clear();
		m_white = nullptr;
		m_layout = nullptr;
		m_shader.reset();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//frame
	void Backend::begin_frame(const IVec2& size)
	{
		auto* render = this->render();
		if (!render || !m_shader) return;
		m_size = size;
		m_transform = Mat4(1.0f);
		//the states of the engine, back at the end
		m_engine_state = render->get_render_state();
		m_engine_scissor = render->get_scissor_state();
		//the window, no depth, no cull, premultiplied alpha
		render->set_viewport_state({ Vec4(0.0f, 0.0f, float(size.x), float(size.y)) });
		render->set_depth_buffer_state({ Render::DM_DISABLE });
		render->set_cullface_state({ Render::CF_DISABLE });
		render->set_blend_state(Render::BlendState(Render::BLEND_ONE, Render::BLEND_ONE_MINUS_SRC_ALPHA));
		m_scissor = Render::ScissorState();
		render->set_scissor_state(m_scissor);
		m_shader->bind();
		if (auto u = m_shader->uniform("ui_size"))      u->set(Vec2(float(size.x), float(size.y)));
		if (auto u = m_shader->uniform("ui_transform")) u->set(m_transform);
	}

	void Backend::end_frame()
	{
		auto* render = this->render();
		if (!render || !m_shader) return;
		m_shader->unbind();
		render->set_scissor_state(m_engine_scissor);
		render->set_render_state(m_engine_state);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//geometry
	Rml::CompiledGeometryHandle Backend::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
	{
		auto* render = this->render();
		if (!render || vertices.empty() || indices.empty()) return 0;
		std::vector<UIVertex> ui_vertices(vertices.size());
		for (size_t i = 0; i != vertices.size(); ++i)
		{
			const Rml::Vertex& vertex = vertices[i];
			ui_vertices[i].m_position = Vec2(vertex.position.x, vertex.position.y);
			ui_vertices[i].m_color = Vec4(vertex.colour.red, vertex.colour.green, vertex.colour.blue, vertex.colour.alpha) / 255.0f;
			ui_vertices[i].m_uv = Vec2(vertex.tex_coord.x, vertex.tex_coord.y);
		}
		auto* geometry = new Geometry();
		geometry->m_vertices = render->create_VBO((const unsigned char*)ui_vertices.data(), sizeof(UIVertex), ui_vertices.size());
		geometry->m_indices = render->create_IBO((const unsigned int*)indices.data(), indices.size());
		geometry->m_count = (unsigned int)indices.size();
		m_geometries.push_back(geometry);
		return (Rml::CompiledGeometryHandle)geometry;
	}

	void Backend::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
	{
		auto* render = this->render();
		auto* geometry = (Geometry*)handle;
		if (!render || !geometry || !m_shader || !m_layout) return;
		auto* geometry_texture = texture ? (Render::Texture*)texture : m_white;
		if (auto u = m_shader->uniform("ui_translation")) u->set(Vec2(translation.x, translation.y));
		if (auto u = m_shader->uniform("ui_texture"))     u->set(geometry_texture);
		render->bind_VBO(geometry->m_vertices);
		render->bind_IBO(geometry->m_indices);
		render->bind_IL(m_layout);
		{
			SQUARE_RENDER_DRAW(*render);
			render->draw_elements(Render::DRAW_TRIANGLES, 0, geometry->m_count);
		}
		render->unbind_IL(m_layout);
		render->unbind_IBO(geometry->m_indices);
		render->unbind_VBO(geometry->m_vertices);
		//unbound: the drivers skip a texture they see bound, and RmlUi makes and releases its
		//textures (a new one can have the address of a released one)
		render->unbind_texture(geometry_texture);
	}

	void Backend::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
	{
		auto* geometry = (Geometry*)handle;
		if (!geometry) return;
		if (auto* render = this->render())
		{
			if (geometry->m_vertices) render->delete_VBO(geometry->m_vertices);
			if (geometry->m_indices) render->delete_IBO(geometry->m_indices);
		}
		m_geometries.erase(std::remove(m_geometries.begin(), m_geometries.end(), geometry), m_geometries.end());
		delete geometry;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//textures
	Render::Texture* Backend::create_texture(const unsigned char* rgba, int width, int height)
	{
		auto* render = this->render();
		if (!render || !rgba || width <= 0 || height <= 0) return nullptr;
		return render->create_texture
		(
			{ Render::TF_RGBA8, (unsigned int)width, (unsigned int)height, rgba, Render::TT_RGBA, Render::TTF_UNSIGNED_BYTE, false },
			{ Render::TMIN_LINEAR, Render::TMAG_LINEAR, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, Render::TEDGE_CLAMP, false, 0, 1 }
		);
	}

	Rml::TextureHandle Backend::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		//the image (PNG, JPEG, BMP, TGA) to premultiplied RGBA
		std::vector<unsigned char> file = Filesystem::binary_file_read_all(source);
		std::vector<unsigned char> pixels;
		unsigned long width = 0, height = 0;
		Render::TextureFormat format;
		Render::TextureType type;
		if (file.empty() || !Data::Image::load(file, pixels, width, height, format, type)) return 0;
		size_t channels = 0;
		switch (format)
		{
		case Render::TF_R8:    channels = 1; break;
		case Render::TF_RG8:   channels = 2; break;
		case Render::TF_RGB8:  channels = 3; break;
		case Render::TF_RGBA8: channels = 4; break;
		default: return 0;
		}
		std::vector<unsigned char> rgba(size_t(width) * height * 4);
		for (size_t i = 0, count = size_t(width) * height; i != count; ++i)
		{
			const unsigned char* in = &pixels[i * channels];
			const unsigned int alpha = channels == 4 ? in[3] : channels == 2 ? in[1] : 255;
			const unsigned int red = in[0];
			const unsigned int green = channels >= 3 ? in[1] : in[0];
			const unsigned int blue = channels >= 3 ? in[2] : in[0];
			rgba[i * 4 + 0] = (unsigned char)(red * alpha / 255);
			rgba[i * 4 + 1] = (unsigned char)(green * alpha / 255);
			rgba[i * 4 + 2] = (unsigned char)(blue * alpha / 255);
			rgba[i * 4 + 3] = (unsigned char)alpha;
		}
		Render::Texture* texture = create_texture(rgba.data(), int(width), int(height));
		if (!texture) return 0;
		m_textures.push_back(texture);
		texture_dimensions = Rml::Vector2i(int(width), int(height));
		return (Rml::TextureHandle)texture;
	}

	Rml::TextureHandle Backend::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
	{
		//already premultiplied RGBA
		Render::Texture* texture = create_texture(source.data(), source_dimensions.x, source_dimensions.y);
		if (!texture) return 0;
		m_textures.push_back(texture);
		return (Rml::TextureHandle)texture;
	}

	void Backend::ReleaseTexture(Rml::TextureHandle handle)
	{
		auto* texture = (Render::Texture*)handle;
		if (!texture) return;
		m_textures.erase(std::remove(m_textures.begin(), m_textures.end(), texture), m_textures.end());
		if (auto* render = this->render()) render->delete_texture(texture);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//scissor and transform
	void Backend::EnableScissorRegion(bool enable)
	{
		m_scissor.m_enable = enable;
		if (auto* render = this->render()) render->set_scissor_state(m_scissor);
	}

	void Backend::SetScissorRegion(Rml::Rectanglei region)
	{
		m_scissor.m_rect = IVec4(region.Left(), region.Top(), region.Width(), region.Height());
		if (auto* render = this->render()) render->set_scissor_state(m_scissor);
	}

	void Backend::SetTransform(const Rml::Matrix4f* transform)
	{
		//column major as glm: the same layout
		m_transform = Mat4(1.0f);
		if (transform) std::memcpy(&m_transform[0][0], transform->data(), sizeof(float) * 16);
		if (m_shader)
		{
			if (auto u = m_shader->uniform("ui_transform")) u->set(m_transform);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//system
	double Backend::GetElapsedTime()
	{
		return Time::get_time() - m_start_time;
	}

	bool Backend::LogMessage(Rml::Log::Type type, const Rml::String& message)
	{
		switch (type)
		{
		case Rml::Log::LT_ERROR:
		case Rml::Log::LT_ASSERT:  m_context.logger()->error("UI: " + message); break;
		case Rml::Log::LT_WARNING: m_context.logger()->warning("UI: " + message); break;
		case Rml::Log::LT_INFO:    m_context.logger()->info("UI: " + message); break;
		default:                   m_context.logger()->debug("UI: " + message); break;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//files
	struct BackendFile
	{
		std::vector<unsigned char> m_bytes;
		size_t                     m_position{ 0 };
	};

	Rml::FileHandle Backend::Open(const Rml::String& path)
	{
		if (!Filesystem::is_file(path)) return 0;
		auto* file = new BackendFile();
		file->m_bytes = Filesystem::binary_file_read_all(path);
		return (Rml::FileHandle)file;
	}

	void Backend::Close(Rml::FileHandle handle)
	{
		delete (BackendFile*)handle;
	}

	size_t Backend::Read(void* buffer, size_t size, Rml::FileHandle handle)
	{
		auto* file = (BackendFile*)handle;
		if (!file) return 0;
		const size_t count = std::min(size, file->m_bytes.size() - file->m_position);
		if (count) std::memcpy(buffer, file->m_bytes.data() + file->m_position, count);
		file->m_position += count;
		return count;
	}

	bool Backend::Seek(Rml::FileHandle handle, long offset, int origin)
	{
		auto* file = (BackendFile*)handle;
		if (!file) return false;
		long long base = 0;
		switch (origin)
		{
		case SEEK_SET: base = 0; break;
		case SEEK_CUR: base = (long long)file->m_position; break;
		case SEEK_END: base = (long long)file->m_bytes.size(); break;
		default: return false;
		}
		const long long position = base + offset;
		if (position < 0 || position > (long long)file->m_bytes.size()) return false;
		file->m_position = size_t(position);
		return true;
	}

	size_t Backend::Tell(Rml::FileHandle handle)
	{
		auto* file = (BackendFile*)handle;
		return file ? file->m_position : 0;
	}

	size_t Backend::Length(Rml::FileHandle handle)
	{
		auto* file = (BackendFile*)handle;
		return file ? file->m_bytes.size() : 0;
	}
}
}
