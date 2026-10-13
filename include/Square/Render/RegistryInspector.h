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
#include "Square/Driver/RenderInspector.h"

namespace Square
{
namespace Render
{
	////////////////////////////////////////////////
	//registry: keeps every texture/render target currently alive on the driver
	class SQUARE_API RegistryInspector : public RenderInspector
	{
	public:

		//inspector interface
		virtual void on_create_texture(Texture* texture, const TextureInfo& info) override;
		virtual void on_delete_texture(const Texture* texture) override;
		virtual void on_create_target(Target* target, const std::vector<TargetField>& fields) override;
		virtual void on_delete_target(const Target* target) override;
		//registry access
		virtual const TextureRegistry& textures() const override;
		virtual const TargetRegistry& targets() const override;
		virtual const TextureInfo* texture_info(const Texture* texture) const override;

	private:
		TextureRegistry m_textures;
		TargetRegistry  m_targets;
	};
}
}
