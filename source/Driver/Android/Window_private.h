//
//  Square
//
//  Android: the window of the engine over the window of the activity (full screen, its size
//  the one of the native window), for the driver of Vulkan (it makes the surface).
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Android
{
	//the device resources of Vulkan: the activity and its native window (null in the background)
	struct DeviceResourcesAVK : public DeviceResources
	{
	public:

		DeviceResourcesAVK(const WindowAndroid& window);
		virtual ~DeviceResourcesAVK();

		virtual void callback_target_changed(std::function<void(DeviceResources*)> callback) override;

		virtual const ContextInfo& get_context_info() override;

		virtual bool get_vsync() override;
		virtual void set_vsync(bool vsync) override;

		//the frames a second: the display of the native window asked at that rate (Android 11+, a
		//hint: the system may keep it faster), the application still waits (false)
		virtual bool set_frame_rate(unsigned int fps) override;
		//the frame rate again on a new native window (back from the background)
		void apply_frame_rate();

		//the size of the native window (the size of the swap chain)
		virtual unsigned int width() override;
		virtual unsigned int height() override;

		//the activity (ANativeActivity), the native window (ANativeWindow)
		virtual void* get_device() override;
		virtual void* get_device_context(size_t i = 0) override;
		virtual void* get_swap_chain() override;

		virtual void* get_render_target() override;
		virtual void* get_depth_stencil_target() override;

		virtual void* get_render_resource() override;
		virtual void* get_depth_stencil_resource() override;

		virtual size_t number_of_device_context() override;

		//the swap of the window: the frame of the driver presented
		void present();

	protected:

		const WindowAndroid& m_window;
		bool                 m_vsync{ true };
		unsigned int         m_frame_rate{ 0 };
		std::function<void(DeviceResources*)> m_callback{ nullptr };
	};

	struct WindowAndroid
	{
		WindowInfo          m_info;              //its size: the size of the render (render_size)
		unsigned int        m_render_height{ 0 }; //asked by set_size (0: the one of the screen)
		InputAndroid*	    m_input_ref  { nullptr };
		Window*    	        m_window_ref { nullptr };
		DeviceResourcesAVK* m_vk_device  { nullptr };

		WindowAndroid(const WindowInfo& info);

		virtual ~WindowAndroid();

		//the device resources of the window (of Vulkan)
		DeviceResources* device() const;

		void swap() const;

		void acquire_context() const;

		virtual bool is_fullscreen() const;

		virtual bool is_resizable() const;

		virtual void get_size(unsigned int size[2]) const;

		//the size of the frames: the height asked (720, 1080...), the width of the aspect of the
		//screen (no stretch), at most the native window (Android scales the frames to the screen)
		void render_size(unsigned int size[2]) const;

		//the size of the frames again (the screen or the height asked changed): true when it changed
		bool update_size();

		virtual void get_position(int position[2]) const;

		virtual void set_size(unsigned int size[2]);

		virtual void set_position(int position[2]);

		virtual bool enable_resize(bool enable);

		virtual bool enable_fullscreen(bool enable);
	};
}
}
}
