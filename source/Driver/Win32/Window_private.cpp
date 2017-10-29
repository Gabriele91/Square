//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Win32
{
	// OpenGL Window
	WindowGL::WindowGL(HDC hDC, HGLRC hRC, HWND  hWnd, const WindowInfo& info)
	{
		m_type = GL_WINDOW;
		m_hDC = hDC;
		m_hRC = hRC;
		m_hWnd = hWnd;
		m_info = info;
		//set
		enable_resize(info.m_fullscreen);
		//get info
		update_last_window_attributes();
		//set full screen
		enable_fullscreen(info.m_fullscreen);
	}

	void WindowGL::swap() const
	{
		if (m_hDC)
		{
			SwapBuffers(m_hDC);
		}
	}

	void WindowGL::acquire_context() const
	{
		if (m_hDC && m_hRC)
		{
			wglMakeCurrent(m_hDC, m_hRC);
		}
	}

	bool WindowGL::is_fullscreen() const
	{
		return m_info.m_fullscreen;
	}

	bool WindowGL::is_resizable() const
	{
		return m_info.m_resize;
	}

	void WindowGL::get_size(unsigned int size[2]) const
	{
		RECT window_box{ 0, 0, 0, 0 };
		GetClientRect(m_hWnd, &window_box);
		//get_rect_of_window_including_aero(m_hWnd, &window_box);
		size[0] = window_box.right - window_box.left;
		size[1] = window_box.bottom - window_box.top;
	}

	void WindowGL::get_position(int position[2]) const
	{
		POINT pos{ 0, 0 };
		ClientToScreen(m_hWnd, &pos);
		position[0] = pos.x;
		position[1] = pos.y;
	}

	void WindowGL::set_size(unsigned int size[2])
	{
		//calc size window
		unsigned int window_real_size[2] = { 0,0 };
		compute_window_size(size, window_real_size);
		//Resize
		if (SetWindowPos(
			m_hWnd
			, HWND_NOTOPMOST
			, m_info.m_position[0]
			, m_info.m_position[1]
			, window_real_size[0]
			, window_real_size[1]
			, SWP_SHOWWINDOW
		))
		{
			//save new size
			m_info.m_size[0] = size[0];
			m_info.m_size[1] = size[1];
		}
	}

	void WindowGL::set_position(int position[2])
	{
		//calc size window
		unsigned int window_real_size[2] = { 0,0 };
		compute_window_size(m_info.m_size, window_real_size);
		//Resize
		if (SetWindowPos(
			m_hWnd
			, HWND_NOTOPMOST
			, position[0]
			, position[1]
			, window_real_size[0]
			, window_real_size[1]
			, SWP_SHOWWINDOW
		))
		{
			//save new position
			m_info.m_position[0] = position[0];
			m_info.m_position[1] = position[1];
		}
	}

	bool WindowGL::enable_resize(bool enable)
	{
		m_info.m_resize = enable;
		LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		return SetWindowLong(
			m_hWnd
			, GWL_STYLE
			, enable ? (style | WS_SIZEBOX) : (style & ~WS_SIZEBOX)
		);
	}

	bool WindowGL::enable_fullscreen(bool enable)
	{
		if (m_info.m_fullscreen == enable) return true;
		//...
		bool is_change_successful;
		//hide window
		ShowWindow(m_hWnd, SW_HIDE);
		//test
		if (enable)
		{
			update_last_window_attributes();

			DEVMODE fullscreen_settings;
			fullscreen_settings.dmSize = sizeof(fullscreen_settings);

			auto* screen = (ScreenWin32*)(m_info.m_screen->conteiner());
			EnumDisplaySettings(screen->m_display_name, 0, &fullscreen_settings);
			fullscreen_settings.dmPelsWidth = m_info.m_size[0];
			fullscreen_settings.dmPelsHeight = m_info.m_size[1];
			fullscreen_settings.dmBitsPerPel = m_info.m_context.m_color;
			fullscreen_settings.dmFields = DM_PELSWIDTH |
				DM_PELSHEIGHT |
				DM_BITSPERPEL;

			is_change_successful = ChangeDisplaySettings(&fullscreen_settings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
			//change only if a success
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(
					m_hWnd
					, HWND_TOPMOST
					, 0
					, 0
					, m_info.m_size[0]
					, m_info.m_size[1]
					, SWP_SHOWWINDOW
				);
				ShowWindow(m_hWnd, SW_MAXIMIZE);
				//update
				m_info.m_fullscreen = enable;
			}
		}
		else
		{
			is_change_successful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
			//change only if a success
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, m_last_window_exstyle);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, m_last_window_style);
				//calc size window
				unsigned int last_window_real_size[2] = { 0,0 };
				compute_window_size(m_last_window_size, last_window_real_size);
				//Resize
				SetWindowPos(
					m_hWnd
					, HWND_NOTOPMOST
					, m_last_window_position[0]
					, m_last_window_position[1]
					, last_window_real_size[0]
					, last_window_real_size[1]
					, SWP_SHOWWINDOW
				);
				//update
				m_info.m_fullscreen = enable;
			}
		}
		//show window
		ShowWindow(m_hWnd, SW_SHOW);
		//end
		return is_change_successful;
	}

	HWND WindowGL::narive() const
	{
		return m_hWnd;
	}

	void WindowGL::update_last_window_attributes()
	{
		get_position(m_last_window_position);
		get_size(m_last_window_size);
		m_last_window_exstyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		m_last_window_style = GetWindowLong(m_hWnd, GWL_STYLE);
	}

	WindowGL::~WindowGL()
	{
		//dattach RC 
		wglMakeCurrent(NULL, NULL);
		//release RC
		if (m_hRC)		     wglDeleteContext(m_hRC);
		//release DC
		if (m_hWnd && m_hDC) ReleaseDC(m_hWnd, m_hDC);
		//delete Window
		if (m_hWnd)			 DestroyWindow(m_hWnd);
		//all to null 
		m_hRC = nullptr;
		m_hDC = nullptr;
		m_hWnd = nullptr;
	}
	///////////////////////////////////////////////////////////////////////////////////
}
}
}