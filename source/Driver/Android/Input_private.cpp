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
		if (m_keyboard)
		{
			m_keyboard(kevent, mode, action);
		}
	}
	void InputAndroid::send_character_event(int character, short mode, bool plain)
	{
		if (m_character)
		{
			m_character(character, mode, plain);
		}
	}
	void InputAndroid::send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
	{
		if (m_mouse_button)
		{
			m_mouse_button(mevent, action);
		}
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
		if (m_mouse_move)
		{
			m_mouse_move(x, y);
		}
	}
	void InputAndroid::send_finger_event(FingerEvent finger, ActionEvent action, double x, double y)
	{
		to_frame(m_window_ref, x, y);
		if (m_finger)
		{
			m_finger(finger, action, x, y);
		}
	}
	void InputAndroid::send_tilt_event(double x, double y, double z)
	{
		if (m_tilt)
		{
			m_tilt(x, y, z);
		}
	}
	void InputAndroid::send_mouse_scroll_event(double scroll)
	{
		if (m_scroll)
		{
			m_scroll(scroll);
		}
	}
	void InputAndroid::send_touch_event(FingerEvent fevent, ActionEvent action)
	{
		if (m_touch)
		{
			m_touch(fevent, action);
		}
	}
	void InputAndroid::send_window_event(WindowEvent wevent)
	{
		if (m_window)
		{
			m_window(wevent);
		}
	}
	void InputAndroid::send_window_move_event(double x, double y)
	{
		if (m_window_move)
		{
			m_window_move(x, y);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// KEYS
	//a key of Android as a key of the engine
	struct KeyOf
	{
		int32_t       m_code;
		KeyboardEvent m_key;
	};
	//a range of keys of Android (its first, its last) as the keys of the engine from the first
	struct KeyRange
	{
		int32_t       m_first;
		int32_t       m_last;
		KeyboardEvent m_key;
	};
	//a state of a modifier of Android as the one of the engine
	struct ModOf
	{
		int32_t          m_meta;
		KeyboardModEvent m_mod;
	};

	//the letters, the digits, the keypad, the functions
	static const KeyRange s_key_ranges[] =
	{
		{ AKEYCODE_A,        AKEYCODE_Z,        KEY_A    },
		{ AKEYCODE_0,        AKEYCODE_9,        KEY_0    },
		{ AKEYCODE_NUMPAD_0, AKEYCODE_NUMPAD_9, KEY_KP_0 },
		{ AKEYCODE_F1,       AKEYCODE_F12,      KEY_F1   },
	};

	//the other keys: the back of the system is the escape of the game (its menus); the buttons of a
	//gamepad are the keys of the menus and of the race (until the engine has its gamepads): A enter,
	//B and start escape, X the power, the triggers forward and backward
	static const KeyOf s_keys[] =
	{
		{ AKEYCODE_SPACE,           KEY_SPACE         },
		{ AKEYCODE_APOSTROPHE,      KEY_APOSTROPHE    },
		{ AKEYCODE_COMMA,           KEY_COMMA         },
		{ AKEYCODE_MINUS,           KEY_MINUS         },
		{ AKEYCODE_PERIOD,          KEY_PERIOD        },
		{ AKEYCODE_SLASH,           KEY_SLASH         },
		{ AKEYCODE_SEMICOLON,       KEY_SEMICOLON     },
		{ AKEYCODE_EQUALS,          KEY_EQUAL         },
		{ AKEYCODE_LEFT_BRACKET,    KEY_LEFT_BRACKET  },
		{ AKEYCODE_BACKSLASH,       KEY_BACKSLASH     },
		{ AKEYCODE_RIGHT_BRACKET,   KEY_RIGHT_BRACKET },
		{ AKEYCODE_GRAVE,           KEY_GRAVE_ACCENT  },
		{ AKEYCODE_BACK,            KEY_ESCAPE        },
		{ AKEYCODE_ESCAPE,          KEY_ESCAPE        },
		{ AKEYCODE_ENTER,           KEY_ENTER         },
		{ AKEYCODE_TAB,             KEY_TAB           },
		{ AKEYCODE_DEL,             KEY_BACKSPACE     },
		{ AKEYCODE_INSERT,          KEY_INSERT        },
		{ AKEYCODE_FORWARD_DEL,     KEY_DELETE        },
		{ AKEYCODE_DPAD_RIGHT,      KEY_RIGHT         },
		{ AKEYCODE_DPAD_LEFT,       KEY_LEFT          },
		{ AKEYCODE_DPAD_DOWN,       KEY_DOWN          },
		{ AKEYCODE_DPAD_UP,         KEY_UP            },
		{ AKEYCODE_PAGE_UP,         KEY_PAGE_UP       },
		{ AKEYCODE_PAGE_DOWN,       KEY_PAGE_DOWN     },
		{ AKEYCODE_MOVE_HOME,       KEY_HOME          },
		{ AKEYCODE_MOVE_END,        KEY_END           },
		{ AKEYCODE_CAPS_LOCK,       KEY_CAPS_LOCK     },
		{ AKEYCODE_SCROLL_LOCK,     KEY_SCROLL_LOCK   },
		{ AKEYCODE_NUM_LOCK,        KEY_NUM_LOCK      },
		{ AKEYCODE_SYSRQ,           KEY_PRINT_SCREEN  },
		{ AKEYCODE_BREAK,           KEY_PAUSE         },
		{ AKEYCODE_NUMPAD_DOT,      KEY_KP_DECIMAL    },
		{ AKEYCODE_NUMPAD_DIVIDE,   KEY_KP_DIVIDE     },
		{ AKEYCODE_NUMPAD_MULTIPLY, KEY_KP_MULTIPLY   },
		{ AKEYCODE_NUMPAD_SUBTRACT, KEY_KP_SUBTRACT   },
		{ AKEYCODE_NUMPAD_ADD,      KEY_KP_ADD        },
		{ AKEYCODE_NUMPAD_ENTER,    KEY_KP_ENTER      },
		{ AKEYCODE_NUMPAD_EQUALS,   KEY_KP_EQUAL      },
		{ AKEYCODE_SHIFT_LEFT,      KEY_LEFT_SHIFT    },
		{ AKEYCODE_CTRL_LEFT,       KEY_LEFT_CONTROL  },
		{ AKEYCODE_ALT_LEFT,        KEY_LEFT_ALT      },
		{ AKEYCODE_META_LEFT,       KEY_LEFT_SUPER    },
		{ AKEYCODE_SHIFT_RIGHT,     KEY_RIGHT_SHIFT   },
		{ AKEYCODE_CTRL_RIGHT,      KEY_RIGHT_CONTROL },
		{ AKEYCODE_ALT_RIGHT,       KEY_RIGHT_ALT     },
		{ AKEYCODE_META_RIGHT,      KEY_RIGHT_SUPER   },
		{ AKEYCODE_MENU,            KEY_MENU          },
		{ AKEYCODE_BUTTON_A,        KEY_ENTER         },
		{ AKEYCODE_BUTTON_B,        KEY_ESCAPE        },
		{ AKEYCODE_BUTTON_START,    KEY_ESCAPE        },
		{ AKEYCODE_BUTTON_X,        KEY_SPACE         },
		{ AKEYCODE_BUTTON_R2,       KEY_UP            },
		{ AKEYCODE_BUTTON_L2,       KEY_DOWN          },
		{ AKEYCODE_DPAD_CENTER,     KEY_ENTER         },
	};

	//the keys of the system, not of the game (its volume)
	static const int32_t s_system_keys[] =
	{
		AKEYCODE_VOLUME_UP,
		AKEYCODE_VOLUME_DOWN,
		AKEYCODE_VOLUME_MUTE,
	};

	static const ModOf s_mods[] =
	{
		{ AMETA_SHIFT_ON, KeyboardModEvent::SHIFT   },
		{ AMETA_CTRL_ON,  KeyboardModEvent::CONTROL },
		{ AMETA_ALT_ON,   KeyboardModEvent::ALT     },
		{ AMETA_META_ON,  KeyboardModEvent::SUPER   },
	};

	static KeyboardEvent translate_key(int32_t code)
	{
		for (const KeyRange& range : s_key_ranges)
		{
			if (code >= range.m_first && code <= range.m_last)
			{
				return KeyboardEvent(range.m_key + (code - range.m_first));
			}
		}
		for (const KeyOf& key : s_keys)
		{
			if (key.m_code == code)
			{
				return key.m_key;
			}
		}
		return KEY_INVALID;
	}

	static bool is_system_key(int32_t code)
	{
		for (int32_t system_key : s_system_keys)
		{
			if (system_key == code)
			{
				return true;
			}
		}
		return false;
	}

	static short translate_mods(int32_t meta)
	{
		short mods = 0;
		for (const ModOf& mod : s_mods)
		{
			if (meta & mod.m_meta)
			{
				mods |= mod.m_mod;
			}
		}
		return mods;
	}

	//the character of a key of the latin layout (the NDK has not the character map of the
	//keyboard): letters, digits, space; -1 none
	static int character_of(KeyboardEvent key, short mods)
	{
		if (key >= KEY_A && key <= KEY_Z)
		{
			//(the keys are the upper case letters)
			if (mods & KeyboardModEvent::SHIFT)
			{
				return int(key);
			}
			return int(key) - int(KEY_A) + 'a';
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
		if (is_system_key(code))
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
			action = ActionEvent::PRESS;
			if (repeat)
			{
				action = ActionEvent::REPEAT;
			}
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

	//a key down or up
	static ActionEvent action_of(bool down)
	{
		if (down)
		{
			return ActionEvent::PRESS;
		}
		return ActionEvent::RELEASE;
	}

	//a direction held: pressed over the half, released under the third (no flicker at the
	//threshold)
	static bool axis_down(const AxisKey& axis_key, float value)
	{
		if (axis_key.m_down)
		{
			return value > 0.33f;
		}
		return value > 0.5f;
	}

	static int32_t joystick_event(InputAndroid& input, const AInputEvent* event)
	{
		for (AxisKey& axis_key : s_axis_keys)
		{
			const float axis = AMotionEvent_getAxisValue(event, axis_key.m_axis, 0);
			const bool  down = axis_down(axis_key, axis * axis_key.m_sign);
			if (down != axis_key.m_down)
			{
				axis_key.m_down = down;
				input.send_keyboard_event(axis_key.m_key, 0, action_of(down));
			}
		}
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// MOUSE
	//a button of Android (its bit in the state of the buttons) as the one of the engine
	struct MouseButtonOf
	{
		int32_t          m_mask;
		MouseButtonEvent m_button;
	};
	static const MouseButtonOf s_mouse_buttons_of[] =
	{
		{ AMOTION_EVENT_BUTTON_PRIMARY,   MOUSE_BUTTON_LEFT   },
		{ AMOTION_EVENT_BUTTON_SECONDARY, MOUSE_BUTTON_RIGHT  },
		{ AMOTION_EVENT_BUTTON_TERTIARY,  MOUSE_BUTTON_MIDDLE },
		{ AMOTION_EVENT_BUTTON_BACK,      MOUSE_BUTTON_4      },
		{ AMOTION_EVENT_BUTTON_FORWARD,   MOUSE_BUTTON_5      },
	};

	//the buttons held by the mouse (its last event): the ones changed are pressed or released
	static int32_t s_mouse_buttons = 0;

	static void mouse_buttons(InputAndroid& input, int32_t buttons)
	{
		const int32_t changed = buttons ^ s_mouse_buttons;
		s_mouse_buttons = buttons;
		for (const MouseButtonOf& button : s_mouse_buttons_of)
		{
			if (changed & button.m_mask)
			{
				const bool down = (buttons & button.m_mask) != 0;
				input.send_mouse_button_event(button.m_button, action_of(down));
			}
		}
	}

	static int32_t mouse_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
		const float x = AMotionEvent_getX(event, 0);
		const float y = AMotionEvent_getY(event, 0);
		input.send_mouse_move_event(x, y);
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
		if (finger >= int32_t(FINGERMAX))
		{
			return FINGER_NULL;
		}
		return FingerEvent(finger);
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
			const float x = AMotionEvent_getX(event, index);
			const float y = AMotionEvent_getY(event, index);
			input.send_finger_event(finger, ActionEvent::PRESS, x, y);
		}
		if (s_os_context.m_mouse_pointer == -1)
		{
			s_os_context.m_mouse_pointer = id;
			const float x = AMotionEvent_getX(event, index);
			const float y = AMotionEvent_getY(event, index);
			input.send_mouse_move_event(x, y);
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
			const float x = AMotionEvent_getX(event, index);
			const float y = AMotionEvent_getY(event, index);
			input.send_mouse_move_event(x, y);
			input.send_mouse_button_event(MOUSE_BUTTON_LEFT, ActionEvent::RELEASE);
		}
		const FingerEvent finger = finger_of(id);
		if (finger != FINGER_NULL)
		{
			input.send_touch_event(finger, ActionEvent::RELEASE);
			const float x = AMotionEvent_getX(event, index);
			const float y = AMotionEvent_getY(event, index);
			input.send_finger_event(finger, ActionEvent::RELEASE, x, y);
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
				const float x = AMotionEvent_getX(event, index);
				const float y = AMotionEvent_getY(event, index);
				input.send_finger_event(finger, ActionEvent::REPEAT, x, y);
			}
			if (id == s_os_context.m_mouse_pointer)
			{
				const float x = AMotionEvent_getX(event, index);
				const float y = AMotionEvent_getY(event, index);
				input.send_mouse_move_event(x, y);
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
		//the pointer of the action (its index in the event)
		const int32_t pointer_bits = full_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK;
		const size_t  index = size_t(pointer_bits >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
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
	//a source of the motions (its bits) and who reads its events (the first one that matches)
	struct MotionSource
	{
		int32_t m_source;
		int32_t (*m_event)(InputAndroid& input, const AInputEvent* event);
	};
	static const MotionSource s_motion_sources[] =
	{
		{ AINPUT_SOURCE_JOYSTICK,    joystick_event },
		{ AINPUT_SOURCE_MOUSE,       mouse_event    },
		{ AINPUT_SOURCE_TOUCHSCREEN, touch_event    },
	};

	static int32_t motion_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t source = AInputEvent_getSource(event);
		for (const MotionSource& motion : s_motion_sources)
		{
			if ((source & motion.m_source) == motion.m_source)
			{
				return motion.m_event(input, event);
			}
		}
		return 0;
	}

	int32_t input_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t type = AInputEvent_getType(event);
		if (type == AINPUT_EVENT_TYPE_KEY)
		{
			return key_event(input, event);
		}
		if (type == AINPUT_EVENT_TYPE_MOTION)
		{
			return motion_event(input, event);
		}
		return 0;
	}
}
}
}
