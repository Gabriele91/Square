//
//  Square
//
//  Android: the window of the engine over the window of the activity (full screen, its size
//  the one of the native window), for the driver of Vulkan (it makes the surface).
//
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"
#include "Square/Core/Application.h"
#include "Square/Core/SharedLibrary.h"
#include <algorithm>

namespace Square
{
namespace Video
{
namespace Android
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DEVICE RESOURCES
	DeviceResourcesAVK::DeviceResourcesAVK(const WindowAndroid& window) : m_window(window) {}

	DeviceResourcesAVK::~DeviceResourcesAVK() {}

	void DeviceResourcesAVK::callback_target_changed(std::function<void(DeviceResources*)> callback)
	{
		m_callback = callback;
	}

	const ContextInfo& DeviceResourcesAVK::get_context_info()
	{
		return m_window.m_info.m_context;
	}

	bool DeviceResourcesAVK::get_vsync() { return m_vsync; }
	void DeviceResourcesAVK::set_vsync(bool vsync) { m_vsync = vsync; }

	//ANativeWindow_setFrameRate (Android 11, API 30: the application runs from Android 10, its
	//function looked for at run time, in libandroid: RTLD_DEFAULT does not see it)
	using SetFrameRate = int32_t (*)(ANativeWindow* window, float frame_rate, int8_t compatibility);

	bool DeviceResourcesAVK::set_frame_rate(unsigned int fps)
	{
		//a hint (a Samsung keeps 120 Hz): the driver of Vulkan keeps the rate
		m_frame_rate = fps;
		apply_frame_rate();
		return false;
	}

	void DeviceResourcesAVK::apply_frame_rate()
	{
		if (!s_activity.m_native_window)
		{
			return;
		}
		//its library open while its function is called (already loaded by the application)
		void* library = SharedLibrary::open("libandroid.so");
		if (!library)
		{
			return;
		}
		if (auto function = (SetFrameRate)SharedLibrary::get(library, "ANativeWindow_setFrameRate"))
		{
			//0: no rate asked, the display as the system wants it (120 Hz)
			const int8_t compatibility = 0; //ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_DEFAULT
			const int32_t result = function(s_activity.m_native_window, float(m_frame_rate), compatibility);
			if (auto app = Application::instance())
			if (auto logger = app->logger())
			{
				logger->info("Android: frame rate " + to_string(m_frame_rate) + " asked to the window (" + to_string(int(result)) + ")");
			}
		}
		SharedLibrary::close(library);
	}

	unsigned int DeviceResourcesAVK::width()
	{
		unsigned int size[2];
		m_window.render_size(size);
		return size[0];
	}

	unsigned int DeviceResourcesAVK::height()
	{
		unsigned int size[2];
		m_window.render_size(size);
		return size[1];
	}

	void* DeviceResourcesAVK::get_device()
	{
		if (!s_activity.m_app)
		{
			return nullptr;
		}
		return (void*)s_activity.m_app->activity;
	}

	void* DeviceResourcesAVK::get_device_context(size_t i)
	{
		return (void*)s_activity.m_native_window;
	}

	void* DeviceResourcesAVK::get_swap_chain()               { return nullptr; }
	void* DeviceResourcesAVK::get_render_target()            { return nullptr; }
	void* DeviceResourcesAVK::get_depth_stencil_target()     { return nullptr; }
	void* DeviceResourcesAVK::get_render_resource()          { return nullptr; }
	void* DeviceResourcesAVK::get_depth_stencil_resource()   { return nullptr; }
	size_t DeviceResourcesAVK::number_of_device_context()    { return 1; }

	void DeviceResourcesAVK::present()
	{
		if (m_callback)
		{
			m_callback(this);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WINDOW
	WindowAndroid::WindowAndroid(const WindowInfo& info)
	{
		m_info = info;
		//full screen, as big as the native window
		m_info.m_fullscreen = true;
		m_info.m_resize = false;
		render_size(m_info.m_size);
		//Vulkan only (the window has not a context of its own)
		if (m_info.m_context.m_type != ContextInfo::CTX_VULKAN)
		{
			if (auto app = Application::instance())
			if (auto logger = app->logger())
			{
				logger->error("Android: only the Vulkan backend (-b vk), not the context " + to_string(int(m_info.m_context.m_type)));
			}
		}
		m_vk_device = new DeviceResourcesAVK(*this);
	}

	WindowAndroid::~WindowAndroid()
	{
		if (m_vk_device)
		{
			delete m_vk_device;
			m_vk_device = nullptr;
		}
	}

	DeviceResources* WindowAndroid::device() const
	{
		return m_vk_device;
	}

	void WindowAndroid::swap() const
	{
		if (m_vk_device)
		{
			m_vk_device->present();
		}
	}

	void WindowAndroid::acquire_context() const
	{
		//none: Vulkan has not a context of the window
	}

	bool WindowAndroid::is_fullscreen() const
	{
		return true;
	}

	bool WindowAndroid::is_resizable() const
	{
		return false;
	}

	void WindowAndroid::get_size(unsigned int size[2]) const
	{
		size[0] = m_info.m_size[0];
		size[1] = m_info.m_size[1];
	}

	void WindowAndroid::render_size(unsigned int size[2]) const
	{
		native_window_size(size);
		if (!m_render_height || !size[1] || m_render_height >= size[1])
		{
			return;
		}
		const unsigned long long width = (unsigned long long)size[0] * m_render_height / size[1];
		size[0] = std::max(1u, (unsigned int)width & ~1u);
		size[1] = m_render_height;
	}

	bool WindowAndroid::update_size()
	{
		unsigned int size[2];
		render_size(size);
		if (!size[0] || !size[1])
		{
			return false;
		}
		if (size[0] == m_info.m_size[0] && size[1] == m_info.m_size[1])
		{
			return false;
		}
		m_info.m_size[0] = size[0];
		m_info.m_size[1] = size[1];
		return true;
	}

	void WindowAndroid::get_position(int position[2]) const
	{
		position[0] = 0;
		position[1] = 0;
	}

	void WindowAndroid::set_size(unsigned int size[2])
	{
		//the height of the frames (the width: the aspect of the screen), the window is the screen
		m_render_height = size[1];
		if (update_size())
		{
			s_activity.m_resize_pending = true;
		}
	}

	void WindowAndroid::set_position(int position[2])
	{
		//none: the activity is the screen
	}

	bool WindowAndroid::enable_resize(bool enable)
	{
		return !enable;
	}

	bool WindowAndroid::enable_fullscreen(bool enable)
	{
		return enable;
	}
}
}
}
