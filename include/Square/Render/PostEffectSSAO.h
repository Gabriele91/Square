//
//  PostEffectSSAO.h
//  Square
//
//  Screen space ambient occlusion, a PES_GBUFFER post effect (deferred): from the world
//  positions and normals of the G-Buffer it estimates how much of the hemisphere of each
//  pixel is hidden by the geometry around it (Alchemy AO: samples in screen space within a
//  radius in world units, nearer occluders count more), blurs it keeping the edges, and
//  multiplies the occlusion of the G-Buffer: only the ambient light is darkened.
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
			float max_pixels{ 64.0f }; //radius on the screen at most (pixels), near the camera
			bool  blur{ true };       //4x4 blur that keeps the edges (the noise of the samples)
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
		Shared<Resource::Shader> m_shader_ssao;
		Shared<Resource::Shader> m_shader_apply;
		//raw occlusion, of the frame size
		Texture* m_ao_texture{ nullptr };
		Target*  m_ao_target{ nullptr };
		//debug view: the blurred occlusion
		Texture* m_debug_texture{ nullptr };
		Target*  m_debug_target{ nullptr };
		IVec2    m_size{ 0, 0 };
	};
}
}
