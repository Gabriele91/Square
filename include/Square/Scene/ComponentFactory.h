//
//  ComponentFactory.h
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Scene/Component.h"
#include <unordered_map>

namespace Square
{
namespace Scene
{
    //class factory of scene component
    class SQUARE_API ComponentFactory
    {
	public:

		//element
		typedef Component::SPtr(*CreateComponent)();

		//element in list
		struct ComponentInfo
		{
			std::string     m_name;
			CreateComponent m_create;
		};
        

		//types
		using ComponentIdMap   = std::unordered_map < uint64, ComponentInfo >;
		using ComponentNameMap = std::unordered_map < std::string, uint64 >;

        //public
        static Component::SPtr create(uint64 id);
        static Component::SPtr create(const std::string& name);
        static void append(const ObjectInfo& info, CreateComponent fun);

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
    class SQUARE_API ComponentItem
    {
        
        static Component::SPtr create()
        {
            return (std::make_shared< T >())->shared_from_this();
        }
        
        ComponentItem()
        {
            ComponentFactory::append(T::static_object_info(), ComponentItem<T>::create);
        }
        
    public:
                
        static ComponentItem<T>& instance()
        {
            static ComponentItem<T> object_item;
            return object_item;
        }
        
    };
    
    #define SQUARE_COMPONENT(class_)\
    namespace\
    {\
    static const ::Square::Scene::ComponentItem<class_>& _Square_ ## scene_ ## class_ ## _component_item=\
                 ::Square::Scene::ComponentItem<class_>::instance();\
    }
}
}