﻿#pragma once
//
//  Square
//
//  Created by Gabriele on 12/07/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include <memory>
#include <string>
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Uncopyable.h"


namespace Square
{
	//declaretion
	enum class ResourceType;
	class ResourceObject;

	//type
	enum class ResourceType
	{
		NETOWRK,
		USERDATA,
		FILE
	};

	//resource interface
	class SQUARE_API ResourceObject 
	: public Object
	, public SharedObject<ResourceObject>
	, public Uncopyable
	{
	public:
		SQUARE_OBJECT(ResourceObject)
		virtual bool         load(Context& context, const std::string& path) = 0;
		virtual ResourceType resource_type() = 0;
	};
}
