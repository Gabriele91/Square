//
//  Renderable .h
//  Square
//
//  Created by Gabriele Di Bari on 08/06/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
	namespace Render
	{
		class Context;
		class Material;
		class Transform;
		class EffectPass;
		struct EffectPassInputs;
	}
	namespace Geometry
	{
		class OBoundingBox;
	}
}

namespace Square
{
namespace Render
{
	class SQUARE_API Renderable : public BaseObject
	{
	public:

		SQUARE_OBJECT(Renderable)

		Renderable() {}

		virtual ~Renderable() {};

		virtual size_t materials_count() const = 0;

		virtual Weak<Material> material(size_t material_id = 0) const = 0;

		virtual void draw(
			  Render::Context& render
			, size_t material_id
			, EffectPassInputs& current_input
			, EffectPass& current_pass
			, int draw_id = 0   //multi-pass index (cube face / cascade); 0 = normal
		) = 0;

		virtual bool support_culling() const = 0;
        
		virtual const Geometry::OBoundingBox& bounding_box() = 0;

		virtual Weak<Transform> transform() const = 0;
        
		virtual bool visible() const { return m_visible; }

		virtual void visible(bool enable)  { m_visible = enable; }

		virtual bool can_draw() const { return visible() && m_lod_shown && material().lock() && transform().lock(); }

		//shown by its level of detail (a LevelOfDetail, Scene::LodGroup: on only in the level it
		//selects for the camera drawn), apart from visible
		inline void lod_shown(bool shown) { m_lod_shown = shown; }
		inline bool lod_shown() const { return m_lod_shown; }

		//drawn instanced (many copies in a draw call: Scene::InstancedMesh): the passes take the
		//"<technique>_instanced" techniques of its effects
		virtual bool instanced() const { return false; }

		//motion blur of its own (a PostEffect that needs the velocity, Render::MotionBlur): its
		//motion on the screen written (deferred, after the G-Buffer), only by the ones on
		inline void motion_blur(bool enable) { m_motion_blur = enable; }
		inline bool motion_blur() const { return m_motion_blur; }
		//its geometry alone (no material, the shader bound by the caller): the velocity pass;
		//false if it cannot (nothing drawn)
		virtual bool draw_geometry(Render::Context& render) const { return false; }

	private:
        
		bool m_visible{ true };
		bool m_lod_shown{ true };
		bool m_motion_blur{ false };
	};
}
}
