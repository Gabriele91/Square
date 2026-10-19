//
//  SurfaceAndroidVK.cpp
//  Square, the driver of Vulkan: the surface of the native window of Android
//  (VK_KHR_android_surface), its function from the instance (volk built without the platforms)
//
#include "SurfaceVK.h"
#include "AllocationVK.h"
#include "Square/Driver/Window.h"
#include <android/native_window.h>
#include <vulkan/vulkan_android.h>

namespace Square
{
namespace Render
{
namespace SurfaceVK
{
	const char* platform_extension()
	{
		return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
	}

	VkResult create(VkInstance instance, Video::DeviceResources* resources, VkSurfaceKHR* surface)
	{
		PFN_vkVoidFunction function = vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
		auto create_android_surface = (PFN_vkCreateAndroidSurfaceKHR)function;
		if (!create_android_surface)
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
		ANativeWindow* window = (ANativeWindow*)resources->get_device_context();
		if (!window)
		{
			return VK_ERROR_INITIALIZATION_FAILED;
		}
		VkAndroidSurfaceCreateInfoKHR surface_info{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
		surface_info.window = window;
		return create_android_surface(instance, &surface_info, AllocationVK::callbacks(), surface);
	}
}
}
}
