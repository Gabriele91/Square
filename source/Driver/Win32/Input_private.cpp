//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Input_private.h"
#include <Xinput.h>

namespace Square
{
namespace Video
{
namespace Win32
{
	void InputWin32::send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action)
	{
		if (m_keyboard) m_keyboard(kevent, mode, action);
	}
	void InputWin32::send_character_event(int character, short mode, bool plain)
	{
		if (m_character) m_character(character, mode, plain);
	}
	void InputWin32::send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
	{
		if (m_mouse_button) m_mouse_button(mevent, action);
	}
	void InputWin32::send_mouse_move_event(double x, double y)
	{
		if (m_mouse_move) m_mouse_move(x, y);
	}
	void InputWin32::send_mouse_scroll_event(double scroll)
	{
		if (m_scroll) m_scroll(scroll);
	}
	void InputWin32::send_touch_event(FingerEvent fevent, ActionEvent action)
	{
		if (m_touch) m_touch(fevent, action);
	}
	void InputWin32::send_window_event(WindowEvent wevent)
	{
		if (m_window) m_window(wevent);
	}
	void InputWin32::send_window_move_event(double x, double y)
	{
		if (m_window_move) m_window_move(x,y);
	}

	namespace AuxGamepads
	{
		//its buttons (in the order of GamepadButton, the triggers apart: from their axes)
		static constexpr WORD s_buttons[] =
		{
			XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
			XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, 0, 0,
			XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
			XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT
		};
		static_assert(std::size(s_buttons) == size_t(GAMEPAD_BUTTON_COUNT), "a flag for every button");

		//the state of a pad of XInput as the one of the engine
		static GamepadState state_of(const XINPUT_GAMEPAD& gamepad)
		{
			GamepadState state;
			state.connected = true;
			for (size_t button = 0; button != state.buttons.size(); ++button)
			{
				state.buttons[button] = (gamepad.wButtons & s_buttons[button]) != 0;
			}
			const float dead = float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 32767.0f;
			gamepad_stick(gamepad.sThumbLX / 32767.0f, gamepad.sThumbLY / 32767.0f, dead, state.axes[GAMEPAD_LEFT_X], state.axes[GAMEPAD_LEFT_Y]);
			gamepad_stick(gamepad.sThumbRX / 32767.0f, gamepad.sThumbRY / 32767.0f, dead, state.axes[GAMEPAD_RIGHT_X], state.axes[GAMEPAD_RIGHT_Y]);
			const float trigger = float(XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / 255.0f;
			state.axes[GAMEPAD_LEFT_TRIGGER_AXIS] = gamepad_trigger(gamepad.bLeftTrigger / 255.0f, trigger);
			state.axes[GAMEPAD_RIGHT_TRIGGER_AXIS] = gamepad_trigger(gamepad.bRightTrigger / 255.0f, trigger);
			return state;
		}
	}

	void InputWin32::gamepads_load()
	{
		//(the newest one there: Windows 10, 7, Vista)
		for (const char* name : { "xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll" })
		{
			SharedLibraryPtr library(SharedLibrary::open(name));
			if (library)
			{
				m_xinput_get_state = (XInputGetState)SharedLibrary::get(library.get(), "XInputGetState");
				if (m_xinput_get_state)
				{
					m_xinput = std::move(library);
					return;
				}
			}
		}
	}

	void InputWin32::gamepads_unload()
	{
		//(its library closed: SharedLibraryClose)
		m_xinput_get_state = nullptr;
		m_xinput.reset();
	}

	void InputWin32::gamepads_update(bool focus)
	{
		const ULONGLONG now = GetTickCount64();
		for (DWORD pad = 0; pad != DWORD(GAMEPAD_MAX); ++pad)
		{
			GamepadState& state = m_gamepad_states[pad];
			const bool ask = m_xinput_get_state && (state.connected || now - m_gamepad_asked[pad] >= 1000);
			if (ask)
			{
				m_gamepad_asked[pad] = now;
				XINPUT_STATE xstate{};
				state = m_xinput_get_state(pad, &xstate) == ERROR_SUCCESS ? AuxGamepads::state_of(xstate.Gamepad) : GamepadState();
			}
			//(not the window in front: as nothing held)
			GamepadState sent = state;
			if (!focus)
			{
				sent = GamepadState();
				sent.connected = state.connected;
			}
			m_gamepads.send(int(pad), sent);
		}
	}
}
}
}