//
//  PresentTimingVK.cpp
//  Square, the driver of Vulkan: the frames on the vsyncs of a frame rate (VK_GOOGLE_display_timing,
//  Android): each present asked at a time, the last one shown plus the time of a frame of the rate
//  (60: 16.7 ms, two vsyncs of a display at 120 Hz), half a vsync before (its slack); the times
//  shown read back from the presentation engine, so the frames stay on its vsyncs (the way of
//  Swappy). The vsync read at each present: a display that changes its rate (Samsung: 24 Hz while
//  the screen is still, at the start) does not hold the frames at its slow rate
//
#include "PresentTimingVK.h"
#include "Square/Config.h"
#include <algorithm>

namespace Square
{
namespace Render
{
	const char* PresentTimingVK::extension()
	{
		return VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME;
	}

	void PresentTimingVK::available(bool available)
	{
		m_available = available;
	}

	bool PresentTimingVK::available() const
	{
		return m_available;
	}

	void PresentTimingVK::swapchain(VkDevice device, VkSwapchainKHR swapchain)
	{
		reset();
		m_device = device;
		m_swapchain = swapchain;
	}

	void PresentTimingVK::reset()
	{
		m_device = VK_NULL_HANDLE;
		m_swapchain = VK_NULL_HANDLE;
		m_present_id = 0;
		m_shown_id = 0;
		m_shown_ns = 0;
	}

	void PresentTimingVK::read_shown()
	{
		uint32_t count = 0;
		vkGetPastPresentationTimingGOOGLE(m_device, m_swapchain, &count, nullptr);
		if (!count)
		{
			return;
		}
		Vector<VkPastPresentationTimingGOOGLE> shown(count);
		vkGetPastPresentationTimingGOOGLE(m_device, m_swapchain, &count, shown.data());
		for (uint32_t index = 0; index != count; ++index)
		{
			const VkPastPresentationTimingGOOGLE& timing = shown[index];
			if (timing.presentID > m_shown_id)
			{
				m_shown_id = timing.presentID;
				m_shown_ns = timing.actualPresentTime;
			}
		}
	}

	uint64_t PresentTimingVK::read_refresh() const
	{
		VkRefreshCycleDurationGOOGLE refresh{};
		if (vkGetRefreshCycleDurationGOOGLE(m_device, m_swapchain, &refresh) != VK_SUCCESS)
		{
			return 0;
		}
		return refresh.refreshDuration;
	}

	bool PresentTimingVK::next(unsigned int frame_rate, VkPresentTimeGOOGLE& time)
	{
		if (!m_available || !m_swapchain || !frame_rate)
		{
			return false;
		}
		const uint64_t refresh_ns = read_refresh();
		if (!refresh_ns)
		{
			return false;
		}
		read_shown();
		++m_present_id;
		time.presentID = m_present_id;
		time.desiredPresentTime = 0;
		//no present shown yet: at once (the next ones from its time)
		if (!m_shown_ns)
		{
			return true;
		}
		//the last one shown, plus the frames since it, half a vsync before (a late frame: its time
		//passed, shown at the next vsync)
		const uint64_t frame_ns = uint64_t(1.0e9 / double(frame_rate));
		const uint64_t frames = uint64_t(m_present_id - m_shown_id);
		time.desiredPresentTime = m_shown_ns + frames * frame_ns - refresh_ns / 2;
		return true;
	}
}
}
