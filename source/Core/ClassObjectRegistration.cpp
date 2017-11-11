//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/ClassObjectRegistration.h"
#include <sstream>
#include <iterator>

namespace Square
{
	//static 
	static ClassObjectRegistration::RegistrationList& global_item_list()
	{
		static ClassObjectRegistration::RegistrationList list;
		return list;
	};
    void ClassObjectRegistration::append(const RegistrationItem& item)
    {
		//failed
		if (exists(item.m_info)) throw std::runtime_error("ObjectFactory: "+ item.m_info.name() +", already registered");
        //add
		global_item_list().emplace_back(item);
    }
	void ClassObjectRegistration::append(const ObjectInfo& info, RegistrationFunction registration)
	{
		//failed
		if (exists(info)) throw std::runtime_error("ObjectFactory: " + info.name() + ", already registered");
		//add
		global_item_list().emplace_back(info, registration);
	}
    //list of methods
	ClassObjectRegistration::RegistrationList& ClassObjectRegistration::item_list()
    {
        return global_item_list();
    }
    //info
	bool ClassObjectRegistration::exists(const std::string& name)
	{
		return  exists(ObjectInfo::compute_id(name));
	}
	bool ClassObjectRegistration::exists(uint64 id)
	{
		for (const auto& item : item_list())
		{
			if (item.m_info.id() == id) return true;
		}
		return false;
	}
	bool ClassObjectRegistration::exists(const ObjectInfo& info)
	{
		return exists(info.id());
	}
}
