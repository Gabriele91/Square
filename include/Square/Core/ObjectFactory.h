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
#include <unordered_map>

namespace Square
{
    //class factory of object
    class SQUARE_API ObjectFactory
    {
	public:

		//element
		typedef Shared<Object>(*CreateObject)();

		//element in list
		struct CreateObjectInfo
		{
			std::string  m_name;
			CreateObject m_create;
		};
        

		//types
		using ObjectIdMap   = std::unordered_map < uint64, CreateObjectInfo >;
		using ObjectNameMap = std::unordered_map < std::string, uint64 >;

        //public
        static Shared<Object> create(uint64 id);
        static Shared<Object> create(const std::string& name);
        static void append(const ObjectInfo& info, CreateObject fun);

		//get id
		static uint64 id(const std::string& name);
		static const std::string& name(size_t name);
        
        //list of methods
        static std::vector< std::string > list_of_components();
        static std::string names_of_components(const std::string& sep = ", ");
        
        //info
		static bool exists(const std::string& name);
		static bool exists(uint64 id);
    };
    
    //class used for static registration of a object class
    template<class T>
    class SQUARE_API ObjectItem
    {
        
        static Shared<Object> create()
        {
            return (std::make_shared< T >())->shared_from_this();
        }
        
        ObjectItem()
        {
            ObjectFactory::append(T::static_object_info(), ObjectItem<T>::create);
        }
        
    public:
                
        static ObjectItem<T>& instance()
        {
            static ObjectItem<T> object_item;
            return object_item;
        }
        
    };
    
    #define SQUARE_OBJECT_REGISTRATION(class_)\
    namespace\
    {\
    static const ::Square::ObjectItem<class_>& _Square_ ## scene_ ## class_ ## _component_item=\
                 ::Square::ObjectItem<class_>::instance();\
    }
}
