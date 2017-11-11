//
//  ObjectFactory.h
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/Attribute.h"

namespace Square
{
    //class factory of object
    class SQUARE_API ClassObjectRegistration
    {
	public:
		//Registration Function
		typedef void(*RegistrationFunction)(Context& context);

		//element in list
		struct RegistrationItem
		{
			const ObjectInfo&          m_info;
			const RegistrationFunction m_registration;
			//init
			RegistrationItem(const ObjectInfo& info, const RegistrationFunction registration)
			:m_info(info)
			,m_registration(registration)
			{

			}
		};

		//type
		using RegistrationList = std::vector< RegistrationItem >;
		
		//add
		static void append(const RegistrationItem& item);
		static void append(const ObjectInfo& info, RegistrationFunction registration);
        
        //list of clas objects
		static RegistrationList& item_list();
        
        //info
		static bool exists(const std::string& name);
		static bool exists(uint64 id);
		static bool exists(const ObjectInfo& info);
    };
    
    //class used for static registration of a object class
    template<class T>
    class SQUARE_API ClassObjectRegistrationItem
    {
		ClassObjectRegistrationItem()
        {
			ClassObjectRegistration::append(T::static_object_info(), &T::object_registration);
        }
        
    public:
                
        static ClassObjectRegistrationItem<T>& instance()
        {
            static ClassObjectRegistrationItem<T> object_item;
            return object_item;
        }
    };
    
    #define SQUARE_CLASS_OBJECT_REGISTRATION(class_)\
    namespace\
    {\
        static const auto& Square_ ## class_ ## _class_object_registration_item =\
		::Square::ClassObjectRegistrationItem<class_>::instance();\
    }
}
