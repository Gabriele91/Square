//
//  PostEffectGodRays.h
//  Square
//
//  God rays, a PES_COLOR post effect of the deferred pipeline (in forward the frame is copied as
//  it is: no G-Buffer): the light of the sun through the gaps of what is in front of it (the
//  volumetric light scattering as a post process, GPU Gems 3, 13):
//   1) mask: the sky around the sun (the background of the G-Buffer, and a sky of geometry: its
//      albedo black, only emissive, as a sky dome; by its distance from the sun on the screen),
//      the rest black (it blocks the light), at a share of the frame size;
//   2) blur: every pixel of the mask the sum of the samples toward the sun, each one weaker
//      (decay), over a share of the way (density);
//   3) composite: the frame plus the rays times their color and intensity, less as the sun goes
//      out of the view (none behind the camera).
//  Only the sun on the screen or near its border makes rays.
//  Volumetric (the default, when the frame has the shadow map of the sun: the deferred pipeline
//  with its shadow): the light of the sun in the air along the ray of each pixel (steps of the
//  ray from the camera to the geometry, at most max_distance, each one lit where the shadow map
//  of the sun says so: the shafts through the leaves, also with the sun out of the view), more
//  looking toward the sun (Henyey-Greenstein, anisotropy), blurred, added with the hue of the
//  sun (its color over its brightest channel: its strength does not count, the intensity does). Add it after the fog, before the bloom.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API GodRays : public PostEffect
	{
	public:
		SQUARE_OBJECT(GodRays)

		struct Settings
		{
			Vec3  sun_direction{ 0.0f, -1.0f, 0.0f }; //where the light of the sun goes (world)
			Vec3  color{ 1.0f, 0.92f, 0.8f };        //of the rays (linear)
			float intensity{ 0.6f };                 //rays added to the frame
			float threshold{ 0.0f };                 //the sky brighter than it makes rays
			float sun_radius{ 0.5f };                //screen heights around the sun of the mask
			float density{ 0.9f };                   //share of the way to the sun the samples cover
			float decay{ 0.96f };                    //each sample this of the one before
			float weight{ 0.05f };                   //of a sample
			int   samples{ 48 };                     //toward the sun (at most 128)
			PostEffectResolution resolution{ PER_HALF }; //size of the mask and of the blur
			float fade{ 0.25f };                     //screen shares out of the view where the rays fade
			bool  emissive_sky{ true };              //the black emissive geometry is sky too (a dome)
			//volumetric (the shadow map of the sun)
			bool  volumetric{ true };                //with the shadow of the sun (else on the screen)
			float max_distance{ 120.0f };            //world units of the ray in the air at most
			int   steps{ 32 };                       //of the ray (at most 128)
			float anisotropy{ 0.6f };                //the light forward (0 all around, toward 1 toward the sun)
			float air{ 0.015f };                     //density of the air: the scattered share is 1 - e^(-air distance)
		};

		GodRays(Square::Context& context);
		virtual ~GodRays();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//it draws only with the deferred pipeline, rays
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_gbuffer && frame.m_camera && m_settings.intensity > 0.0f; }
		virtual void draw(PostEffectFrame& frame) override;

	protected:
		virtual void on_release() override;

	private:
		void copy(PostEffectFrame& frame);
		bool build(const IVec2& size);
		void draw_volumetric(PostEffectFrame& frame, const IVec2& size);

		Settings                 m_settings;
		IVec2                    m_size{ 0, 0 };
		Texture*                 m_mask_texture{ nullptr };
		Target*                  m_mask_target{ nullptr };
		Texture*                 m_rays_texture{ nullptr };
		Target*                  m_rays_target{ nullptr };
		Shared<Resource::Shader> m_shader_mask;
		Shared<Resource::Shader> m_shader_blur;
		Shared<Resource::Shader> m_shader_composite;
		Shared<Resource::Shader> m_shader_copy;
		Shared<Resource::Shader> m_shader_volume;
		Shared<Resource::Shader> m_shader_volume_blur;
	};
}
}
