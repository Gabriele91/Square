//
//  PostEffectFXAA.h
//  Square
//
//  Anti-aliasing of the frame (FXAA: fast approximate anti-aliasing, as Lottes): a PES_COLOR post
//  effect, forward and deferred (it reads only the colors of the frame): where the luma of a pixel
//  changes sharply against its neighbors (an edge), the pixel is blended along the edge (its
//  direction from the gradient of the luma), its stairs smoothed. Add it last (after the bloom).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API FXAA : public PostEffect
	{
	public:
		SQUARE_OBJECT(FXAA)

		struct Settings
		{
			float edge_threshold{ 0.125f };     //a contrast of luma under this (relative): no edge
			float edge_threshold_min{ 0.0312f };//a contrast under this (absolute, the dark): no edge
			float span_max{ 8.0f };             //pixels the blend reaches along an edge
		};

		FXAA(Square::Context& context);
		virtual ~FXAA();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		virtual void draw(PostEffectFrame& frame) override;

	protected:
		virtual void on_release() override;

	private:
		Settings                 m_settings;
		Shared<Resource::Shader> m_shader;
	};
}
}
