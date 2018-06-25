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
	struct WindowWin32;
	struct WindowGL;
	struct WindowDX;
	struct WindowVK;
	struct DeviceResourcesDX;
	struct DeviceResourcesGL;
	struct DeviceResourcesVK;

	struct SQUARE_API WindowWin32
	{
		enum window_type
		{
			UNKNOW_WINDOW,
			GL_WINDOW,
			DX_WINDOW
		};		
		//conteiner
		Window*	m_window_ref{ nullptr };
		//window type
		window_type m_type{ UNKNOW_WINDOW };
		//info about init windows
		WindowInfo m_info;
		//virtual methods
		virtual void swap() const = 0;
		virtual void acquire_context() const = 0;
		virtual bool is_fullscreen() const = 0;
		virtual bool is_resizable() const = 0;
		virtual void get_size(unsigned int size[2]) const = 0;
		virtual void get_position(int position[2]) const = 0;
		virtual void set_size(unsigned int size[2]) = 0;
		virtual void set_position(int position[2]) = 0;
		virtual bool enable_resize(bool enable) = 0;
		virtual bool enable_fullscreen(bool enable) = 0;
		virtual HWND narive() const = 0;
		virtual DeviceResources* device() const = 0;
		//virtual destructor
		virtual ~WindowWin32() {};
	};

	struct SQUARE_API WindowGL : WindowWin32
	{
		HDC		    m_hDC;				/* device context */
		HGLRC	    m_hRC;				/* opengl context */
		HWND	    m_hWnd;				/* window */
		DeviceResourcesGL* m_device;    /* opengl context wrapper */

		WindowGL(HDC hDC, HGLRC hRC, HWND  hWnd, const WindowInfo& info);

		virtual void swap() const;

		virtual void acquire_context() const;

		virtual bool is_fullscreen() const;
		
		virtual bool is_resizable() const;

		virtual void get_size(unsigned int size[2]) const;

		virtual void get_position(int position[2]) const;

		virtual void set_size(unsigned int size[2]);

		virtual void set_position(int position[2]);

		virtual bool enable_resize(bool enable);

		virtual bool enable_fullscreen(bool enable);

		virtual HWND narive() const;

		virtual DeviceResources* device() const;

		virtual ~WindowGL();

	protected:

		//info bf fullscreen
		int          m_last_window_position[2]{ 0, 0 };
		unsigned int m_last_window_size[2]{ 0,0 };
		LONG         m_last_window_style;
		LONG         m_last_window_exstyle;
		void		 update_last_window_attributes();
	};
	
	struct SQUARE_API WindowDX : WindowWin32
	{
		HDC						m_hDC;				/* device context */
		DeviceResourcesDX*	    m_device;		    /* DX context */
		HWND					m_hWnd;			    /* window */

		WindowDX(HDC hDC, HWND  hWnd, const WindowInfo& info);

		virtual void swap() const;

		virtual void acquire_context() const;

		virtual bool is_fullscreen() const;

		virtual bool is_resizable() const;

		virtual void get_size(unsigned int size[2]) const;

		virtual void get_position(int position[2]) const;

		virtual void set_size(unsigned int size[2]);

		virtual void set_position(int position[2]);

		virtual bool enable_resize(bool enable);

		virtual bool enable_fullscreen(bool enable);

		virtual HWND narive() const;

		virtual DeviceResources* device() const;

		virtual ~WindowDX();

	protected:

		//info bf fullscreen
		int          m_last_window_position[2]{ 0, 0 };
		unsigned int m_last_window_size[2]{ 0,0 };
		LONG         m_last_window_style;
		LONG         m_last_window_exstyle;
		void		 update_last_window_attributes();
	};
}
}
}