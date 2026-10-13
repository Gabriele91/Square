//
//  Square
//
//  The window of the driver of Vulkan: a native window only (the driver makes its surface and
//  its swap chain from it, it follows its size); its swap is the present of the driver (the
//  callback of its device resources).
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
	///////////////////////////////////////////////////////////////////////////////////
	//the native window for the driver: get_device the instance of the module, get_device_context
	//the window; the callback (callback_target_changed) is called at each swap: the driver
	//presents its frame there (as Metal on Cocoa)
	struct SQUARE_API DeviceResourcesVK : public DeviceResources
	{
	public:
		DeviceResourcesVK(HINSTANCE instance, HWND window, const WindowInfo& info)
		: m_instance(instance)
		, m_window(window)
		, m_info(info)
		{
		}

		virtual void callback_target_changed(std::function<void(DeviceResources*)> callback) override
		{
			m_callback = callback;
		}

		virtual const ContextInfo& get_context_info() override
		{
			return m_info.m_context;
		}

		virtual bool get_vsync() override { return m_vsync; }
		virtual void set_vsync(bool vsync) override { m_vsync = vsync; }

		//the client area of the window (the size of the swap chain)
		virtual unsigned int width() override
		{
			RECT box{ 0, 0, 0, 0 };
			GetClientRect(m_window, &box);
			return (unsigned int)(box.right - box.left);
		}

		virtual unsigned int height() override
		{
			RECT box{ 0, 0, 0, 0 };
			GetClientRect(m_window, &box);
			return (unsigned int)(box.bottom - box.top);
		}

		virtual void* get_device()                     override { return (void*)m_instance; }
		virtual void* get_device_context(size_t i = 0) override { return (void*)m_window; }
		virtual void* get_swap_chain()                 override { return nullptr; }

		virtual void* get_render_target()        override { return nullptr; }
		virtual void* get_depth_stencil_target() override { return nullptr; }

		virtual void* get_render_resource()        override { return nullptr; }
		virtual void* get_depth_stencil_resource() override { return nullptr; }

		virtual size_t number_of_device_context()  override { return 1; }

		//the swap of the window: the frame of the driver presented
		void present()
		{
			if (m_callback)
			{
				m_callback(this);
			}
		}

	protected:

		HINSTANCE         m_instance{ nullptr };
		HWND              m_window{ nullptr };
		const WindowInfo& m_info;
		bool              m_vsync{ false };
		std::function<void(DeviceResources*)> m_callback{ nullptr };
	};
	///////////////////////////////////////////////////////////////////////////////////
	WindowVK::WindowVK(HDC hDC, HWND  hWnd, const WindowInfo& info)
	{
		m_type   = VK_WINDOW;
		m_hDC    = hDC;
		m_hWnd   = hWnd;
		m_info   = info;
		m_device = new DeviceResourcesVK((HINSTANCE)GetModuleHandle(NULL), hWnd, m_info);
		//set
		enable_resize(info.m_resize);
		//get info
		update_last_window_attributes();
		//set full screen
		enable_fullscreen(info.m_fullscreen);
	}

	void WindowVK::swap() const
	{
		if (m_device)
		{
			m_device->present();
		}
	}

	void WindowVK::acquire_context() const
	{
		//none
	}

	bool WindowVK::is_fullscreen() const
	{
		return m_info.m_fullscreen;
	}

	bool WindowVK::is_resizable() const
	{
		return m_info.m_resize;
	}

	void WindowVK::get_size(unsigned int size[2]) const
	{
		//stretched: the size of its frame (not of the monitor)
		if (m_scaled)
		{
			size[0] = m_info.m_size[0];
			size[1] = m_info.m_size[1];
			return;
		}
		RECT window_box{ 0, 0, 0, 0 };
		GetClientRect(m_hWnd, &window_box);
		size[0] = window_box.right - window_box.left;
		size[1] = window_box.bottom - window_box.top;
	}

	void WindowVK::get_position(int position[2]) const
	{
		POINT pos{ 0, 0 };
		ClientToScreen(m_hWnd, &pos);
		position[0] = pos.x;
		position[1] = pos.y;
	}

	void WindowVK::set_size(unsigned int size[2])
	{
		//stretched: the window stays over the monitor, only its frame of the new size
		if (m_scaled)
		{
			m_info.m_size[0] = size[0];
			m_info.m_size[1] = size[1];
			return;
		}
		//calc size window
		unsigned int window_real_size[2] = { 0,0 };
		compute_window_size(size, window_real_size);
		//Resize (the swap chain follows the client area: the driver)
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
			m_info.m_size[0] = size[0];
			m_info.m_size[1] = size[1];
		}
	}

	void WindowVK::set_position(int position[2])
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
			m_info.m_position[0] = position[0];
			m_info.m_position[1] = position[1];
		}
	}

	bool WindowVK::enable_resize(bool enable)
	{
		m_info.m_resize = enable;
		LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		if (enable)
		{
			style |= WS_SIZEBOX;
		}
		else
		{
			style &= ~WS_SIZEBOX;
		}
		return SetWindowLong(m_hWnd, GWL_STYLE, style);
	}

	bool WindowVK::enable_fullscreen(bool enable)
	{
		if (m_info.m_fullscreen == enable) return true;
		bool is_change_successful = false;
		ShowWindow(m_hWnd, SW_HIDE);
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
			fullscreen_settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
			is_change_successful = ChangeDisplaySettings(&fullscreen_settings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, m_info.m_size[0], m_info.m_size[1], SWP_SHOWWINDOW);
				ShowWindow(m_hWnd, SW_MAXIMIZE);
				m_info.m_fullscreen = enable;
			}
			else
			{
				//the screen has no mode of this size: borderless over the whole monitor (the swap
				//chain of the size of the monitor; the frame asked drawn in it)
				MONITORINFO monitor;
				monitor.cbSize = sizeof(monitor);
				GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &monitor);
				const RECT& box = monitor.rcMonitor;
				m_scaled = true;
				m_info.m_fullscreen = true;
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(m_hWnd, HWND_TOP, box.left, box.top, box.right - box.left, box.bottom - box.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
				is_change_successful = true;
			}
		}
		else
		{
			if (m_scaled)
			{
				m_scaled = false;
				is_change_successful = true;
			}
			else
			{
				is_change_successful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
			}
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, m_last_window_exstyle);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, m_last_window_style);
				//no more maximized (SW_MAXIMIZE of the fullscreen), else it keeps the work area
				ShowWindow(m_hWnd, SW_SHOWNORMAL);
				unsigned int last_window_real_size[2] = { 0,0 };
				compute_window_size(m_last_window_size, last_window_real_size);
				SetWindowPos(
					m_hWnd
					, HWND_NOTOPMOST
					, m_last_window_position[0]
					, m_last_window_position[1]
					, last_window_real_size[0]
					, last_window_real_size[1]
					, SWP_SHOWWINDOW
				);
				m_info.m_fullscreen = enable;
			}
		}
		ShowWindow(m_hWnd, SW_SHOW);
		return is_change_successful;
	}

	HWND WindowVK::narive() const
	{
		return m_hWnd;
	}

	DeviceResources* WindowVK::device() const
	{
		return m_device;
	}

	void WindowVK::update_last_window_attributes()
	{
		get_position(m_last_window_position);
		get_size(m_last_window_size);
		m_last_window_exstyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		m_last_window_style = GetWindowLong(m_hWnd, GWL_STYLE);
	}

	WindowVK::~WindowVK()
	{
		if (m_device)		 delete m_device;
		if (m_hWnd && m_hDC) ReleaseDC(m_hWnd, m_hDC);
		if (m_hWnd)			 DestroyWindow(m_hWnd);
		m_device = nullptr;
		m_hDC = nullptr;
		m_hWnd = nullptr;
	}
}
}
}
