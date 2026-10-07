//
//  PostEffectSnow.h
//  Square
//
//  Falling snow, a PES_COLOR post effect: flakes in the world, not on the screen. The view ray of
//  each pixel walks a grid of cells anchored to the world (a 3D DDA, up to max_distance), each
//  cell with at most a flake (a random place, size); the grid falls with the time and drifts with
//  the wind, so the flakes have parallax, a size by their distance, a place in depth: hidden by
//  the geometry before them (deferred: the positions of the G-Buffer; forward: never hidden). A
//  flake is a short segment along its fall (the motion blur of a shutter), out of focus near the
//  camera (a soft disc), never smaller than a pixel (no shimmer), fading with the distance, lit
//  by the frame behind it (the light of the scene there).
//  Add it after the fog, before the bloom.
//
#pragma once
#include <chrono>
#include "Square/Config.h"
#include "Square/Render/PostEffect.h"

namespace Square
{
namespace Render
{
	class SQUARE_API Snow : public PostEffect
	{
	public:
		SQUARE_OBJECT(Snow)

		struct Settings
		{
			float density{ 0.35f };          //share of the cells with a flake [0, 1]
			float spacing{ 1.6f };           //side of a cell (world units)
			float size{ 0.025f };            //radius of a flake (world units)
			float speed{ 1.4f };             //fall, world units per second
			Vec2  wind{ 0.4f, 0.15f };       //drift along x, z, world units per second
			float near_distance{ 0.5f };     //no flake nearer than this (world units)
			float max_distance{ 30.0f };     //no flake farther than this (world units)
			float focus_distance{ 4.0f };    //nearer: out of focus (a soft disc)
			float shutter{ 0.04f };          //seconds of motion blur (the length of a flake along its fall)
			Vec3  color{ 0.9f, 0.92f, 0.95f }; //linear, as the light of the frame
			float intensity{ 0.8f };         //opacity of the flakes
		};

		Snow(Square::Context& context);
		virtual ~Snow();

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//it draws only with a camera
		virtual bool active(const PostEffectFrame& frame) const override { return frame.m_camera != nullptr; }
		virtual void draw(PostEffectFrame& frame) override;

	protected:
		virtual void on_release() override;

	private:
		void copy(PostEffectFrame& frame);
		//seconds of the snow (the frames it was drawn: a long frame counts at most 0.1)
		float advance_time();

		using Clock = std::chrono::steady_clock;
		Settings                 m_settings;
		Shared<Resource::Shader> m_shader;
		Shared<Resource::Shader> m_shader_copy;
		float                    m_time{ 0.0f };
		Clock::time_point        m_last{};
		bool                     m_started{ false };
	};
}
}
