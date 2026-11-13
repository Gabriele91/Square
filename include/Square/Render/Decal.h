//
//  Decal.h
//  Square
//
//  A decal (as the decals of the other engines): a texture projected straight through a box on
//  what is in it (the ground, a slope, a step, a wall), its xz the plane of the texture, projected
//  along its -y; drawn by the deferred pass (DrawerPassDeferred: a volume around its box, the
//  G-Buffer read in it), its color added as it is (emissive: it glows, not lit), fading at the top
//  and at the bottom of its box. Its box: the unit box (-1 to 1) by its model matrix.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"
#include "Square/Render/ConstantBuffer.h"

namespace Square
{
	namespace Resource
	{
		class Texture;
	}
	namespace Geometry
	{
		class Sphere;
	}
}

namespace Square
{
namespace Render
{
	//its buffer: from the world to its box (-1 to 1), its color (rgb: as it is, HDR; a: how much)
	CBStruct UniformDecal
	{
		Mat4 m_inverse;
		Vec4 m_color;
	};

	class SQUARE_API Decal : public BaseObject
	{
	public:

		SQUARE_OBJECT(Decal)

		//its box in the world: the unit box (-1 to 1) to it
		virtual Mat4 box() const = 0;
		//a sphere around its box (the world)
		virtual const Geometry::Sphere& bounding_sphere() const = 0;

		//its texture (white: as its color; its alpha: how much of it), its color
		void texture(const Shared<Resource::Texture>& texture);
		const Shared<Resource::Texture>& texture() const;
		void color(const Vec4& color);
		const Vec4& color() const;
		//drawn or not
		void visible(bool visible);
		bool visible() const;

		//its buffer
		void set(UniformDecal* data) const;

	protected:

		Shared<Resource::Texture> m_texture;
		Vec4                      m_color{ 1.0f };
		bool                      m_visible{ true };
	};
}
}
