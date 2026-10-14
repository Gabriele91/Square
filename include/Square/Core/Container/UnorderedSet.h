//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <unordered_set>
#include <functional>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//the hash sets of the engine: the ones of the STL on the heap of the engine
	template < class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = STLHeapAllocator<Key> >
	using UnorderedSet = std::unordered_set<Key, Hash, KeyEqual, Allocator>;
	template < class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = STLHeapAllocator<Key> >
	using UnorderedMultiSet = std::unordered_multiset<Key, Hash, KeyEqual, Allocator>;
}
