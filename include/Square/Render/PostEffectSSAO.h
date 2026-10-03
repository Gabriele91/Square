//
//  PostEffectSSAO.h
//  Square
//
//  Screen space ambient occlusion, a PES_GBUFFER post effect (deferred): from the world
//  positions and normals of the G-Buffer it estimates how much of the hemisphere of each
//  pixel is hidden by the geometry around it (Alchemy AO: samples in screen space within a
//  radius in world units, nearer occluders count more), blurs it keeping the edges, and
//  multiplies the occlusion of the G-Buffer: only the ambient light is darkened.
//  Passes, at half size by default (the last one is full size):
//   1) depth: the view depth of the G-Buffer in a R32F texture (4 bytes a sample instead of
//      the 16 of the positions; the positions are rebuilt from it and the view rays);
//   2) occlusion (R8);
//   3) blur (Settings::blur): off, 2x2, 4x4 or a separable bilateral gaussian (two passes);
//   4) apply: up to the frame size (bilateral: by the depth) on the G-Buffer occlusion.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API SSAO : public PostEffect
	{
	public:
		SQUARE_OBJECT(SSAO)

		struct Settings
		{
			float radius{ 1.0f };     //world units around the pixel where the occluders are searched
			float intensity{ 1.0f };  //strength of the occlusion
			float bias{ 0.01f };      //world units: nearly flat occluders do not count (self occlusion)
			float contrast{ 1.15f };  //exponent of the result: darker creases
			float max_pixels{ 64.0f }; //radius on the screen at most (pixels of the frame), near the camera
			bool  half_resolution{ true }; //occlusion at half size (the apply is full size)
			//blur of the occlusion (the noise of the samples), all keeping the edges
			enum BlurQuality : int
			{
				BLUR_OFF,      //raw: the 4x4 pattern of the samples is visible
				BLUR_VERY_LOW, //2x2 box, one pass (the pattern is still a little visible)
				BLUR_LOW,      //4x4 box (removes the pattern), one pass
				BLUR_MEDIUM, //gaussian, radius 3, two passes
				BLUR_HIGH    //gaussian, radius 6, two passes: the smoothest
			};
			BlurQuality blur{ BLUR_MEDIUM };
			float blur_sharpness{ 32.0f }; //how much the blur keeps the edges (depth differences)
			bool  debug{ false };     //debug view: the occlusion on the screen (white open, black closed)
		};

		SSAO(Square::Context& context);
		virtual ~SSAO();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		virtual void draw(PostEffectFrame& frame) override;
		virtual Texture* debug_texture() const override;

	protected:
		virtual void on_release() override;

	private:
		Settings                 m_settings;
		Shared<Resource::Shader> m_shader_depth;
		Shared<Resource::Shader> m_shader_ssao;
		Shared<Resource::Shader> m_shader_blur;
		Shared<Resource::Shader> m_shader_apply;
		//view depth (R32F), occlusion and blur (R8, ping pong), of the occlusion size
		Texture* m_depth_texture{ nullptr };
		Target*  m_depth_target{ nullptr };
		Texture* m_ao_texture{ nullptr };
		Target*  m_ao_target{ nullptr };
		Texture* m_blur_texture{ nullptr };
		Target*  m_blur_target{ nullptr };
		//debug view: the blurred occlusion, of the frame size
		Texture* m_debug_texture{ nullptr };
		Target*  m_debug_target{ nullptr };
		IVec2    m_size{ 0, 0 };    //frame
		IVec2    m_ao_size{ 0, 0 }; //occlusion
		bool create_targets(const IVec2& size, const IVec2& ao_size);
	};
}
}
