//
//  Decal.cpp
//  Square
//
//  See Decal.h.
//
#include "Square/Render/Decal.h"
#include "Square/Resource/Texture.h"

namespace Square
{
namespace Render
{
	void Decal::texture(const Shared<Resource::Texture>& texture) { m_texture = texture; }
	const Shared<Resource::Texture>& Decal::texture() const { return m_texture; }
	void Decal::color(const Vec4& color) { m_color = color; }
	const Vec4& Decal::color() const { return m_color; }
	void Decal::visible(bool visible) { m_visible = visible; }
	bool Decal::visible() const { return m_visible; }

	void Decal::set(UniformDecal* data) const
	{
		data->m_inverse = inverse(box());
		data->m_color = m_color;
	}
}
}
