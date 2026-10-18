//
//  Square
//
//  Android: the input of the window (the events of the activity): keyboard and gamepad keys,
//  mouse, touch screen (its first finger also as the mouse: the UI).
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Android
{
	struct InputAndroid
	{
		WindowAndroid*																m_window_ref{ nullptr };
		Input*																		m_input_ref{ nullptr };
		std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)>   m_keyboard{ nullptr };
		std::function<void(int character, short mode, int plain)>                   m_character{ nullptr };
		std::function<void(MouseButtonEvent mevent, ActionEvent action)>			m_mouse_button{ nullptr };
		std::function<void(double x, double y)>										m_mouse_move{ nullptr };
		std::function<void(double scroll)>											m_scroll{ nullptr };
		std::function<void(FingerEvent fevent, ActionEvent action)>					m_touch{ nullptr };
		std::function<void(FingerEvent finger, ActionEvent action, double x, double y)> m_finger{ nullptr };
		std::function<void(double x, double y, double z)>							m_tilt{ nullptr };
		std::function<void(WindowEvent wevent)>										m_window{ nullptr };
		std::function<void(double x, double y)>										m_window_move{ nullptr };

		void send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action);
		void send_character_event(int character, short mode, bool plain);
		void send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action);
		void send_mouse_move_event(double x, double y);
		void send_mouse_scroll_event(double scroll);
		void send_touch_event(FingerEvent fevent, ActionEvent action);
		//a finger (where: the pixels of the screen, sent in the ones of the frames)
		void send_finger_event(FingerEvent finger, ActionEvent action, double x, double y);
		void send_tilt_event(double x, double y, double z);
		void send_window_event(WindowEvent wevent);
		void send_window_move_event(double x, double y);
	};

	//an event of the input of the activity to the input of the engine: 1 when it is used
	int32_t input_event(InputAndroid& input, const AInputEvent* event);
}
}
}
