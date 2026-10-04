//
//  UIOption.h
//  Rush
//
//  An option of a menu in sync with the game: m_value is the one of its control (check box,
//  select), bound to the data model, m_shown the last one it had. sync_option every frame: the
//  control changed, to the game; the game changed (a key, a load), to the control.
//
#pragma once

template < typename T >
struct UIOption
{
	T m_value{};
	T m_shown{};
};

template < typename T, typename Apply >
void sync_option(UIOption<T>& option, T game, Apply apply)
{
	//the control changed: to the game
	if (option.m_value != option.m_shown)
	{
		apply(option.m_value);
		option.m_shown = option.m_value;
		return;
	}
	//the game changed: to the control
	if (option.m_value != game)
	{
		option.m_value = game;
		option.m_shown = game;
	}
}
