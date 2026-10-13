//
//  PostEffectBloom.h
//  Square
//
//  Bloom, a PES_COLOR post effect (forward and deferred): the light brighter than a threshold
//  spreads around it. On a chain of textures, each one half the size of the one before:
//   1) prefilter: the frame to the first level (half size), 13 samples with the average of
//      Karis (a lone very bright pixel does not flicker) and a soft threshold (knee);
//   2) downsample: every level from the one before, 13 samples (a wide, smooth blur);
//   3) upsample: from the last level back to the first, each level mixed with the one under it
//      (3x3 tent), by scatter: the wider levels give the wide glow;
//   4) composite: the frame plus the first level times the intensity.
//  The frame is linear HDR (RGBA16F): the values over 1 (lights, emissive) glow.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"
#include <vector>

namespace Square
{
namespace Render
{
	class SQUARE_API Bloom : public PostEffect
	{
	public:
		SQUARE_OBJECT(Bloom)

		struct Settings
		{
			float threshold{ 1.0f };     //linear brightness where the bloom starts
			float knee{ 0.5f };          //soft zone under the threshold (0: a hard cut)
			float intensity{ 0.5f };     //bloom added to the frame
			float scatter{ 0.7f };       //share of the wider levels in each level: more, a wider glow
			int   levels{ 6 };           //levels of the chain (the first one half the frame size)
			float clamp{ 65000.0f };     //brightest value that enters (no Inf, no fireflies)
			bool  debug{ false };        //debug view: only the bloom on the screen
		};

		Bloom(Square::Context& context);
		virtual ~Bloom();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		virtual void draw(PostEffectFrame& frame) override;
		virtual void debug_options(std::vector<DebugOption>& options) override;

	protected:
		virtual void on_release() override;

	private:
		//a level of the chain: downsampled, and upsampled (with the levels under it)
		struct Level
		{
			IVec2    m_size{ 0, 0 };
			Texture* m_down_texture{ nullptr };
			Target*  m_down_target{ nullptr };
			Texture* m_up_texture{ nullptr };
			Target*  m_up_target{ nullptr };
		};

		Settings                 m_settings;
		Shared<Resource::Shader> m_shader_prefilter;
		Shared<Resource::Shader> m_shader_downsample;
		Shared<Resource::Shader> m_shader_upsample;
		Shared<Resource::Shader> m_shader_composite;
		Shared<Resource::Shader> m_shader_copy;    //the frame as it is, when the bloom cannot be drawn
		std::vector<Level>       m_levels;
		IVec2                    m_size{ 0, 0 };   //frame size of the chain
		int                      m_levels_count{ 0 }; //levels asked when it was built

		bool build_levels(const IVec2& size);
		//the destination must be written (it is the next frame): the frame as it is
		void copy(PostEffectFrame& frame);
		void release_levels();
	};
}
}
