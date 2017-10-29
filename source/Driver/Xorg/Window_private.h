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
#include "Wrapper_private.h"

///////////////////////////////////////////////////

namespace Square
{
namespace Video
{
	struct WindowXorg
	{
		enum WindowType
		{
			UNKNOW_WINDOW,
			GL_WINDOW,
			DX_WINDOW
		};
		//window type
		WindowType          m_type       { UNKNOW_WINDOW };
		WindowInfo          m_info;
		XVisualInfo*        m_xinfo      { nullptr };
		XWindow       	    m_xwindow    { 0 };
		XF86VidModeModeInfo m_desktop_info;
		GLXContext          m_gl_xcontext{ nullptr };
		InputXorg*	        m_input_ref  { nullptr };
		Window*    	        m_window_ref { nullptr };
		
		WindowXorg(const WindowInfo& info);

		WindowXorg
		(
			WindowType  type,
			WindowInfo  info,
			XVisualInfo* xinfo,
			XWindow      xwindow,
			GLXContext   gl_xcontext
		);

		WindowXorg
		(
			WindowType  type,
			WindowInfo  info,
			XVisualInfo* xinfo,
			XWindow             xwindow,
			XF86VidModeModeInfo desktop_info,
			GLXContext          gl_xcontext
		);

		virtual ~WindowXorg();

		void swap() const;

		void acquire_context() const;

		virtual bool is_fullscreen() const;

		virtual bool is_resizable() const;

		virtual void get_size(unsigned int size[2]) const;

		virtual void get_position(int position[2]) const;

		virtual void set_size(unsigned int size[2]);

		virtual void set_position(int position[2]);

		virtual bool enable_resize(bool enable);

		virtual bool enable_fullscreen(bool enable);

	};
}
}