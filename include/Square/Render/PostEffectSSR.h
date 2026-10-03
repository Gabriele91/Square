//
//  PostEffectSSR.h
//  Square
//
//  Screen space reflections, a PES_COLOR post effect of the deferred pipeline (as in Unity and
//  Unreal: the G-Buffer gives position, normal and material of every pixel; in forward the
//  frame is copied as it is):
//   1) trace (half size by default): from the world position of each pixel smooth enough
//      (roughness under max_roughness) a ray reflected on its normal marches: on the screen
//      (screen_march, by default: steps of the same length in pixels, DDA) or in world space
//      (steps of the same length along the ray, each one projected on the screen); every step
//      is compared with the surface of the G-Buffer there (distance from the camera, both on
//      the same view ray): behind it within thickness is a hit, refined by bisection. Output: the color of the frame at the hit,
//      alpha the confidence (faded at the screen border, far along the ray, back faces out);
//   2) composite: the frame plus the reflection (blurred by the roughness) times the Fresnel
//      of the material (F0 from metallic and albedo; Legacy: its specular color).
//  What is not on the screen cannot be reflected (the fade hides where the ray leaves it).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API SSR : public PostEffect
	{
	public:
		SQUARE_OBJECT(SSR)

		struct Settings
		{
			float intensity{ 1.0f };      //strength of the reflection
			float max_distance{ 25.0f };  //world units a ray travels at most
			int   steps{ 48 };            //steps of a ray (at most 128)
			float thickness{ 0.6f };      //world units behind a surface that are still a hit
			float max_roughness{ 0.6f };  //rougher surfaces reflect nothing (it fades before)
			float edge_fade{ 0.1f };      //share of the screen border where the reflection fades
			bool  half_resolution{ true };//trace at half size (the composite is full size)
			bool  screen_march{ true };   //march on the screen (DDA), false: in world space
			int   debug{ 0 };             //1: only the reflection; 2: projection check (black ok, red wrong)
		};

		SSR(Square::Context& context);
		virtual ~SSR();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		virtual void draw(PostEffectFrame& frame) override;

	protected:
		virtual void on_release() override;

	private:
		Settings                 m_settings;
		Shared<Resource::Shader> m_shader_trace;
		Shared<Resource::Shader> m_shader_composite;
		Shared<Resource::Shader> m_shader_copy;
		//the reflections: color of the hit, alpha the confidence
		Texture* m_trace_texture{ nullptr };
		Target*  m_trace_target{ nullptr };
		IVec2    m_trace_size{ 0, 0 };

		//the destination must be written (it is the next frame): the frame as it is
		void copy(PostEffectFrame& frame);
	};
}
}
