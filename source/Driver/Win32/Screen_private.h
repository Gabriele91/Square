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
#include <windows.h>

namespace Square
{
namespace Video
{
namespace Win32
{
	struct ScreenWin32
	{
		bool m_only_display;
		//screen name
		TCHAR    m_display_name[32];
		TCHAR	 m_monitor_name[32];
		HMONITOR m_monitor{ nullptr };
		//set name
		ScreenWin32(const TCHAR display[32]);
		ScreenWin32(const TCHAR display[32], const TCHAR monitor[32]);
		//utility
		void get_position(int& xpos, int& ypos) const;
		void get_size(unsigned int& width, unsigned int& height) const;
		void get_monitor_size(unsigned int& width, unsigned int& height) const;

	protected:

		bool try_to_get_monitor_hendler();

	};
}
}
}