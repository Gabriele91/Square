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
    //class factory of object
    class SQUARE_API ComponentFactory
    {
	public:

		//element
        typedef Shared<Component>(*CreateComponent)();
        typedef void (*AttributesComponent)(SerializeContext& context);

		//element in list
		struct CreateComponentInfo
		{
			std::string  m_name;
			CreateComponent m_create;
            AttributesComponent m_attributes;
		};
        

		//types
		using ComponentIdMap   = std::unordered_map < uint64, CreateComponentInfo >;
		using ComponentNameMap = std::unordered_map < std::string, uint64 >;

        //public
        static Shared<Component> create(uint64 id);
        static Shared<Component> create(const std::string& name);
        static void append(const ObjectInfo& info, CreateComponent create, AttributesComponent attrs);
        static void attributes_registration(SerializeContext& context);

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
        
        static Shared<Component> create()
        {
            return (std::make_shared< T >())->shared_from_this();
        }
        
        ComponentItem()
        {
            ComponentFactory::append(T::static_object_info(), ComponentItem<T>::create, T::attributes);
        }
        
    public:
                
        static ComponentItem<T>& instance()
        {
            static ComponentItem<T> object_item;
            return object_item;
        }
        
    };
    
    #define SQUARE_COMPONENT_REGISTRATION(class_)\
    namespace\
    {\
        SQUARE_OBJECT_REGISTRATION(class_)\
        static const auto& _Square_scene_ ## class_ ## _component_item = ::Square::Scene::ComponentItem<class_>::instance();\
    }
}
}
