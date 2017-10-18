//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
	template < class T >
	class SmartPointers : public std::enable_shared_from_this< T >
	{
	public:

		using SPtr = std::shared_ptr< T >;
		using UPtr = std::unique_ptr< T >;
		using WPtr = std::weak_ptr< T >;

		template < class... Args >
		static SPtr snew(Args&&... args)
		{
			return std::make_shared< T >(args...);
		}

	};
}
