//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"
#include "../Gamepad_private.h"
#include <chrono>

///////////////////////////////////////////////////

namespace Square
{
namespace Video
{
namespace Xorg
{
	struct InputXorg
	{
		WindowXorg*																	m_window_ref{ nullptr };
		Input*																		m_input_ref{ nullptr };
		std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)>   m_keyboard{ nullptr };
		std::function<void(int character, short mode, int plain)>                   m_character{ nullptr };
		std::function<void(MouseButtonEvent mevent, ActionEvent action)>			m_mouse_button{ nullptr };
		std::function<void(double x, double y)>										m_mouse_move{ nullptr };
		std::function<void(double scroll)>											m_scroll{ nullptr };
		std::function<void(FingerEvent fevent, ActionEvent action)>					m_touch{ nullptr };
		std::function<void(WindowEvent wevent)>										m_window{ nullptr };
		std::function<void(double x, double y)>										m_window_move{ nullptr };
		//its gamepads (evdev: /dev/input/event*, the ones with the buttons of a gamepad): each one
		//its device (-1: none), its path, the range of its axes (by ABS_*), its raw axes, its
		//state; when they were looked for (again every two seconds: one plugged in); the focus of
		//its window (none: as nothing held); the changes sent
		struct GamepadDevice
		{
			int                       fd{ -1 };
			std::string               path;
			std::array<float, 0x40>   min{};
			std::array<float, 0x40>   max{};
			std::array<float, 0x40>   raw{};
		};
		std::array<GamepadDevice, GAMEPAD_MAX>     m_gamepad_devices{};
		std::array<GamepadState, GAMEPAD_MAX>      m_gamepad_states{};
		std::chrono::steady_clock::time_point     m_gamepad_scan{};
		bool                                       m_gamepad_scanned{ false };
		bool                                       m_focus{ true };
		GamepadEvents                              m_gamepads;

		~InputXorg();
		//the devices plugged in looked for, their events read, the changes sent
		void gamepads_update();

		void send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action);
		void send_character_event(int character, short mode, bool plain);
		void send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action);
		void send_mouse_move_event(double x, double y);
		void send_mouse_scroll_event(double scroll);
		void send_touch_event(FingerEvent fevent, ActionEvent action);
		void send_window_event(WindowEvent wevent);
		void send_window_move_event(double x, double y);
	};
}
}
}
