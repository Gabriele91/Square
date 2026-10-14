//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <map>
#include <functional>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//the ordered maps of the engine: the ones of the STL on the heap of the engine
	template < class Key, class Value, class Compare = std::less<Key>, class Allocator = STLHeapAllocator<std::pair<const Key, Value>> >
	using Map = std::map<Key, Value, Compare, Allocator>;
	template < class Key, class Value, class Compare = std::less<Key>, class Allocator = STLHeapAllocator<std::pair<const Key, Value>> >
	using MultiMap = std::multimap<Key, Value, Compare, Allocator>;
}
