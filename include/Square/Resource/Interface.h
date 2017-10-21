//
//  Square
//
//  Created by Gabriele on 12/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <memory>
#include <string>
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Uncopyable.h"


namespace Square
{
namespace Resource
{
	class Manager;
	class SQUARE_API Interface : public SmartPointers<Interface>, public Uncopyable
	{
	public:
		virtual bool load(Manager& resources, const std::string& path) = 0;
	};
}
}