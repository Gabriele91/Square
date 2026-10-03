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
#include <cmath>

#ifndef GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB
#define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20B2
#endif


namespace Square
{
namespace Video
{
namespace Xorg
{
    struct SQUARE_API DeviceResourcesXGL : public DeviceResources
	{
	public:

    DeviceResourcesXGL(const ContextInfo& info_context): m_info_context(info_context) {}
		virtual ~DeviceResourcesXGL() {}
		//implement
		virtual unsigned int width() override 
		{
            return -1;
		}
		virtual unsigned int height() override 
		{
            return -1;
		}
		virtual const ContextInfo& get_context_info() override
		{
			return m_info_context;
		}

		virtual void callback_target_changed(std::function<void(DeviceResources*)> callback) override
		{
			//none
		}

		virtual bool get_vsync() { return m_vsync; }
		virtual void set_vsync(bool vsync)
		{
			typedef void(*PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);
			typedef int(*PFNGLXSWAPINTERVALMESAPROC)(unsigned int);
			typedef int(*PFNGLXSWAPINTERVALSGIPROC)(int);
			//get ptrs
			auto glXSwapIntervalEXT  = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
			auto glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
			auto glXSwapIntervalSGI  = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
			//on the current context
			GLXDrawable drawable = glXGetCurrentDrawable();
			if (glXSwapIntervalEXT && drawable)
			{
				glXSwapIntervalEXT(glXGetCurrentDisplay(), drawable, vsync ? 1 : 0);
				m_vsync = vsync;
			}
			else if (glXSwapIntervalMESA)
			{
				if (glXSwapIntervalMESA(vsync ? 1 : 0) == 0) m_vsync = vsync;
			}
			else if (glXSwapIntervalSGI && vsync) // SGI can't set 0
			{
				if (glXSwapIntervalSGI(1) == 0) m_vsync = vsync;
			}
		}

		virtual void* get_device()					   override { return (void*)nullptr; }
		virtual void* get_device_context(size_t i = 0) override { return (void*)nullptr; }
		virtual void* get_swap_chain()				   override { return (void*)nullptr; }

		virtual void* get_render_target()        override { return (void*)nullptr; }
		virtual void* get_depth_stencil_target() override { return (void*)nullptr; }

		virtual void* get_render_resource()        override { return (void*)nullptr; }
		virtual void* get_depth_stencil_resource() override { return (void*)nullptr; }

		virtual size_t number_of_device_context()  override { return 0; }

	protected:
		const ContextInfo& m_info_context;
		bool m_vsync{ true };
	};
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool x11_create_visual(const WindowInfo& wnd_info, XVisualInfo*& visual, GLXFBConfig& fb_config)
	{
		//init
		visual = nullptr;
		fb_config = nullptr;
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
			GLX_DRAWABLE_TYPE,            GLX_WINDOW_BIT,           //[0]  [1]
			GLX_RENDER_TYPE,              GLX_RGBA_BIT,	           //[2]  [3]
			GLX_RED_SIZE,                 red_bits,		           //[4]  [5]
			GLX_GREEN_SIZE,               green_bits,	           //[6]  [7]
			GLX_BLUE_SIZE,                blue_bits,	           //[8]  [9]
			GLX_ALPHA_SIZE,               alpha_bits,	           //[10] [11]
			GLX_DEPTH_SIZE,               depth_bits,	           //[12] [13]
			GLX_STENCIL_SIZE,             stencil_bits,	           //[14] [15]
		    GLX_DOUBLEBUFFER,             True,                     //[16] [17]
			GLX_SAMPLE_BUFFERS,           True,		               //[18] [19] // <-- MSAA
			GLX_SAMPLES,                  ctx_info.m_anti_aliasing, //[20] [21] // <-- MSAA
			GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,                 //[22] [23] // <-- sRGB
			X11None
		};
		//tests — tried in order when glXChooseFBConfig returns no configs
		int try_to_disable[][2] =
		{
			//Disable sRGB (most common fallback; shader will do gamma instead)
			{ 22, X11None },
			//disable double buffer
			{ 16, False },
			//Disable MSAA
			{ 18, X11None },
			//Disable stencil
			{ 15, 0 }
		};
		//number of tests
		int n_test = sizeof(try_to_disable) / sizeof(int[2]);
		int i_test = 0;
		//no msaa
		if (ctx_info.m_anti_aliasing < ContextInfo::MSAAx2 || ctx_info.m_anti_aliasing > ContextInfo::MSAAx64)
		{
			buffer_OpenGL[19] = False; // GLX_SAMPLE_BUFFERS
			buffer_OpenGL[21] = 0;     // GLX_SAMPLES
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
		//get visual color map (and keep the config: the GL context must be created from the same one)
		if(n_return)
		{
			fb_config = fb_configs[0];
			visual = glXGetVisualFromFBConfig(s_os_context.m_xdisplay, fb_config);
		}
		if(fb_configs) XFree(fb_configs);
		//success?
		return visual != nullptr;
	}
    
    //size hints: fixed size if not resizable, free in fullscreen (the WM refuses to fullscreen a fixed size window)
	static void x11_set_size_hints(XWindow wnd, bool resize, bool fullscreen, const unsigned int size[2])
	{
		XSizeHints* size_hints = XAllocSizeHints();
		if (fullscreen)
		{
			size_hints->flags = 0;
		}
		else if (resize)
		{
			size_hints->flags      = X11_SIZE_HINTS_RESIZE;
			size_hints->min_height = 1;
			size_hints->min_width  = 1;
		}
		else
		{
			size_hints->flags      = X11_SIZE_HINTS_NO_RESIZE;
			size_hints->min_width  = size_hints->max_width  = size[0];
			size_hints->min_height = size_hints->max_height = size[1];
		}
		size_hints->flags      |= PWinGravity;
		size_hints->win_gravity = StaticGravity;
		XSetWMNormalHints(s_os_context.m_xdisplay, wnd, size_hints);
		XFree(size_hints);
	}

	//EWMH fullscreen of a mapped window (asked to the window manager)
	static void x11_send_net_wm_fullscreen(XWindow wnd, bool enable)
	{
		Display* display = s_os_context.m_xdisplay;
		XEvent event{};
		event.xclient.type         = ClientMessage;
		event.xclient.window       = wnd;
		event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
		event.xclient.format       = 32;
		event.xclient.data.l[0]    = enable ? 1 : 0; // _NET_WM_STATE_ADD / _NET_WM_STATE_REMOVE
		event.xclient.data.l[1]    = (long)XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
		event.xclient.data.l[2]    = 0;
		event.xclient.data.l[3]    = 1; // normal application
		XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
		XFlush(display);
	}

	//the CRTC of the primary output (or of the first connected one)
	static RRCrtc x11_randr_crtc(XRRScreenResources* resources, XWindow root_xwindow)
	{
		Display* display = s_os_context.m_xdisplay;
		RROutput primary = XRRGetOutputPrimary(display, root_xwindow);
		for (int pass = 0; pass != 2; ++pass)
		for (int i = 0; i != resources->noutput; ++i)
		{
			if (pass == 0 && resources->outputs[i] != primary) continue;
			XRROutputInfo* output = XRRGetOutputInfo(display, resources, resources->outputs[i]);
			RRCrtc crtc = (output && output->connection == RR_Connected) ? output->crtc : 0;
			if (output) XRRFreeOutputInfo(output);
			if (crtc) return crtc;
		}
		return 0;
	}

	static double x11_randr_refresh(const XRRModeInfo& mode)
	{
		return (mode.hTotal && mode.vTotal) ? double(mode.dotClock) / (double(mode.hTotal) * double(mode.vTotal)) : 0.0;
	}

	//switch the display to a mode of the given size (as win32 does), saving the desktop one
	static bool x11_randr_switch_to(XWindow root_xwindow, const unsigned int size[2], RRCrtc& crtc, RRMode& desktop_mode)
	{
		Display* display = s_os_context.m_xdisplay;
		XRRScreenResources* resources = XRRGetScreenResourcesCurrent(display, root_xwindow);
		if (!resources) return false;
		bool success = false;
		crtc = x11_randr_crtc(resources, root_xwindow);
		XRRCrtcInfo* crtc_info = crtc ? XRRGetCrtcInfo(display, resources, crtc) : nullptr;
		if (crtc_info && crtc_info->noutput > 0)
		{
			desktop_mode = crtc_info->mode;
			//refresh of the desktop
			double desktop_refresh = 0.0;
			for (int i = 0; i != resources->nmode; ++i)
				if (resources->modes[i].id == desktop_mode) desktop_refresh = x11_randr_refresh(resources->modes[i]);
			//a mode of the output with the size, the refresh nearest to the desktop one
			XRROutputInfo* output = XRRGetOutputInfo(display, resources, crtc_info->outputs[0]);
			RRMode best_mode = 0;
			double best_delta = 0.0;
			for (int o = 0; output && o != output->nmode; ++o)
			for (int i = 0; i != resources->nmode; ++i)
			{
				const XRRModeInfo& mode = resources->modes[i];
				if (mode.id != output->modes[o] || mode.width != size[0] || mode.height != size[1]) continue;
				double delta = std::abs(x11_randr_refresh(mode) - desktop_refresh);
				if (!best_mode || delta < best_delta) { best_mode = mode.id; best_delta = delta; }
			}
			if (output) XRRFreeOutputInfo(output);
			//switch
			if (best_mode == desktop_mode)
			{
				success = true;
			}
			else if (best_mode)
			{
				success = XRRSetCrtcConfig
				(
					  display, resources, crtc, CurrentTime
					, crtc_info->x, crtc_info->y, best_mode, crtc_info->rotation
					, crtc_info->outputs, crtc_info->noutput
				) == RRSetConfigSuccess;
			}
		}
		if (crtc_info) XRRFreeCrtcInfo(crtc_info);
		XRRFreeScreenResources(resources);
		if (!success) { crtc = 0; desktop_mode = 0; }
		return success;
	}

	//back to the desktop mode
	static void x11_randr_restore(XWindow root_xwindow, RRCrtc crtc, RRMode desktop_mode)
	{
		if (!crtc || !desktop_mode) return;
		Display* display = s_os_context.m_xdisplay;
		XRRScreenResources* resources = XRRGetScreenResourcesCurrent(display, root_xwindow);
		if (!resources) return;
		if (XRRCrtcInfo* crtc_info = XRRGetCrtcInfo(display, resources, crtc))
		{
			if (crtc_info->mode != desktop_mode)
			{
				XRRSetCrtcConfig
				(
					  display, resources, crtc, CurrentTime
					, crtc_info->x, crtc_info->y, desktop_mode, crtc_info->rotation
					, crtc_info->outputs, crtc_info->noutput
				);
			}
			XRRFreeCrtcInfo(crtc_info);
		}
		XRRFreeScreenResources(resources);
	}

	static bool x11_create_screen_window(const WindowInfo& info, XWindow root_xwindow, const XVisualInfo* visual_info, XWindow& wnd)
	{
		XSetWindowAttributes win_attributes;
		//color map
		win_attributes.event_mask = X11_WINDOW_ATTRIBUTE;
		win_attributes.colormap = XCreateColormap(s_os_context.m_xdisplay, root_xwindow, visual_info->visual, AllocNone);
		win_attributes.border_pixel = 0;
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
		//handle wm_delete_events
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
		//disable/enable resize
		x11_set_size_hints(wnd, info.m_resize, info.m_fullscreen, info.m_size);
		//fullscreen from the start: the state is a property before the map (after it, a request to the WM)
		if (info.m_fullscreen)
		{
			Atom fullscreen = XInternAtom(s_os_context.m_xdisplay, "_NET_WM_STATE_FULLSCREEN", False);
			XChangeProperty
			(
				  s_os_context.m_xdisplay, wnd
				, XInternAtom(s_os_context.m_xdisplay, "_NET_WM_STATE", False)
				, XA_ATOM, 32, PropModeReplace, (unsigned char*)&fullscreen, 1
			);
		}
		XMapRaised(s_os_context.m_xdisplay, wnd);
		//return window
		return true;
	}

	static bool x11_create_OpenGL_context(const WindowInfo& wnd_info, GLXFBConfig frame_buffer_config, GLXContext& context)
	{
		// create a GLX context
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");
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
				, frame_buffer_config
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
				, frame_buffer_config
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
		//create a window in window mode
		XVisualInfo* visual_info;
		GLXFBConfig fb_config;
		x11_create_visual(info, visual_info, fb_config);
		//failed 
		if (!visual_info) throw std::runtime_error("Error: can't create XVisualInfo context");
		//OpenGL
		GLXContext xgl_ctx = NULL;
		if (!x11_create_OpenGL_context(info, fb_config, xgl_ctx)) throw std::runtime_error("Error: can't create the OpenGL context");
		//Root xwindow				
		auto* screen = (ScreenXorg*)info.m_screen->conteiner();
		XWindow root_xwindow = RootWindow(s_os_context.m_xdisplay, screen->m_screen_id);
		//fullscreen: the display to the window size (if there is a mode of that size)
		if (info.m_fullscreen)
		{
			m_rr_switched = x11_randr_switch_to(root_xwindow, info.m_size, m_rr_crtc, m_rr_desktop_mode);
		}
		//window
		XWindow wnd;
		x11_create_screen_window(info, root_xwindow, visual_info, wnd);
		//save
		m_type = WindowXorg::GL_WINDOW;
		m_info = info;
		m_windowed_size[0] = info.m_size[0];
		m_windowed_size[1] = info.m_size[1];
		m_xinfo = visual_info;
		m_xwindow = wnd;
		m_gl_xcontext = xgl_ctx;
		m_gl_device  = new DeviceResourcesXGL(m_info.m_context);
		//get context
		acquire_context();
        //save
        XSaveContext(s_os_context.m_xdisplay, m_xwindow, s_os_context.m_xcontext, (XPointer)this);
	}

    WindowXorg::~WindowXorg()
    {
        //back to the desktop mode
        if (m_rr_switched)
        {
            x11_randr_restore(DefaultRootWindow(s_os_context.m_xdisplay), m_rr_crtc, m_rr_desktop_mode);
            m_rr_switched = false;
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
		// remove device
		if (m_gl_device)
		{
			delete m_gl_device;
			m_gl_device = nullptr;
		}
    }

	DeviceResourcesXGL* WindowXorg::get_device_wrapper() const
	{
		return m_gl_device;
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
        x11_set_size_hints(m_xwindow, m_info.m_resize, m_info.m_fullscreen, m_windowed_size);
        return true;
    }

    bool WindowXorg::enable_fullscreen(bool enable)
    {
        if (m_info.m_fullscreen == enable) return true;
        //root
        auto* screen = (ScreenXorg*)m_info.m_screen->conteiner();
        XWindow root_xwindow = RootWindow(s_os_context.m_xdisplay, screen->m_screen_id);
        if (enable)
        {
            //save the window size
            m_windowed_size[0] = m_info.m_size[0];
            m_windowed_size[1] = m_info.m_size[1];
            //the display to the window size, as win32 (else: fullscreen at the desktop resolution)
            m_rr_switched = x11_randr_switch_to(root_xwindow, m_windowed_size, m_rr_crtc, m_rr_desktop_mode);
            //free size, then ask the fullscreen to the WM
            x11_set_size_hints(m_xwindow, m_info.m_resize, true, m_windowed_size);
            x11_send_net_wm_fullscreen(m_xwindow, true);
        }
        else
        {
            //leave the fullscreen
            x11_send_net_wm_fullscreen(m_xwindow, false);
            //back to the desktop mode
            if (m_rr_switched)
            {
                x11_randr_restore(root_xwindow, m_rr_crtc, m_rr_desktop_mode);
                m_rr_switched = false;
            }
            //back to the window size
            x11_set_size_hints(m_xwindow, m_info.m_resize, false, m_windowed_size);
            XResizeWindow(s_os_context.m_xdisplay, m_xwindow, m_windowed_size[0], m_windowed_size[1]);
        }
        XFlush(s_os_context.m_xdisplay);
        //
        m_info.m_fullscreen = enable;
        //end
        return true;
    }
}
}
}
