//
//  Square
//
//  Created by Gabriele Di Bari on 09/08/2024.
//  Copyright © 2024 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"

namespace Square
{
	//The allocator of the C runtime: malloc/free (the aligned ones of the platform with an alignment)
	class SQUARE_API SystemAllocator : public Allocator
	{
	public:
		virtual ~SystemAllocator();
		virtual void* alloc(size_t size, const char* name, uint8 flag) override;
		virtual void* alloc(size_t size, const char* name, uint8 flag, uint16 align, uint16 align_offset) override;
		virtual void  free(void* ptr, size_t size) override;
		virtual void  free(void* ptr) override;
	};
	//its old name
	using DefaultAllocator = SystemAllocator;
}
