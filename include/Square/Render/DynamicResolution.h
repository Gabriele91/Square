//
//  DynamicResolution.h
//  Square
//
//  The scene lower while the frames are late (RenderInstance: its resolution scale times this
//  one, by axis), by the frames of the last moment only (a window of half a second: a loading,
//  a hitch forgotten soon) against the budget of their rate (the target, else the limit of the
//  application, else the refresh of the screen): late (over the budget plus its tolerance, 10%)
//  lower at once, as much as they are late (its pixels: the time of the scene), then a new
//  window before it looks again; on time, a little higher every second (none with the GPU near
//  the budget: Render::Context::gpu_frame_ms). Its share glides to its target (no jump); a long
//  frame (a loading) is not counted, its window again.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Square
{
namespace Render
{
	class SQUARE_API DynamicResolution
	{
	public:

		struct Settings
		{
			float m_range{ 0.0f };      //how much lower at most (0.5: half; 0: off)
			bool  m_width{ true };      //the axes it lowers
			bool  m_height{ true };
			float m_target_fps{ 0.0f }; //the rate of the frames (0: the limit of the application, else the refresh)
		};

		//its settings (its share kept, within its range)
		void settings(const Settings& settings);
		const Settings& settings() const;

		//a frame (its seconds, the rate of the frames, the GPU time of the last one done, ms; < 0:
		//unknown): the share of the scene now, by axis (1: as it is)
		Vec2 update(float delta_time, float fps, float gpu_ms);
		//its share back to the top
		void reset();

	private:

		//how it goes: a new window filling (after a change, a loading), watching its frames
		enum class State
		{
			SETTLE,
			WATCH
		};

		//a frame in its window (the older ones out of it)
		void push(float delta_time);
		void enter(State state);
		//the lowest share of its range
		float lowest() const;

		Settings      m_settings;
		State         m_state{ State::SETTLE };
		float         m_share{ 1.0f };        //its target (1: as it is)
		float         m_shown{ 1.0f };        //the share shown (gliding to it)
		Vector<float> m_frames;               //the seconds of the frames of its window
		float         m_window{ 0.0f };       //their sum
	};
}
}
