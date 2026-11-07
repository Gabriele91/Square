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
#include "Square/Core/SharedLibrary.h"
#include "../Gamepad_private.h"
#include <memory>
#include <windows.h>
#include <Variant>

namespace Square
{
namespace Video
{
namespace Win32
{
	//a shared library, closed when it goes
	struct SharedLibraryClose
	{
		void operator()(void* library) const { SharedLibrary::close(library); }
	};
	using SharedLibraryPtr = std::unique_ptr<void, SharedLibraryClose>;

	struct SQUARE_API InputWin32
	{
		Window*																					 m_window_ref   { nullptr };
		Input*																					 m_input_ref    { nullptr };
		std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)>                m_keyboard     { nullptr };
		std::function<void(int character, short mode, int plain)>                                m_character    { nullptr };
		std::function<void(MouseButtonEvent mevent, ActionEvent action)>						 m_mouse_button { nullptr };
		std::function<void(double x, double y)>													 m_mouse_move   { nullptr };
		std::function<void(double scroll)>														 m_scroll	    { nullptr };
		std::function<void(FingerEvent fevent, ActionEvent action)>						         m_touch        { nullptr };
		std::function<void(WindowEvent wevent)>												     m_window       { nullptr };
		std::function<void(double x, double y)>												     m_window_move  { nullptr };

		void send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action);
		void send_character_event(int character, short mode, bool plain);
		void send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action);
		void send_mouse_move_event(double x, double y);
		void send_mouse_scroll_event(double scroll);
		void send_touch_event(FingerEvent fevent, ActionEvent action);
		void send_window_event(WindowEvent wevent);
		void send_window_move_event(double x, double y);

		//its gamepads (XInput: an Xbox pad and the ones like it): its library (none: no gamepad),
		//their state, when a slot with nothing in it was asked (slow: again once a second), the
		//changes sent (no focus: as nothing held)
		using XInputGetState = DWORD(WINAPI*)(DWORD, void*);
		SharedLibraryPtr                      m_xinput;
		XInputGetState                        m_xinput_get_state{ nullptr };
		std::array<GamepadState, GAMEPAD_MAX> m_gamepad_states{};
		std::array<ULONGLONG, GAMEPAD_MAX>    m_gamepad_asked{};
		GamepadEvents                         m_gamepads;

		//its library loaded, unloaded
		void gamepads_load();
		void gamepads_unload();
		//their state now, sent (focus: the window in front)
		void gamepads_update(bool focus);

		using EventKey = std::variant < KeyboardEvent, MouseButtonEvent >;
		UnorderedMap<EventKey, MSG> m_event_pool;
	};

}
}
}