//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <set>
#include <functional>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//the ordered sets of the engine: the ones of the STL on the heap of the engine
	template < class Key, class Compare = std::less<Key>, class Allocator = STLHeapAllocator<Key> >
	using Set = std::set<Key, Compare, Allocator>;
	template < class Key, class Compare = std::less<Key>, class Allocator = STLHeapAllocator<Key> >
	using MultiSet = std::multiset<Key, Compare, Allocator>;
}
