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
    class ComponentFactory
    {
        
    public:
        //EvolutionMethod classes map
        typedef Component::SPtr(*CreateComponent)(const std::string& name, size_t id);
        
        //public
        static Component::SPtr create(size_t id);
        static Component::SPtr create(const std::string& name);
        static void append(const std::string& name, CreateComponent fun);
        static size_t id(const std::string& name);
        
        //list of methods
        static std::vector< std::string > list_of_components();
        static std::string names_of_components(const std::string& sep = ", ");
        
        //info
        static bool exists(const std::string& name);
        
    protected:
        
        struct Object
        {
            size_t          m_id;
            CreateComponent m_create;
        };
        
        using ObjectMap = std::unordered_map < std::string, Object >;
        using IdMap     = std::unordered_map < size_t, std::string >;

        static std::unique_ptr< size_t >    m_id_counter;
        static std::unique_ptr< ObjectMap > m_component_map;
        static std::unique_ptr< IdMap >     m_id_map;

    };
    
    //class used for static registration of a object class
    template<class T>
    class ComponentItem
    {
        
        static Component::SPtr create(const std::string& name, size_t id)
        {
            return (std::make_shared< T >(name, id))->shared_from_this();
        }
        
        ComponentItem(const std::string& name)
        {
            ComponentFactory::append(name, ComponentItem<T>::create);
        }
        
    public:
        
        
        static ComponentItem<T>& instance(const std::string& name)
        {
            static ComponentItem<T> object_item(name);
            return object_item;
        }
        
    };
    
    #define REGISTERED_COMPONENT(class_,name_)\
    namespace\
    {\
    static const ::Square::Scene::ComponentItem<class_>& _Square_ ## scene_ ## class_ ## _component_item=\
                 ::Square::Scene::ComponentItem<class_>::instance( name_ );\
    }
}
}
