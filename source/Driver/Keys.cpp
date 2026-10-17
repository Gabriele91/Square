//
//  Keys.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <array>
#include "Square/Config.h"
#include "Square/Driver/Input.h"

namespace Square
{
namespace Video
{
namespace AuxKeys
{
	//name of a key
	struct Name
	{
		KeyboardEvent key;
		StringView    name;
	};

	//names of the keys (KeyboardEvent)
	static constexpr Name s_names[] =
	{
		{ KEY_SPACE,         "SPACE" },
		{ KEY_APOSTROPHE,    "'" },
		{ KEY_COMMA,         "," },
		{ KEY_MINUS,         "-" },
		{ KEY_PERIOD,        "." },
		{ KEY_SLASH,         "/" },
		{ KEY_0,             "0" },
		{ KEY_1,             "1" },
		{ KEY_2,             "2" },
		{ KEY_3,             "3" },
		{ KEY_4,             "4" },
		{ KEY_5,             "5" },
		{ KEY_6,             "6" },
		{ KEY_7,             "7" },
		{ KEY_8,             "8" },
		{ KEY_9,             "9" },
		{ KEY_SEMICOLON,     ";" },
		{ KEY_EQUAL,         "=" },
		{ KEY_A,             "A" },
		{ KEY_B,             "B" },
		{ KEY_C,             "C" },
		{ KEY_D,             "D" },
		{ KEY_E,             "E" },
		{ KEY_F,             "F" },
		{ KEY_G,             "G" },
		{ KEY_H,             "H" },
		{ KEY_I,             "I" },
		{ KEY_J,             "J" },
		{ KEY_K,             "K" },
		{ KEY_L,             "L" },
		{ KEY_M,             "M" },
		{ KEY_N,             "N" },
		{ KEY_O,             "O" },
		{ KEY_P,             "P" },
		{ KEY_Q,             "Q" },
		{ KEY_R,             "R" },
		{ KEY_S,             "S" },
		{ KEY_T,             "T" },
		{ KEY_U,             "U" },
		{ KEY_V,             "V" },
		{ KEY_W,             "W" },
		{ KEY_X,             "X" },
		{ KEY_Y,             "Y" },
		{ KEY_Z,             "Z" },
		{ KEY_LEFT_BRACKET,  "[" },
		{ KEY_BACKSLASH,     "\\" },
		{ KEY_RIGHT_BRACKET, "]" },
		{ KEY_GRAVE_ACCENT,  "`" },
		{ KEY_WORLD_1,       "WORLD 1" },
		{ KEY_WORLD_2,       "WORLD 2" },
		{ KEY_ESCAPE,        "ESCAPE" },
		{ KEY_ENTER,         "ENTER" },
		{ KEY_TAB,           "TAB" },
		{ KEY_BACKSPACE,     "BACKSPACE" },
		{ KEY_INSERT,        "INSERT" },
		{ KEY_DELETE,        "DELETE" },
		{ KEY_RIGHT,         "RIGHT" },
		{ KEY_LEFT,          "LEFT" },
		{ KEY_DOWN,          "DOWN" },
		{ KEY_UP,            "UP" },
		{ KEY_PAGE_UP,       "PAGE UP" },
		{ KEY_PAGE_DOWN,     "PAGE DOWN" },
		{ KEY_HOME,          "HOME" },
		{ KEY_END,           "END" },
		{ KEY_CAPS_LOCK,     "CAPS LOCK" },
		{ KEY_SCROLL_LOCK,   "SCROLL LOCK" },
		{ KEY_NUM_LOCK,      "NUM LOCK" },
		{ KEY_PRINT_SCREEN,  "PRINT SCREEN" },
		{ KEY_PAUSE,         "PAUSE" },
		{ KEY_F1,            "F1" },
		{ KEY_F2,            "F2" },
		{ KEY_F3,            "F3" },
		{ KEY_F4,            "F4" },
		{ KEY_F5,            "F5" },
		{ KEY_F6,            "F6" },
		{ KEY_F7,            "F7" },
		{ KEY_F8,            "F8" },
		{ KEY_F9,            "F9" },
		{ KEY_F10,           "F10" },
		{ KEY_F11,           "F11" },
		{ KEY_F12,           "F12" },
		{ KEY_F13,           "F13" },
		{ KEY_F14,           "F14" },
		{ KEY_F15,           "F15" },
		{ KEY_F16,           "F16" },
		{ KEY_F17,           "F17" },
		{ KEY_F18,           "F18" },
		{ KEY_F19,           "F19" },
		{ KEY_F20,           "F20" },
		{ KEY_F21,           "F21" },
		{ KEY_F22,           "F22" },
		{ KEY_F23,           "F23" },
		{ KEY_F24,           "F24" },
		{ KEY_F25,           "F25" },
		{ KEY_KP_0,          "NUM 0" },
		{ KEY_KP_1,          "NUM 1" },
		{ KEY_KP_2,          "NUM 2" },
		{ KEY_KP_3,          "NUM 3" },
		{ KEY_KP_4,          "NUM 4" },
		{ KEY_KP_5,          "NUM 5" },
		{ KEY_KP_6,          "NUM 6" },
		{ KEY_KP_7,          "NUM 7" },
		{ KEY_KP_8,          "NUM 8" },
		{ KEY_KP_9,          "NUM 9" },
		{ KEY_KP_DECIMAL,    "NUM DECIMAL" },
		{ KEY_KP_DIVIDE,     "NUM DIVIDE" },
		{ KEY_KP_MULTIPLY,   "NUM MULTIPLY" },
		{ KEY_KP_SUBTRACT,   "NUM SUBTRACT" },
		{ KEY_KP_ADD,        "NUM ADD" },
		{ KEY_KP_ENTER,      "NUM ENTER" },
		{ KEY_KP_EQUAL,      "NUM EQUAL" },
		{ KEY_LEFT_SHIFT,    "LEFT SHIFT" },
		{ KEY_LEFT_CONTROL,  "LEFT CONTROL" },
		{ KEY_LEFT_ALT,      "LEFT ALT" },
		{ KEY_LEFT_SUPER,    "LEFT SUPER" },
		{ KEY_RIGHT_SHIFT,   "RIGHT SHIFT" },
		{ KEY_RIGHT_CONTROL, "RIGHT CONTROL" },
		{ KEY_RIGHT_ALT,     "RIGHT ALT" },
		{ KEY_RIGHT_SUPER,   "RIGHT SUPER" },
		{ KEY_MENU,          "MENU" }
	};

	//all the keys: the name of a key at its value ("": no name)
	static constexpr size_t s_count = size_t(KEY_LAST) + 1;
	using Table = std::array<StringView, s_count>;

	static constexpr Table make_table()
	{
		Table table{};
		for (const Name& entry : s_names)
		{
			table[size_t(entry.key)] = entry.name;
		}
		return table;
	}

	static constexpr Table s_table = make_table();
}
	//////////////////////////////////////////////////////////////
	//serialize
	StringView key_to_string(KeyboardEvent key)
	{
		if (key < 0 || size_t(key) >= AuxKeys::s_count)
		{
			return StringView();
		}
		return AuxKeys::s_table[size_t(key)];
	}

	KeyboardEvent key_from_string(StringView key, KeyboardEvent key_default)
	{
		for (const AuxKeys::Name& entry : AuxKeys::s_names)
		{
			if (entry.name == key)
			{
				return entry.key;
			}
		}
		return key_default;
	}
}
}
