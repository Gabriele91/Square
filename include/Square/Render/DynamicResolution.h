//
//  DynamicResolution.h
//  Square
//
//  The scene lower while the frames are late (RenderInstance: its resolution scale times this
//  one, by axis): its frames against the budget of their rate (the target, else the limit of the
//  application, else the refresh of the screen), lower at once when late, a step higher after
//  some seconds on time (a try: late again, back and a longer wait before the next one). Its
//  share glides to its level (no jump); a long frame (a loading) is not counted.
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

		//its settings (its level kept, within its range)
		void settings(const Settings& settings);
		const Settings& settings() const;

		//a frame (its seconds, the rate of the frames): the share of the scene now, by axis (1: as it is)
		Vec2 update(float delta_time, float fps);
		//its level back to the top
		void reset();

	private:

		//how it goes: steady, a step higher on trial, a wait after a change
		enum class State
		{
			STEADY,
			PROBE,
			COOLDOWN
		};

		//late: the frames of the last moment over the budget
		bool late(float fps) const;
		void enter(State state);

		Settings m_settings;
		State    m_state{ State::STEADY };
		float    m_level{ 0.0f };    //0: as it is, 1: all the range
		float    m_shown{ 0.0f };    //the level shown (gliding to it)
		float    m_frame_ms{ 0.0f }; //the frames of the last moment (an average)
		float    m_time{ 0.0f };     //seconds in its state
		float    m_wait{ 2.0f };     //seconds on time before a try
	};
}
}
