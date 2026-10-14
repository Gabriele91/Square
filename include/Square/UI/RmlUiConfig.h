//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
//  The configuration of RmlUi (RMLUI_CUSTOM_CONFIGURATION_FILE, in place of RmlUi/Config/Config.h):
//  its containers of the STL are the ones of the engine (Core/Container: the heap of the engine),
//  so its strings and lists are the ones of Square (no copy between them); its flat maps and sets
//  (robin_hood, itlib) as by default (they allocate with malloc: freed by them)
//
#pragma once
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include "Square/Config.h"
#include "Square/Core/Container/Container.h"
#include "RmlUi/Core/Containers/itlib/flat_map.hpp"
#include "RmlUi/Core/Containers/itlib/flat_set.hpp"
#include "RmlUi/Core/Containers/robin_hood.h"

namespace Rml {

	// Default matrix type to be used (see RmlUi/Config/Config.h)
	#ifdef RMLUI_MATRIX_ROW_MAJOR
		#define RMLUI_MATRIX4_TYPE RowMajorMatrix4f
	#else
		#define RMLUI_MATRIX4_TYPE ColumnMajorMatrix4f
	#endif

	// A way to disable 'final' specified for Rml::Releaser class.
	#define RMLUI_RELEASER_FINAL final

// Containers types: the ones of the engine
template <typename T>
using Vector = Square::Vector<T>;
template <typename T, size_t N = 1>
using Array = Square::Array<T, N>;
template <typename T>
using Stack = Square::Stack<T>;
template <typename T>
using List = Square::List<T>;
template <typename T>
using Queue = Square::Queue<T>;
template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;
template <typename Key, typename Value>
using StableMap = Square::Map<Key, Value>;
template <typename Key, typename Value>
using StableUnorderedMap = Square::UnorderedMap<Key, Value>;
template <typename Key, typename Value>
using UnorderedMultimap = Square::UnorderedMultiMap<Key, Value>;

template <typename Key, typename Value>
using UnorderedMap = robin_hood::unordered_flat_map<Key, Value>;
template <typename Key, typename Value>
using SmallUnorderedMap = itlib::flat_map<Key, Value>;
template <typename Key, typename Value>
using SmallOrderedMap = itlib::flat_map<Key, Value>;
template <typename T>
using UnorderedSet = robin_hood::unordered_flat_set<T>;
template <typename T>
using SmallUnorderedSet = itlib::flat_set<T>;
template <typename T>
using SmallOrderedSet = itlib::flat_set<T>;

// Utilities.
template <typename T>
using Hash = std::hash<T>;
template <typename T>
using Function = std::function<T>;
template <typename Iterator>
inline std::move_iterator<Iterator> MakeMoveIterator(Iterator it)
{
	return std::make_move_iterator(it);
}

// Strings: the ones of the engine
using String = Square::String;
using StringList = Vector<String>;

// Smart pointer types.
template <typename T>
using UniquePtr = std::unique_ptr<T>;
template <typename T>
class Releaser;
template <typename T>
using UniqueReleaserPtr = std::unique_ptr<T, Releaser<T>>;
template <typename T>
using SharedPtr = std::shared_ptr<T>;
template <typename T>
using WeakPtr = std::weak_ptr<T>;
template <typename T, typename... Args>
inline SharedPtr<T> MakeShared(Args&&... args)
{
	return std::make_shared<T, Args...>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Args&&... args)
{
	return std::make_unique<T, Args...>(std::forward<Args>(args)...);
}

} // namespace Rml
