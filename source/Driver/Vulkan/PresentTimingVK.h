//
//  PresentTimingVK.h
//  Square, the driver of Vulkan: the frames on the vsyncs of a frame rate (VK_GOOGLE_display_timing,
//  Android): each present asked at a time, the last one shown plus the time of a frame of the rate
//  (60: 16.7 ms, two vsyncs of a display at 120 Hz), half a vsync before (its slack); the times
//  shown read back from the presentation engine, so the frames stay on its vsyncs (the way of
//  Swappy). The vsync read at each present: a display that changes its rate (Samsung: 24 Hz while
//  the screen is still, at the start) does not hold the frames at its slow rate
//
#pragma once
#include "VulkanConfig.h"

namespace Square
{
namespace Render
{
	class PresentTimingVK
	{
	public:
		//the extension, if the device has it (enabled with the device)
		static const char* extension();
		void available(bool available);
		bool available() const;

		//a new swap chain: the times shown again
		void swapchain(VkDevice device, VkSwapchainKHR swapchain);
		void reset();

		//the time of the next present (its id, its time): false, no time asked (no frame rate, no
		//extension, no vsync of the display)
		bool next(unsigned int frame_rate, VkPresentTimeGOOGLE& time);

	private:
		//the times shown since the last call: the last one kept (its id, its time)
		void read_shown();
		//the vsync of the display now (0: unknown)
		uint64_t read_refresh() const;

		bool           m_available{ false };
		VkDevice       m_device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
		uint32_t       m_present_id{ 0 };   //the last present asked
		uint32_t       m_shown_id{ 0 };     //the last present shown
		uint64_t       m_shown_ns{ 0 };     //its time
	};
}
}
