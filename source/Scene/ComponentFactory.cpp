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
	static ComponentFactory::ComponentList& component_list()
	{
		static ComponentFactory::ComponentList component_list;
		return component_list;
	};
	static ComponentFactory::ComponentIdMap& component_id_map()
	{
		static ComponentFactory::ComponentIdMap component_list;
		return component_list;
	}
    //public
    Component::SPtr ComponentFactory::create(size_t id)
    {
        //test
		if (id >= component_list().size()) return nullptr;
		//objs
		auto& idcc = component_list()[id];
        //return
		return idcc.m_create(idcc.m_name, id);
    }
    Component::SPtr ComponentFactory::create(const std::string& name)
    {
        //find
        auto obj = component_id_map().find(name);
		//find?
		if (obj == component_id_map().end()) return nullptr;
        //return
        return create(obj->second);
    }
    void ComponentFactory::append(const std::string& name, ComponentFactory::CreateComponent fun)
    {
		//failed
		if (exists(name)) throw std::runtime_error("ComponentFactory: "+name+", already registered");
        //add
		component_id_map().insert({ name, component_list().size() });
		component_list().push_back({ name, fun });
    }
    size_t ComponentFactory::id(const std::string& name)
    {
        //find
        auto obj = component_id_map().find(name);
        //return
        return (*obj).second;
    }
    //list of methods
    std::vector< std::string > ComponentFactory::list_of_components()
    {
        std::vector< std::string > list;
        for (const auto & occid : component_list()) list.push_back(occid.m_name);
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
        return  component_id_map().find(name) != component_id_map().end();
    }
}
}
