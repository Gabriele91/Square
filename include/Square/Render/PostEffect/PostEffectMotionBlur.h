//
//  PostEffectMotionBlur.h
//  Square
//
//  Motion blur of the objects, a PES_COLOR post effect of the deferred pipeline: only the
//  renderables with their own motion blur (Renderable::motion_blur) are blurred, along their
//  motion on the screen since the last frame (the velocity of the frame, drawn by the deferred
//  pass after the G-Buffer: their model and the camera then, against now). The world stays sharp.
//  A pixel takes the longest motion around it (a few taps: the blur goes a little out of the
//  silhouette, as an exposure), a share of it (the shutter), at most max_pixels. No velocity (no
//  renderable on, forward): the frame as it is. Add it before the bloom.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API MotionBlur : public PostEffect
	{
	public:
		SQUARE_OBJECT(MotionBlur)

		struct Settings
		{
			float shutter{ 0.5f };      //share of the motion of a frame blurred
			float max_pixels{ 40.0f };  //the blur at its most, pixels of a 1080p frame (scaled to the frame)
			float min_pixels{ 0.5f };   //under it no blur (pixels of the frame)
			int   samples{ 12 };        //along the motion (at most 32)
		};

		MotionBlur(Square::Context& context);
		virtual ~MotionBlur();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		virtual bool needs_velocity() const override { return true; }
		//it draws only with a velocity drawn (something with its motion blur on)
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_velocity != nullptr; }
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
