//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Scene/Actor.h"
#include "Square/Scene/ComponentFactory.h"
#include <sstream>
#include <iterator>

namespace Square
{
namespace Scene
{
	//static 
	static ComponentFactory::ComponentIdMap& component_id_map()
	{
		static ComponentFactory::ComponentIdMap component_list;
		return component_list;
	};
	static ComponentFactory::ComponentNameMap& component_name_map()
	{
		static ComponentFactory::ComponentNameMap component_list;
		return component_list;
	}
    //public
    Component::SPtr ComponentFactory::create(uint64 id)
    {
		//find
		auto obj = component_id_map().find(id);
        //test
		if (obj == component_id_map().end()) return nullptr;
        //return
		return obj->second.m_create();
    }
    Component::SPtr ComponentFactory::create(const std::string& name)
    {
		//return
		return create(id(name));
    }
    void ComponentFactory::append(const ObjectInfo& info, ComponentFactory::CreateComponent fun)
    {
		//failed
		if (exists(info.id())) throw std::runtime_error("ComponentFactory: "+ info.name() +", already registered");
        //add
		component_id_map().insert({ info.id(), ComponentInfo{ info.name(), fun } });
		component_name_map().insert({  info.name(), info.id() });
    }
	//find
	uint64 ComponentFactory::id(const std::string& name)
    {
        //find
        auto obj = component_name_map().find(name);
        //return
        return (*obj).second;
    }
	static const std::string& name(size_t id)
	{
		//find
		auto obj = component_id_map().find(id);
		//return
		return (*obj).second.m_name;
	}
    //list of methods
    std::vector< std::string > ComponentFactory::list_of_components()
    {
        std::vector< std::string > list;
        for (const auto & occid : component_name_map()) list.push_back(occid.first);
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
		return  exists(id(name));
	}
	bool ComponentFactory::exists(uint64 id)
	{
		return  component_id_map().find(id) != component_id_map().end();
	}
}
}
