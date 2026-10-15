//
//  RenderVK.cpp
//  Square, the driver of Vulkan: the device, the swap chain, the frames, the states, the render
//  passes, the pipelines, the descriptors, the draws
//
#include "RenderVK.h"
#include "AllocationVK.h"
#include "Square/Driver/Window.h"
#if defined(__linux__)
	#include "SurfaceVK.h"
#endif
#include <algorithm>
#include <cstring>
#include <sstream>

namespace Square
{
namespace Render
{
	namespace AuxVK
	{
		//the bytes of the ring of a frame, a buffer of it (more when it is full)
		static constexpr VkDeviceSize s_ring_bytes = 4 * 1024 * 1024;
		//the zeros of the bindings and the attributes missing
		static constexpr VkDeviceSize s_zero_bytes = 64 * 1024;
		//the sets of a descriptor pool (more pools when it is full)
		static constexpr uint32_t s_pool_sets = 2048;

		static VkCompareOp compare(DepthFuncType type)
		{
			switch (type)
			{
			case DT_NEVER:         return VK_COMPARE_OP_NEVER;
			case DT_LESS:          return VK_COMPARE_OP_LESS;
			case DT_GREATER:       return VK_COMPARE_OP_GREATER;
			case DT_EQUAL:         return VK_COMPARE_OP_EQUAL;
			case DT_LESS_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
			case DT_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case DT_NOT_EQUAL:     return VK_COMPARE_OP_NOT_EQUAL;
			case DT_ALWAYS:        return VK_COMPARE_OP_ALWAYS;
			default:               return VK_COMPARE_OP_LESS;
			}
		}

		//a factor of the blend (the alpha: the *_COLOR factors as their *_ALPHA ones, as DirectX)
		static VkBlendFactor blend(BlendType type, bool alpha)
		{
			switch (type)
			{
			case BLEND_ZERO:                return VK_BLEND_FACTOR_ZERO;
			case BLEND_ONE:                 return VK_BLEND_FACTOR_ONE;
			case BLEND_ONE_MINUS_DST_COLOR: return alpha ? VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BLEND_ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case BLEND_ONE_MINUS_SRC_COLOR: return alpha ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BLEND_ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BLEND_DST_COLOR:           return alpha ? VK_BLEND_FACTOR_DST_ALPHA : VK_BLEND_FACTOR_DST_COLOR;
			case BLEND_DST_ALPHA:           return VK_BLEND_FACTOR_DST_ALPHA;
			case BLEND_SRC_COLOR:           return alpha ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_SRC_COLOR;
			case BLEND_SRC_ALPHA:           return VK_BLEND_FACTOR_SRC_ALPHA;
			case BLEND_SRC_ALPHA_SATURATE:  return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			default:                        return VK_BLEND_FACTOR_ONE;
			}
		}

		static VkPrimitiveTopology topology(DrawType type)
		{
			switch (type)
			{
			case DRAW_POINTS:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case DRAW_LINES:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case DRAW_LINE_LOOP:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case DRAW_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case DRAW_TRIANGLES:
			default:                  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			}
		}

		static VkFormat attribute_format(AttributeStripType type)
		{
			switch (type)
			{
			case AST_FLOAT:  case AST_TLESS:  return VK_FORMAT_R32_SFLOAT;
			case AST_FLOAT2: case AST_TLESS2: return VK_FORMAT_R32G32_SFLOAT;
			case AST_FLOAT3: case AST_TLESS3: return VK_FORMAT_R32G32B32_SFLOAT;
			case AST_FLOAT4: case AST_TLESS4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case AST_INT:    return VK_FORMAT_R32_SINT;
			case AST_INT2:   return VK_FORMAT_R32G32_SINT;
			case AST_INT3:   return VK_FORMAT_R32G32B32_SINT;
			case AST_INT4:   return VK_FORMAT_R32G32B32A32_SINT;
			case AST_UINT:   return VK_FORMAT_R32_UINT;
			case AST_UINT2:  return VK_FORMAT_R32G32_UINT;
			case AST_UINT3:  return VK_FORMAT_R32G32B32_UINT;
			case AST_UINT4:  return VK_FORMAT_R32G32B32A32_UINT;
			default:         return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}

		//the format of an input of a shader without its attribute (read from the zeros)
		static VkFormat input_format(const SpirvReflection::Input& input)
		{
			static const VkFormat floats[] = { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };
			static const VkFormat ints[] = { VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT };
			static const VkFormat uints[] = { VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT };
			const uint32_t index = std::min<uint32_t>(std::max<uint32_t>(input.m_components, 1), 4) - 1;
			if (input.m_integer)
			{
				return input.m_signed ? ints[index] : uints[index];
			}
			return floats[index];
		}

		static bool has_stencil(VkFormat format)
		{
			return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
		}

		//the messages of the validation layers into the log of the engine
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_message
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* data,
			void* user
		)
		{
			Logger* logger = static_cast<Logger*>(user);
			if (logger && data && data->pMessage)
			{
				const String message = String("Vulkan: ") + data->pMessage;
				if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				{
					logger->error(message);
				}
				else
				{
					logger->warning(message);
				}
			}
			return VK_FALSE;
		}

		static bool has_extension(const Vector<VkExtensionProperties>& extensions, const char* name)
		{
			for (const VkExtensionProperties& extension : extensions)
			{
				if (std::strcmp(extension.extensionName, name) == 0) return true;
			}
			return false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Device
	RenderDriver ContextVK::get_render_driver()
	{
		return DR_VULKAN;
	}

	const RenderDriverInfo& ContextVK::get_render_driver_info() const
	{
		return m_info;
	}

	void ContextVK::print_info()
	{
		logger()->info(String("Renderer: Vulkan, ") + m_properties.deviceName
		               + " (API " + to_string(VK_API_VERSION_MAJOR(m_properties.apiVersion))
		               + "." + to_string(VK_API_VERSION_MINOR(m_properties.apiVersion))
		               + "." + to_string(VK_API_VERSION_PATCH(m_properties.apiVersion)) + ")");
	}

	bool ContextVK::is_srgb_framebuffer() const
	{
		return m_srgb;
	}

	bool ContextVK::init(Video::DeviceResources* resource)
	{
		m_resources = resource;
		if (!m_resources)
		{
			logger()->error("Vulkan: no window (device resources)");
			return false;
		}
		if (volkInitialize() != VK_SUCCESS)
		{
			logger()->error("Vulkan: no loader (vulkan-1)");
			return false;
		}
		const Video::ContextInfo& context = m_resources->get_context_info();
		m_srgb = context.m_srgb;
		if (!create_instance(context.m_debug)) return false;
#if defined(_WIN32)
		VkWin32SurfaceCreateInfoKHR surface_info{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surface_info.hinstance = (HINSTANCE)m_resources->get_device();
		surface_info.hwnd = (HWND)m_resources->get_device_context();
		if (vkCreateWin32SurfaceKHR(m_instance, &surface_info, AllocationVK::callbacks(), &m_surface) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the surface of the window");
			return false;
		}
#elif defined(__linux__)
		if (SurfaceVK::create(m_instance, m_resources, &m_surface) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the surface of the window");
			return false;
		}
#endif
		if (!pick_device(context.m_gpu_type)) return false;
		if (!create_device()) return false;
		if (!create_frames()) return false;
		compute_driver_info();
		//the zeros: the bindings without a buffer, the attributes without their input
		m_zero_buffer = create_buffer(AuxVK::s_zero_bytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true);
		if (m_zero_buffer.m_mapped)
		{
			std::memset(m_zero_buffer.m_mapped, 0, size_t(AuxVK::s_zero_bytes));
		}
		if (!create_swapchain()) return false;
		//the zeros of the textures not bound (made now: not while a draw records its pass)
		for (bool arrayed : { false, true })
		{
			dummy_texture(SpirvReflection::ImageDim::DIM_2D, arrayed);
			if (m_features.imageCubeArray || !arrayed)
			{
				dummy_texture(SpirvReflection::ImageDim::DIM_CUBE, arrayed);
			}
		}
		//the swap of the window: the frame presented
		m_resources->callback_target_changed([this](Video::DeviceResources*) { present_frame(); });
		//the states of DirectX at the start
		m_state.m_cullface.m_cullface = CF_BACK;
		m_state.m_depth.m_mode = DM_ENABLE_AND_WRITE;
		m_state.m_depth.m_type = DT_LESS;
		m_state.m_blend = BlendState();
		m_state.m_viewport.m_viewport = Vec4(0.0f);
		m_state.m_clear_color.m_color = Vec4(0.1f, 0.5f, 1.0f, 1.0f);
		m_clear_color = false;
		m_clear_depth = false;
		return true;
	}

	bool ContextVK::create_instance(bool debug)
	{
		uint32_t loader = VK_API_VERSION_1_0;
		if (vkEnumerateInstanceVersion)
		{
			vkEnumerateInstanceVersion(&loader);
		}
		if (loader < VK_API_VERSION_1_1)
		{
			logger()->error("Vulkan: the loader has not Vulkan 1.1");
			return false;
		}
		m_api_version = std::min<uint32_t>(loader, VK_API_VERSION_1_2);
		//extensions: the surface of the window, the messages of the validation
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		Vector<VkExtensionProperties> available(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());
		Vector<const char*> extensions{ VK_KHR_SURFACE_EXTENSION_NAME };
#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
		extensions.push_back(SurfaceVK::platform_extension());
#endif
		const bool messages = debug && AuxVK::has_extension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		if (messages)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		//the validation layer (debug, when it is installed: the Vulkan SDK)
		Vector<const char*> layers;
		if (debug)
		{
			uint32_t layer_count = 0;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			Vector<VkLayerProperties> available_layers(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
			for (const VkLayerProperties& layer : available_layers)
			{
				if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
				{
					layers.push_back("VK_LAYER_KHRONOS_validation");
				}
			}
			if (layers.empty())
			{
				logger()->warning("Vulkan: no validation layer (the Vulkan SDK)");
			}
		}
		VkApplicationInfo application{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		application.pApplicationName = "Square";
		application.pEngineName = "Square";
		application.apiVersion = m_api_version;
		VkInstanceCreateInfo info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		info.pApplicationInfo = &application;
		info.enabledExtensionCount = uint32_t(extensions.size());
		info.ppEnabledExtensionNames = extensions.data();
		info.enabledLayerCount = uint32_t(layers.size());
		info.ppEnabledLayerNames = layers.data();
		if (vkCreateInstance(&info, AllocationVK::callbacks(), &m_instance) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the instance");
			return false;
		}
		volkLoadInstance(m_instance);
		if (messages)
		{
			VkDebugUtilsMessengerCreateInfoEXT messenger{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
			messenger.pfnUserCallback = AuxVK::debug_message;
			messenger.pUserData = logger();
			vkCreateDebugUtilsMessengerEXT(m_instance, &messenger, AllocationVK::callbacks(), &m_messenger);
		}
		return true;
	}

	bool ContextVK::pick_device(Video::ContextInfo::gpu_type gpu)
	{
		uint32_t count = 0;
		vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
		Vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(m_instance, &count, devices.data());
		int best_score = -1;
		for (VkPhysicalDevice device : devices)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(device, &properties);
			if (properties.apiVersion < VK_API_VERSION_1_1) continue;
			//a queue of the draws that presents to the window
			uint32_t families = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &families, nullptr);
			Vector<VkQueueFamilyProperties> family_properties(families);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &families, family_properties.data());
			int family = -1;
			for (uint32_t i = 0; i != families && family < 0; ++i)
			{
				VkBool32 present = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present);
				if ((family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present)
				{
					family = int(i);
				}
			}
			if (family < 0) continue;
			//the swap chain
			uint32_t extension_count = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
			Vector<VkExtensionProperties> extensions(extension_count);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());
			if (!AuxVK::has_extension(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;
			//the kind asked first (low: the integrated one; else the discrete one)
			int score = 1;
			const bool discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			const bool integrated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
			if (gpu == Video::ContextInfo::GPU_TYPE_LOW)
			{
				score += integrated ? 100 : (discrete ? 50 : 0);
			}
			else
			{
				score += discrete ? 100 : (integrated ? 50 : 0);
			}
			if (score > best_score)
			{
				best_score = score;
				m_physical = device;
				m_queue_family = uint32_t(family);
				m_properties = properties;
			}
		}
		if (m_physical == VK_NULL_HANDLE)
		{
			logger()->error("Vulkan: no device of Vulkan 1.1 that draws into the window");
			return false;
		}
		vkGetPhysicalDeviceFeatures(m_physical, &m_features);
		m_api_version = std::min<uint32_t>(m_api_version, m_properties.apiVersion);
		return true;
	}

	bool ContextVK::create_device()
	{
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(m_physical, nullptr, &extension_count, nullptr);
		Vector<VkExtensionProperties> available(extension_count);
		vkEnumerateDeviceExtensionProperties(m_physical, nullptr, &extension_count, available.data());
		Vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		//the features used, when the device has them
		VkPhysicalDeviceFeatures enabled{};
		enabled.geometryShader = m_features.geometryShader;
		enabled.samplerAnisotropy = m_features.samplerAnisotropy;
		enabled.fillModeNonSolid = m_features.fillModeNonSolid;
		enabled.textureCompressionBC = m_features.textureCompressionBC;
		enabled.textureCompressionASTC_LDR = m_features.textureCompressionASTC_LDR;
		enabled.imageCubeArray = m_features.imageCubeArray;
		enabled.shaderClipDistance = m_features.shaderClipDistance;
		enabled.shaderCullDistance = m_features.shaderCullDistance;
		enabled.independentBlend = m_features.independentBlend;
		enabled.depthClamp = m_features.depthClamp;
		//the layer from the vertex shader: Vulkan 1.2 (shaderOutputLayer), else its extension
		VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan12Features enabled12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		m_vertex_layer = false;
		if (m_api_version >= VK_API_VERSION_1_2)
		{
			features2.pNext = &features12;
			vkGetPhysicalDeviceFeatures2(m_physical, &features2);
			if (features12.shaderOutputLayer && features12.shaderOutputViewportIndex)
			{
				enabled12.shaderOutputLayer = VK_TRUE;
				enabled12.shaderOutputViewportIndex = VK_TRUE;
				m_vertex_layer = true;
			}
		}
		else if (AuxVK::has_extension(available, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME))
		{
			extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
			m_vertex_layer = true;
		}
		const float priority = 1.0f;
		VkDeviceQueueCreateInfo queue{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queue.queueFamilyIndex = m_queue_family;
		queue.queueCount = 1;
		queue.pQueuePriorities = &priority;
		VkDeviceCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queue;
		info.enabledExtensionCount = uint32_t(extensions.size());
		info.ppEnabledExtensionNames = extensions.data();
		info.pEnabledFeatures = &enabled;
		if (m_api_version >= VK_API_VERSION_1_2)
		{
			info.pNext = &enabled12;
		}
		if (vkCreateDevice(m_physical, &info, AllocationVK::callbacks(), &m_device) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the device");
			return false;
		}
		volkLoadDevice(m_device);
		vkGetDeviceQueue(m_device, m_queue_family, 0, &m_queue);
		//the memory (VMA, its functions by volk)
		VmaVulkanFunctions functions{};
		functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		VmaAllocatorCreateInfo allocator_info{};
		allocator_info.vulkanApiVersion = m_api_version;
		allocator_info.physicalDevice = m_physical;
		allocator_info.device = m_device;
		allocator_info.instance = m_instance;
		allocator_info.pVulkanFunctions = &functions;
		allocator_info.pAllocationCallbacks = AllocationVK::callbacks();
		if (vmaCreateAllocator(&allocator_info, &m_allocator) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the allocator of the memory");
			return false;
		}
		VkPipelineCacheCreateInfo cache{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		vkCreatePipelineCache(m_device, &cache, AllocationVK::callbacks(), &m_pipeline_cache);
		m_ring_alignment = std::max<VkDeviceSize>(m_properties.limits.minUniformBufferOffsetAlignment, 16);
		return true;
	}

	bool ContextVK::create_frames()
	{
		for (FrameVK& frame : m_frames)
		{
			VkCommandPoolCreateInfo pool{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			pool.queueFamilyIndex = m_queue_family;
			if (vkCreateCommandPool(m_device, &pool, AllocationVK::callbacks(), &frame.m_pool) != VK_SUCCESS) return false;
			VkCommandBuffer buffers[2];
			VkCommandBufferAllocateInfo allocate{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			allocate.commandPool = frame.m_pool;
			allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocate.commandBufferCount = 2;
			if (vkAllocateCommandBuffers(m_device, &allocate, buffers) != VK_SUCCESS) return false;
			frame.m_commands = buffers[0];
			frame.m_uploads = buffers[1];
			VkFenceCreateInfo fence{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(m_device, &fence, AllocationVK::callbacks(), &frame.m_fence) != VK_SUCCESS) return false;
			VkSemaphoreCreateInfo semaphore{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			if (vkCreateSemaphore(m_device, &semaphore, AllocationVK::callbacks(), &frame.m_acquired) != VK_SUCCESS) return false;
#if defined(RENDER_PROFILER)
			if (m_properties.limits.timestampComputeAndGraphics)
			{
				VkQueryPoolCreateInfo queries{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
				queries.queryType = VK_QUERY_TYPE_TIMESTAMP;
				queries.queryCount = GPU_TIMER_MAX_TIMESTAMPS;
				vkCreateQueryPool(m_device, &queries, AllocationVK::callbacks(), &frame.m_queries);
			}
#endif
		}
		VkFenceCreateInfo fence{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		return vkCreateFence(m_device, &fence, AllocationVK::callbacks(), &m_wait_fence) == VK_SUCCESS;
	}

	void ContextVK::compute_driver_info()
	{
		m_info.m_render_driver = DR_VULKAN;
		m_info.m_name = m_properties.deviceName;
		m_info.m_major_version = int(VK_API_VERSION_MAJOR(m_api_version));
		m_info.m_minor_version = int(VK_API_VERSION_MINOR(m_api_version));
		m_info.m_shader_language = "SPIRV";
		m_info.m_shader_version = 100;
		m_info.m_geometry_shader = m_features.geometryShader == VK_TRUE;
		m_info.m_vertex_viewport_index = m_vertex_layer;
		m_info.m_draw_instanced = true;
		m_info.m_texture_bc = m_features.textureCompressionBC == VK_TRUE;
		m_info.m_texture_astc = m_features.textureCompressionASTC_LDR == VK_TRUE;
#if defined(RENDER_PROFILER)
		m_gpu_timer_supported = m_properties.limits.timestampComputeAndGraphics && m_frames[0].m_queries != VK_NULL_HANDLE;
#endif
	}

	void ContextVK::close()
	{
		if (m_device == VK_NULL_HANDLE)
		{
			if (m_instance != VK_NULL_HANDLE)
			{
				if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, AllocationVK::callbacks());
				if (m_messenger) vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, AllocationVK::callbacks());
				vkDestroyInstance(m_instance, AllocationVK::callbacks());
				m_instance = VK_NULL_HANDLE;
			}
			return;
		}
		if (m_resources)
		{
			m_resources->callback_target_changed(nullptr);
			m_resources = nullptr;
		}
		vkDeviceWaitIdle(m_device);
		m_frame_open = false;
		//what was deleted
		destroy_garbage(true);
		destroy_swapchain();
		for (auto& pipeline : m_pipelines)
		{
			if (pipeline.second) vkDestroyPipeline(m_device, pipeline.second, AllocationVK::callbacks());
		}
		m_pipelines.clear();
		for (auto& pass : m_render_passes) vkDestroyRenderPass(m_device, pass.second, AllocationVK::callbacks());
		m_render_passes.clear();
		for (auto& dummy : m_dummy_textures)
		{
			for (auto& view : dummy.second->m_views) vkDestroyImageView(m_device, view.second, AllocationVK::callbacks());
			vmaDestroyImage(m_allocator, dummy.second->m_image, dummy.second->m_allocation);
		}
		m_dummy_textures.clear();
		for (auto& sampler : m_samplers) vkDestroySampler(m_device, sampler.second, AllocationVK::callbacks());
		m_samplers.clear();
		if (m_zero_buffer.m_buffer) vmaDestroyBuffer(m_allocator, m_zero_buffer.m_buffer, m_zero_buffer.m_allocation);
		m_zero_buffer = BufferVK();
		for (FrameVK& frame : m_frames)
		{
			for (BufferVK& ring : frame.m_ring) vmaDestroyBuffer(m_allocator, ring.m_buffer, ring.m_allocation);
			frame.m_ring.clear();
			for (VkDescriptorPool pool : frame.m_descriptor_pools) vkDestroyDescriptorPool(m_device, pool, AllocationVK::callbacks());
			frame.m_descriptor_pools.clear();
			if (frame.m_pool) vkDestroyCommandPool(m_device, frame.m_pool, AllocationVK::callbacks());
			if (frame.m_fence) vkDestroyFence(m_device, frame.m_fence, AllocationVK::callbacks());
			if (frame.m_acquired) vkDestroySemaphore(m_device, frame.m_acquired, AllocationVK::callbacks());
#if defined(RENDER_PROFILER)
			if (frame.m_queries) vkDestroyQueryPool(m_device, frame.m_queries, AllocationVK::callbacks());
#endif
			frame = FrameVK();
		}
		if (m_wait_fence) vkDestroyFence(m_device, m_wait_fence, AllocationVK::callbacks());
		m_wait_fence = VK_NULL_HANDLE;
		if (m_pipeline_cache) vkDestroyPipelineCache(m_device, m_pipeline_cache, AllocationVK::callbacks());
		m_pipeline_cache = VK_NULL_HANDLE;
		if (m_allocator) vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;
		vkDestroyDevice(m_device, AllocationVK::callbacks());
		m_device = VK_NULL_HANDLE;
		if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, AllocationVK::callbacks());
		m_surface = VK_NULL_HANDLE;
		if (m_messenger) vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, AllocationVK::callbacks());
		m_messenger = VK_NULL_HANDLE;
		vkDestroyInstance(m_instance, AllocationVK::callbacks());
		m_instance = VK_NULL_HANDLE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Swap chain
	bool ContextVK::create_swapchain()
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical, m_surface, &capabilities);
		VkExtent2D extent = capabilities.currentExtent;
		if (extent.width == 0xFFFFFFFF)
		{
			extent.width = std::clamp(m_resources->width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(m_resources->height(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}
		m_swapchain.m_extent = extent;
		m_swapchain.m_vsync = m_resources->get_vsync();
		m_swapchain.m_dirty = false;
		//minimized: no swap chain (nothing drawn into the window)
		if (extent.width == 0 || extent.height == 0)
		{
			return true;
		}
		//its format: BGRA 8 bit, sRGB when the window asks it
		uint32_t count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_surface, &count, nullptr);
		Vector<VkSurfaceFormatKHR> formats(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_surface, &count, formats.data());
		const VkFormat wanted[] =
		{
			m_srgb ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM,
			m_srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM
		};
		VkSurfaceFormatKHR format = formats.empty() ? VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } : formats[0];
		bool found = false;
		for (VkFormat want : wanted)
		{
			for (const VkSurfaceFormatKHR& available : formats)
			{
				if (!found && available.format == want && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					format = available;
					found = true;
				}
			}
		}
		//vsync: FIFO; else at once (mailbox, immediate)
		uint32_t mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_surface, &mode_count, nullptr);
		Vector<VkPresentModeKHR> modes(mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_surface, &mode_count, modes.data());
		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
		if (!m_swapchain.m_vsync)
		{
			if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != modes.end())
			{
				mode = VK_PRESENT_MODE_MAILBOX_KHR;
			}
			else if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != modes.end())
			{
				mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
		uint32_t images = std::max<uint32_t>(capabilities.minImageCount + 1, 2);
		if (capabilities.maxImageCount)
		{
			images = std::min(images, capabilities.maxImageCount);
		}
		VkSwapchainKHR old = m_swapchain.m_swapchain;
		VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		info.surface = m_surface;
		info.minImageCount = images;
		info.imageFormat = format.format;
		info.imageColorSpace = format.colorSpace;
		info.imageExtent = extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		                | (capabilities.supportedUsageFlags & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform = capabilities.currentTransform;
		info.compositeAlpha = (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		                    ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
		                    : VkCompositeAlphaFlagBitsKHR(capabilities.supportedCompositeAlpha & -int(capabilities.supportedCompositeAlpha));
		info.presentMode = mode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = old;
		if (vkCreateSwapchainKHR(m_device, &info, AllocationVK::callbacks(), &m_swapchain.m_swapchain) != VK_SUCCESS)
		{
			logger()->error("Vulkan: unable to make the swap chain");
			m_swapchain.m_swapchain = old;
			return false;
		}
		if (old)
		{
			vkDestroySwapchainKHR(m_device, old, AllocationVK::callbacks());
		}
		m_swapchain.m_format = format.format;
		//its images (as textures), the semaphores of their presents
		uint32_t image_count = 0;
		vkGetSwapchainImagesKHR(m_device, m_swapchain.m_swapchain, &image_count, nullptr);
		Vector<VkImage> swap_images(image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain.m_swapchain, &image_count, swap_images.data());
		for (VkImage image : swap_images)
		{
			auto texture = MakeUniquePtr<Texture>();
			texture->m_image = image;
			texture->m_format = format.format;
			texture->m_width = extent.width;
			texture->m_height = extent.height;
			texture->m_srgb = m_srgb;
			m_swapchain.m_images.push_back(std::move(texture));
			VkSemaphore semaphore = VK_NULL_HANDLE;
			VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vkCreateSemaphore(m_device, &semaphore_info, AllocationVK::callbacks(), &semaphore);
			m_swapchain.m_presented.push_back(semaphore);
		}
		//the depth of the window
		const Video::ContextInfo& context = m_resources->get_context_info();
		TextureFormat depth = TF_DEPTH24_STENCIL8;
		if (context.m_depth == 32) depth = context.m_stencil ? TF_DEPTH32_STENCIL8 : TF_DEPTH_COMPONENT32;
		if (context.m_depth == 16) depth = TF_DEPTH_COMPONENT16;
		TextureRawDataInformation depth_data{ depth, extent.width, extent.height, nullptr, TT_DEPTH_STENCIL, TTF_FLOAT, false, 1 };
		TextureGpuDataInformation depth_info(TMIN_NEAREST, TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP);
		m_default_depth.reset(make_texture(&depth_data, 1, false, depth_info));
		//the default target
		m_default_target.m_colors.assign(1, Target::Attachment{ nullptr, RT_COLOR, -1 });
		m_default_target.m_depth = Target::Attachment{ m_default_depth.get(), RT_DEPTH_STENCIL, -1 };
		m_default_target.m_width = extent.width;
		m_default_target.m_height = extent.height;
		m_default_target.m_layers = 1;
		return true;
	}

	void ContextVK::destroy_swapchain()
	{
		if (m_device == VK_NULL_HANDLE) return;
		vkDeviceWaitIdle(m_device);
		for (auto& framebuffer : m_default_target.m_framebuffers) vkDestroyFramebuffer(m_device, framebuffer.second, AllocationVK::callbacks());
		m_default_target.m_framebuffers.clear();
		for (auto& image : m_swapchain.m_images)
		{
			for (auto& view : image->m_views) vkDestroyImageView(m_device, view.second, AllocationVK::callbacks());
		}
		m_swapchain.m_images.clear();
		for (VkSemaphore semaphore : m_swapchain.m_presented) vkDestroySemaphore(m_device, semaphore, AllocationVK::callbacks());
		m_swapchain.m_presented.clear();
		if (m_default_depth)
		{
			for (auto& view : m_default_depth->m_views) vkDestroyImageView(m_device, view.second, AllocationVK::callbacks());
			vmaDestroyImage(m_allocator, m_default_depth->m_image, m_default_depth->m_allocation);
			m_default_depth.reset();
		}
		if (m_swapchain.m_swapchain)
		{
			vkDestroySwapchainKHR(m_device, m_swapchain.m_swapchain, AllocationVK::callbacks());
			m_swapchain.m_swapchain = VK_NULL_HANDLE;
		}
		m_swapchain.m_acquired = false;
	}

	bool ContextVK::acquire_image()
	{
		if (m_swapchain.m_acquired) return true;
		//the window changed (its size, its vsync): the swap chain made again
		const bool resized = m_resources->width() != m_swapchain.m_extent.width || m_resources->height() != m_swapchain.m_extent.height;
		const bool vsync = m_resources->get_vsync() != m_swapchain.m_vsync;
		if (m_swapchain.m_dirty || resized || vsync || !m_swapchain.m_swapchain)
		{
			end_render_pass();
			//(the frame recorded so far: done first, the images of the old one not used again)
			if (m_frame_open)
			{
				flush_and_wait();
			}
			destroy_swapchain();
			if (!create_swapchain()) return false;
		}
		if (!m_swapchain.m_swapchain) return false;
		begin_frame();
		uint32_t image = 0;
		VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain.m_swapchain, UINT64_MAX, frame().m_acquired, VK_NULL_HANDLE, &image);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_swapchain.m_dirty = true;
			return false;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			return false;
		}
		m_swapchain.m_dirty = result == VK_SUBOPTIMAL_KHR;
		m_swapchain.m_image = image;
		m_swapchain.m_acquired = true;
		m_swapchain.m_acquired_waited = false;
		//its last frame was presented: nothing of it kept
		Texture* texture = m_swapchain.m_images[image].get();
		texture->m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_default_target.m_colors[0].m_texture = texture;
		return true;
	}

	void ContextVK::present_frame()
	{
		if (!m_frame_open) return;
		const bool present = m_swapchain.m_acquired;
		const uint32_t image = m_swapchain.m_image;
		submit_frame(present);
		if (present)
		{
			VkPresentInfoKHR info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &m_swapchain.m_presented[image];
			info.swapchainCount = 1;
			info.pSwapchains = &m_swapchain.m_swapchain;
			info.pImageIndices = &image;
			const VkResult result = vkQueuePresentKHR(m_queue, &info);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				m_swapchain.m_dirty = true;
			}
		}
		m_swapchain.m_acquired = false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Frames
	void ContextVK::begin_frame()
	{
		if (m_frame_open) return;
		FrameVK& current = frame();
		//the GPU done with the frame of this slot (VK_FRAMES ago)
		vkWaitForFences(m_device, 1, &current.m_fence, VK_TRUE, UINT64_MAX);
		if (m_frame_index + 1 > VK_FRAMES)
		{
			m_completed_frame = std::max<uint64>(m_completed_frame, m_frame_index + 1 - VK_FRAMES);
		}
		vkResetFences(m_device, 1, &current.m_fence);
		destroy_garbage(false);
		current.m_staging_bytes = 0;
		current.m_ring_buffer = 0;
		current.m_ring_offset = 0;
		for (VkDescriptorPool pool : current.m_descriptor_pools) vkResetDescriptorPool(m_device, pool, 0);
		current.m_descriptor_pool = 0;
		vkResetCommandPool(m_device, current.m_pool, 0);
		VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(current.m_commands, &begin);
		current.m_uploads_open = false;
		m_frame_open = true;
		m_pass_open = false;
		m_bound_pipeline = VK_NULL_HANDLE;
		m_dynamic_dirty = true;
	}

	VkCommandBuffer ContextVK::commands()
	{
		begin_frame();
		return frame().m_commands;
	}

	VkCommandBuffer ContextVK::uploads()
	{
		begin_frame();
		FrameVK& current = frame();
		if (!current.m_uploads_open)
		{
			VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(current.m_uploads, &begin);
			current.m_uploads_open = true;
		}
		return current.m_uploads;
	}

	void ContextVK::submit_frame(bool present)
	{
		FrameVK& current = frame();
		end_render_pass();
		if (present)
		{
			transition(current.m_commands, m_swapchain.m_images[m_swapchain.m_image].get(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
#if defined(RENDER_PROFILER)
		if (m_gpu_timer_in_frame)
		{
			gpu_timer_end_frame();
		}
#endif
		Vector<VkCommandBuffer> buffers;
		if (current.m_uploads_open)
		{
			//the uploads seen by the draws after them
			VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			vkCmdPipelineBarrier(current.m_uploads, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			vkEndCommandBuffer(current.m_uploads);
			buffers.push_back(current.m_uploads);
			current.m_uploads_open = false;
		}
		vkEndCommandBuffer(current.m_commands);
		buffers.push_back(current.m_commands);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = uint32_t(buffers.size());
		submit.pCommandBuffers = buffers.data();
		const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		if (m_swapchain.m_acquired && !m_swapchain.m_acquired_waited)
		{
			submit.waitSemaphoreCount = 1;
			submit.pWaitSemaphores = &current.m_acquired;
			submit.pWaitDstStageMask = &wait_stage;
			m_swapchain.m_acquired_waited = true;
		}
		if (present)
		{
			submit.signalSemaphoreCount = 1;
			submit.pSignalSemaphores = &m_swapchain.m_presented[m_swapchain.m_image];
		}
		if (vkQueueSubmit(m_queue, 1, &submit, current.m_fence) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: the submit of the frame failed");
		}
		m_frame_open = false;
		m_pass_open = false;
		m_frame_index += 1;
	}

	void ContextVK::flush_and_wait()
	{
		//what is recorded done by the GPU, the frame open again (in the same slot: its ring, its
		//descriptors, its garbage kept until the frame is presented)
		if (!m_frame_open) return;
		FrameVK& current = frame();
		end_render_pass();
		Vector<VkCommandBuffer> buffers;
		if (current.m_uploads_open)
		{
			vkEndCommandBuffer(current.m_uploads);
			buffers.push_back(current.m_uploads);
			current.m_uploads_open = false;
		}
		vkEndCommandBuffer(current.m_commands);
		buffers.push_back(current.m_commands);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = uint32_t(buffers.size());
		submit.pCommandBuffers = buffers.data();
		const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		if (m_swapchain.m_acquired && !m_swapchain.m_acquired_waited)
		{
			submit.waitSemaphoreCount = 1;
			submit.pWaitSemaphores = &current.m_acquired;
			submit.pWaitDstStageMask = &wait_stage;
			m_swapchain.m_acquired_waited = true;
		}
		vkResetFences(m_device, 1, &m_wait_fence);
		vkQueueSubmit(m_queue, 1, &submit, m_wait_fence);
		vkWaitForFences(m_device, 1, &m_wait_fence, VK_TRUE, UINT64_MAX);
		//every frame before this one is done too (the queue in its order): what they deleted, the
		//staging buffers of the uploads, deleted now
		m_completed_frame = std::max<uint64>(m_completed_frame, m_frame_index);
		destroy_garbage(true);
		current.m_staging_bytes = 0;
		vkResetCommandBuffer(current.m_commands, 0);
		vkResetCommandBuffer(current.m_uploads, 0);
		VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(current.m_commands, &begin);
		m_bound_pipeline = VK_NULL_HANDLE;
		m_dynamic_dirty = true;
#if defined(RENDER_PROFILER)
		//(its timestamps lost: the queries of the frame reset at its start)
		m_gpu_timer_in_frame = false;
#endif
	}

	void ContextVK::destroy_later(std::function<void()> destroy)
	{
		//the last frame that may use it: the one recording, else the last one submitted
		const uint64 last = m_frame_open ? m_frame_index : (m_frame_index ? m_frame_index - 1 : 0);
		m_garbage.push_back({ last, std::move(destroy) });
	}

	void ContextVK::destroy_garbage(bool all)
	{
		//what the frames done do not use any more (all: the GPU idle, everything recorded done)
		auto done = [this, all](const std::pair<uint64, std::function<void()>>& garbage) { return all || garbage.first < m_completed_frame; };
		for (auto& garbage : m_garbage)
		{
			if (done(garbage)) garbage.second();
		}
		m_garbage.erase(std::remove_if(m_garbage.begin(), m_garbage.end(), done), m_garbage.end());
	}

	SliceVK ContextVK::ring_copy(const void* data, size_t size, size_t reserve)
	{
		begin_frame();
		FrameVK& current = frame();
		const VkDeviceSize bytes = std::max<VkDeviceSize>(VkDeviceSize(std::max(size, reserve)), 16);
		VkDeviceSize offset = (current.m_ring_offset + m_ring_alignment - 1) & ~(m_ring_alignment - 1);
		while (current.m_ring_buffer < current.m_ring.size() && offset + bytes > current.m_ring[current.m_ring_buffer].m_size)
		{
			current.m_ring_buffer += 1;
			offset = 0;
		}
		if (current.m_ring_buffer == current.m_ring.size())
		{
			current.m_ring.push_back(create_buffer(std::max(AuxVK::s_ring_bytes, bytes), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true));
			offset = 0;
		}
		BufferVK& ring = current.m_ring[current.m_ring_buffer];
		if (data && size)
		{
			std::memcpy(ring.m_mapped + offset, data, size);
		}
		if (bytes > size)
		{
			std::memset(ring.m_mapped + offset + size, 0, size_t(bytes - size));
		}
		current.m_ring_offset = offset + bytes;
		SliceVK slice;
		slice.m_buffer = ring.m_buffer;
		slice.m_offset = offset;
		slice.m_frame = m_frame_index;
		return slice;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// States
	const ClearColorState& ContextVK::get_clear_color_state() { return m_state.m_clear_color; }
	void ContextVK::set_clear_color_state(const ClearColorState& cf) { m_state.m_clear_color = cf; }

	const DepthBufferState& ContextVK::get_depth_buffer_state() { return m_state.m_depth; }
	void ContextVK::set_depth_buffer_state(const DepthBufferState& cf) { m_state.m_depth = cf; }

	const CullfaceState& ContextVK::get_cullface_state() { return m_state.m_cullface; }
	void ContextVK::set_cullface_state(const CullfaceState& cf) { m_state.m_cullface = cf; }

	const ViewportState& ContextVK::get_viewport_state() { return m_state.m_viewport; }
	void ContextVK::set_viewport_state(const ViewportState& vs)
	{
		if (m_state.m_viewport != vs)
		{
			m_state.m_viewport = vs;
			m_dynamic_dirty = true;
		}
	}

	const ScissorState& ContextVK::get_scissor_state() { return m_scissor; }
	void ContextVK::set_scissor_state(const ScissorState& ss)
	{
		if (m_scissor != ss)
		{
			m_scissor = ss;
			m_dynamic_dirty = true;
		}
	}

	const BlendState& ContextVK::get_blend_state() { return m_state.m_blend; }
	void ContextVK::set_blend_state(const BlendState& bs) { m_state.m_blend = bs; }

	const State& ContextVK::get_render_state() { return m_state; }
	void ContextVK::set_render_state(const State& rs)
	{
		set_clear_color_state(rs.m_clear_color);
		set_cullface_state(rs.m_cullface);
		set_viewport_state(rs.m_viewport);
		set_depth_buffer_state(rs.m_depth);
		set_blend_state(rs.m_blend);
	}

	void ContextVK::clear(int type)
	{
		//the render pass of the target open: its attachments cleared now (all of them, not by the
		//scissor), else at its load (its next pass), as DirectX clears the target at once
		Target* target = current_target();
		if (!target) return;
		if (m_pass_open && m_pass_target == target)
		{
			Vector<VkClearAttachment> clears;
			if (type & CLEAR_COLOR)
			{
				for (uint32_t i = 0; i != uint32_t(target->m_colors.size()); ++i)
				{
					VkClearAttachment clear{};
					clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					clear.colorAttachment = i;
					const Vec4& color = m_state.m_clear_color.m_color;
					clear.clearValue.color = { { color.x, color.y, color.z, color.w } };
					clears.push_back(clear);
				}
			}
			if ((type & CLEAR_DEPTH) && target->m_depth.m_texture)
			{
				VkClearAttachment clear{};
				clear.aspectMask = target->m_depth.m_texture->m_aspect;
				clear.clearValue.depthStencil = { 1.0f, 0 };
				clears.push_back(clear);
			}
			if (clears.size())
			{
				VkClearRect rect{};
				rect.rect.extent = { target->m_width, target->m_height };
				rect.layerCount = target->m_layers;
				vkCmdClearAttachments(commands(), uint32_t(clears.size()), clears.data(), 1, &rect);
			}
		}
		else
		{
			m_clear_color = m_clear_color || (type & CLEAR_COLOR) != 0;
			m_clear_depth = m_clear_depth || (type & CLEAR_DEPTH) != 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Targets, render passes
	Target* ContextVK::current_target()
	{
		if (m_target) return m_target;
		return acquire_image() ? &m_default_target : nullptr;
	}

	RenderPassKeyVK ContextVK::render_pass_key(const Target* target, bool clear_color, bool clear_depth) const
	{
		RenderPassKeyVK key;
		key.m_count = uint32_t(std::min<size_t>(target->m_colors.size(), VK_MAX_COLORS));
		for (uint32_t i = 0; i != key.m_count; ++i)
		{
			const Texture* texture = target->m_colors[i].m_texture;
			key.m_colors[i] = texture ? texture->m_format : VK_FORMAT_UNDEFINED;
		}
		key.m_depth = target->m_depth.m_texture ? target->m_depth.m_texture->m_format : VK_FORMAT_UNDEFINED;
		key.m_clear_color = clear_color && key.m_count;
		key.m_clear_depth = clear_depth && key.m_depth != VK_FORMAT_UNDEFINED;
		return key;
	}

	VkRenderPass ContextVK::render_pass(const RenderPassKeyVK& key)
	{
		auto it = m_render_passes.find(key);
		if (it != m_render_passes.end()) return it->second;
		Vector<VkAttachmentDescription> attachments;
		Vector<VkAttachmentReference> colors;
		for (uint32_t i = 0; i != key.m_count; ++i)
		{
			VkAttachmentDescription color{};
			color.format = key.m_colors[i];
			color.samples = VK_SAMPLE_COUNT_1_BIT;
			color.loadOp = key.m_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colors.push_back({ uint32_t(attachments.size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			attachments.push_back(color);
		}
		VkAttachmentReference depth_reference{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		if (key.m_depth != VK_FORMAT_UNDEFINED)
		{
			VkAttachmentDescription depth{};
			depth.format = key.m_depth;
			depth.samples = VK_SAMPLE_COUNT_1_BIT;
			depth.loadOp = key.m_clear_depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			const bool stencil = AuxVK::has_stencil(key.m_depth);
			depth.stencilLoadOp = stencil ? depth.loadOp : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth.stencilStoreOp = stencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_reference.attachment = uint32_t(attachments.size());
			attachments.push_back(depth);
		}
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = uint32_t(colors.size());
		subpass.pColorAttachments = colors.data();
		subpass.pDepthStencilAttachment = key.m_depth != VK_FORMAT_UNDEFINED ? &depth_reference : nullptr;
		VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		info.attachmentCount = uint32_t(attachments.size());
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		VkRenderPass pass = VK_NULL_HANDLE;
		if (vkCreateRenderPass(m_device, &info, AllocationVK::callbacks(), &pass) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make a render pass");
		}
		m_render_passes[key] = pass;
		return pass;
	}

	VkFramebuffer ContextVK::framebuffer(Target* target, VkRenderPass pass)
	{
		//(the swap chain: a framebuffer for each of its images, by the pass and the image)
		VkRenderPass key = pass;
		auto it = target->m_framebuffers.find(key);
		const bool default_target = target == &m_default_target;
		if (!default_target && it != target->m_framebuffers.end()) return it->second;
		Vector<VkImageView> views;
		auto attachment_view = [this, target](const Target::Attachment& attachment, VkImageAspectFlags aspect) -> VkImageView
		{
			Texture* texture = attachment.m_texture;
			const bool layered = attachment.m_layer < 0 && texture->m_layers > 1;
			if (layered)
			{
				return view_of(texture, VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, texture->m_layers, aspect, false);
			}
			return view_of(texture, VK_IMAGE_VIEW_TYPE_2D, uint32_t(std::max(attachment.m_layer, 0)), 1, aspect, false);
		};
		for (const Target::Attachment& color : target->m_colors)
		{
			views.push_back(attachment_view(color, VK_IMAGE_ASPECT_COLOR_BIT));
		}
		if (target->m_depth.m_texture)
		{
			views.push_back(attachment_view(target->m_depth, target->m_depth.m_texture->m_aspect));
		}
		//the default target: by its image too
		if (default_target)
		{
			key = VkRenderPass(uint64(pass) ^ (uint64(m_swapchain.m_image + 1) << 48));
			it = target->m_framebuffers.find(key);
			if (it != target->m_framebuffers.end()) return it->second;
		}
		VkFramebufferCreateInfo info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		info.renderPass = pass;
		info.attachmentCount = uint32_t(views.size());
		info.pAttachments = views.data();
		info.width = target->m_width;
		info.height = target->m_height;
		info.layers = target->m_layers;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		if (vkCreateFramebuffer(m_device, &info, AllocationVK::callbacks(), &framebuffer) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make a framebuffer");
		}
		target->m_framebuffers[key] = framebuffer;
		return framebuffer;
	}

	void ContextVK::destroy_framebuffers_later(Target* target)
	{
		Vector<VkFramebuffer> framebuffers;
		for (auto& framebuffer : target->m_framebuffers) framebuffers.push_back(framebuffer.second);
		target->m_framebuffers.clear();
		VkDevice device = m_device;
		destroy_later([device, framebuffers]()
		{
			for (VkFramebuffer framebuffer : framebuffers) vkDestroyFramebuffer(device, framebuffer, AllocationVK::callbacks());
		});
	}

	void ContextVK::prepare_target_textures(Target* target)
	{
		VkCommandBuffer buffer = commands();
		for (const Target::Attachment& color : target->m_colors)
		{
			transition(buffer, color.m_texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			color.m_texture->m_last_frame = m_frame_index + 1;
		}
		if (target->m_depth.m_texture)
		{
			transition(buffer, target->m_depth.m_texture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			target->m_depth.m_texture->m_last_frame = m_frame_index + 1;
		}
	}

	bool ContextVK::begin_render_pass()
	{
		Target* target = current_target();
		if (!target) return false;
		if (m_pass_open && m_pass_target == target) return true;
		end_render_pass();
		for (const Target::Attachment& color : target->m_colors)
		{
			if (!color.m_texture) return false;
		}
		prepare_target_textures(target);
		const RenderPassKeyVK key = render_pass_key(target, m_clear_color, m_clear_depth);
		const VkRenderPass pass = render_pass(key);
		const VkRenderPass compatible = render_pass(render_pass_key(target, false, false));
		const VkFramebuffer framebuffer = this->framebuffer(target, compatible);
		if (!pass || !framebuffer) return false;
		Vector<VkClearValue> clears(key.m_count + (key.m_depth != VK_FORMAT_UNDEFINED ? 1 : 0));
		const Vec4& color = m_state.m_clear_color.m_color;
		for (uint32_t i = 0; i != key.m_count; ++i)
		{
			clears[i].color = { { color.x, color.y, color.z, color.w } };
		}
		if (key.m_depth != VK_FORMAT_UNDEFINED)
		{
			clears[key.m_count].depthStencil = { 1.0f, 0 };
		}
		VkRenderPassBeginInfo begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		begin.renderPass = pass;
		begin.framebuffer = framebuffer;
		begin.renderArea.extent = { target->m_width, target->m_height };
		begin.clearValueCount = uint32_t(clears.size());
		begin.pClearValues = clears.data();
		vkCmdBeginRenderPass(commands(), &begin, VK_SUBPASS_CONTENTS_INLINE);
		m_pass_open = true;
		m_pass_target = target;
		m_pass = compatible;
		m_clear_color = false;
		m_clear_depth = false;
		m_bound_pipeline = VK_NULL_HANDLE;
		m_dynamic_dirty = true;
		return true;
	}

	void ContextVK::end_render_pass()
	{
		if (!m_pass_open) return;
		VkCommandBuffer buffer = frame().m_commands;
		vkCmdEndRenderPass(buffer);
		m_pass_open = false;
		//its textures ready to be read (most targets are read after: the G-Buffer, the shadows)
		if (m_pass_target != &m_default_target)
		{
			for (const Target::Attachment& color : m_pass_target->m_colors)
			{
				transition(buffer, color.m_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			if (m_pass_target->m_depth.m_texture)
			{
				transition(buffer, m_pass_target->m_depth.m_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		m_pass_target = nullptr;
	}

	Target* ContextVK::create_render_target(const Vector<TargetField>& textures)
	{
		auto* target = new Target();
		target->m_width = ~uint32_t(0);
		target->m_height = ~uint32_t(0);
		target->m_layers = ~uint32_t(0);
		for (const TargetField& field : textures)
		{
			Texture* texture = field.m_texture;
			if (!texture) continue;
			Target::Attachment attachment{ texture, field.m_type, field.m_layer };
			const bool depth = field.m_type == RT_DEPTH || field.m_type == RT_DEPTH_STENCIL || field.m_type == RT_STENCIL;
			if (depth)
			{
				if (!target->m_depth.m_texture) target->m_depth = attachment;
			}
			else if (target->m_colors.size() < VK_MAX_COLORS)
			{
				target->m_colors.push_back(attachment);
			}
			target->m_width = std::min(target->m_width, texture->m_width);
			target->m_height = std::min(target->m_height, texture->m_height);
			const uint32_t layers = field.m_layer < 0 ? texture->m_layers : 1;
			target->m_layers = std::min(target->m_layers, layers);
		}
		if (target->m_width == ~uint32_t(0))
		{
			target->m_width = target->m_height = target->m_layers = 1;
		}
#if defined(TEXTURE_INTROSPECTION)
		if (auto render_inspector = inspector())
		{
			render_inspector->on_create_target(target, textures);
		}
#endif
		return target;
	}

	void ContextVK::enable_render_target(Target* target)
	{
		if (target == m_target) return;
		//the clears asked of the target left: done (DirectX clears at once)
		if (m_clear_color || m_clear_depth)
		{
			begin_render_pass();
		}
		end_render_pass();
		m_clear_color = false;
		m_clear_depth = false;
		m_target = target;
	}

	void ContextVK::disable_render_target(Target* target)
	{
		if (target && target == m_target)
		{
			enable_render_target(nullptr);
		}
	}

	void ContextVK::delete_render_target(Target*& target)
	{
		if (!target) return;
		if (m_target == target)
		{
			enable_render_target(nullptr);
		}
		if (m_pass_target == target)
		{
			end_render_pass();
		}
#if defined(TEXTURE_INTROSPECTION)
		if (auto render_inspector = inspector())
		{
			render_inspector->on_delete_target(target);
		}
#endif
		destroy_framebuffers_later(target);
		delete target;
		target = nullptr;
	}

	void ContextVK::copy_target_to_target(const IVec4& from_area, Target* from, const IVec4& to_area, Target* to, TargetType mask)
	{
		//nullptr: the default target (the image of the swap chain, the depth of the window)
		auto resolve = [this](Target* target) -> Target* { return target ? target : (acquire_image() ? &m_default_target : nullptr); };
		Target* source = resolve(from);
		Target* destination = resolve(to);
		if (!source || !destination || source == destination) return;
		end_render_pass();
		VkCommandBuffer buffer = commands();
		const bool depth = mask == RT_DEPTH || mask == RT_STENCIL || mask == RT_DEPTH_STENCIL;
		if (depth)
		{
			//the depth: all of it, between depths of the same size and format (as DirectX)
			Texture* a = source->m_depth.m_texture;
			Texture* b = destination->m_depth.m_texture;
			if (!a || !b || a->m_format != b->m_format || a->m_width != b->m_width || a->m_height != b->m_height)
			{
				return;
			}
			transition(buffer, a, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			transition(buffer, b, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VkImageCopy region{};
			region.srcSubresource = { a->m_aspect, 0, 0, 1 };
			region.dstSubresource = { b->m_aspect, 0, 0, 1 };
			region.extent = { a->m_width, a->m_height, 1 };
			vkCmdCopyImage(buffer, a->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, b->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			return;
		}
		//the colors: the area (x0, y0, x1, y1) at the corner of the other one
		const size_t count = std::min(source->m_colors.size(), destination->m_colors.size());
		for (size_t i = 0; i != count; ++i)
		{
			Texture* a = source->m_colors[i].m_texture;
			Texture* b = destination->m_colors[i].m_texture;
			if (!a || !b) continue;
			const int32_t x = std::max(from_area.x, 0), y = std::max(from_area.y, 0);
			int32_t width = std::min<int32_t>(from_area.z, int32_t(a->m_width)) - x;
			int32_t height = std::min<int32_t>(from_area.w, int32_t(a->m_height)) - y;
			width = std::min<int32_t>(width, int32_t(b->m_width) - to_area.x);
			height = std::min<int32_t>(height, int32_t(b->m_height) - to_area.y);
			if (width <= 0 || height <= 0) continue;
			transition(buffer, a, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			transition(buffer, b, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			const uint32_t source_layer = uint32_t(std::max(source->m_colors[i].m_layer, 0));
			const uint32_t destination_layer = uint32_t(std::max(destination->m_colors[i].m_layer, 0));
			if (a->m_format == b->m_format)
			{
				VkImageCopy region{};
				region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, source_layer, 1 };
				region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, destination_layer, 1 };
				region.srcOffset = { x, y, 0 };
				region.dstOffset = { to_area.x, to_area.y, 0 };
				region.extent = { uint32_t(width), uint32_t(height), 1 };
				vkCmdCopyImage(buffer, a->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, b->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			}
			else
			{
				//other formats (a texture into the swap chain): converted by a blit, the same size
				VkImageBlit region{};
				region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, source_layer, 1 };
				region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, destination_layer, 1 };
				region.srcOffsets[0] = { x, y, 0 };
				region.srcOffsets[1] = { x + width, y + height, 1 };
				region.dstOffsets[0] = { to_area.x, to_area.y, 0 };
				region.dstOffsets[1] = { to_area.x + width, to_area.y + height, 1 };
				vkCmdBlitImage(buffer, a->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, b->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
			}
			a->m_last_frame = m_frame_index + 1;
			b->m_last_frame = m_frame_index + 1;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Draws
	VkPipeline ContextVK::pipeline(const PipelineKeyVK& key)
	{
		auto it = m_pipelines.find(key);
		if (it != m_pipelines.end()) return it->second;
		Shader* shader = key.m_shader;
		//its stages
		static const VkShaderStageFlagBits stages_of[ST_N_SHADER] =
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			VK_SHADER_STAGE_COMPUTE_BIT
		};
		Vector<VkPipelineShaderStageCreateInfo> stages;
		for (int type = 0; type != ST_COMPUTE_SHADER; ++type)
		{
			if (shader->m_modules[type])
			{
				VkPipelineShaderStageCreateInfo stage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
				stage.stage = stages_of[type];
				stage.module = shader->m_modules[type];
				stage.pName = shader->m_entries[type].c_str();
				stages.push_back(stage);
			}
		}
		//its vertices: the attributes of the layout at the location of their type (the inputs of
		//the shader without one: zeros, the binding 1)
		Vector<VkVertexInputBindingDescription> bindings;
		Vector<VkVertexInputAttributeDescription> attributes;
		bool zeros = false;
		for (const SpirvReflection::Input& input : shader->m_inputs)
		{
			const Attribute* found = nullptr;
			if (key.m_input_layout)
			{
				for (const Attribute& attribute : key.m_input_layout->m_list)
				{
					if (uint32_t(attribute.m_attribute) == input.m_location) found = &attribute;
				}
			}
			VkVertexInputAttributeDescription description{};
			description.location = input.m_location;
			if (found)
			{
				description.binding = 0;
				description.format = AuxVK::attribute_format(found->m_strip);
				description.offset = uint32_t(found->m_offset);
			}
			else
			{
				description.binding = 1;
				description.format = AuxVK::input_format(input);
				description.offset = 0;
				zeros = true;
				m_errors.push_back("Vulkan: the input at location " + to_string(input.m_location) + " of a shader is not in the input layout (zeros)");
			}
			attributes.push_back(description);
		}
		if (key.m_input_layout)
		{
			bindings.push_back({ 0, uint32_t(key.m_input_layout->m_list.size()), VK_VERTEX_INPUT_RATE_VERTEX });
		}
		if (zeros)
		{
			bindings.push_back({ 1, 0, VK_VERTEX_INPUT_RATE_VERTEX });
		}
		VkPipelineVertexInputStateCreateInfo vertex{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertex.vertexBindingDescriptionCount = uint32_t(bindings.size());
		vertex.pVertexBindingDescriptions = bindings.data();
		vertex.vertexAttributeDescriptionCount = uint32_t(attributes.size());
		vertex.pVertexAttributeDescriptions = attributes.data();
		VkPipelineInputAssemblyStateCreateInfo assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		assembly.topology = key.m_topology;
		VkPipelineViewportStateCreateInfo viewport{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewport.viewportCount = 1;
		viewport.scissorCount = 1;
		//the faces: clockwise the front (DirectX, the viewport flipped); both culled: the wires
		VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = VK_CULL_MODE_NONE;
		switch (key.m_cullface)
		{
		case CF_FRONT: raster.cullMode = VK_CULL_MODE_FRONT_BIT; break;
		case CF_BACK:  raster.cullMode = VK_CULL_MODE_BACK_BIT;  break;
		case CF_FRONT_AND_BACK:
			raster.polygonMode = m_features.fillModeNonSolid ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
			break;
		default: break;
		}
		raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster.lineWidth = 1.0f;
		VkPipelineMultisampleStateCreateInfo multisample{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkPipelineDepthStencilStateCreateInfo depth{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depth.depthTestEnable = key.m_depth.m_mode != DM_DISABLE;
		depth.depthWriteEnable = key.m_depth.m_mode == DM_ENABLE_AND_WRITE;
		depth.depthCompareOp = AuxVK::compare(key.m_depth.m_type);
		depth.minDepthBounds = 0.0f;
		depth.maxDepthBounds = 1.0f;
		//the same blend for every color (DirectX: IndependentBlendEnable false)
		Vector<VkPipelineColorBlendAttachmentState> blends(key.m_colors);
		for (VkPipelineColorBlendAttachmentState& blend : blends)
		{
			blend = VkPipelineColorBlendAttachmentState{};
			blend.blendEnable = key.m_blend.m_enable;
			blend.srcColorBlendFactor = AuxVK::blend(key.m_blend.m_src, false);
			blend.dstColorBlendFactor = AuxVK::blend(key.m_blend.m_dst, false);
			blend.colorBlendOp = VK_BLEND_OP_ADD;
			blend.srcAlphaBlendFactor = AuxVK::blend(key.m_blend.m_src, true);
			blend.dstAlphaBlendFactor = AuxVK::blend(key.m_blend.m_dst, true);
			blend.alphaBlendOp = VK_BLEND_OP_ADD;
			blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}
		VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.attachmentCount = uint32_t(blends.size());
		blend.pAttachments = blends.data();
		const VkDynamicState dynamics[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamics;
		VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		info.stageCount = uint32_t(stages.size());
		info.pStages = stages.data();
		info.pVertexInputState = &vertex;
		info.pInputAssemblyState = &assembly;
		info.pViewportState = &viewport;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisample;
		info.pDepthStencilState = &depth;
		info.pColorBlendState = &blend;
		info.pDynamicState = &dynamic;
		info.layout = shader->m_pipeline_layout;
		info.renderPass = key.m_render_pass;
		info.subpass = 0;
		VkPipeline pipeline = VK_NULL_HANDLE;
		if (vkCreateGraphicsPipelines(m_device, m_pipeline_cache, 1, &info, AllocationVK::callbacks(), &pipeline) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make a pipeline");
		}
		m_pipelines[key] = pipeline;
		return pipeline;
	}

	bool ContextVK::prepare_textures()
	{
		//each texture read by the shader in the layout to be read (out of the render pass), not a
		//texture the pass draws into (its zeros instead)
		VkCommandBuffer buffer = commands();
		for (const BindingVK& binding : m_shader->m_bindings)
		{
			if (binding.m_type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) continue;
			Texture* texture = m_shader->m_textures[binding.m_binding];
			if (!texture || texture->m_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) continue;
			bool attachment = false;
			Target* target = m_target ? m_target : nullptr;
			if (target)
			{
				for (const Target::Attachment& color : target->m_colors) attachment = attachment || color.m_texture == texture;
				attachment = attachment || target->m_depth.m_texture == texture;
			}
			if (attachment) continue;
			end_render_pass();
			transition(buffer, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		return true;
	}

	bool ContextVK::bind_resources(VkCommandBuffer buffer)
	{
		Shader* shader = m_shader;
		if (!shader->m_set_layout) return true;
		//what each binding reads: its slice of the ring (a buffer), its view and sampler (a texture)
		Vector<uint64>& key = m_bind_key;
		Vector<uint32_t>& offsets = m_bind_offsets;
		Vector<VkDescriptorBufferInfo>& buffers = m_bind_buffers;
		Vector<VkDescriptorImageInfo>& images = m_bind_images;
		key.clear();
		offsets.clear();
		buffers.clear();
		images.clear();
		buffers.reserve(shader->m_bindings.size());
		images.reserve(shader->m_bindings.size());
		offsets.reserve(shader->m_bindings.size());
		key.reserve(shader->m_bindings.size() * 2);
		for (const BindingVK& binding : shader->m_bindings)
		{
			const bool uniform_buffer = binding.m_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || binding.m_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			if (uniform_buffer)
			{
				ConstBuffer* source = nullptr;
				if (binding.m_global)
				{
					source = &shader->m_global;
				}
				else
				{
					source = const_cast<ConstBuffer*>(shader->m_buffers[binding.m_binding]);
				}
				VkDescriptorBufferInfo info{};
				info.range = std::max<VkDeviceSize>(binding.m_size, 16);
				VkDeviceSize offset = 0;
				if (source)
				{
					if (source->m_dirty || source->m_slice.m_frame != m_frame_index)
					{
						source->m_slice = ring_copy(source->m_bytes.data(), source->m_bytes.size(), size_t(info.range));
						source->m_dirty = false;
					}
					info.buffer = source->m_slice.m_buffer;
					offset = source->m_slice.m_offset;
				}
				else
				{
					info.buffer = m_zero_buffer.m_buffer;
					info.range = std::min<VkDeviceSize>(info.range, AuxVK::s_zero_bytes);
				}
				if (binding.m_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
				{
					offsets.push_back(uint32_t(offset));
				}
				else
				{
					info.offset = offset;
				}
				key.push_back(uint64(info.buffer));
				key.push_back(info.offset ^ (info.range << 40));
				buffers.push_back(info);
			}
			else if (binding.m_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				Texture* texture = shader->m_textures[binding.m_binding];
				const bool readable = texture && texture->m_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				Texture* read = readable ? texture : dummy_texture(binding.m_dim, binding.m_arrayed);
				VkDescriptorImageInfo info{};
				info.imageView = sampled_view(read, binding.m_dim, binding.m_arrayed);
				info.sampler = read->m_sampler;
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (!info.imageView)
				{
					read = dummy_texture(binding.m_dim, binding.m_arrayed);
					info.imageView = sampled_view(read, binding.m_dim, binding.m_arrayed);
					info.sampler = read->m_sampler;
				}
				read->m_last_frame = m_frame_index + 1;
				key.push_back(uint64(info.imageView));
				key.push_back(uint64(info.sampler));
				images.push_back(info);
			}
		}
		//the same resources of its last set (in this frame): the same set, other offsets
		if (shader->m_set_frame != m_frame_index || shader->m_set_key != key || !shader->m_set)
		{
			FrameVK& current = frame();
			VkDescriptorSet set = VK_NULL_HANDLE;
			while (set == VK_NULL_HANDLE)
			{
				if (current.m_descriptor_pool == current.m_descriptor_pools.size())
				{
					const uint32_t sets = AuxVK::s_pool_sets;
					const VkDescriptorPoolSize sizes[] =
					{
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, sets * 8 },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, sets * 4 },
						{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sets * 16 }
					};
					VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
					info.maxSets = sets;
					info.poolSizeCount = 3;
					info.pPoolSizes = sizes;
					VkDescriptorPool pool = VK_NULL_HANDLE;
					if (vkCreateDescriptorPool(m_device, &info, AllocationVK::callbacks(), &pool) != VK_SUCCESS)
					{
						m_errors.push_back("Vulkan: unable to make a descriptor pool");
						return false;
					}
					current.m_descriptor_pools.push_back(pool);
				}
				VkDescriptorSetAllocateInfo allocate{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
				allocate.descriptorPool = current.m_descriptor_pools[current.m_descriptor_pool];
				allocate.descriptorSetCount = 1;
				allocate.pSetLayouts = &shader->m_set_layout;
				if (vkAllocateDescriptorSets(m_device, &allocate, &set) != VK_SUCCESS)
				{
					set = VK_NULL_HANDLE;
					current.m_descriptor_pool += 1;
				}
			}
			Vector<VkWriteDescriptorSet>& writes = m_bind_writes;
			writes.clear();
			writes.reserve(shader->m_bindings.size());
			size_t buffer_index = 0, image_index = 0;
			for (const BindingVK& binding : shader->m_bindings)
			{
				VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				write.dstSet = set;
				write.dstBinding = binding.m_binding;
				write.descriptorCount = 1;
				write.descriptorType = binding.m_type;
				if (binding.m_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					write.pImageInfo = &images[image_index++];
				}
				else
				{
					write.pBufferInfo = &buffers[buffer_index++];
				}
				writes.push_back(write);
			}
			vkUpdateDescriptorSets(m_device, uint32_t(writes.size()), writes.data(), 0, nullptr);
			shader->m_set = set;
			//a copy (its memory of the last one kept): the list of the context kept for the next draw
			shader->m_set_key.assign(key.begin(), key.end());
			shader->m_set_frame = m_frame_index;
		}
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->m_pipeline_layout, 0, 1, &shader->m_set, uint32_t(offsets.size()), offsets.data());
		return true;
	}

	bool ContextVK::prepare_draw(DrawType type)
	{
		if (!m_shader) return false;
		begin_frame();
		prepare_textures();
		if (!begin_render_pass()) return false;
		Target* target = m_pass_target;
		VkCommandBuffer buffer = commands();
		//the pipeline of the states
		PipelineKeyVK key;
		key.m_shader = m_shader;
		key.m_input_layout = m_input_layout;
		key.m_render_pass = m_pass;
		key.m_colors = uint32_t(target->m_colors.size());
		key.m_blend = m_state.m_blend;
		key.m_depth = m_state.m_depth;
		key.m_cullface = m_state.m_cullface.m_cullface;
		key.m_topology = AuxVK::topology(type);
		const VkPipeline pipeline = this->pipeline(key);
		if (!pipeline) return false;
		if (pipeline != m_bound_pipeline)
		{
			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			m_bound_pipeline = pipeline;
		}
		//the viewport (flipped: the rows of DirectX) and the scissor (of the target)
		if (m_dynamic_dirty)
		{
			Vec4 area = m_state.m_viewport.m_viewport;
			if (area.z <= 0.0f || area.w <= 0.0f)
			{
				area = Vec4(0.0f, 0.0f, float(target->m_width), float(target->m_height));
			}
			VkViewport viewport{ area.x, area.y + area.w, area.z, -area.w, 0.0f, 1.0f };
			vkCmdSetViewport(buffer, 0, 1, &viewport);
			VkRect2D scissor{ { 0, 0 }, { target->m_width, target->m_height } };
			if (m_scissor.m_enable)
			{
				const int32_t x0 = std::clamp(m_scissor.m_rect.x, 0, int32_t(target->m_width));
				const int32_t y0 = std::clamp(m_scissor.m_rect.y, 0, int32_t(target->m_height));
				const int32_t x1 = std::clamp(m_scissor.m_rect.x + m_scissor.m_rect.z, x0, int32_t(target->m_width));
				const int32_t y1 = std::clamp(m_scissor.m_rect.y + m_scissor.m_rect.w, y0, int32_t(target->m_height));
				scissor = { { x0, y0 }, { uint32_t(x1 - x0), uint32_t(y1 - y0) } };
			}
			vkCmdSetScissor(buffer, 0, 1, &scissor);
			m_dynamic_dirty = false;
		}
		//the vertices (the zeros: the inputs without an attribute)
		const VkDeviceSize zero = 0;
		if (m_vertex_buffer && m_vertex_buffer->m_buffer.m_buffer)
		{
			vkCmdBindVertexBuffers(buffer, 0, 1, &m_vertex_buffer->m_buffer.m_buffer, &zero);
			m_vertex_buffer->m_last_frame = m_frame_index + 1;
		}
		vkCmdBindVertexBuffers(buffer, 1, 1, &m_zero_buffer.m_buffer, &zero);
		return bind_resources(buffer);
	}

	void ContextVK::draw_arrays(DrawType type, unsigned int n)
	{
		draw_arrays(type, 0, n);
	}

	void ContextVK::draw_arrays(DrawType type, unsigned int start, unsigned int size)
	{
		if (prepare_draw(type))
		{
			vkCmdDraw(commands(), size, 1, start, 0);
		}
	}

	void ContextVK::draw_elements(DrawType type, unsigned int n)
	{
		draw_elements(type, 0, n);
	}

	void ContextVK::draw_elements(DrawType type, unsigned int start, unsigned int n)
	{
		draw_elements_instanced(type, start, n, 1);
	}

	void ContextVK::draw_arrays_instanced(DrawType type, unsigned int start, unsigned int size, unsigned int instances)
	{
		if (prepare_draw(type))
		{
			vkCmdDraw(commands(), size, instances, start, 0);
		}
	}

	void ContextVK::draw_elements_instanced(DrawType type, unsigned int start, unsigned int n, unsigned int instances)
	{
		if (!m_index_buffer || !m_index_buffer->m_buffer.m_buffer) return;
		if (prepare_draw(type))
		{
			VkCommandBuffer buffer = commands();
			vkCmdBindIndexBuffer(buffer, m_index_buffer->m_buffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
			m_index_buffer->m_last_frame = m_frame_index + 1;
			vkCmdDrawIndexed(buffer, n, instances, start, 0, 0);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Errors
	bool ContextVK::print_errors() const
	{
		if (m_errors.empty()) return false;
		for (const String& error : m_errors)
		{
			logger()->warning(error);
		}
		m_errors.clear();
		return true;
	}

	bool ContextVK::print_errors(const char* source_file_name, int line) const
	{
		if (m_errors.empty()) return false;
		StringStream output;
		output << "At file: " << source_file_name << " (" << line << "):" << std::endl;
		for (const String& error : m_errors)
		{
			output << error << std::endl;
		}
		m_errors.clear();
		logger()->warning(output.str());
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// GPU timer: timestamps in the command buffer of the frame, read when its fence is done
#if defined(RENDER_PROFILER)
	bool ContextVK::gpu_timer_supported() const
	{
		return m_gpu_timer_supported;
	}

	void ContextVK::gpu_timer_begin_frame()
	{
		if (!m_gpu_timer_supported) return;
		begin_frame();
		end_render_pass();
		GpuTimerFrame& timer = m_gpu_timer_frames[m_frame_index % VK_FRAMES];
		timer.m_id = m_gpu_timer_frame_id++;
		timer.m_frame = m_frame_index;
		timer.m_count = 0;
		timer.m_pending = false;
		vkCmdResetQueryPool(commands(), frame().m_queries, 0, GPU_TIMER_MAX_TIMESTAMPS);
		m_gpu_timer_in_frame = true;
	}

	void ContextVK::gpu_timer_end_frame()
	{
		if (!m_gpu_timer_in_frame) return;
		GpuTimerFrame& timer = m_gpu_timer_frames[m_frame_index % VK_FRAMES];
		timer.m_pending = timer.m_count > 0;
		m_gpu_timer_in_frame = false;
	}

	int ContextVK::gpu_timer_timestamp()
	{
		if (!m_gpu_timer_in_frame) return -1;
		GpuTimerFrame& timer = m_gpu_timer_frames[m_frame_index % VK_FRAMES];
		if (timer.m_count >= GPU_TIMER_MAX_TIMESTAMPS) return -1;
		vkCmdWriteTimestamp(commands(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, frame().m_queries, timer.m_count);
		return int(timer.m_count++);
	}

	bool ContextVK::gpu_timer_read_frame(uint64& frame_id, Vector<double>& timestamps_ms)
	{
		//the oldest frame pending, done by the GPU (its fence: the frames completed)
		GpuTimerFrame* oldest = nullptr;
		for (GpuTimerFrame& timer : m_gpu_timer_frames)
		{
			if (timer.m_pending && (!oldest || timer.m_id < oldest->m_id)) oldest = &timer;
		}
		if (!oldest) return false;
		const FrameVK& slot = m_frames[oldest->m_frame % VK_FRAMES];
		Vector<uint64_t> ticks(oldest->m_count);
		const VkResult result = vkGetQueryPoolResults(m_device, slot.m_queries, 0, oldest->m_count, ticks.size() * sizeof(uint64_t), ticks.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
		if (result != VK_SUCCESS) return false;
		const double period = double(m_properties.limits.timestampPeriod) * 1.0e-6;
		timestamps_ms.resize(oldest->m_count);
		for (uint32_t i = 0; i != oldest->m_count; ++i)
		{
			timestamps_ms[i] = ticks[i] >= ticks[0] ? double(ticks[i] - ticks[0]) * period : 0.0;
		}
		frame_id = oldest->m_id;
		oldest->m_pending = false;
		return true;
	}
#endif
}
}
