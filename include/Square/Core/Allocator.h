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

	class Allocator 
	{
	public:
		virtual void* alloc(size_t size, const char* name, uint8 flag) = 0;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) = 0;
		virtual void  free(void* ptr, size_t size) = 0;
		virtual void  free(void* ptr) = 0;
	};

	class DefaultAllocator : public Allocator
	{
	public:
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};

	class DebugAllocator : public Allocator
	{
		Logger* m_logger;
		Allocator* m_allocator;
		std::unordered_map<void*, std::string> m_debug_map;
	public:
		DebugAllocator(Allocator* , Logger*);
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};

	#define SQ_NEW(allocator, T, ...) new ((allocator)->alloc(sizeof(T), typeid(T).name(), __VA_ARGS__))
}