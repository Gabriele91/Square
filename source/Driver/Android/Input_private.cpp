//
//  Square
//
//  Android: the input of the window (the events of the activity): keyboard and gamepad keys,
//  mouse, touch screen (its first finger also as the mouse: the UI).
//
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"
#include <android/keycodes.h>

namespace Square
{
namespace Video
{
namespace Android
{
	void InputAndroid::send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action)
	{
		if (m_keyboard) m_keyboard(kevent, mode, action);
	}
	void InputAndroid::send_character_event(int character, short mode, bool plain)
	{
		if (m_character) m_character(character, mode, plain);
	}
	void InputAndroid::send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
	{
		if (m_mouse_button) m_mouse_button(mevent, action);
	}
	//the pixels of the screen to the pixels of the frames (their size: set_size)
	static void to_frame(const WindowAndroid* window, double& x, double& y)
	{
		unsigned int native[2];
		native_window_size(native);
		if (window && native[0] && native[1])
		{
			x *= double(window->m_info.m_size[0]) / double(native[0]);
			y *= double(window->m_info.m_size[1]) / double(native[1]);
		}
	}

	void InputAndroid::send_mouse_move_event(double x, double y)
	{
		to_frame(m_window_ref, x, y);
		if (m_mouse_move) m_mouse_move(x, y);
	}
	void InputAndroid::send_finger_event(FingerEvent finger, ActionEvent action, double x, double y)
	{
		to_frame(m_window_ref, x, y);
		if (m_finger) m_finger(finger, action, x, y);
	}
	void InputAndroid::send_tilt_event(double x, double y, double z)
	{
		if (m_tilt) m_tilt(x, y, z);
	}
	void InputAndroid::send_mouse_scroll_event(double scroll)
	{
		if (m_scroll) m_scroll(scroll);
	}
	void InputAndroid::send_touch_event(FingerEvent fevent, ActionEvent action)
	{
		if (m_touch) m_touch(fevent, action);
	}
	void InputAndroid::send_window_event(WindowEvent wevent)
	{
		if (m_window) m_window(wevent);
	}
	void InputAndroid::send_window_move_event(double x, double y)
	{
		if (m_window_move) m_window_move(x, y);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// KEYS
	//the letters, the digits and the keypad: ranges of both
	static KeyboardEvent translate_range_key(int32_t code)
	{
		if (code >= AKEYCODE_A && code <= AKEYCODE_Z)
		{
			return KeyboardEvent(KEY_A + (code - AKEYCODE_A));
		}
		if (code >= AKEYCODE_0 && code <= AKEYCODE_9)
		{
			return KeyboardEvent(KEY_0 + (code - AKEYCODE_0));
		}
		if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9)
		{
			return KeyboardEvent(KEY_KP_0 + (code - AKEYCODE_NUMPAD_0));
		}
		if (code >= AKEYCODE_F1 && code <= AKEYCODE_F12)
		{
			return KeyboardEvent(KEY_F1 + (code - AKEYCODE_F1));
		}
		return KEY_INVALID;
	}

	//the buttons of a gamepad, as the keys of the menus and of the race (until the engine has its
	//gamepads): A enter, B and start escape, X the power, the triggers forward and backward
	static KeyboardEvent translate_gamepad_key(int32_t code)
	{
		switch (code)
		{
		case AKEYCODE_BUTTON_A:      return KEY_ENTER;
		case AKEYCODE_BUTTON_B:      return KEY_ESCAPE;
		case AKEYCODE_BUTTON_START:  return KEY_ESCAPE;
		case AKEYCODE_BUTTON_X:      return KEY_SPACE;
		case AKEYCODE_BUTTON_R2:     return KEY_UP;
		case AKEYCODE_BUTTON_L2:     return KEY_DOWN;
		case AKEYCODE_DPAD_CENTER:   return KEY_ENTER;
		default:                     return KEY_INVALID;
		}
	}

	static KeyboardEvent translate_key(int32_t code)
	{
		const KeyboardEvent range = translate_range_key(code);
		if (range != KEY_INVALID)
		{
			return range;
		}
		switch (code)
		{
		case AKEYCODE_SPACE:          return KEY_SPACE;
		case AKEYCODE_APOSTROPHE:     return KEY_APOSTROPHE;
		case AKEYCODE_COMMA:          return KEY_COMMA;
		case AKEYCODE_MINUS:          return KEY_MINUS;
		case AKEYCODE_PERIOD:         return KEY_PERIOD;
		case AKEYCODE_SLASH:          return KEY_SLASH;
		case AKEYCODE_SEMICOLON:      return KEY_SEMICOLON;
		case AKEYCODE_EQUALS:         return KEY_EQUAL;
		case AKEYCODE_LEFT_BRACKET:   return KEY_LEFT_BRACKET;
		case AKEYCODE_BACKSLASH:      return KEY_BACKSLASH;
		case AKEYCODE_RIGHT_BRACKET:  return KEY_RIGHT_BRACKET;
		case AKEYCODE_GRAVE:          return KEY_GRAVE_ACCENT;
		//the back of the system: the escape of the game (its menus)
		case AKEYCODE_BACK:           return KEY_ESCAPE;
		case AKEYCODE_ESCAPE:         return KEY_ESCAPE;
		case AKEYCODE_ENTER:          return KEY_ENTER;
		case AKEYCODE_TAB:            return KEY_TAB;
		case AKEYCODE_DEL:            return KEY_BACKSPACE;
		case AKEYCODE_INSERT:         return KEY_INSERT;
		case AKEYCODE_FORWARD_DEL:    return KEY_DELETE;
		case AKEYCODE_DPAD_RIGHT:     return KEY_RIGHT;
		case AKEYCODE_DPAD_LEFT:      return KEY_LEFT;
		case AKEYCODE_DPAD_DOWN:      return KEY_DOWN;
		case AKEYCODE_DPAD_UP:        return KEY_UP;
		case AKEYCODE_PAGE_UP:        return KEY_PAGE_UP;
		case AKEYCODE_PAGE_DOWN:      return KEY_PAGE_DOWN;
		case AKEYCODE_MOVE_HOME:      return KEY_HOME;
		case AKEYCODE_MOVE_END:       return KEY_END;
		case AKEYCODE_CAPS_LOCK:      return KEY_CAPS_LOCK;
		case AKEYCODE_SCROLL_LOCK:    return KEY_SCROLL_LOCK;
		case AKEYCODE_NUM_LOCK:       return KEY_NUM_LOCK;
		case AKEYCODE_SYSRQ:          return KEY_PRINT_SCREEN;
		case AKEYCODE_BREAK:          return KEY_PAUSE;
		case AKEYCODE_NUMPAD_DOT:     return KEY_KP_DECIMAL;
		case AKEYCODE_NUMPAD_DIVIDE:  return KEY_KP_DIVIDE;
		case AKEYCODE_NUMPAD_MULTIPLY:return KEY_KP_MULTIPLY;
		case AKEYCODE_NUMPAD_SUBTRACT:return KEY_KP_SUBTRACT;
		case AKEYCODE_NUMPAD_ADD:     return KEY_KP_ADD;
		case AKEYCODE_NUMPAD_ENTER:   return KEY_KP_ENTER;
		case AKEYCODE_NUMPAD_EQUALS:  return KEY_KP_EQUAL;
		case AKEYCODE_SHIFT_LEFT:     return KEY_LEFT_SHIFT;
		case AKEYCODE_CTRL_LEFT:      return KEY_LEFT_CONTROL;
		case AKEYCODE_ALT_LEFT:       return KEY_LEFT_ALT;
		case AKEYCODE_META_LEFT:      return KEY_LEFT_SUPER;
		case AKEYCODE_SHIFT_RIGHT:    return KEY_RIGHT_SHIFT;
		case AKEYCODE_CTRL_RIGHT:     return KEY_RIGHT_CONTROL;
		case AKEYCODE_ALT_RIGHT:      return KEY_RIGHT_ALT;
		case AKEYCODE_META_RIGHT:     return KEY_RIGHT_SUPER;
		case AKEYCODE_MENU:           return KEY_MENU;
		default:                      return translate_gamepad_key(code);
		}
	}

	static short translate_mods(int32_t meta)
	{
		short mods = 0;
		if (meta & AMETA_SHIFT_ON) mods |= KeyboardModEvent::SHIFT;
		if (meta & AMETA_CTRL_ON)  mods |= KeyboardModEvent::CONTROL;
		if (meta & AMETA_ALT_ON)   mods |= KeyboardModEvent::ALT;
		if (meta & AMETA_META_ON)  mods |= KeyboardModEvent::SUPER;
		return mods;
	}

	//the character of a key of the latin layout (the NDK has not the character map of the
	//keyboard): letters, digits, space; -1 none
	static int character_of(KeyboardEvent key, short mods)
	{
		if (key >= KEY_A && key <= KEY_Z)
		{
			return (mods & KeyboardModEvent::SHIFT) ? int(key) : int(key) - int(KEY_A) + 'a';
		}
		if (key >= KEY_0 && key <= KEY_9)
		{
			return int(key);
		}
		if (key == KEY_SPACE)
		{
			return ' ';
		}
		return -1;
	}

	static int32_t key_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t code = AKeyEvent_getKeyCode(event);
		//the volume of the system: not of the game
		if (code == AKEYCODE_VOLUME_UP || code == AKEYCODE_VOLUME_DOWN || code == AKEYCODE_VOLUME_MUTE)
		{
			return 0;
		}
		const KeyboardEvent key = translate_key(code);
		if (key == KEY_INVALID)
		{
			return 0;
		}
		const int32_t key_action = AKeyEvent_getAction(event);
		if (key_action != AKEY_EVENT_ACTION_DOWN && key_action != AKEY_EVENT_ACTION_UP)
		{
			return 1;
		}
		const short mods = translate_mods(AKeyEvent_getMetaState(event));
		const bool  repeat = AKeyEvent_getRepeatCount(event) > 0;
		ActionEvent action = ActionEvent::RELEASE;
		if (key_action == AKEY_EVENT_ACTION_DOWN)
		{
			action = repeat ? ActionEvent::REPEAT : ActionEvent::PRESS;
			const int character = character_of(key, mods);
			const bool plain = !(mods & (KeyboardModEvent::CONTROL | KeyboardModEvent::ALT));
			if (character != -1)
			{
				input.send_character_event(character, mods, plain);
			}
		}
		input.send_keyboard_event(key, mods, action);
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// GAMEPAD AXES
	//a direction of a stick (or of the hat) as a key held, while it is over its threshold
	struct AxisKey
	{
		int32_t       m_axis;
		float         m_sign;
		KeyboardEvent m_key;
		bool          m_down;
	};
	static AxisKey s_axis_keys[] =
	{
		{ AMOTION_EVENT_AXIS_X,     -1.0f, KEY_LEFT,  false },
		{ AMOTION_EVENT_AXIS_X,      1.0f, KEY_RIGHT, false },
		{ AMOTION_EVENT_AXIS_Y,     -1.0f, KEY_UP,    false },
		{ AMOTION_EVENT_AXIS_Y,      1.0f, KEY_DOWN,  false },
		{ AMOTION_EVENT_AXIS_HAT_X, -1.0f, KEY_LEFT,  false },
		{ AMOTION_EVENT_AXIS_HAT_X,  1.0f, KEY_RIGHT, false },
		{ AMOTION_EVENT_AXIS_HAT_Y, -1.0f, KEY_UP,    false },
		{ AMOTION_EVENT_AXIS_HAT_Y,  1.0f, KEY_DOWN,  false },
	};

	static int32_t joystick_event(InputAndroid& input, const AInputEvent* event)
	{
		//pressed over the half, released under the third (no flicker at the threshold)
		for (AxisKey& axis_key : s_axis_keys)
		{
			const float value = AMotionEvent_getAxisValue(event, axis_key.m_axis, 0) * axis_key.m_sign;
			const bool  down = axis_key.m_down ? value > 0.33f : value > 0.5f;
			if (down != axis_key.m_down)
			{
				axis_key.m_down = down;
				input.send_keyboard_event(axis_key.m_key, 0, down ? ActionEvent::PRESS : ActionEvent::RELEASE);
			}
		}
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// MOUSE
	static MouseButtonEvent translate_mouse_button(int32_t button)
	{
		switch (button)
		{
		case AMOTION_EVENT_BUTTON_PRIMARY:   return MOUSE_BUTTON_LEFT;
		case AMOTION_EVENT_BUTTON_SECONDARY: return MOUSE_BUTTON_RIGHT;
		case AMOTION_EVENT_BUTTON_TERTIARY:  return MOUSE_BUTTON_MIDDLE;
		case AMOTION_EVENT_BUTTON_BACK:      return MOUSE_BUTTON_4;
		case AMOTION_EVENT_BUTTON_FORWARD:   return MOUSE_BUTTON_5;
		default:                             return MOUSE_UNKNOWN;
		}
	}

	//the buttons held by the mouse (its last event): the ones changed are pressed or released
	static int32_t s_mouse_buttons = 0;

	static void mouse_buttons(InputAndroid& input, int32_t buttons)
	{
		const int32_t changed = buttons ^ s_mouse_buttons;
		s_mouse_buttons = buttons;
		for (int32_t bit = 0; bit != 8; ++bit)
		{
			const int32_t mask = 1 << bit;
			const MouseButtonEvent button = translate_mouse_button(mask);
			if ((changed & mask) && button != MOUSE_UNKNOWN)
			{
				input.send_mouse_button_event(button, (buttons & mask) ? ActionEvent::PRESS : ActionEvent::RELEASE);
			}
		}
	}

	static int32_t mouse_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
		input.send_mouse_move_event(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
		mouse_buttons(input, AMotionEvent_getButtonState(event));
		if (action == AMOTION_EVENT_ACTION_SCROLL)
		{
			const float scroll = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
			if (scroll != 0.0f)
			{
				input.send_mouse_scroll_event(scroll);
			}
		}
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// TOUCH
	static FingerEvent finger_of(int32_t pointer_id)
	{
		const int32_t finger = pointer_id + 1;
		return finger < int32_t(FINGERMAX) ? FingerEvent(finger) : FINGER_NULL;
	}

	//a finger down (with where it is): the first one is also the mouse (its left button), after it
	//(who reads the fingers knows the mouse that follows is a finger)
	static void touch_down(InputAndroid& input, const AInputEvent* event, size_t index)
	{
		const int32_t id = AMotionEvent_getPointerId(event, index);
		const FingerEvent finger = finger_of(id);
		if (finger != FINGER_NULL)
		{
			input.send_touch_event(finger, ActionEvent::PRESS);
			input.send_finger_event(finger, ActionEvent::PRESS, AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
		}
		if (s_os_context.m_mouse_pointer == -1)
		{
			s_os_context.m_mouse_pointer = id;
			input.send_mouse_move_event(AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
			input.send_mouse_button_event(MOUSE_BUTTON_LEFT, ActionEvent::PRESS);
		}
	}

	//a finger up: the mouse first (the finger still down), then the finger
	static void touch_up(InputAndroid& input, const AInputEvent* event, size_t index)
	{
		const int32_t id = AMotionEvent_getPointerId(event, index);
		if (s_os_context.m_mouse_pointer == id)
		{
			s_os_context.m_mouse_pointer = -1;
			input.send_mouse_move_event(AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
			input.send_mouse_button_event(MOUSE_BUTTON_LEFT, ActionEvent::RELEASE);
		}
		const FingerEvent finger = finger_of(id);
		if (finger != FINGER_NULL)
		{
			input.send_touch_event(finger, ActionEvent::RELEASE);
			input.send_finger_event(finger, ActionEvent::RELEASE, AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
		}
	}

	//the fingers moved: each one (REPEAT), the mouse the first one
	static void touch_move(InputAndroid& input, const AInputEvent* event)
	{
		const size_t count = AMotionEvent_getPointerCount(event);
		for (size_t index = 0; index != count; ++index)
		{
			const int32_t id = AMotionEvent_getPointerId(event, index);
			const FingerEvent finger = finger_of(id);
			if (finger != FINGER_NULL)
			{
				input.send_finger_event(finger, ActionEvent::REPEAT, AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
			}
			if (id == s_os_context.m_mouse_pointer)
			{
				input.send_mouse_move_event(AMotionEvent_getX(event, index), AMotionEvent_getY(event, index));
			}
		}
	}

	//the gesture stopped by the system: every finger up
	static void touch_cancel(InputAndroid& input, const AInputEvent* event)
	{
		const size_t count = AMotionEvent_getPointerCount(event);
		for (size_t index = 0; index != count; ++index)
		{
			touch_up(input, event, index);
		}
	}

	static int32_t touch_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t full_action = AMotionEvent_getAction(event);
		const int32_t action = full_action & AMOTION_EVENT_ACTION_MASK;
		const size_t  index = size_t((full_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
		switch (action)
		{
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			touch_down(input, event, index);
			break;
		case AMOTION_EVENT_ACTION_UP:
		case AMOTION_EVENT_ACTION_POINTER_UP:
			touch_up(input, event, index);
			break;
		case AMOTION_EVENT_ACTION_MOVE:
			touch_move(input, event);
			break;
		case AMOTION_EVENT_ACTION_CANCEL:
			touch_cancel(input, event);
			break;
		default: break;
		}
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	static bool is_source(int32_t source, int32_t kind)
	{
		return (source & kind) == kind;
	}

	int32_t input_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t type = AInputEvent_getType(event);
		const int32_t source = AInputEvent_getSource(event);
		if (type == AINPUT_EVENT_TYPE_KEY)
		{
			return key_event(input, event);
		}
		if (type != AINPUT_EVENT_TYPE_MOTION)
		{
			return 0;
		}
		if (is_source(source, AINPUT_SOURCE_JOYSTICK))
		{
			return joystick_event(input, event);
		}
		if (is_source(source, AINPUT_SOURCE_MOUSE))
		{
			return mouse_event(input, event);
		}
		if (is_source(source, AINPUT_SOURCE_TOUCHSCREEN))
		{
			return touch_event(input, event);
		}
		return 0;
	}
}
}
}
