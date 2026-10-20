//
//  DynamicThermal.h
//  Square
//
//  The scene lower while the device is hot (RenderInstance: its resolution scale times this one,
//  by axis), before the system slows it: the heat of its device (Video::DeviceResources::
//  get_thermal: its headroom some seconds ahead, else its status) as a level between from (cool
//  enough: as it is) and to (the system slows it hard: all the range). Its level up quickly (the
//  heat coming), down slowly (the heat stays: no back and forth); its share glides to it. A
//  device that does not say its heat (a computer): as it is.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Driver/Window.h"

namespace Square
{
namespace Render
{
	class SQUARE_API DynamicThermal
	{
	public:

		struct Settings
		{
			float m_range{ 0.0f };   //how much lower at most (0.25: a quarter; 0: off)
			bool  m_width{ true };   //the axes it lowers
			bool  m_height{ true };
			float m_from{ 0.6f };    //the headroom it starts from (lower: as it is)
			float m_to{ 1.0f };      //the headroom of all its range (1: the system slows it hard)
			int   m_forecast{ 10 };  //the headroom of these seconds ahead
		};

		//its settings (its level kept)
		void settings(const Settings& settings);
		const Settings& settings() const;

		//a frame (its seconds, the device of the window; none: as it is): the share of the scene
		//now, by axis (1: as it is)
		Vec2 update(float delta_time, Video::DeviceResources* device);
		//its level now (0: cool, 1: all the range), the heat last read
		float level() const;
		const Video::ThermalState& state() const;
		//its level back to cool
		void reset();

	private:

		//the level of a heat (its headroom, else its status; unknown: 0)
		float level_of(const Video::ThermalState& state) const;

		Settings            m_settings;
		Video::ThermalState m_state;
		float               m_target{ 0.0f }; //the level of the heat read
		float               m_level{ 0.0f };  //going to it (up quickly, down slowly)
		float               m_shown{ 0.0f };  //the level shown (gliding to it)
		float               m_time{ 0.0f };   //seconds since the heat was read
	};
}
}
