//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Logger.h"
#include <unordered_map>

namespace Square
{
	enum AllocType : uint8
	{
		ALCT_DEFAULT,
		ALCT_PERSISTENT,
		ALCT_SHORT
	};

	class SQUARE_API Allocator
	{
	public:
		virtual ~Allocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) = 0;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) = 0;
		virtual void  free(void* ptr, size_t size) = 0;
		virtual void  free(void* ptr) = 0;
	};

	class SQUARE_API DefaultAllocator : public Allocator
	{
	public:
		virtual ~DefaultAllocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};

	class SQUARE_API DebugAllocator : public Allocator
	{
		Logger* m_logger;
		Allocator* m_allocator;
		std::unordered_map<void*, std::string> m_debug_map;
	public:
		DebugAllocator(Allocator* , Logger*);
		virtual ~DebugAllocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};

	#define SQ_NEW(allocator, T, ...) new ((allocator)->alloc(sizeof(T), typeid(T).name(), __VA_ARGS__))
#define SQ_DELETE(allocator, T, ptr) { (ptr)->~T(); (allocator)->free(ptr, sizeof(T)); }
#define SQ_DELETE_NAMESPACE(allocator, NAMESPACE, T, ptr) { (ptr)->~T(); (allocator)->free(ptr, sizeof(NAMESPACE::T)); }
}