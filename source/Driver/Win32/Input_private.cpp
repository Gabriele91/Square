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
#include "Input_private.h"

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
}
}
}