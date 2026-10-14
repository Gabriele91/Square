//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <array>
#include <cstddef>

namespace Square
{
	//the array of the engine (in its object: no allocator)
	template < class T, std::size_t N >
	using Array = std::array<T, N>;
}
