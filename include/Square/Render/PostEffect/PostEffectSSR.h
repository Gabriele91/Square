//
//  PostEffectSSR.h
//  Square
//
//  Screen space reflections, a PES_COLOR post effect of the deferred pipeline (as in Unity and
//  Unreal: the G-Buffer gives position, normal and material of every pixel; in forward the
//  frame is copied as it is):
//   1) trace (half size by default, a quarter the cheapest): from the world position of each pixel smooth enough
//      (roughness under max_roughness) a ray reflected on its normal marches: on the screen
//      (screen_march, by default: steps of the same length in pixels, DDA) or in world space
//      (steps of the same length along the ray, each one projected on the screen); every step
//      is compared with the surface of the G-Buffer there (distance from the camera, both on
//      the same view ray): behind it within thickness is a hit, refined by bisection. Output: the color of the frame at the hit,
//      alpha the confidence (faded at the screen border, far along the ray, back faces out);
//   2) denoise (Settings::denoise): a bilateral blur of the trace, horizontal then vertical
//      (as the denoiser of HotBite): wider with the roughness, not over the edges (normals,
//      positions of the G-Buffer);
//   3) blur (Settings::blur medium, high): a chain of levels from the trace, each one half the
//      one before (13 samples, as the bloom), premultiplied by the confidence;
//   4) composite: the frame plus the reflection times the Fresnel of the material (F0 from
//      metallic and albedo; Legacy: its specular color), blurred by the roughness: 5 samples
//      of the trace (the noise of the rays out), on a rougher surface a wider level of the
//      chain (two near levels mixed).
//  What is not on the screen cannot be reflected (the fade hides where the ray leaves it).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"
#include <vector>

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
			float max_distance{ 50.0f };  //world units a ray travels at most
			int   steps{ 64 };            //steps of a ray (at most 128)
			float thickness{ 0.6f };      //world units behind a surface that are still a hit
			float max_roughness{ 0.6f };  //rougher surfaces reflect nothing (it fades before)
			float edge_fade{ 0.1f };      //share of the screen border where the reflection fades
			PostEffectResolution resolution{ PER_HALF }; //size of the trace (the composite is full size)
			bool  screen_march{ false };   //march on the screen (DDA), false: in world space
			//blur of the reflection by the roughness (the noise of the rays, the rough surfaces)
			enum BlurQuality : int
			{
				BLUR_OFF,    //the trace as it is: the noise of the rays is visible
				BLUR_LOW,    //5 samples, wider with the roughness (one pass, in the composite)
				BLUR_MEDIUM, //the 5 samples on a mirror, 3 levels of a chain on the rough surfaces
				BLUR_HIGH    //the 5 samples on a mirror, 5 levels: the widest, the smoothest
			};
			BlurQuality blur{ BLUR_MEDIUM };
			bool  denoise{ true };        //bilateral denoiser of the trace (two passes)
			float denoise_radius{ 8.0f }; //its kernel at most, pixels of the trace (the roughest surfaces, at most 16)
			int   debug{ 0 };             //1: only the reflection; 2: projection check (black ok, red wrong)
		};

		SSR(Square::Context& context);
		virtual ~SSR();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//it draws only with the deferred pipeline (its G-Buffer)
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_gbuffer && frame.m_camera; }
		virtual void draw(PostEffectFrame& frame) override;
		virtual void debug_options(std::vector<DebugOption>& options) override;

	protected:
		virtual void on_release() override;

	private:
		//a level of the blur chain
		struct Level
		{
			IVec2    m_size{ 0, 0 };
			Texture* m_texture{ nullptr };
			Target*  m_target{ nullptr };
		};
		static constexpr int s_max_blur_levels{ 5 };

		Settings                 m_settings;
		Shared<Resource::Shader> m_shader_trace;
		Shared<Resource::Shader> m_shader_denoise;
		Shared<Resource::Shader> m_shader_downsample;
		Shared<Resource::Shader> m_shader_composite;
		Shared<Resource::Shader> m_shader_copy;
		//the reflections: color of the hit, alpha the confidence
		Texture* m_trace_texture{ nullptr };
		Target*  m_trace_target{ nullptr };
		IVec2    m_trace_size{ 0, 0 };
		//the first pass of the denoiser (horizontal), of the trace size
		Texture* m_denoise_texture{ nullptr };
		Target*  m_denoise_target{ nullptr };
		//the blur chain (from the trace)
		std::vector<Level> m_levels;
		int                m_levels_count{ 0 }; //levels asked when it was built

		//the levels of the chain of a quality (0: no chain)
		static int blur_levels(Settings::BlurQuality blur);
		bool build_levels(const IVec2& trace_size);
		void release_levels();
		void denoise(const PostEffectFrame& frame, Texture* position, Texture* normal, const Vec4& params);
		void blur(const PostEffectFrame& frame);
		//the destination must be written (it is the next frame): the frame as it is
		void copy(PostEffectFrame& frame);
	};
}
}
