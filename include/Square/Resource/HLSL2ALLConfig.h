//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
//  The configuration of HLSL2ALL (HLSL2ALL_CUSTOM_CONFIGURATION_FILE): the containers of its
//  interface are the ones of the engine (Core/Container: the heap of the engine), so its sources and
//  its outputs are the strings and the vectors of Square (no copy between them)
//
#pragma once
#include "Square/Core/Container/Container.h"

namespace HLSL2ALL
{
	using String = Square::String;
	template < class T >
	using Vector = Square::Vector<T>;
}
