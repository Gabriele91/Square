#pragma once
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
        UNKNOWN,
		NETOWRK,
		USERDATA,
		FILE
	};

	//resource interface
	class SQUARE_API ResourceObject 
	: public Object
	, public InheritableSharedObject<ResourceObject>
	, public Uncopyable
	{
	public:
		SQUARE_OBJECT(ResourceObject)
		//Resource
		ResourceObject(Context& context) ;
		//load from file
		virtual bool         load(const std::string& path) = 0;
        virtual ResourceType resource_type()               { return ResourceType::UNKNOWN;                  }
		virtual const char*  resource_name()         const { return m_resource_name ? m_resource_name : ""; }
		virtual const char*  resource_untyped_name() const 
		{ 
			for (size_t i = 0; *(m_resource_name + i); ++i)
				if (*(m_resource_name + i) == ':')
					return m_resource_name + i + 1;
			return m_resource_name; 
		}

	protected:
		//info resource
		const char* m_resource_name{nullptr};
		//set values
		void resource_name(const char* name) { m_resource_name = name; }
		//friend
		friend class Context;
		friend class BaseContext;
	};

}
