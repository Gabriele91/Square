//
//  DebugOptions.h
//  Square
//
//  What the debug panel of the engine (F1, UISystem) shows: options in sections. An option is
//  a toggle, a choice (one of its names), a button or a text (read every frame); it reads its
//  value from what it stands for every frame (a change made elsewhere shows) and writes it when
//  its control changes. The engine fills its tabs (Pipeline: the post effects declare theirs,
//  PostEffect::debug_options; Debug draw; UI); a game adds sections to them or tabs of its own
//  (UISystem::debug_sections).
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Square/Config.h"

namespace Square
{
	struct DebugOption
	{
		enum class Type : unsigned char
		{
			TOGGLE,
			CHOICE,
			BUTTON,
			TEXT
		};

		Type                         m_type{ Type::TEXT };
		std::string                  m_name;
		std::vector<std::string>     m_choices;  //CHOICE: its values by index
		std::function<int()>         m_get;      //TOGGLE: 0 / 1, CHOICE: the index
		std::function<void(int)>     m_set;      //TOGGLE: 0 / 1, CHOICE: the index, BUTTON: pressed
		std::function<std::string()> m_text;     //TEXT

		static DebugOption toggle(const std::string& name, std::function<bool()> get, std::function<void(bool)> set)
		{
			DebugOption option;
			option.m_type = Type::TOGGLE;
			option.m_name = name;
			option.m_get = [get]() { return get() ? 1 : 0; };
			option.m_set = [set](int value) { set(value != 0); };
			return option;
		}

		static DebugOption choice(const std::string& name, const std::vector<std::string>& choices, std::function<int()> get, std::function<void(int)> set)
		{
			DebugOption option;
			option.m_type = Type::CHOICE;
			option.m_name = name;
			option.m_choices = choices;
			option.m_get = get;
			option.m_set = set;
			return option;
		}

		static DebugOption button(const std::string& name, std::function<void()> press)
		{
			DebugOption option;
			option.m_type = Type::BUTTON;
			option.m_name = name;
			option.m_set = [press](int) { press(); };
			return option;
		}

		static DebugOption text(const std::string& name, std::function<std::string()> text)
		{
			DebugOption option;
			option.m_type = Type::TEXT;
			option.m_name = name;
			option.m_text = text;
			return option;
		}
	};

	struct DebugSection
	{
		std::string              m_title;
		std::vector<DebugOption> m_options;
	};

	//the sections of a tab of the debug panel, asked when the panel is made again (it opens, the
	//worlds or their effects change)
	using DebugSections = std::function<void(std::vector<DebugSection>& sections)>;
}
