//
//  PostEffectDOF.h
//  Square
//
//  Depth of field, a PES_COLOR post effect of the deferred pipeline (in forward the frame is
//  copied as it is: no world positions): each pixel blurred by its circle of confusion, from the
//  distance of its world position (G-Buffer) to the camera: sharp around the focus distance,
//  blurrier farther (and nearer, if near is on) up to max_radius pixels; the background (no
//  geometry) as far as it can be. A gather of a disc of samples (golden angle spiral), each
//  weighted by its own circle (a sharp thing in front does not bleed into the blur behind it).
//  At the size of the frame, or smaller (Settings::resolution: half, a quarter; the blur and the
//  motion drawn small, then put over the frame by the circle of each of its pixels: the sharp
//  ones stay as they are). Add it before the bloom.
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/PostEffect/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API DOF : public PostEffect
	{
	public:
		SQUARE_OBJECT(DOF)

		struct Settings
		{
			float focus_distance{ 15.0f }; //world units from the camera: sharp
			float focus_range{ 8.0f };     //around it still sharp (half of it on each side)
			float far_range{ 60.0f };      //from the end of the sharp range to the blurriest
			float near_range{ 6.0f };      //from the start of the sharp range to the blurriest, nearer
			bool  near{ false };           //the things nearer than the focus blurred too
			float max_radius{ 10.0f };     //the blur at its most, pixels of a 1080p frame (scaled to the frame)
			int   samples{ 32 };           //of the disc
			//a motion blur along the screen x on what is near (within the end of the sharp range):
			//none left of motion_from, its most (motion pixels of a 1080p frame) right of motion_to
			//(screen shares, 0 left, 1 right); 0: none
			float motion{ 0.0f };
			float motion_from{ 0.5f };
			float motion_to{ 0.9f };
			//the size of the blur (full: one pass at the size of the frame)
			PostEffectResolution resolution{ PER_FULL };
		};

		DOF(Square::Context& context);
		virtual ~DOF();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//it draws only with the deferred pipeline (its G-Buffer)
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_gbuffer && frame.m_camera; }
		virtual void draw(PostEffectFrame& frame) override;
		virtual void debug_options(std::vector<DebugOption>& options) override;

	protected:
		virtual void on_release() override;

	private:
		void copy(PostEffectFrame& frame);
		//the blur of a frame into a target (its size: pixels of the pass), the uniforms of the
		//pass (radius and motion scaled to its pixels)
		void blur(PostEffectFrame& frame, Target* target, const IVec2& size);
		//the uniforms of the pass of a size (the settings: of a 1080p frame)
		void uniforms(Resource::Shader& shader, const IVec2& size) const;

		Settings                 m_settings;
		Shared<Resource::Shader> m_shader;
		Shared<Resource::Shader> m_shader_composite;
		Shared<Resource::Shader> m_shader_copy;
		//the blur smaller than the frame (resolution: half, a quarter)
		Texture*                 m_blur_texture{ nullptr };
		Target*                  m_blur_target{ nullptr };
		IVec2                    m_blur_size{ 0, 0 };
	};
}
}
