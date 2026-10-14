//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <unordered_map>
#include <functional>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//the hash maps of the engine: the ones of the STL on the heap of the engine
	template < class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = STLHeapAllocator<std::pair<const Key, Value>> >
	using UnorderedMap = std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>;
	template < class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = STLHeapAllocator<std::pair<const Key, Value>> >
	using UnorderedMultiMap = std::unordered_multimap<Key, Value, Hash, KeyEqual, Allocator>;
}
