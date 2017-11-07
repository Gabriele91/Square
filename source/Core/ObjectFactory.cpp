//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/ObjectFactory.h"
#include <sstream>
#include <iterator>

namespace Square
{
	//static 
	static ObjectFactory::ObjectIdMap& component_id_map()
	{
		static ObjectFactory::ObjectIdMap component_list;
		return component_list;
	};
	static ObjectFactory::ObjectNameMap& component_name_map()
	{
		static ObjectFactory::ObjectNameMap component_list;
		return component_list;
	}
    //public
    Shared<Object> ObjectFactory::create(uint64 id)
    {
		//find
		auto obj = component_id_map().find(id);
        //test
		if (obj == component_id_map().end()) return nullptr;
        //return
		return obj->second.m_create();
    }
    Shared<Object> ObjectFactory::create(const std::string& name)
    {
		//return
		return create(id(name));
    }
    void ObjectFactory::append(const ObjectInfo& info, ObjectFactory::CreateObject fun)
    {
		//failed
		if (exists(info.id())) throw std::runtime_error("ObjectFactory: "+ info.name() +", already registered");
        //add
		component_id_map().insert({ info.id(), CreateObjectInfo{ info.name(), fun } });
		component_name_map().insert({  info.name(), info.id() });
    }
	//find
	uint64 ObjectFactory::id(const std::string& name)
    {
        //find
        auto obj = component_name_map().find(name);
        //return
        return (*obj).second;
    }
 
    const std::string& ObjectFactory::name(size_t id)
	{
		//find
		auto obj = component_id_map().find(id);
		//return
		return (*obj).second.m_name;
	}
    //list of methods
    std::vector< std::string > ObjectFactory::list_of_components()
    {
        std::vector< std::string > list;
        for (const auto & occid : component_name_map()) list.push_back(occid.first);
        return list;
    }
    std::string  ObjectFactory::names_of_components(const std::string& sep)
    {
        std::stringstream sout;
        auto list = list_of_components();
        std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
        sout << *(list.end() - 1);
        return sout.str();
    }
    //info
	bool ObjectFactory::exists(const std::string& name)
	{
		return  exists(id(name));
	}
	bool ObjectFactory::exists(uint64 id)
	{
		return  component_id_map().find(id) != component_id_map().end();
	}
}
