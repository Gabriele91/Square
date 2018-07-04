//
//  Square
//
//  Created by Gabriele on 03/06/2018.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
	static inline void replace(std::string& source, std::string const& find, std::string const& replace)
	{
		std::string::size_type i = source.find(find, 0);
		if (i != std::string::npos) source.replace(i, find.length(), replace);
	}

	static inline void replace_all(std::string& source, std::string const& find, std::string const& replace)
	{
		for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
	}
}
