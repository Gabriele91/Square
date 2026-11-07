//
//  Square
//
//  The gamepads of a driver: their changes as the events of the Input, their dead zones.
//
#include "Gamepad_private.h"
#include <algorithm>
#include <cmath>

namespace Square
{
namespace Video
{
	void gamepad_stick(float x, float y, float dead, float& out_x, float& out_y)
	{
		const float length = std::sqrt(x * x + y * y);
		out_x = out_y = 0.0f;
		if (length > dead && length > 0.0f)
		{
			const float share = std::min((length - dead) / std::max(1.0f - dead, 0.001f), 1.0f) / length;
			out_x = std::clamp(x * share, -1.0f, 1.0f);
			out_y = std::clamp(y * share, -1.0f, 1.0f);
		}
	}

	float gamepad_trigger(float value, float dead)
	{
		return value <= dead ? 0.0f : std::min((value - dead) / std::max(1.0f - dead, 0.001f), 1.0f);
	}

	void GamepadEvents::send(int pad, GamepadState now)
	{
		if (pad >= 0 && pad < GAMEPAD_MAX)
		{
			GamepadState& was = m_sent[size_t(pad)];
			if (!now.connected)
			{
				now = GamepadState();
			}
			//its triggers as buttons: down over half their way, up under 0.4 of it
			const float left = now.axes[GAMEPAD_LEFT_TRIGGER_AXIS];
			const float right = now.axes[GAMEPAD_RIGHT_TRIGGER_AXIS];
			now.buttons[GAMEPAD_LEFT_TRIGGER] = now.buttons[GAMEPAD_LEFT_TRIGGER] || left > (was.buttons[GAMEPAD_LEFT_TRIGGER] ? 0.4f : 0.5f);
			now.buttons[GAMEPAD_RIGHT_TRIGGER] = now.buttons[GAMEPAD_RIGHT_TRIGGER] || right > (was.buttons[GAMEPAD_RIGHT_TRIGGER] ? 0.4f : 0.5f);
			if (now.connected && !was.connected && m_connected)
			{
				m_connected(pad, true);
			}
			for (size_t button = 0; button != now.buttons.size(); ++button)
			{
				if (now.buttons[button] != was.buttons[button] && m_button)
				{
					m_button(pad, GamepadButton(button), now.buttons[button] ? ActionEvent::PRESS : ActionEvent::RELEASE);
				}
			}
			for (size_t axis = 0; axis != now.axes.size(); ++axis)
			{
				if (now.axes[axis] != was.axes[axis] && m_axis)
				{
					m_axis(pad, GamepadAxis(axis), double(now.axes[axis]));
				}
			}
			if (!now.connected && was.connected && m_connected)
			{
				m_connected(pad, false);
			}
			was = now;
		}
	}
}
}
