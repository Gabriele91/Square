//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <queue>
#include <stack>
#include <functional>
#include "Square/Core/Container/Deque.h"
#include "Square/Core/Container/Vector.h"

namespace Square
{
	//the adaptors of the STL on the containers of the engine
	template < class T, class Container = Deque<T> >
	using Queue = std::queue<T, Container>;
	template < class T, class Container = Deque<T> >
	using Stack = std::stack<T, Container>;
	template < class T, class Container = Vector<T>, class Compare = std::less<typename Container::value_type> >
	using PriorityQueue = std::priority_queue<T, Container, Compare>;
}
