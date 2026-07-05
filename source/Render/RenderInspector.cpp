//
//  RenderInspector.cpp
//  Square
//
//  See RenderInspector.h: registry of the alive driver textures/render
//  targets, filled by the backends (TEXTURE_INTROSPECTION) through the
//  RenderInspector interface.
//
#include "Square/Config.h"
#include "Square/Render/RegistryInspector.h"

namespace Square
{
namespace Render
{
	void RegistryInspector::on_create_texture(Texture* texture, const TextureInfo& info)
	{
		if (texture) m_textures.push_back({ texture, info });
	}

	void RegistryInspector::on_delete_texture(const Texture* texture)
	{
		for (auto it = m_textures.begin(); it != m_textures.end(); ++it)
		{
			if (it->first == texture) { m_textures.erase(it); return; }
		}
	}

	void RegistryInspector::on_create_target(Target* target, const std::vector<TargetField>& fields)
	{
		if (target) m_targets.push_back({ target, TargetInfo{ fields } });
	}

	void RegistryInspector::on_delete_target(const Target* target)
	{
		for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
		{
			if (it->first == target) { m_targets.erase(it); return; }
		}
	}

	const RenderInspector::TextureRegistry& RegistryInspector::textures() const
	{
		return m_textures;
	}

	const RenderInspector::TargetRegistry& RegistryInspector::targets() const
	{
		return m_targets;
	}

	const TextureInfo* RegistryInspector::texture_info(const Texture* texture) const
	{
		for (auto& texture_it : m_textures)
		{
			if (texture_it.first == texture) return &texture_it.second;
		}
		return nullptr;
	}
}
}
