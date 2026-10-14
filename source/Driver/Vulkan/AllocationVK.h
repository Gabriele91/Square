//
//  AllocationVK.h
//  Square, the driver of Vulkan
//
//  The memory of the host of Vulkan (its objects, the ones of VMA) on the heap of the engine: the
//  allocation callbacks given to every vkCreate/vkDestroy and to VMA.
//
#pragma once
#include "VulkanConfig.h"

namespace Square
{
namespace Render
{
namespace AllocationVK
{
	//the callbacks (the same ones for the whole life of an object: its creation, its destruction)
	const VkAllocationCallbacks* callbacks();
}
}
}
