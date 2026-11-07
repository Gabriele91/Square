//
//  Square
//
//  The gamepads of a driver: the state of each (read from its system), its changes sent as the
//  events of the Input (connected, buttons, axes); the dead zones of the sticks and triggers.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Input.h"
#include <array>
#include <functional>

namespace Square
{
namespace Video
{
	//the state of a gamepad (its triggers as buttons: from their axes, see GamepadEvents::send)
	struct GamepadState
	{
		bool                                    connected{ false };
		std::array<bool, GAMEPAD_BUTTON_COUNT>  buttons{};
		std::array<float, GAMEPAD_AXIS_COUNT>   axes{};
	};

	//a stick past its dead zone (round), from 0 at its edge to 1 (x right, y up)
	void gamepad_stick(float x, float y, float dead, float& out_x, float& out_y);
	//a trigger past its dead zone, from 0 to 1
	float gamepad_trigger(float value, float dead);

	//the listeners of the gamepads, and what they were told
	struct GamepadEvents
	{
		std::function<void(int pad, bool connected)>                                  m_connected{ nullptr };
		std::function<void(int pad, GamepadButton button, ActionEvent action)>        m_button{ nullptr };
		std::function<void(int pad, GamepadAxis axis, double value)>                  m_axis{ nullptr };
		std::array<GamepadState, GAMEPAD_MAX>                                         m_sent{};

		//its state now: the changes sent (gone: as nothing held)
		void send(int pad, GamepadState now);
	};
}
}
