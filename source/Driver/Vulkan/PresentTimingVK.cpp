//
//  PresentTimingVK.cpp
//  Square, the driver of Vulkan: the presents at a frame rate (see PresentTimingVK.h)
//
#include "PresentTimingVK.h"
#include "Square/Config.h"
#include "Square/Core/Time.h"
#include "Square/Driver/Window.h"
#include <cstring>

namespace Square
{
namespace Render
{
	namespace AuxPresentTimingVK
	{
		//PRESENT_WAIT: the longest wait of a present shown (ns), a wait at once (ms: shown before)
		static constexpr uint64_t s_wait_timeout_ns = 100000000;
		static constexpr double   s_wait_at_once_ms = 0.5;

		static bool has_extension(const Vector<VkExtensionProperties>& extensions, const char* name)
		{
			for (const VkExtensionProperties& extension : extensions)
			{
				if (std::strcmp(extension.extensionName, name) == 0) return true;
			}
			return false;
		}
	}

	void PresentTimingVK::choose(VkPhysicalDevice physical, uint32_t api_version, const Vector<VkExtensionProperties>& available, Vector<const char*>& extensions, void*& features)
	{
		using namespace AuxPresentTimingVK;
		m_method = Method::NONE;
		//its times (Android, MoltenVK)
		if (has_extension(available, VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
		{
			extensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
			m_method = Method::DISPLAY_TIMING;
			return;
		}
		//its waits (Windows, Linux): both extensions, both features
		const bool waits = api_version >= VK_API_VERSION_1_1
		                && has_extension(available, VK_KHR_PRESENT_ID_EXTENSION_NAME)
		                && has_extension(available, VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
		if (!waits) return;
		VkPhysicalDeviceFeatures2 query{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		m_wait_features.pNext = &m_id_features;
		query.pNext = &m_wait_features;
		vkGetPhysicalDeviceFeatures2(physical, &query);
		if (!m_id_features.presentId || !m_wait_features.presentWait) return;
		extensions.push_back(VK_KHR_PRESENT_ID_EXTENSION_NAME);
		extensions.push_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
		m_id_features.pNext = features;
		features = &m_wait_features;
		m_method = Method::PRESENT_WAIT;
	}

	PresentTimingVK::Method PresentTimingVK::method() const
	{
		return m_method;
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
		m_wait_id = 0;
		m_wait_shown_ms = 0.0;
	}

	bool PresentTimingVK::frame_rate(unsigned int fps, unsigned int refresh)
	{
		m_frame_rate = fps;
		m_interval = 0;
		m_vsync_ms = 0.0;
		if (!fps) return false;
		switch (m_method)
		{
		case Method::DISPLAY_TIMING:
			return true;
		case Method::PRESENT_WAIT:
			//the vsyncs of a frame (the rate: a divisor of the refresh)
			m_interval = Video::DeviceResources::swap_interval(refresh, fps);
			if (!m_interval) return false;
			m_vsync_ms = 1000.0 / double(refresh);
			return true;
		case Method::NONE:
		default:
			return false;
		}
	}

	void PresentTimingVK::present(VkPresentInfoKHR& info)
	{
		if (!m_swapchain || !m_frame_rate) return;
		switch (m_method)
		{
		case Method::DISPLAY_TIMING:
			if (display_timing(m_frame_rate))
			{
				m_times.swapchainCount = 1;
				m_times.pTimes = &m_time;
				m_times.pNext = info.pNext;
				info.pNext = &m_times;
			}
		break;
		case Method::PRESENT_WAIT:
			if (present_wait())
			{
				m_ids.swapchainCount = 1;
				m_ids.pPresentIds = &m_wait_id;
				m_ids.pNext = info.pNext;
				info.pNext = &m_ids;
			}
		break;
		case Method::NONE:
		default: break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// DISPLAY_TIMING
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

	bool PresentTimingVK::display_timing(unsigned int frame_rate)
	{
		//(the vsync at each present: a display can change its rate)
		const uint64_t refresh_ns = read_refresh();
		if (!refresh_ns)
		{
			return false;
		}
		read_shown();
		++m_present_id;
		m_time.presentID = m_present_id;
		m_time.desiredPresentTime = 0;
		//none shown yet: at once
		if (!m_shown_ns)
		{
			return true;
		}
		//the last one shown, a frame for each one after it, half a vsync before
		const uint64_t frame_ns = uint64_t(1.0e9 / double(frame_rate));
		const uint64_t frames = uint64_t(m_present_id - m_shown_id);
		m_time.desiredPresentTime = m_shown_ns + frames * frame_ns - refresh_ns / 2;
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// PRESENT_WAIT
	bool PresentTimingVK::present_wait()
	{
		using namespace AuxPresentTimingVK;
		if (!m_interval) return false;
		//a vsync a frame: the FIFO keeps it
		if (m_interval > 1 && m_wait_id)
		{
			//the last one shown: now (just after its vsync), else a frame after the one before
			const double frame_ms = double(m_interval) * m_vsync_ms;
			const double before = Time::get_ms_time();
			vkWaitForPresentKHR(m_device, m_swapchain, m_wait_id, s_wait_timeout_ns);
			const double after = Time::get_ms_time();
			m_wait_shown_ms = after - before > s_wait_at_once_ms ? after : m_wait_shown_ms + frame_ms;
			//this one its vsyncs after it, half a vsync before (late: at once)
			Time::wait_until_ms(m_wait_shown_ms + frame_ms - m_vsync_ms / 2.0);
		}
		++m_wait_id;
		return true;
	}
}
}
