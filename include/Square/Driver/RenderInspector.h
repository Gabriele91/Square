//
//  RenderInspector.h
//  Square
//
//  Introspection of the driver resources (TEXTURE_INTROSPECTION).
//
//  RenderInspector is a pure interface: the render backends notify it about
//  texture/render target lifetime events through Context::inspector(), only
//  when TEXTURE_INTROSPECTION is enabled (debug builds by default), so a
//  release driver never pays for it. The backends call it via vtable and do
//  not link the Square library.
//
//  RegistryInspector is the Square-side implementation: it keeps the
//  list of the alive textures/render targets (with creation-time info), the
//  Application owns one and sets it on the render context; the debug passes
//  read it back to draw the texture panel.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Render
{
	////////////////////////////////////////////////
	//shape of a texture as it was created on the driver
	enum TextureShape : unsigned char
	{
		TS_TEXTURE_2D,
		TS_TEXTURE_ARRAY,
		TS_TEXTURE_CUBE,

		TS_INVALID = 0xFF
	};

	//creation-time description of a texture alive on the driver
	struct TextureInfo
	{
		TextureShape  m_shape { TS_INVALID };
		TextureFormat m_format{ TF_INVALID };
		unsigned int  m_width { 0 };
		unsigned int  m_height{ 0 };
		int           m_layers{ 1 }; //array layers (6 for a cube)

		bool is_depth() const
		{
			switch (m_format)
			{
			case TF_DEPTH16_STENCIL8:
			case TF_DEPTH24_STENCIL8:
			case TF_DEPTH32_STENCIL8:
			case TF_DEPTH_COMPONENT16:
			case TF_DEPTH_COMPONENT24:
			case TF_DEPTH_COMPONENT32: return true;
			default:                   return false;
			}
		}
	};

	//attachments of a render target alive on the driver
	struct TargetInfo
	{
		std::vector<TargetField> m_fields;
	};

	////////////////////////////////////////////////
	//interface: the backends notify the resource lifetime events
	class SQUARE_API RenderInspector
	{
	public:
		using TextureRegistry = std::vector< std::pair<Texture*, TextureInfo> >;
		using TargetRegistry  = std::vector< std::pair<Target*, TargetInfo> >;

		virtual ~RenderInspector() = default;

		virtual void on_create_texture(Texture* texture, const TextureInfo& info) = 0;
		virtual void on_delete_texture(const Texture* texture) = 0;
		virtual void on_create_target(Target* target, const std::vector<TargetField>& fields) = 0;
		virtual void on_delete_target(const Target* target) = 0;

		//registry access
		virtual const TextureRegistry& textures() const = 0;
		virtual const TargetRegistry& targets() const = 0;
		virtual const TextureInfo* texture_info(const Texture* texture) const = 0;
	};
}
}
