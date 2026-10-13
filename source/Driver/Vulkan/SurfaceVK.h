//
//  SurfaceVK.h
//  Square, the driver of Vulkan: the surface of the native window of Xorg (its own unit: the
//  headers of Xlib and their macros, None, Bool, Status..., out of the rest of the driver)
//
#pragma once
#include "VulkanConfig.h"

namespace Square
{
namespace Video
{
	struct DeviceResources;
}
namespace Render
{
namespace SurfaceVK
{
	//the extension of the instance for the surface of the window
	const char* platform_extension();
	//the surface of the window of the device resources (get_device the display, get_device_context the window)
	VkResult create(VkInstance instance, Video::DeviceResources* resources, VkSurfaceKHR* surface);
}
}
}
