//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Scene/Actor.h"
#include "Square/Scene/ComponentFactory.h"
#include <sstream>

namespace Square
{
namespace Scene
{
    //values
    std::unique_ptr< size_t >                      ComponentFactory::m_id_counter;
    std::unique_ptr< ComponentFactory::ObjectMap > ComponentFactory::m_component_map;
    std::unique_ptr< ComponentFactory::IdMap  >    ComponentFactory::m_id_map;
    //public
    Component::SPtr ComponentFactory::create(size_t id)
    {
        //map is alloc?
        if (!m_id_map) return nullptr;
        //find
        auto obj = m_id_map->find(id);
        //return
        return create((*obj).second);
    }
    Component::SPtr ComponentFactory::create(const std::string& name)
    {
        //map is alloc?
        if (!m_component_map) return nullptr;
        //find
        auto obj = m_component_map->find(name);
        //return
        return (*obj).second.m_create(name, (*obj).second.m_id);
    }
    void ComponentFactory::append(const std::string& name, ComponentFactory::CreateComponent fun)
    {
        //alloc
        if (!m_id_counter)    m_id_counter    = std::make_unique< size_t >(0);
        if (!m_component_map) m_component_map = std::make_unique< ComponentFactory::ObjectMap >();
        if (!m_id_map)        m_id_map        = std::make_unique< ComponentFactory::IdMap >();
        //add
        m_id_map->operator[](*m_id_counter) = name;
        m_component_map->operator[](name)   = Object{ (*m_id_counter)++, fun };
    }
    size_t ComponentFactory::id(const std::string& name)
    {
        //find
        auto obj = m_component_map->find(name);
        //return
        return (*obj).second.m_id;
    }
    //list of methods
    std::vector< std::string > ComponentFactory::list_of_components()
    {
        std::vector< std::string > list;
        for (const auto & pair : *m_component_map) list.push_back(pair.first);
        return list;
    }
    std::string  ComponentFactory::names_of_components(const std::string& sep)
    {
        std::stringstream sout;
        auto list = list_of_components();
        std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
        sout << *(list.end() - 1);
        return sout.str();
    }
    //info
    bool ComponentFactory::exists(const std::string& name)
    {
        //find
        auto it = m_component_map->find(name);
        //return
        return it != m_component_map->end();
    }
}
}
