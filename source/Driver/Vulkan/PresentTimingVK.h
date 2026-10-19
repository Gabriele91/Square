//
//  PresentTimingVK.h
//  Square, the driver of Vulkan: the presents at a frame rate, on the vsyncs of the display.
//  Their times asked (VK_GOOGLE_display_timing: Android, MoltenVK), else the last one waited
//  shown (VK_KHR_present_wait: Windows, Linux)
//
#pragma once
#include "Square/Config.h"
#include "VulkanConfig.h"

namespace Square
{
namespace Render
{
	class PresentTimingVK
	{
	public:
		enum class Method
		{
			NONE,
			DISPLAY_TIMING,
			PRESENT_WAIT
		};

		//the method of a device: its extensions, its features (in the chain of its create info)
		void choose(VkPhysicalDevice physical, uint32_t api_version, const Vector<VkExtensionProperties>& available, Vector<const char*>& extensions, void*& features);
		Method method() const;

		//a new swap chain
		void swapchain(VkDevice device, VkSwapchainKHR swapchain);
		void reset();

		//its frame rate (0: none) on a display (its refresh, Hz): false, not kept
		bool frame_rate(unsigned int fps, unsigned int refresh);
		//a present: its time or its id in its info
		void present(VkPresentInfoKHR& info);

	private:
		//the time of the next present (false: none)
		bool display_timing(unsigned int frame_rate);
		//the last present shown, then its vsyncs after it (false: none)
		bool present_wait();
		//the last present shown (its id, its time), the vsync of the display (0: unknown)
		void read_shown();
		uint64_t read_refresh() const;

		Method         m_method{ Method::NONE };
		unsigned int   m_frame_rate{ 0 };
		VkDevice       m_device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
		//DISPLAY_TIMING
		uint32_t       m_present_id{ 0 };
		uint32_t       m_shown_id{ 0 };
		uint64_t       m_shown_ns{ 0 };
		VkPresentTimeGOOGLE      m_time{};
		VkPresentTimesInfoGOOGLE m_times{ VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE };
		//PRESENT_WAIT
		uint64_t       m_wait_id{ 0 };
		double         m_wait_shown_ms{ 0.0 };
		unsigned int   m_interval{ 0 };     //vsyncs of a frame (0: not kept)
		double         m_vsync_ms{ 0.0 };
		VkPresentIdKHR m_ids{ VK_STRUCTURE_TYPE_PRESENT_ID_KHR };
		VkPhysicalDevicePresentIdFeaturesKHR   m_id_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR };
		VkPhysicalDevicePresentWaitFeaturesKHR m_wait_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR };
	};
}
}
