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
        
		//static: its transform does not move (Transform::is_static: an actor "static"): kept in
		//the caches of the shadow maps of the lights that do not move; else drawn in them every
		//frame
		bool is_static() const;

		virtual bool visible() const { return m_visible; }

		virtual void visible(bool enable)  { m_visible = enable; }

		virtual bool can_draw() const { return visible() && lod_shown() && material().lock() && transform().lock(); }

		//its fade by its level of detail (a LevelOfDetail, Scene::LodGroup: the level it selects
		//for the camera drawn), apart from visible: 1 drawn, 0 not; t in (0, 1) fading in, drawn
		//on the pixels of a pattern of the screen under t; -t fading out, on the others (a level
		//fading in and one fading out: every pixel once). The shaders read it (Transform.hlsl)
		inline void  lod_fade(float fade) { m_lod_fade = fade; }
		inline float lod_fade() const { return m_lod_fade; }
		inline bool  lod_shown() const { return m_lod_fade != 0.0f; }

		//drawn instanced (many copies in a draw call: Scene::InstancedMesh): the passes take the
		//instanced variant of the techniques of its effects (Render::EV_INSTANCED)
		virtual bool instanced() const { return false; }

		//its vertices moved by joints (Scene::SkinnedMesh): the passes take the skinned variant of
		//the techniques of its effects (Render::EV_SKINNED)
		virtual bool skinned() const { return false; }

		//the variant of the techniques of its draws (Render::EffectVariant: instanced, skinned)
		unsigned char variant() const;

		//it can cast a shadow (its own flag, "cast_shadow" of its component; the converter: the
		//"square_shadow_cast" of its node): false, never drawn in the shadow maps
		inline void cast_shadow(bool cast) { m_cast_shadow = cast; }
		inline bool cast_shadow() const { return m_cast_shadow; }

		//it casts a shadow: its flag, and a material of it without "mask_shadow" or under 1 (every
		//one 1 or more: all its pixels out of the shadow maps, the sky, far decor, grass); else not
		//drawn there, not in the depth of the cascades of the sun
		bool casts_shadow() const;

		//motion blur of its own (a PostEffect that needs the velocity, Render::MotionBlur): its
		//motion on the screen written (deferred, after the G-Buffer), only by the ones on
		inline void motion_blur(bool enable) { m_motion_blur = enable; }
		inline bool motion_blur() const { return m_motion_blur; }
		//its geometry alone (no material, the shader bound by the caller): the velocity pass;
		//false if it cannot (nothing drawn)
		virtual bool draw_geometry(Render::Context& render) const { return false; }

	private:

		bool  m_visible{ true };
		float m_lod_fade{ 1.0f };
		bool  m_motion_blur{ false };
		bool  m_cast_shadow{ true };
	};
}
}
