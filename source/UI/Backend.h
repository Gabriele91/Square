//
//  Backend.h
//  Square
//
//  The backend of RmlUi in the engine (inside Square, not exposed): its render on
//  Render::Context (geometry in buffers, textures, scissor, transforms; one shader, UI.hlsl),
//  its time, its log and its files (by Filesystem: also in the archives, .sqz/.zip). Clip masks, layers and filters of RmlUi are not there (RCSS without
//  them).
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"
#include "Square/Driver/Render.h"

namespace Square
{
	class Context;
	namespace Resource
	{
		class Shader;
	}
}

namespace Square
{
namespace UI
{
	class Backend : public Rml::RenderInterface, public Rml::SystemInterface, public Rml::FileInterface
	{
	public:
		Backend(Square::Context& context);
		virtual ~Backend();

		//the shader, the white texture (false: no UI)
		bool initialize();
		//the GPU objects of RmlUi left (after Rml::Shutdown)
		void release();

		//a frame: the states of the UI on the window of size pixels, then the ones of the engine back
		void begin_frame(const IVec2& size);
		void end_frame();

		//the textures of the engine an image can show (its source: the texture, not owned by the
		//UI; none: nullptr, the source is a file)
		void external_textures(std::function<Render::Texture*(const std::string& source, IVec2& size)> textures) { m_external = textures; }

		//Rml::RenderInterface
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
		void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
		void ReleaseTexture(Rml::TextureHandle texture) override;
		void EnableScissorRegion(bool enable) override;
		void SetScissorRegion(Rml::Rectanglei region) override;
		void SetTransform(const Rml::Matrix4f* transform) override;

		//Rml::SystemInterface
		double GetElapsedTime() override;
		bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

		//Rml::FileInterface: a file read whole (on the disk or in an archive)
		Rml::FileHandle Open(const Rml::String& path) override;
		void Close(Rml::FileHandle file) override;
		size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
		bool Seek(Rml::FileHandle file, long offset, int origin) override;
		size_t Tell(Rml::FileHandle file) override;
		size_t Length(Rml::FileHandle file) override;

	protected:
		//a geometry: its buffers
		struct Geometry
		{
			Render::VertexBuffer* m_vertices{ nullptr };
			Render::IndexBuffer*  m_indices{ nullptr };
			unsigned int          m_count{ 0 };
		};
		//a texture of premultiplied RGBA pixels
		Render::Texture* create_texture(const unsigned char* rgba, int width, int height);
		Render::Context* render() const;

		Square::Context&         m_context;
		Shared<Resource::Shader> m_shader;
		Render::InputLayout*     m_layout{ nullptr };
		Render::Texture*         m_white{ nullptr };
		std::vector<Geometry*>   m_geometries;  //alive (released by release())
		std::vector<Render::Texture*> m_textures;
		std::function<Render::Texture*(const std::string& source, IVec2& size)> m_external;
		std::vector<Render::Texture*> m_external_textures; //given by m_external (not released)
		//the frame
		IVec2               m_size{ 0, 0 };
		Mat4                m_transform{ 1.0f };
		Render::ScissorState m_scissor;
		Render::State        m_engine_state;    //of the engine, back at end_frame
		Render::ScissorState m_engine_scissor;
		double               m_start_time{ 0.0 };
	};
}
}
