//
//  PostEffectFog.h
//  Square
//
//  Height fog, a PES_COLOR post effect of the deferred pipeline (in forward the frame is copied
//  as it is: no world positions): the fog of each pixel by the way from the camera to its world
//  position (G-Buffer), dense under a height and thinner over it (exponential height fog, as in
//  Unreal: the density integrated along the view ray, in closed form), brighter toward the sun
//  (in-scattering). The background (no geometry) takes the fog the sky setting says.
//  The translucent objects (drawn after the G-Buffer) take the fog of what is behind them.
//  Add it before the bloom: the lights in the fog still glow.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API Fog : public PostEffect
	{
	public:
		SQUARE_OBJECT(Fog)

		struct Settings
		{
			Vec3  color{ 0.5f, 0.6f, 0.7f };   //color of the fog (linear, as the light of the frame)
			float density{ 0.02f };            //per world unit, at the height
			float height{ 0.0f };              //world height (y) of the density
			float falloff{ 0.1f };             //over the height the density is e^-1 every 1/falloff units (0: the same everywhere)
			float start{ 0.0f };               //world units from the camera without fog
			float max_opacity{ 1.0f };         //the fog covers at most this
			float sky{ 1.0f };                 //the fog of the background (no geometry), [0, 1]
			Vec3  sun_color{ 0.0f };           //the glow looking toward the sun (0: none)
			Vec3  sun_direction{ 0.0f, -1.0f, 0.0f }; //where the light of the sun goes (world)
			float sun_exponent{ 8.0f };        //how narrow the glow is
		};

		Fog(Square::Context& context);
		virtual ~Fog();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//it draws only with the deferred pipeline, a fog
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_gbuffer && frame.m_camera && m_settings.density > 0.0f; }
		virtual void draw(PostEffectFrame& frame) override;

	protected:
		virtual void on_release() override;

	private:
		void copy(PostEffectFrame& frame);

		Settings                 m_settings;
		Shared<Resource::Shader> m_shader;
		Shared<Resource::Shader> m_shader_copy;
	};
}
}
