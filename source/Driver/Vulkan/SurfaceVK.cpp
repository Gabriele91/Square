//
//  SurfaceVK.cpp
//  Square, the driver of Vulkan: the surface of the native window of Xorg (VK_KHR_xlib_surface),
//  its function from the instance (volk built without the platforms: no Xlib in its header)
//
#include "SurfaceVK.h"
#include "AllocationVK.h"
#include "Square/Driver/Window.h"
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

namespace Square
{
namespace Render
{
namespace SurfaceVK
{
	const char* platform_extension()
	{
		return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
	}

	VkResult create(VkInstance instance, Video::DeviceResources* resources, VkSurfaceKHR* surface)
	{
		auto create_xlib_surface = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
		if (!create_xlib_surface) return VK_ERROR_EXTENSION_NOT_PRESENT;
		VkXlibSurfaceCreateInfoKHR surface_info{ VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR };
		surface_info.dpy = (Display*)resources->get_device();
		surface_info.window = (::Window)resources->get_device_context();
		return create_xlib_surface(instance, &surface_info, AllocationVK::callbacks(), surface);
	}
}
}
}
