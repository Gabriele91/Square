//
//  Square
//
//  Created by Gabriele on 11/03/18.
//  Copyright © 2018 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"


namespace Square
{
namespace SharedLibrary
{
	void* open(const std::string& path);
	void* get(void* shared, const std::string& name);
	void  close(void*);
}
}