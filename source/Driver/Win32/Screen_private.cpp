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
#include "Screen_private.h"

namespace Square
{
namespace Video
{
namespace Win32
{
	///////////////////////////////////////////////////////////////////////////////////
	// Screen Window
	ScreenWin32::ScreenWin32(const TCHAR display[32])
	{
		m_only_display = true;
		std::memcpy(m_display_name, display, sizeof(m_display_name));
		try_to_get_monitor_hendler();
	}
	ScreenWin32::ScreenWin32(const TCHAR display[32], const TCHAR monitor[32])
	{
		m_only_display = false;
		std::memcpy(m_display_name, display, sizeof(m_display_name));
		std::memcpy(m_monitor_name, monitor, sizeof(m_monitor_name));
		try_to_get_monitor_hendler();
	}
	//utility
	void ScreenWin32::get_position(int& xpos, int& ypos) const
	{
		DEVMODE dev_mode;
		ZeroMemory(&dev_mode, sizeof(DEVMODE));
		dev_mode.dmSize = sizeof(DEVMODE);

		EnumDisplaySettings(m_display_name, ENUM_CURRENT_SETTINGS, &dev_mode);
		xpos = dev_mode.dmPosition.x;
		ypos = dev_mode.dmPosition.y;
	}
	void ScreenWin32::get_size(unsigned int& width, unsigned int& height) const
	{
		DEVMODE dev_mode;
		ZeroMemory(&dev_mode, sizeof(DEVMODE));
		dev_mode.dmSize = sizeof(DEVMODE);

		EnumDisplaySettings(m_display_name, ENUM_CURRENT_SETTINGS, &dev_mode);
		width = dev_mode.dmPelsWidth;
		height = dev_mode.dmPelsHeight;
	}
	void ScreenWin32::get_monitor_size(unsigned int& width, unsigned int& height) const
	{
		if (m_monitor)
		{
			//alloc
			MONITORINFOEX minfo;
			minfo.cbSize = sizeof(minfo);
			//info 
			GetMonitorInfo(m_monitor, &minfo);
			width = minfo.rcWork.right - minfo.rcWork.left;
			height = minfo.rcWork.bottom - minfo.rcWork.top;
		}
		else
		{
			get_size(width, height);
		}
	}
	//private
	bool ScreenWin32::try_to_get_monitor_hendler()
	{
		BOOL res = EnumDisplayMonitors
		(
			NULL, NULL,
			[](HMONITOR hMonitor, HDC hDC, LPRECT rc, LPARAM data) -> BOOL
		{
			//self
			auto* self = (ScreenWin32*)data;
			//info
			MONITORINFOEX minfo;
			minfo.cbSize = sizeof(minfo);
			//test
			if (GetMonitorInfo(hMonitor, &minfo) && std::strcmp(minfo.szDevice, self->m_display_name) == 0)
			{
				self->m_monitor = hMonitor;
				return FALSE;
			}
			return TRUE;
		},
			(LPARAM)(this)
			);
		return res;
	}
}
}
}