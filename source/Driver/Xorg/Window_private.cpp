//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Wrapper_private.h"
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"


namespace Square
{
namespace Video
{  	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool x11_create_visual(const WindowInfo& wnd_info, XVisualInfo*& visual)
	{
		//init
		visual = nullptr;
		//select color map
		int n_return = 0;
		GLXFBConfig *fb_configs = nullptr;
		//alias
		const ContextInfo& ctx_info = wnd_info.m_context;
		//
		int color_bits   = ctx_info.m_color;
		int red_bits	 = 0;
		int green_bits   = 0;
		int blue_bits    = 0;
		int alpha_bits   = 0;
		int depth_bits   = ctx_info.m_depth;
		int stencil_bits = ctx_info.m_stencil;

		switch (color_bits)
		{
		case 16:
			red_bits   = 4;
			green_bits = 4;
			blue_bits  = 4;
			alpha_bits = 4;
			break;
		case 24:
			red_bits   = 8;
			green_bits = 8;
			blue_bits  = 8;
			alpha_bits = 0;
			break;
		case 32:
		default:
			red_bits   = 8;
			green_bits = 8;
			blue_bits  = 8;
			alpha_bits = 8;
			break;
		};

		//SET BUFFERS
		int buffer_OpenGL[] =
		{
			GLX_DRAWABLE_TYPE,   GLX_WINDOW_BIT,          //[0]  [1]
			GLX_RENDER_TYPE,     GLX_RGBA_BIT,		      //[1]  [2]
			GLX_RED_SIZE,        red_bits,			      //[3]  [4]
			GLX_GREEN_SIZE,      green_bits,		      //[5]  [6]
			GLX_BLUE_SIZE,       blue_bits,			      //[7]  [8]
			GLX_ALPHA_SIZE,      alpha_bits,		      //[9]  [10]
			GLX_DEPTH_SIZE,      depth_bits,		      //[11] [12]
			GLX_STENCIL_SIZE,    stencil_bits,		      //[13] [14]
		    GLX_DOUBLEBUFFER,	 True, 				      //[15] [16]
			GLX_SAMPLE_BUFFERS,  True,					  //[17] [18] // <-- MSAA
			GLX_SAMPLES,         ctx_info.m_anti_aliasing,//[19] [20] // <-- MSAA
			X11None
		};
		//tests
		int try_to_disable[][2] =
		{
			//disable double buffer
			{ 16, False },
			//Disable MSAA
			{ 17, X11None },
			//Disable stencil
			{ 14, 0 }
		};
		//number of tests
		int n_test = sizeof(try_to_disable) / sizeof(int[2]);
		int i_test = 0;
		//no msaa
		if (ctx_info.m_anti_aliasing < ContextInfo::MSAAx2 || ctx_info.m_anti_aliasing > ContextInfo::MSAAx64)
		{
			buffer_OpenGL[17] = X11None;
		}
		//try all
		while(true)
		{
			//get screen
			auto* screen = (ScreenXorg*)(wnd_info.m_screen->conteiner());
			//get config
			fb_configs = glXChooseFBConfig(s_os_context.m_xdisplay, screen->m_screen_id, buffer_OpenGL, &n_return);
			//next
			if (!n_return && i_test < n_test) 
				buffer_OpenGL[try_to_disable[i_test][0]] = try_to_disable[i_test++][1];
			else		   
				break;
		}
		//get visual color map
		if(n_return) visual = glXGetVisualFromFBConfig(s_os_context.m_xdisplay, fb_configs[0]);
		//success?
		return visual != nullptr;
	}
    
    //build window screen
	static bool x11_is_the_resolution_for_fullscreen_support(const WindowInfo& info)
	{		
		////////////////////////////////////
		//set fullscreen
		XF86VidModeModeInfo **modes;
		int mode_size = 0;
		bool res_found = false;				
		auto* screen = (ScreenXorg*)info.m_screen->conteiner();		
		//get info
		XF86VidModeGetAllModeLines(s_os_context.m_xdisplay, screen->m_screen_id, &mode_size, &modes);
		//look for mode with requested resolution
		for (size_t i = 0; i < mode_size; i++)
		if ((modes[i]->hdisplay == info.m_size[0]) && (modes[i]->vdisplay == info.m_size[1]))
		{
			res_found = true;
			break;
		}
		XFree(modes);
		return res_found;
	}

	static bool x11_create_screen_window(const WindowInfo& info, XWindow root_xwindow, const XVisualInfo* visual_info, XWindow& wnd)
	{
		XSetWindowAttributes win_attributes;
		//color map
		//win_attributes.override_redirect = True;
		win_attributes.event_mask = X11_WINDOW_ATTRIBUTE;
		win_attributes.colormap = XCreateColormap(s_os_context.m_xdisplay, root_xwindow, visual_info->visual, AllocNone);
		//window
		wnd = XCreateWindow
		(
			 s_os_context.m_xdisplay // display
			, root_xwindow           // parent
			, 0				         // x
			, 0                      // y
			, info.m_size[0]         // width
			, info.m_size[1]         // height
			, 0				         // border_width
			, visual_info->depth
			, InputOutput
			, visual_info->visual
			, CWBorderPixel | CWColormap | CWEventMask
			, &win_attributes
		);
		//only set window title and handle wm_delete_events if in windowed mode
		Atom wm_delete = XInternAtom(s_os_context.m_xdisplay, "WM_DELETE_WINDOW", 1);
		XSetWMProtocols(s_os_context.m_xdisplay, wnd, &wm_delete, 1);
		XSetStandardProperties
		(
			s_os_context.m_xdisplay
			, wnd
			, info.m_title.c_str()
			, info.m_title.c_str()
			, X11None
			, NULL
			, 0
			, NULL
		);
		XMapRaised(s_os_context.m_xdisplay, wnd);
		//disable/enable resize
		XSizeHints* size_hints = XAllocSizeHints();
		if (info.m_resize)
		{
			size_hints->flags      = X11_SIZE_HINTS_RESIZE;
			size_hints->min_height = 1;
			size_hints->min_width  = 1;
		}
		else
		{
			size_hints->flags      = X11_SIZE_HINTS_NO_RESIZE;
			size_hints->min_width  = size_hints->max_width  = info.m_size[0];
			size_hints->min_height = size_hints->max_height = info.m_size[1];
		}
		size_hints->flags      |= PWinGravity;
		size_hints->win_gravity = StaticGravity;
		XSetWMNormalHints(s_os_context.m_xdisplay, wnd, size_hints);
		XFree(size_hints);
		//return window
		return true;
	}

	static bool x11_create_fullscreen_window(const WindowInfo& info, XWindow root_xwindow, const XVisualInfo* visual_info, XWindow& wnd, XF86VidModeModeInfo& desktop_mode)
	{
		////////////////////////////////////
		//set fullscreen
		XF86VidModeModeInfo **modes;
		int mode_size  =  0;
		int best_mode  =  0;
		bool res_found = false;				
		auto* screen = (ScreenXorg*)info.m_screen->conteiner();		
		//get info
		XF86VidModeGetAllModeLines(s_os_context.m_xdisplay, screen->m_screen_id, &mode_size, &modes);
		// save desktop-resolution before switching modes
		desktop_mode = *modes[0];
		//look for mode with requested resolution
		for (size_t i = 0; i < mode_size; i++)
		if ((modes[i]->hdisplay == info.m_size[0]) && (modes[i]->vdisplay == info.m_size[1]))
		{
			best_mode = i;
			res_found = true;
			break;
		}
		//test
		if (!res_found) 
		{
			XFree(modes); 
			return false;
		}
		//////////////////////////////////////////////////////////////////////////////
		//witch to fullscreen				
		XF86VidModeSwitchToMode(s_os_context.m_xdisplay, screen->m_screen_id, modes[best_mode]);
		XF86VidModeSetViewPort(s_os_context.m_xdisplay,  screen->m_screen_id, 0, 0);
		unsigned int width  = modes[best_mode]->hdisplay;
		unsigned int height = modes[best_mode]->vdisplay;
		XFree(modes);
		////////////////////////////////////
		XSetWindowAttributes win_attributes;
		//color map
		win_attributes.override_redirect = True;
		win_attributes.event_mask = X11_FULL_SCREEN_WINDOW_ATTRIBUTE;
		win_attributes.colormap = XCreateColormap(s_os_context.m_xdisplay, root_xwindow, visual_info->visual, AllocNone);
        win_attributes.border_pixel = 0;
		//window
		wnd = XCreateWindow
		(
			s_os_context.m_xdisplay
			, root_xwindow
			, 0				  // x
			, 0               // y
			, width			  // width
			, height          // height
			, 0				  // border_width
			, visual_info->depth
			, InputOutput
			, visual_info->visual
			, CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect  //CWBorderPixel | CWColormap | CWEventMask
			, &win_attributes
		);
		//only set window title and handle wm_delete_events if in windowed mode
		XWarpPointer(s_os_context.m_xdisplay, X11None, wnd, 0, 0, 0, 0, 0, 0);
		XMapRaised(s_os_context.m_xdisplay, wnd);		
		XGrabKeyboard(s_os_context.m_xdisplay, wnd, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		XGrabPointer(s_os_context.m_xdisplay, wnd, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, wnd, X11None, CurrentTime);
		//return window
		return wnd;
	}
	
	static bool x11_create_OpenGL_context(const WindowInfo& wnd_info, const XVisualInfo* visual, GLXContext& context)
	{
		// create a GLX context
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");
		///////////////////////////////////////////////////////////////////////
		int n_config = 0;				
		auto* screen = (ScreenXorg*)wnd_info.m_screen->conteiner();		
		//Get a framebuffer config using the default attributes
		GLXFBConfig* frame_buffer_config = glXChooseFBConfig(s_os_context.m_xdisplay, screen->m_screen_id, 0, &n_config);
		//test
		if (!n_config) return false;
		///////////////////////////////////////////////////////////////////////
		if (glXCreateContextAttribsARB)
		{
			int context_attribs[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, (int)wnd_info.m_context.m_version[0],  //[0] [1]
				GLX_CONTEXT_MINOR_VERSION_ARB, (int)wnd_info.m_context.m_version[1],  //[2] [3]
				X11None
			};
			//create context
			context = glXCreateContextAttribsARB
			(
				  s_os_context.m_xdisplay
				, frame_buffer_config[0]
				, NULL
				, GL_TRUE
				, context_attribs
			);
		}		
		else 
		{
			context = glXCreateNewContext
			(
				  s_os_context.m_xdisplay
				, frame_buffer_config[0]
				, GLX_RGBA_TYPE
				, NULL
				, True
			);
		}
		//ret
		return context != nullptr;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	WindowXorg::WindowXorg(const WindowInfo& info)
	{
		XSetWindowAttributes win_attributes;
		//create a window in window mode
		XVisualInfo* visual_info;
		x11_create_visual(info,visual_info);
		//failed 
		if (!visual_info) throw std::runtime_error("Error: can't create XVisualInfo context");
		//OpenGL
		GLXContext xgl_ctx = NULL;
		x11_create_OpenGL_context(info, visual_info, xgl_ctx);
		//Root xwindow				
		auto* screen = (ScreenXorg*)info.m_screen->conteiner();
		XWindow root_xwindow = RootWindow(s_os_context.m_xdisplay, screen->m_screen_id);
		//window
		XWindow wnd;		  
		XF86VidModeModeInfo desktop_info;
		WindowXorg* wrapper_wnd = nullptr;
		//
		if(info.m_fullscreen)
		{
			x11_create_fullscreen_window(info, root_xwindow, visual_info, wnd, desktop_info);
			//save
			m_type = WindowXorg::GL_WINDOW;
			m_info = info;
			m_xinfo = visual_info;
			m_xwindow = wnd;
			m_desktop_info = desktop_info;
			m_gl_xcontext = xgl_ctx;
		}
		else 
		{
			x11_create_screen_window(info,  root_xwindow, visual_info, wnd);
			//end window
			m_type = WindowXorg::GL_WINDOW;
			m_info = info;
			m_xinfo = visual_info;
			m_xwindow = wnd;
			m_gl_xcontext = xgl_ctx;
		}
		//get context
		acquire_context();
        //save
        XSaveContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext, (XPointer)this);
	}

	
    WindowXorg::WindowXorg
    (
        WindowType  type,
        WindowInfo  info,
        XVisualInfo* xinfo,
        XWindow      xwindow,
        GLXContext   gl_xcontext
    )
    {
        m_type = type;
        m_info = info;
        m_xinfo = xinfo;
        m_xwindow = xwindow;
        m_gl_xcontext = gl_xcontext;
        //save
        XSaveContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext, (XPointer)this);
    }

    WindowXorg::WindowXorg
    (
        WindowType  type,
        WindowInfo  info,
        XVisualInfo* xinfo,
        XWindow             xwindow,
        XF86VidModeModeInfo desktop_info,
        GLXContext          gl_xcontext
    )
    {
        m_type = type;
        m_info = info;
        m_xinfo = xinfo;
        m_xwindow = xwindow;
        m_desktop_info = desktop_info;
        m_gl_xcontext  = gl_xcontext;
        //save
        XSaveContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext, (XPointer)this);
    }

    WindowXorg::~WindowXorg()
    {
        //back to old settings
        if (m_info.m_fullscreen)
        {
            auto* screen = (ScreenXorg*)m_info.m_screen->conteiner();
            XF86VidModeSwitchToMode(s_os_context.m_xdisplay, screen->m_screen_id, &m_desktop_info);
            XF86VidModeSetViewPort(s_os_context.m_xdisplay, screen->m_screen_id, 0, 0);
        }
        //remove
        XDeleteContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext);
        //delete glcontext
        if (glXGetCurrentContext() == m_gl_xcontext)
        {
            glXMakeCurrent(s_os_context.m_xdisplay, X11None, NULL);
        }
        glXDestroyContext(s_os_context.m_xdisplay, m_gl_xcontext);
        //Delete xwindow
        XDestroyWindow(s_os_context.m_xdisplay, m_xwindow);
        // Free the visual info
        XFree(m_xinfo);
    }

    void WindowXorg::swap() const
    {
        glXSwapBuffers(s_os_context.m_xdisplay, m_xwindow);
    }

    void WindowXorg::acquire_context() const
    {
        glXMakeCurrent(s_os_context.m_xdisplay, m_xwindow, m_gl_xcontext);
    }

    bool WindowXorg::is_fullscreen() const
    {
        return m_info.m_fullscreen;
    }

    bool WindowXorg::is_resizable() const
    {
        return m_info.m_resize;
    }

    void WindowXorg::get_size(unsigned int size[2]) const
    {
        int position[2];
        unsigned int depth;
        unsigned int border_width;
        XGetGeometry
        (
                s_os_context.m_xdisplay
            , m_xwindow
            , &DefaultRootWindow(s_os_context.m_xdisplay)
            //the position of m_xwindow in root_window
            , &position[0]
            , &position[1]
            , &size[0]
            , &size[1]
            , &border_width
            , &depth
        );
    }

    void WindowXorg::get_position(int position[2]) const
    {
        unsigned int size[2];
        unsigned int depth;
        unsigned int border_width;
        XGetGeometry
        (
            s_os_context.m_xdisplay
            , m_xwindow
            , &DefaultRootWindow(s_os_context.m_xdisplay)
            //the position of m_xwindow in root_window
            , &position[0]
            , &position[1]
            , &size[0]
            , &size[1]
            , &border_width
            , &depth
        );
    }

    void WindowXorg::set_size(unsigned int size[2])
    {
        if (XMoveResizeWindow(
                s_os_context.m_xdisplay
            , m_xwindow
            , m_info.m_position[0]
            , m_info.m_position[1]
            , size[0]
            , size[1])!=BadValue)
        {
            m_info.m_size[0] = size[0];
            m_info.m_size[1] = size[1];
        }
    }

    void WindowXorg::set_position(int position[2])
    {
        if (XMoveResizeWindow(
            s_os_context.m_xdisplay
            , m_xwindow
            , position[0]
            , position[1]
            , m_info.m_size[0]
            , m_info.m_size[1]) != BadValue)
        {
            m_info.m_position[0] = position[0];
            m_info.m_position[1] = position[1];
        }
    }

    bool WindowXorg::enable_resize(bool enable)
    {
        m_info.m_resize = enable;

        //disable/enable resize
        long user_supplied = false;
        XSizeHints size_hints;
        XGetWMNormalHints(s_os_context.m_xdisplay, m_xwindow, &size_hints, &user_supplied);
        size_hints.flags = m_info.m_resize ? X11_SIZE_HINTS_RESIZE : X11_SIZE_HINTS_NO_RESIZE;
        XSetWMNormalHints(s_os_context.m_xdisplay, m_xwindow, &size_hints);

        return true;
    }

    bool WindowXorg::enable_fullscreen(bool enable)
    {
        if (m_info.m_fullscreen == enable) return true;
        if (enable && !x11_is_the_resolution_for_fullscreen_support(m_info)) return false;
        //disable context
        if (glXGetCurrentContext() == m_gl_xcontext)
        {
            glXMakeCurrent(s_os_context.m_xdisplay, X11None, NULL);
        }
        //remove
        XDeleteContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext);
        //Delete xwindow
        XDestroyWindow(s_os_context.m_xdisplay, m_xwindow);
        //Get screen
        auto* screen = (ScreenXorg*)m_info.m_screen->conteiner();
        //Root xwindow				
        XWindow root_xwindow = RootWindow(s_os_context.m_xdisplay, screen->m_screen_id);
        //alloc
        if(enable)
        {
            x11_create_fullscreen_window(m_info, root_xwindow, m_xinfo, m_xwindow, m_desktop_info);
        }
        else
        {
            //back to old settings
            XF86VidModeSwitchToMode(s_os_context.m_xdisplay, screen->m_screen_id, &m_desktop_info);
            XF86VidModeSetViewPort(s_os_context.m_xdisplay,  screen->m_screen_id, 0, 0);
            //create window
            x11_create_screen_window(m_info, root_xwindow, m_xinfo, m_xwindow);
        }
        //save
        XSaveContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext, (XPointer)this);
        //aquired context
        acquire_context();
        //
        m_info.m_fullscreen = enable;
        //end
        return true;
    }
}
}