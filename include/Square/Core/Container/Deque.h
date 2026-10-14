//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <deque>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//the double-ended queue of the engine: the one of the STL on the heap of the engine
	template < class T, class Allocator = STLHeapAllocator<T> >
	using Deque = std::deque<T, Allocator>;
}
