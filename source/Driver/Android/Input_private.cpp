//
//  Square
//
//  Android: the input of the window (the events of the activity): keyboard keys, gamepads
//  (their keys and their sticks, as gamepad events), mouse, touch screen (its first finger also
//  as the mouse: the UI).
//
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"
#include <android/keycodes.h>
#include <algorithm>

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

	////////////////////////////////////////////////////////////////////////////////////
	// GAMEPADS
	//its slot (a device not seen yet: the first free one, connected; none free: -1)
	static int gamepad_of(InputAndroid& input, int32_t device)
	{
		for (int pad = 0; pad != GAMEPAD_MAX; ++pad)
		{
			if (input.m_gamepad_devices[size_t(pad)] == device)
			{
				return pad;
			}
		}
		for (int pad = 0; pad != GAMEPAD_MAX; ++pad)
		{
			if (input.m_gamepad_devices[size_t(pad)] == -1)
			{
				input.m_gamepad_devices[size_t(pad)] = device;
				input.m_gamepad_states[size_t(pad)] = GamepadState();
				input.m_gamepad_states[size_t(pad)].connected = true;
				return pad;
			}
		}
		return -1;
	}

	//the keys of a gamepad as its buttons (the triggers: also from their axes)
	struct PadButtonOf
	{
		int32_t       m_code;
		GamepadButton m_button;
	};
	static const PadButtonOf s_pad_buttons[] =
	{
		{ AKEYCODE_BUTTON_A,      GAMEPAD_A              },
		{ AKEYCODE_BUTTON_B,      GAMEPAD_B              },
		{ AKEYCODE_BUTTON_X,      GAMEPAD_X              },
		{ AKEYCODE_BUTTON_Y,      GAMEPAD_Y              },
		{ AKEYCODE_BUTTON_L1,     GAMEPAD_LEFT_SHOULDER  },
		{ AKEYCODE_BUTTON_R1,     GAMEPAD_RIGHT_SHOULDER },
		{ AKEYCODE_BUTTON_L2,     GAMEPAD_LEFT_TRIGGER   },
		{ AKEYCODE_BUTTON_R2,     GAMEPAD_RIGHT_TRIGGER  },
		{ AKEYCODE_BUTTON_SELECT, GAMEPAD_BACK           },
		{ AKEYCODE_BACK,          GAMEPAD_BACK           },
		{ AKEYCODE_BUTTON_START,  GAMEPAD_START          },
		{ AKEYCODE_BUTTON_THUMBL, GAMEPAD_LEFT_THUMB     },
		{ AKEYCODE_BUTTON_THUMBR, GAMEPAD_RIGHT_THUMB    },
		{ AKEYCODE_DPAD_UP,       GAMEPAD_DPAD_UP        },
		{ AKEYCODE_DPAD_DOWN,     GAMEPAD_DPAD_DOWN      },
		{ AKEYCODE_DPAD_LEFT,     GAMEPAD_DPAD_LEFT      },
		{ AKEYCODE_DPAD_RIGHT,    GAMEPAD_DPAD_RIGHT     },
	};

	static GamepadButton pad_button_of(int32_t code)
	{
		for (const PadButtonOf& button : s_pad_buttons)
		{
			if (button.m_code == code)
			{
				return button.m_button;
			}
		}
		return GAMEPAD_UNKNOWN;
	}

	//a key of a gamepad (its source a gamepad, or a key only of gamepads): 1 used
	static int32_t gamepad_key_event(InputAndroid& input, const AInputEvent* event, int32_t code)
	{
		const GamepadButton button = pad_button_of(code);
		const bool from_pad = (AInputEvent_getSource(event) & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD;
		const bool pad_only = code >= AKEYCODE_BUTTON_A && code <= AKEYCODE_BUTTON_MODE;
		if (button == GAMEPAD_UNKNOWN || !(from_pad || pad_only))
		{
			return 0;
		}
		const int pad = gamepad_of(input, AInputEvent_getDeviceId(event));
		const int32_t key_action = AKeyEvent_getAction(event);
		if (pad >= 0 && (key_action == AKEY_EVENT_ACTION_DOWN || key_action == AKEY_EVENT_ACTION_UP))
		{
			GamepadState& state = input.m_gamepad_states[size_t(pad)];
			state.buttons[size_t(button)] = key_action == AKEY_EVENT_ACTION_DOWN;
			input.m_gamepads.send(pad, state);
		}
		return 1;
	}

	static int32_t key_event(InputAndroid& input, const AInputEvent* event)
	{
		const int32_t code = AKeyEvent_getKeyCode(event);
		if (is_system_key(code))
		{
			return 0;
		}
		if (gamepad_key_event(input, event, code))
		{
			return 1;
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
	//its sticks (y up: Android down), its triggers (some pads: brake and gas), its hat as the
	//buttons of its pad
	static constexpr float s_stick_dead = 0.15f;
	static constexpr float s_trigger_dead = 0.05f;

	//a key down or up
	static ActionEvent action_of(bool down)
	{
		if (down)
		{
			return ActionEvent::PRESS;
		}
		return ActionEvent::RELEASE;
	}

	static int32_t joystick_event(InputAndroid& input, const AInputEvent* event)
	{
		const int pad = gamepad_of(input, AInputEvent_getDeviceId(event));
		if (pad < 0)
		{
			return 1;
		}
		auto axis = [event](int32_t which) { return AMotionEvent_getAxisValue(event, which, 0); };
		GamepadState& state = input.m_gamepad_states[size_t(pad)];
		gamepad_stick(axis(AMOTION_EVENT_AXIS_X), -axis(AMOTION_EVENT_AXIS_Y), s_stick_dead, state.axes[GAMEPAD_LEFT_X], state.axes[GAMEPAD_LEFT_Y]);
		gamepad_stick(axis(AMOTION_EVENT_AXIS_Z), -axis(AMOTION_EVENT_AXIS_RZ), s_stick_dead, state.axes[GAMEPAD_RIGHT_X], state.axes[GAMEPAD_RIGHT_Y]);
		const float left = std::max(axis(AMOTION_EVENT_AXIS_LTRIGGER), axis(AMOTION_EVENT_AXIS_BRAKE));
		const float right = std::max(axis(AMOTION_EVENT_AXIS_RTRIGGER), axis(AMOTION_EVENT_AXIS_GAS));
		state.axes[GAMEPAD_LEFT_TRIGGER_AXIS] = gamepad_trigger(left, s_trigger_dead);
		state.axes[GAMEPAD_RIGHT_TRIGGER_AXIS] = gamepad_trigger(right, s_trigger_dead);
		//(its hat: the d-pad of the pads that do not send it as keys)
		const float hat_x = axis(AMOTION_EVENT_AXIS_HAT_X);
		const float hat_y = axis(AMOTION_EVENT_AXIS_HAT_Y);
		state.buttons[GAMEPAD_DPAD_LEFT] = hat_x < -0.5f;
		state.buttons[GAMEPAD_DPAD_RIGHT] = hat_x > 0.5f;
		state.buttons[GAMEPAD_DPAD_UP] = hat_y < -0.5f;
		state.buttons[GAMEPAD_DPAD_DOWN] = hat_y > 0.5f;
		input.m_gamepads.send(pad, state);
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

	//finger down (the first one: also the mouse, after it)
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

	//finger up (its mouse before it)
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

	//fingers moved
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
