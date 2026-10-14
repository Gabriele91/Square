//
//  AllocationVK.cpp
//  Square, the driver of Vulkan
//
//  See AllocationVK.h.
//
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "AllocationVK.h"
#include "Square/Core/Allocator/MemoryOperators.h"

namespace Square
{
namespace Render
{
namespace AllocationVK
{
namespace
{
	//before each block: the block of the heap, its size (Vulkan asks for any alignment, and for
	//realloc without the old size)
	struct Header
	{
		void*  m_block;
		size_t m_size;
	};
	static_assert(sizeof(Header) <= 16, "the header in the 16 bytes before a block");

	static Header* header_of(void* ptr)
	{
		return reinterpret_cast<Header*>(static_cast<char*>(ptr) - sizeof(Header));
	}

	void* VKAPI_PTR allocate(void*, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		alignment = std::max<size_t>(alignment, 16);
		char* block = static_cast<char*>(MemoryOperators::allocate(size + alignment + sizeof(Header), 16));
		if (!block) return nullptr;
		char* ptr = reinterpret_cast<char*>((uintptr_t(block) + sizeof(Header) + alignment - 1) & ~uintptr_t(alignment - 1));
		*header_of(ptr) = Header{ block, size };
		return ptr;
	}

	void VKAPI_PTR deallocate(void*, void* ptr)
	{
		if (ptr) MemoryOperators::deallocate(header_of(ptr)->m_block);
	}

	void* VKAPI_PTR reallocate(void* user, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
	{
		if (!original) return allocate(user, size, alignment, scope);
		if (!size)
		{
			deallocate(user, original);
			return nullptr;
		}
		void* ptr = allocate(user, size, alignment, scope);
		if (!ptr) return nullptr;
		std::memcpy(ptr, original, std::min(size, header_of(original)->m_size));
		deallocate(user, original);
		return ptr;
	}
}

	const VkAllocationCallbacks* callbacks()
	{
		static const VkAllocationCallbacks s_callbacks
		{
			nullptr,     //pUserData
			&allocate,   //pfnAllocation
			&reallocate, //pfnReallocation
			&deallocate, //pfnFree
			nullptr,     //pfnInternalAllocation
			nullptr      //pfnInternalFree
		};
		return &s_callbacks;
	}
}
}
}
