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
namespace Resource
{
	//declaretion
	class Manager;
	enum class Type;
	class Interface;

	//type
	enum class Type
	{
		NETOWRK,
		USERDATA,
		FILE
	};

	//resource interface
	class SQUARE_API Interface : public Object
							   , public SharedObject<Interface>
							   , public Uncopyable
	{
	public:
		SQUARE_OBJECT(Interface)
		virtual bool load(Manager& resources, const std::string& path) = 0;
		virtual Type resource_type() = 0;
	};

	//class used for static registration of a resource
	template<class T>
	class SQUARE_API ClassResouceRegistrationItem
	{
		ClassResouceRegistrationItem()
		{
			ClassObjectRegistration::append(T::static_object_info(), &T::object_registration);
		}

	public:

		static ClassResouceRegistrationItem<T>& instance()
		{
			static ClassResouceRegistrationItem<T> object_item;
			return object_item;
		}
	};

	#define SQUARE_CLASS_RESOURCE_REGISTRATION(class_, ...)\
    namespace\
    {\
		SQUARE_CLASS_RESOURCE_REGISTRATION(class_)\
        static const auto& Square_ ## class_ ## _class_resouce_registration_item =\
		::Square::ClassObjectRegistrationItem<class_>::instance();\
    }
}
}
