//
//  ResourcesVK.cpp
//  Square, the driver of Vulkan: the buffers, the input layouts, the textures (their images,
//  views, samplers, layouts), what is read back from the GPU
//
#include "RenderVK.h"
#include <algorithm>
#include <cstring>

namespace Square
{
namespace Render
{
	namespace AuxResourcesVK
	{
		//the staging bytes of the uploads of a frame before they are done at once (a loading)
		static constexpr VkDeviceSize s_staging_flush = 128 * 1024 * 1024;

		//the format of a texture (as DirectX: RGBA 8 bit never sRGB, RGB 8 bit as RGBA)
		static VkFormat format(TextureFormat format, bool srgb)
		{
			switch (format)
			{
			case TF_RGBA8:    return VK_FORMAT_R8G8B8A8_UNORM;
			case TF_RGBA16UI: return VK_FORMAT_R16G16B16A16_UINT;
			case TF_RGBA32UI: return VK_FORMAT_R32G32B32A32_UINT;
			case TF_RGBA16I:  return VK_FORMAT_R16G16B16A16_SINT;
			case TF_RGBA32I:  return VK_FORMAT_R32G32B32A32_SINT;
			case TF_RGBA16F:  return VK_FORMAT_R16G16B16A16_SFLOAT;
			case TF_RGBA32F:  return VK_FORMAT_R32G32B32A32_SFLOAT;
			case TF_RGB8:     return srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
			case TF_RGB16UI:  return VK_FORMAT_R16G16B16A16_UINT;
			case TF_RGB32UI:  return VK_FORMAT_R32G32B32A32_UINT;
			case TF_RGB16I:   return VK_FORMAT_R16G16B16A16_SINT;
			case TF_RGB32I:   return VK_FORMAT_R32G32B32A32_SINT;
			case TF_RGB16F:   return VK_FORMAT_R16G16B16A16_SFLOAT;
			case TF_RGB32F:   return VK_FORMAT_R32G32B32A32_SFLOAT;
			case TF_RG8:      return VK_FORMAT_R8G8_UNORM;
			case TF_RG16UI:   return VK_FORMAT_R16G16_UINT;
			case TF_RG32UI:   return VK_FORMAT_R32G32_UINT;
			case TF_RG16I:    return VK_FORMAT_R16G16_SINT;
			case TF_RG32I:    return VK_FORMAT_R32G32_SINT;
			case TF_RG16F:    return VK_FORMAT_R16G16_SFLOAT;
			case TF_RG32F:    return VK_FORMAT_R32G32_SFLOAT;
			case TF_R8:       return VK_FORMAT_R8_UNORM;
			case TF_R16UI:    return VK_FORMAT_R16_UINT;
			case TF_R32UI:    return VK_FORMAT_R32_UINT;
			case TF_R16I:     return VK_FORMAT_R16_SINT;
			case TF_R32I:     return VK_FORMAT_R32_SINT;
			case TF_R16F:     return VK_FORMAT_R16_SFLOAT;
			case TF_R32F:     return VK_FORMAT_R32_SFLOAT;
			case TF_RGB565:   return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case TF_RGB5A1:   return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
			case TF_RGBA4:    return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
			case TF_BC1:      return srgb ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case TF_BC3:      return srgb ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
			case TF_BC4:      return VK_FORMAT_BC4_UNORM_BLOCK;
			case TF_BC5:      return VK_FORMAT_BC5_UNORM_BLOCK;
			case TF_ASTC_4x4: return srgb ? VK_FORMAT_ASTC_4x4_SRGB_BLOCK : VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
			default:          return VK_FORMAT_UNDEFINED;
			}
		}

		static bool is_depth(TextureFormat format)
		{
			switch (format)
			{
			case TF_DEPTH16_STENCIL8:
			case TF_DEPTH24_STENCIL8:
			case TF_DEPTH32_STENCIL8:
			case TF_DEPTH_COMPONENT16:
			case TF_DEPTH_COMPONENT24:
			case TF_DEPTH_COMPONENT32: return true;
			default:                   return false;
			}
		}

		//the bytes of a pixel of the data given (an RGB one: its 3 channels)
		static uint32_t pixel_bytes(TextureFormat format)
		{
			switch (format)
			{
			case TF_RGBA8: return 4;
			case TF_RGBA16UI: case TF_RGBA16I: case TF_RGBA16F: return 8;
			case TF_RGBA32UI: case TF_RGBA32I: case TF_RGBA32F: return 16;
			case TF_RGB8: return 3;
			case TF_RGB16UI: case TF_RGB16I: case TF_RGB16F: return 6;
			case TF_RGB32UI: case TF_RGB32I: case TF_RGB32F: return 12;
			case TF_RG8: return 2;
			case TF_RG16UI: case TF_RG16I: case TF_RG16F: return 4;
			case TF_RG32UI: case TF_RG32I: case TF_RG32F: return 8;
			case TF_R8: return 1;
			case TF_R16UI: case TF_R16I: case TF_R16F: return 2;
			case TF_R32UI: case TF_R32I: case TF_R32F: return 4;
			case TF_RGB565: case TF_RGB5A1: case TF_RGBA4: return 2;
			default: return 4;
			}
		}

		//an RGB format: its pixels with their alpha (the image has 4 channels)
		static bool is_rgb(TextureFormat format)
		{
			switch (format)
			{
			case TF_RGB8: case TF_RGB16UI: case TF_RGB16I: case TF_RGB16F:
			case TF_RGB32UI: case TF_RGB32I: case TF_RGB32F: return true;
			default: return false;
			}
		}

		//the pixels of an RGB image with their alpha (the alpha of DirectX: the most of its type)
		static std::vector<unsigned char> with_alpha(const unsigned char* rgb, size_t pixels, TextureFormat format)
		{
			const size_t channel = pixel_bytes(format) / 3;
			std::vector<unsigned char> alpha(channel, 0xFF);
			switch (format)
			{
			case TF_RGB16I: alpha = { 0xFF, 0x7F }; break;
			case TF_RGB32I: alpha = { 0xFF, 0xFF, 0xFF, 0x7F }; break;
			case TF_RGB16F: alpha = { 0x00, 0x3C }; break;             //1.0 (half)
			case TF_RGB32F: alpha = { 0x00, 0x00, 0x80, 0x3F }; break; //1.0
			default: break;
			}
			std::vector<unsigned char> out(pixels * channel * 4);
			for (size_t i = 0; i != pixels; ++i)
			{
				std::memcpy(&out[i * channel * 4], rgb + i * channel * 3, channel * 3);
				std::memcpy(&out[i * channel * 4 + channel * 3], alpha.data(), channel);
			}
			return out;
		}

		//the bytes of a pixel of a format of an image (read back)
		static uint32_t texel_bytes(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R8_UNORM: return 1;
			case VK_FORMAT_R8G8_UNORM: case VK_FORMAT_R16_SFLOAT: case VK_FORMAT_R16_UINT: case VK_FORMAT_R16_SINT:
			case VK_FORMAT_D16_UNORM: case VK_FORMAT_R5G6B5_UNORM_PACK16: case VK_FORMAT_R5G5B5A1_UNORM_PACK16: case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return 2;
			case VK_FORMAT_R16G16B16A16_SFLOAT: case VK_FORMAT_R16G16B16A16_UINT: case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R32G32_SFLOAT: case VK_FORMAT_R32G32_UINT: case VK_FORMAT_R32G32_SINT: return 8;
			case VK_FORMAT_R32G32B32A32_SFLOAT: case VK_FORMAT_R32G32B32A32_UINT: case VK_FORMAT_R32G32B32A32_SINT: return 16;
			default: return 4;
			}
		}

		static VkFilter filter(bool nearest)
		{
			return nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		}

		static VkSamplerAddressMode address(TextureEdgeType edge)
		{
			return edge == TEDGE_REPEAT ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers
	BufferVK ContextVK::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool host, bool readback)
	{
		BufferVK out;
		VkBufferCreateInfo info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = std::max<VkDeviceSize>(size, 16);
		info.usage = usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VmaAllocationCreateInfo allocation{};
		allocation.usage = VMA_MEMORY_USAGE_AUTO;
		if (host)
		{
			allocation.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT
			                 | (readback ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		}
		VmaAllocationInfo allocated{};
		if (vmaCreateBuffer(m_allocator, &info, &allocation, &out.m_buffer, &out.m_allocation, &allocated) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make a buffer of " + std::to_string(size) + " bytes");
			return BufferVK();
		}
		out.m_mapped = static_cast<unsigned char*>(allocated.pMappedData);
		out.m_size = info.size;
		return out;
	}

	void ContextVK::destroy_buffer_later(BufferVK& buffer)
	{
		if (buffer.m_buffer)
		{
			VmaAllocator allocator = m_allocator;
			VkBuffer handle = buffer.m_buffer;
			VmaAllocation allocation = buffer.m_allocation;
			destroy_later([allocator, handle, allocation]() { vmaDestroyBuffer(allocator, handle, allocation); });
		}
		buffer = BufferVK();
	}

	bool ContextVK::upload_buffer(BufferVK& buffer, const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		if (!data || !size || !buffer.m_buffer) return false;
		BufferVK staging = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
		if (!staging.m_mapped) return false;
		std::memcpy(staging.m_mapped, data, size_t(size));
		VkBufferCopy region{ 0, offset, size };
		vkCmdCopyBuffer(uploads(), staging.m_buffer, buffer.m_buffer, 1, &region);
		frame().m_staging_bytes += size;
		destroy_buffer_later(staging);
		//a loading: its uploads done before they keep too much memory
		if (frame().m_staging_bytes > AuxResourcesVK::s_staging_flush)
		{
			flush_and_wait();
		}
		return true;
	}

	void ContextVK::stream_write(GeometryBufferVK& buffer, VkBufferUsageFlags usage, const void* data, size_t size)
	{
		//a buffer the GPU may still read (or static, or small): a new one (as DirectX discards)
		const bool in_use = buffer.m_last_frame > m_completed_frame;
		if (!buffer.m_buffer.m_buffer || !buffer.m_stream || in_use || size > buffer.m_buffer.m_size)
		{
			destroy_buffer_later(buffer.m_buffer);
			buffer.m_buffer = create_buffer(std::max(size, buffer.m_size), usage, true);
			buffer.m_size = std::max(size, buffer.m_size);
			buffer.m_stream = true;
			buffer.m_last_frame = 0;
		}
		if (data && size && buffer.m_buffer.m_mapped)
		{
			std::memcpy(buffer.m_buffer.m_mapped, data, size);
		}
	}

	std::vector<unsigned char> ContextVK::read_buffer(const BufferVK& buffer, size_t size)
	{
		if (!buffer.m_buffer || !size) return {};
		if (buffer.m_mapped)
		{
			return std::vector<unsigned char>(buffer.m_mapped, buffer.m_mapped + size);
		}
		//a static one: copied to the host after what is recorded (done at once)
		end_render_pass();
		BufferVK staging = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, true);
		if (!staging.m_mapped) return {};
		VkBufferCopy region{ 0, 0, VkDeviceSize(size) };
		vkCmdCopyBuffer(commands(), buffer.m_buffer, staging.m_buffer, 1, &region);
		flush_and_wait();
		vmaInvalidateAllocation(m_allocator, staging.m_allocation, 0, VK_WHOLE_SIZE);
		std::vector<unsigned char> out(staging.m_mapped, staging.m_mapped + size);
		vmaDestroyBuffer(m_allocator, staging.m_buffer, staging.m_allocation);
		return out;
	}

	ConstBuffer* ContextVK::create_stream_CB(const unsigned char* data, size_t size)
	{
		auto* buffer = new ConstBuffer();
		buffer->m_bytes.assign(size, 0);
		if (data) std::memcpy(buffer->m_bytes.data(), data, size);
		return buffer;
	}

	ConstBuffer* ContextVK::create_CB(const unsigned char* data, size_t size)
	{
		return create_stream_CB(data, size);
	}

	VertexBuffer* ContextVK::create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto* buffer = new VertexBuffer();
		buffer->m_stride = stride;
		buffer->m_count = n;
		buffer->m_size = stride * n;
		stream_write(*buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vbo, buffer->m_size);
		return buffer;
	}

	IndexBuffer* ContextVK::create_stream_IBO(const unsigned int* ibo, size_t size)
	{
		auto* buffer = new IndexBuffer();
		buffer->m_size = sizeof(unsigned int) * size;
		stream_write(*buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ibo, buffer->m_size);
		return buffer;
	}

	VertexBuffer* ContextVK::create_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto* buffer = new VertexBuffer();
		buffer->m_stride = stride;
		buffer->m_count = n;
		buffer->m_size = stride * n;
		buffer->m_buffer = create_buffer(buffer->m_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
		upload_buffer(buffer->m_buffer, vbo, buffer->m_size);
		return buffer;
	}

	IndexBuffer* ContextVK::create_IBO(const unsigned int* ibo, size_t size)
	{
		auto* buffer = new IndexBuffer();
		buffer->m_size = sizeof(unsigned int) * size;
		buffer->m_buffer = create_buffer(buffer->m_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
		upload_buffer(buffer->m_buffer, ibo, buffer->m_size);
		return buffer;
	}

	uint64 ContextVK::get_native_CB(const ConstBuffer* cb) const { return uint64(reinterpret_cast<uintptr_t>(cb)); }
	uint64 ContextVK::get_native_VBO(const VertexBuffer* vb) const { return vb ? uint64(vb->m_buffer.m_buffer) : 0; }
	uint64 ContextVK::get_native_IBO(const IndexBuffer* ib) const { return ib ? uint64(ib->m_buffer.m_buffer) : 0; }

	size_t ContextVK::get_size_CB(const ConstBuffer* cb) const { return cb ? cb->m_bytes.size() : 0; }
	size_t ContextVK::get_size_VBO(const VertexBuffer* vb) const { return vb ? vb->m_size : 0; }
	size_t ContextVK::get_size_IBO(const IndexBuffer* ib) const { return ib ? ib->m_size : 0; }

	void ContextVK::update_steam_CB(ConstBuffer* cb, const unsigned char* data, size_t n)
	{
		if (!cb || !data) return;
		if (n > cb->m_bytes.size()) cb->m_bytes.resize(n, 0);
		std::memcpy(cb->m_bytes.data(), data, n);
		cb->m_dirty = true;
	}

	void ContextVK::update_steam_VBO(VertexBuffer* vbo, const unsigned char* data, size_t n)
	{
		if (!vbo) return;
		stream_write(*vbo, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, n);
	}

	void ContextVK::update_steam_IBO(IndexBuffer* ibo, const unsigned int* data, size_t n)
	{
		if (!ibo) return;
		stream_write(*ibo, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data, n * sizeof(unsigned int));
	}

	void ContextVK::bind_CB(ConstBuffer*) {}
	void ContextVK::unbind_CB(ConstBuffer*) {}
	void ContextVK::bind_VBO(VertexBuffer* vbo) { m_vertex_buffer = vbo; }
	void ContextVK::unbind_VBO(VertexBuffer* vbo) { if (m_vertex_buffer == vbo) m_vertex_buffer = nullptr; }
	void ContextVK::bind_IBO(IndexBuffer* ibo) { m_index_buffer = ibo; }
	void ContextVK::unbind_IBO(IndexBuffer* ibo) { if (m_index_buffer == ibo) m_index_buffer = nullptr; }

	std::vector<unsigned char> ContextVK::copy_buffer_CB(const ConstBuffer* cb)
	{
		return cb ? cb->m_bytes : std::vector<unsigned char>();
	}

	std::vector<unsigned char> ContextVK::copy_buffer_VBO(const VertexBuffer* vbo)
	{
		return vbo ? read_buffer(vbo->m_buffer, vbo->m_size) : std::vector<unsigned char>();
	}

	std::vector<unsigned char> ContextVK::copy_buffer_IBO(const IndexBuffer* ibo)
	{
		return ibo ? read_buffer(ibo->m_buffer, ibo->m_size) : std::vector<unsigned char>();
	}

	unsigned char* ContextVK::map_CB(ConstBuffer* cb, size_t start, size_t n, MappingType type)
	{
		if (!cb) return nullptr;
		if (start + n > cb->m_bytes.size()) cb->m_bytes.resize(start + n, 0);
		return cb->m_bytes.data() + start;
	}

	unsigned char* ContextVK::map_CB(ConstBuffer* cb, MappingType type)
	{
		return map_CB(cb, 0, cb ? cb->m_bytes.size() : 0, type);
	}

	void ContextVK::unmap_CB(ConstBuffer* cb)
	{
		if (cb) cb->m_dirty = true;
	}

	unsigned char* ContextVK::map_VBO(VertexBuffer* vbo, size_t start, size_t n, MappingType type)
	{
		if (!vbo) return nullptr;
		if (type == MAP_READ)
		{
			vbo->m_read = read_buffer(vbo->m_buffer, vbo->m_size);
			return vbo->m_read.size() > start ? vbo->m_read.data() + start : nullptr;
		}
		//written: a buffer of the host the GPU does not read now (its old bytes discarded)
		stream_write(*vbo, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, nullptr, vbo->m_size);
		return vbo->m_buffer.m_mapped ? vbo->m_buffer.m_mapped + start : nullptr;
	}

	unsigned char* ContextVK::map_VBO(VertexBuffer* vbo, MappingType type)
	{
		return map_VBO(vbo, 0, vbo ? vbo->m_size : 0, type);
	}

	void ContextVK::unmap_VBO(VertexBuffer* vbo)
	{
		if (vbo) vbo->m_read.clear();
	}

	unsigned int* ContextVK::map_IBO(IndexBuffer* ibo, size_t start, size_t n, MappingType type)
	{
		if (!ibo) return nullptr;
		const size_t offset = start * sizeof(unsigned int);
		if (type == MAP_READ)
		{
			ibo->m_read = read_buffer(ibo->m_buffer, ibo->m_size);
			return ibo->m_read.size() > offset ? reinterpret_cast<unsigned int*>(ibo->m_read.data() + offset) : nullptr;
		}
		stream_write(*ibo, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, nullptr, ibo->m_size);
		return ibo->m_buffer.m_mapped ? reinterpret_cast<unsigned int*>(ibo->m_buffer.m_mapped + offset) : nullptr;
	}

	unsigned int* ContextVK::map_IBO(IndexBuffer* ibo, MappingType type)
	{
		return map_IBO(ibo, 0, ibo ? ibo->m_size / sizeof(unsigned int) : 0, type);
	}

	void ContextVK::unmap_IBO(IndexBuffer* ibo)
	{
		if (ibo) ibo->m_read.clear();
	}

	unsigned char* ContextVK::map_TBO(Texture*, MappingType)
	{
		return nullptr;
	}

	void ContextVK::unmap_TBO(Texture*)
	{
	}

	void ContextVK::delete_CB(ConstBuffer*& cb)
	{
		forget(cb);
		delete cb;
		cb = nullptr;
	}

	void ContextVK::delete_VBO(VertexBuffer*& vbo)
	{
		if (!vbo) return;
		if (m_vertex_buffer == vbo) m_vertex_buffer = nullptr;
		destroy_buffer_later(vbo->m_buffer);
		delete vbo;
		vbo = nullptr;
	}

	void ContextVK::delete_IBO(IndexBuffer*& ibo)
	{
		if (!ibo) return;
		if (m_index_buffer == ibo) m_index_buffer = nullptr;
		destroy_buffer_later(ibo->m_buffer);
		delete ibo;
		ibo = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Input layouts
	InputLayout* ContextVK::create_IL(const AttributeList& atl)
	{
		auto* layout = new InputLayout();
		layout->m_list = atl;
		return layout;
	}

	void ContextVK::delete_IL(InputLayout*& il)
	{
		if (!il) return;
		if (m_input_layout == il) m_input_layout = nullptr;
		forget_pipelines(nullptr, il);
		delete il;
		il = nullptr;
	}

	void ContextVK::bind_IL(InputLayout* il) { m_input_layout = il; }
	void ContextVK::unbind_IL(InputLayout* il) { if (m_input_layout == il) m_input_layout = nullptr; }
	size_t ContextVK::size_IL(const InputLayout* layout) { return layout ? layout->m_list.size() : 0; }

	bool ContextVK::has_a_position_IL(const InputLayout* layout)
	{
		if (!layout) return false;
		for (const Attribute& attribute : layout->m_list)
		{
			if (attribute.is_position_transform()) return true;
		}
		return false;
	}

	size_t ContextVK::position_offset_IL(const InputLayout* layout)
	{
		if (!layout) return ~size_t(0);
		for (const Attribute& attribute : layout->m_list)
		{
			if (attribute.is_position_transform()) return attribute.m_offset;
		}
		return ~size_t(0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Textures
	VkFormat ContextVK::depth_format(TextureFormat format) const
	{
		//the depth asked, else the nearest one the device draws into (AMD: no 24 bit + stencil)
		auto supported = [this](VkFormat candidate)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(m_physical, candidate, &properties);
			return (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
		};
		std::vector<VkFormat> candidates;
		switch (format)
		{
		case TF_DEPTH16_STENCIL8:   candidates = { VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT }; break;
		case TF_DEPTH24_STENCIL8:   candidates = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT }; break;
		case TF_DEPTH32_STENCIL8:   candidates = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }; break;
		case TF_DEPTH_COMPONENT16:  candidates = { VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT }; break;
		case TF_DEPTH_COMPONENT24:  candidates = { VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT }; break;
		case TF_DEPTH_COMPONENT32:
		default:                    candidates = { VK_FORMAT_D32_SFLOAT }; break;
		}
		for (VkFormat candidate : candidates)
		{
			if (supported(candidate)) return candidate;
		}
		return VK_FORMAT_D32_SFLOAT;
	}

	VkSampler ContextVK::sampler_of(const TextureGpuDataInformation& info, uint32_t levels)
	{
		//(the samplers of the textures: few different ones, made once)
		const int anisotropy = std::max(1, info.m_anisotropy);
		uint64 key = uint64(info.m_min_type) | uint64(info.m_mag_type) << 8 | uint64(info.m_edge_s) << 16
		           | uint64(info.m_edge_t) << 20 | uint64(info.m_edge_r) << 24 | uint64(std::min(anisotropy, 255)) << 28
		           | uint64(std::max(0, std::min(info.m_mipmap_min, 255))) << 36 | uint64(std::max(0, std::min(info.m_mipmap_max, 255))) << 44;
		auto it = m_samplers.find(key);
		if (it != m_samplers.end()) return it->second;
		//the filters of DirectX: the mipmaps by the minification (linear: among them too)
		const TextureMinFilterType min = info.m_min_type;
		const bool min_nearest = min == TMIN_NEAREST || min == TMIN_NEAREST_MIPMAP_NEAREST || min == TMIN_NEAREST_MIPMAP_LINEAR;
		const bool mip_nearest = min == TMIN_NEAREST || min == TMIN_NEAREST_MIPMAP_NEAREST || min == TMIN_LINEAR_MIPMAP_NEAREST;
		const TextureMagFilterType mag = info.m_mag_type;
		const bool mag_nearest = mag == TMAG_NEAREST || mag == TMAG_NEAREST_MIPMAP_NEAREST || mag == TMAG_NEAREST_MIPMAP_LINEAR;
		VkSamplerCreateInfo sampler{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		sampler.magFilter = AuxResourcesVK::filter(mag_nearest);
		sampler.minFilter = AuxResourcesVK::filter(min_nearest);
		sampler.mipmapMode = mip_nearest ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = AuxResourcesVK::address(info.m_edge_s);
		sampler.addressModeV = AuxResourcesVK::address(info.m_edge_t);
		sampler.addressModeW = AuxResourcesVK::address(info.m_edge_r);
		sampler.anisotropyEnable = anisotropy > 1 && m_features.samplerAnisotropy;
		sampler.maxAnisotropy = std::min(float(anisotropy), m_properties.limits.maxSamplerAnisotropy);
		sampler.minLod = float(std::max(0, info.m_mipmap_min));
		sampler.maxLod = float(std::max(info.m_mipmap_min, info.m_mipmap_max));
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		VkSampler out = VK_NULL_HANDLE;
		if (vkCreateSampler(m_device, &sampler, nullptr, &out) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make a sampler");
		}
		m_samplers[key] = out;
		return out;
	}

	VkImageView ContextVK::view_of(Texture* texture, VkImageViewType type, uint32_t layer, uint32_t layers, VkImageAspectFlags aspect, bool all_levels)
	{
		const uint64 key = uint64(type) | uint64(layer & 0xFFFF) << 8 | uint64(layers & 0xFFFF) << 24 | uint64(aspect & 0xFF) << 40 | uint64(all_levels) << 48;
		auto it = texture->m_views.find(key);
		if (it != texture->m_views.end()) return it->second;
		VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		info.image = texture->m_image;
		info.viewType = type;
		info.format = texture->m_format;
		info.subresourceRange.aspectMask = aspect;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = all_levels ? texture->m_levels : 1;
		info.subresourceRange.baseArrayLayer = layer;
		info.subresourceRange.layerCount = layers;
		VkImageView view = VK_NULL_HANDLE;
		if (vkCreateImageView(m_device, &info, nullptr, &view) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make an image view");
		}
		texture->m_views[key] = view;
		return view;
	}

	VkImageView ContextVK::sampled_view(Texture* texture, SpirvReflection::ImageDim dim, bool arrayed)
	{
		//the shape the shader reads (not its shape: none, its zeros instead)
		switch (dim)
		{
		case SpirvReflection::ImageDim::DIM_CUBE:
			if (!texture->m_cube) return VK_NULL_HANDLE;
			if (arrayed)
			{
				return view_of(texture, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, 0, texture->m_layers - texture->m_layers % 6, texture->m_view_aspect, true);
			}
			return view_of(texture, VK_IMAGE_VIEW_TYPE_CUBE, 0, 6, texture->m_view_aspect, true);
		case SpirvReflection::ImageDim::DIM_2D:
			if (arrayed)
			{
				return view_of(texture, VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, texture->m_layers, texture->m_view_aspect, true);
			}
			return view_of(texture, VK_IMAGE_VIEW_TYPE_2D, 0, 1, texture->m_view_aspect, true);
		default:
			return VK_NULL_HANDLE;
		}
	}

	void ContextVK::transition(VkCommandBuffer commands, Texture* texture, VkImageLayout layout)
	{
		if (!texture || texture->m_layout == layout) return;
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.oldLayout = texture->m_layout;
		barrier.newLayout = layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = texture->m_image;
		barrier.subresourceRange = { texture->m_aspect, 0, texture->m_levels, 0, texture->m_layers };
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		texture->m_layout = layout;
	}

	void ContextVK::generate_mipmaps(VkCommandBuffer commands, Texture* texture)
	{
		//each level from the one before (a blit), all the layers; then all of them to be read
		auto barrier = [&](uint32_t level, VkImageLayout from, VkImageLayout to)
		{
			VkImageMemoryBarrier image{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			image.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
			image.oldLayout = from;
			image.newLayout = to;
			image.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image.image = texture->m_image;
			image.subresourceRange = { texture->m_aspect, level, 1, 0, texture->m_layers };
			vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &image);
		};
		int32_t width = int32_t(texture->m_width), height = int32_t(texture->m_height);
		for (uint32_t level = 1; level < texture->m_levels; ++level)
		{
			barrier(level - 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			const int32_t next_width = std::max(width / 2, 1), next_height = std::max(height / 2, 1);
			VkImageBlit blit{};
			blit.srcSubresource = { texture->m_aspect, level - 1, 0, texture->m_layers };
			blit.srcOffsets[1] = { width, height, 1 };
			blit.dstSubresource = { texture->m_aspect, level, 0, texture->m_layers };
			blit.dstOffsets[1] = { next_width, next_height, 1 };
			vkCmdBlitImage(commands, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
			width = next_width;
			height = next_height;
		}
		for (uint32_t level = 0; level + 1 < texture->m_levels; ++level)
		{
			barrier(level, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		barrier(texture->m_levels - 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		texture->m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	Texture* ContextVK::make_texture(const TextureRawDataInformation* data, int layers, bool cube, const TextureGpuDataInformation& info)
	{
		const TextureRawDataInformation& first = data[0];
		const bool depth = AuxResourcesVK::is_depth(first.m_format);
		const bool compressed = is_compressed_format(first.m_format);
		const VkFormat format = depth ? depth_format(first.m_format) : AuxResourcesVK::format(first.m_format, first.m_is_srgb);
		if (format == VK_FORMAT_UNDEFINED)
		{
			m_errors.push_back("Vulkan: create_texture, format not supported");
			return nullptr;
		}
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(m_physical, format, &properties);
		const VkFormatFeatureFlags features = properties.optimalTilingFeatures;
		auto texture = std::make_unique<Texture>();
		texture->m_format = format;
		texture->m_texture_format = first.m_format;
		texture->m_width = std::max(first.m_width, 1u);
		texture->m_height = std::max(first.m_height, 1u);
		texture->m_layers = cube ? 6 : uint32_t(std::max(layers, 1));
		texture->m_cube = cube;
		texture->m_array = !cube && layers > 1;
		texture->m_depth = depth;
		texture->m_srgb = first.m_is_srgb;
		texture->m_aspect = depth ? VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT) : VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT);
		if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			texture->m_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		texture->m_view_aspect = depth ? VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT) : VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT);
		//its levels: the ones given (compressed), the whole chain (built: when the device blits it)
		const VkFormatFeatureFlags blit = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
		const bool build_mipmap = info.m_build_mipmap && !compressed && !depth && (features & blit) == blit;
		texture->m_levels = 1;
		if (compressed)
		{
			texture->m_levels = std::max(first.m_levels, 1u);
		}
		else if (build_mipmap)
		{
			for (uint32_t size = std::max(texture->m_width, texture->m_height); size > 1; size >>= 1) texture->m_levels += 1;
		}
		//its image: read, copied, drawn into (not a compressed one)
		VkImageCreateInfo image{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		image.flags = cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = format;
		image.extent = { texture->m_width, texture->m_height, 1 };
		image.mipLevels = texture->m_levels;
		image.arrayLayers = texture->m_layers;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (depth)
		{
			image.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else if (!compressed && (features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
		{
			image.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaAllocationCreateInfo allocation{};
		allocation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		if (vmaCreateImage(m_allocator, &image, &allocation, &texture->m_image, &texture->m_allocation, nullptr) != VK_SUCCESS)
		{
			m_errors.push_back("Vulkan: unable to make an image of " + std::to_string(texture->m_width) + "x" + std::to_string(texture->m_height));
			return nullptr;
		}
		//its pixels: level 0 of each layer (an array: its first one, as DirectX), all the levels of
		//a compressed one; by a staging buffer, in the uploads of the frame
		VkCommandBuffer buffer = uploads();
		std::vector<unsigned char> bytes;
		std::vector<VkBufferImageCopy> regions;
		const uint32_t faces = cube ? 6 : 1;
		for (uint32_t face = 0; face != faces; ++face)
		{
			const TextureRawDataInformation& layer = data[face];
			if (!layer.m_bytes) continue;
			if (compressed)
			{
				const unsigned char* level_bytes = layer.m_bytes;
				for (uint32_t level = 0; level != texture->m_levels; ++level)
				{
					const uint32_t width = texture_level_size(layer.m_width, level);
					const uint32_t height = texture_level_size(layer.m_height, level);
					const size_t size = texture_level_bytes(layer.m_format, width, height);
					VkBufferImageCopy region{};
					region.bufferOffset = bytes.size();
					region.imageSubresource = { texture->m_aspect, level, face, 1 };
					region.imageExtent = { width, height, 1 };
					regions.push_back(region);
					bytes.insert(bytes.end(), level_bytes, level_bytes + size);
					level_bytes += size;
				}
			}
			else
			{
				const size_t pixels = size_t(layer.m_width) * size_t(layer.m_height);
				VkBufferImageCopy region{};
				region.bufferOffset = bytes.size();
				region.imageSubresource = { texture->m_aspect & ~VkImageAspectFlags(VK_IMAGE_ASPECT_STENCIL_BIT), 0, face, 1 };
				region.imageExtent = { texture->m_width, texture->m_height, 1 };
				regions.push_back(region);
				if (AuxResourcesVK::is_rgb(layer.m_format))
				{
					const std::vector<unsigned char> rgba = AuxResourcesVK::with_alpha(layer.m_bytes, pixels, layer.m_format);
					bytes.insert(bytes.end(), rgba.begin(), rgba.end());
				}
				else
				{
					bytes.insert(bytes.end(), layer.m_bytes, layer.m_bytes + pixels * AuxResourcesVK::pixel_bytes(layer.m_format));
				}
			}
			//(4 bytes aligned offsets: the copies of the next regions)
			while (bytes.size() % 16) bytes.push_back(0);
		}
		if (regions.size())
		{
			BufferVK staging = create_buffer(bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
			if (staging.m_mapped)
			{
				std::memcpy(staging.m_mapped, bytes.data(), bytes.size());
				transition(buffer, texture.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				vkCmdCopyBufferToImage(buffer, staging.m_buffer, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, uint32_t(regions.size()), regions.data());
				frame().m_staging_bytes += bytes.size();
				if (build_mipmap && texture->m_levels > 1)
				{
					generate_mipmaps(buffer, texture.get());
				}
			}
			destroy_buffer_later(staging);
		}
		transition(buffer, texture.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		texture->m_sampler = sampler_of(info, texture->m_levels);
		Texture* out = texture.release();
		if (frame().m_staging_bytes > AuxResourcesVK::s_staging_flush)
		{
			flush_and_wait();
		}
		return out;
	}

	Texture* ContextVK::create_texture(const TextureRawDataInformation& data, const TextureGpuDataInformation& info)
	{
		Texture* texture = make_texture(&data, 1, false, info);
#if defined(TEXTURE_INTROSPECTION)
		if (texture)
		{
			if (auto render_inspector = inspector())
			{
				render_inspector->on_create_texture(texture, { TS_TEXTURE_2D, data.m_format, data.m_width, data.m_height, 1 });
			}
		}
#endif
		return texture;
	}

	Texture* ContextVK::create_texture_array(const TextureRawDataInformation& data, const TextureGpuDataInformation& info, int size)
	{
		Texture* texture = make_texture(&data, std::max(size, 1), false, info);
		if (texture) texture->m_array = true;
#if defined(TEXTURE_INTROSPECTION)
		if (texture)
		{
			if (auto render_inspector = inspector())
			{
				render_inspector->on_create_texture(texture, { TS_TEXTURE_ARRAY, data.m_format, data.m_width, data.m_height, size });
			}
		}
#endif
		return texture;
	}

	Texture* ContextVK::create_cube_texture(const TextureRawDataInformation data[6], const TextureGpuDataInformation& info)
	{
		Texture* texture = make_texture(data, 6, true, info);
#if defined(TEXTURE_INTROSPECTION)
		if (texture)
		{
			if (auto render_inspector = inspector())
			{
				render_inspector->on_create_texture(texture, { TS_TEXTURE_CUBE, data[0].m_format, data[0].m_width, data[0].m_height, 6 });
			}
		}
#endif
		return texture;
	}

	Texture* ContextVK::dummy_texture(SpirvReflection::ImageDim dim, bool arrayed)
	{
		//zeros (as DirectX reads a texture not bound) of the shape the shader reads
		const bool cube = dim == SpirvReflection::ImageDim::DIM_CUBE;
		const uint32_t key = uint32_t(cube) << 1 | uint32_t(arrayed);
		auto it = m_dummy_textures.find(key);
		if (it != m_dummy_textures.end()) return it->second.get();
		const unsigned char zero = 0;
		const unsigned char zeros[4] = { zero, zero, zero, zero };
		TextureRawDataInformation pixel{ TF_RGBA8, 1, 1, zeros, TT_RGBA, TTF_UNSIGNED_BYTE, false, 1 };
		TextureRawDataInformation faces[6] = { pixel, pixel, pixel, pixel, pixel, pixel };
		TextureGpuDataInformation info(TMIN_LINEAR, TMAG_LINEAR, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP, false, 0, 0);
		Texture* texture = cube ? make_texture(faces, 6, true, info) : make_texture(&pixel, 1, false, info);
		if (texture) texture->m_array = arrayed;
		m_dummy_textures[key].reset(texture);
		return texture;
	}

	void ContextVK::destroy_texture_later(Texture* texture)
	{
		if (!texture || !texture->m_image) return;
		VkDevice device = m_device;
		VmaAllocator allocator = m_allocator;
		VkImage image = texture->m_image;
		VmaAllocation allocation = texture->m_allocation;
		std::vector<VkImageView> views;
		for (auto& view : texture->m_views) views.push_back(view.second);
		texture->m_views.clear();
		destroy_later([device, allocator, image, allocation, views]()
		{
			for (VkImageView view : views) vkDestroyImageView(device, view, nullptr);
			if (allocation) vmaDestroyImage(allocator, image, allocation);
		});
		texture->m_image = VK_NULL_HANDLE;
	}

	void ContextVK::delete_texture(Texture*& texture)
	{
		if (!texture) return;
		if (m_pass_open && m_pass_target)
		{
			bool attached = m_pass_target->m_depth.m_texture == texture;
			for (const Target::Attachment& color : m_pass_target->m_colors) attached = attached || color.m_texture == texture;
			if (attached) end_render_pass();
		}
		forget(texture);
#if defined(TEXTURE_INTROSPECTION)
		if (auto render_inspector = inspector())
		{
			render_inspector->on_delete_texture(texture);
		}
#endif
		destroy_texture_later(texture);
		delete texture;
		texture = nullptr;
	}

	void ContextVK::update_texture(Texture* texture, const TextureRawDataInformation& data)
	{
		//the level 0 of a 2D texture again, after what is recorded (in the draws of the frame)
		if (!texture || !data.m_bytes || is_compressed_format(data.m_format) || texture->m_layers != 1) return;
		const size_t pixels = size_t(data.m_width) * size_t(data.m_height);
		std::vector<unsigned char> rgba;
		const unsigned char* bytes = data.m_bytes;
		size_t size = pixels * AuxResourcesVK::pixel_bytes(data.m_format);
		if (AuxResourcesVK::is_rgb(data.m_format))
		{
			rgba = AuxResourcesVK::with_alpha(data.m_bytes, pixels, data.m_format);
			bytes = rgba.data();
			size = rgba.size();
		}
		BufferVK staging = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
		if (!staging.m_mapped) return;
		std::memcpy(staging.m_mapped, bytes, size);
		end_render_pass();
		VkCommandBuffer buffer = commands();
		transition(buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VkBufferImageCopy region{};
		region.imageSubresource = { texture->m_view_aspect, 0, 0, 1 };
		region.imageExtent = { std::min(data.m_width, texture->m_width), std::min(data.m_height, texture->m_height), 1 };
		region.bufferRowLength = data.m_width;
		vkCmdCopyBufferToImage(buffer, staging.m_buffer, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		transition(buffer, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		texture->m_last_frame = m_frame_index + 1;
		destroy_buffer_later(staging);
	}

	void ContextVK::copy_texture(Texture* source, Texture* destination)
	{
		//all of it (its layers, level 0) into another of its size and format, after the draws before
		if (!source || !destination || source == destination) return;
		end_render_pass();
		VkCommandBuffer buffer = commands();
		transition(buffer, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		transition(buffer, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VkImageCopy region{};
		region.srcSubresource = { source->m_aspect, 0, 0, std::min(source->m_layers, destination->m_layers) };
		region.dstSubresource = { destination->m_aspect, 0, 0, std::min(source->m_layers, destination->m_layers) };
		region.extent = { std::min(source->m_width, destination->m_width), std::min(source->m_height, destination->m_height), 1 };
		vkCmdCopyImage(buffer, source->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		source->m_last_frame = m_frame_index + 1;
		destination->m_last_frame = m_frame_index + 1;
	}

	void ContextVK::copy_texture_layer(Texture* source, unsigned int source_layer, Texture* destination, unsigned int destination_layer)
	{
		if (!source || !destination) return;
		if (source_layer >= source->m_layers || destination_layer >= destination->m_layers) return;
		end_render_pass();
		VkCommandBuffer buffer = commands();
		if (source == destination)
		{
			transition(buffer, source, VK_IMAGE_LAYOUT_GENERAL);
		}
		else
		{
			transition(buffer, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			transition(buffer, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}
		VkImageCopy region{};
		region.srcSubresource = { source->m_aspect, 0, source_layer, 1 };
		region.dstSubresource = { destination->m_aspect, 0, destination_layer, 1 };
		region.extent = { std::min(source->m_width, destination->m_width), std::min(source->m_height, destination->m_height), 1 };
		vkCmdCopyImage(buffer, source->m_image, source->m_layout, destination->m_image, destination->m_layout, 1, &region);
		source->m_last_frame = m_frame_index + 1;
		destination->m_last_frame = m_frame_index + 1;
	}

	std::vector<unsigned char> ContextVK::get_texture(Texture* texture, int level)
	{
		return get_texture(texture, 0, level);
	}

	std::vector<unsigned char> ContextVK::get_texture(Texture* texture, int face, int level)
	{
		//a layer, a level: its pixels (rows packed), read after what is recorded (done at once)
		if (!texture || is_compressed_format(texture->m_texture_format)) return {};
		if (uint32_t(face) >= texture->m_layers || uint32_t(level) >= texture->m_levels) return {};
		const uint32_t width = texture_level_size(texture->m_width, uint32_t(level));
		const uint32_t height = texture_level_size(texture->m_height, uint32_t(level));
		const size_t size = size_t(width) * size_t(height) * AuxResourcesVK::texel_bytes(texture->m_format);
		BufferVK staging = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, true);
		if (!staging.m_mapped) return {};
		end_render_pass();
		VkCommandBuffer buffer = commands();
		transition(buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VkBufferImageCopy region{};
		region.imageSubresource = { texture->m_view_aspect, uint32_t(level), uint32_t(face), 1 };
		region.imageExtent = { width, height, 1 };
		vkCmdCopyImageToBuffer(buffer, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging.m_buffer, 1, &region);
		flush_and_wait();
		vmaInvalidateAllocation(m_allocator, staging.m_allocation, 0, VK_WHOLE_SIZE);
		std::vector<unsigned char> out(staging.m_mapped, staging.m_mapped + size);
		vmaDestroyBuffer(m_allocator, staging.m_buffer, staging.m_allocation);
		return out;
	}

	void ContextVK::bind_texture(Texture* texture, int texture_id, int)
	{
		//the binding of the shader bound (the textures: by their uniforms, by name)
		if (m_shader && texture_id >= 0 && size_t(texture_id) < m_shader->m_textures.size())
		{
			m_shader->m_textures[size_t(texture_id)] = texture;
		}
	}

	void ContextVK::unbind_texture(Texture* texture)
	{
		if (!m_shader || !texture) return;
		for (Texture*& bound : m_shader->m_textures)
		{
			if (bound == texture) bound = nullptr;
		}
	}

	void ContextVK::unbind_texture(int n)
	{
		if (m_shader && n >= 0 && size_t(n) < m_shader->m_textures.size())
		{
			m_shader->m_textures[size_t(n)] = nullptr;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// What the shaders bind (a buffer, a texture deleted: forgotten)
	void ContextVK::forget(const ConstBuffer* buffer)
	{
		if (!buffer) return;
		for (Shader* shader : m_shaders)
		{
			for (const ConstBuffer*& bound : shader->m_buffers)
			{
				if (bound == buffer) bound = nullptr;
			}
		}
	}

	void ContextVK::forget(const Texture* texture)
	{
		if (!texture) return;
		for (Shader* shader : m_shaders)
		{
			for (Texture*& bound : shader->m_textures)
			{
				if (bound == texture) bound = nullptr;
			}
		}
	}
}
}
