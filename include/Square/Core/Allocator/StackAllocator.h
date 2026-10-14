//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/LinearAllocator.h"

namespace Square
{
	//A linear allocator freed back to a marker (the blocks after it): scopes, last in first out
	//    StackAllocator::Scope scope(stack); ... (its blocks freed at the end of the scope)
	//Not thread safe.
	class SQUARE_API StackAllocator : public LinearAllocator
	{
	public:
		using Marker = LinearAllocator::Marker;

		explicit StackAllocator(Allocator* parent, size_t chunk_size = 64 * 1024);
		virtual ~StackAllocator();

		//where it is now, its blocks after a marker freed
		Marker marker() const;
		void   rewind(const Marker& marker);

		//the blocks of a scope
		class SQUARE_API Scope
		{
		public:
			explicit Scope(StackAllocator& stack);
			~Scope();
			Scope(const Scope&) = delete;
			Scope& operator=(const Scope&) = delete;
		private:
			StackAllocator& m_stack;
			Marker          m_marker;
		};
	};
}
