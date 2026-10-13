//
//  VulkanConfig.h
//  Square, the driver of Vulkan
//
//  Vulkan by volk (its functions loaded at run time from the loader of the system: no SDK to
//  build), the memory by VMA (its functions given by volk).
//
#pragma once
#if defined(_WIN32)
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif
#include <volk.h>
#ifndef VMA_STATIC_VULKAN_FUNCTIONS
	#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif
#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
	#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>
