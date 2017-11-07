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
    using Shared = std::shared_ptr< T >;
    
    template < class T >
    using Unique = std::unique_ptr< T >;
    
    template < class T >
    using Weak = std::weak_ptr< T >;
    
	template < class T >
	class SharedObject : public std::enable_shared_from_this< T >
	{
	public:
        
		template < class... Args >
		static Shared<T> snew(Args&&... args)
		{
			return std::make_shared< T >(args...);
		}

	};
    
}
