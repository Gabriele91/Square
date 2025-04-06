//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include <memory>
#include <cstring>
#include <algorithm>
#include "Square/Core/Object.h"

namespace Square
{
	//circle shift
	template< typename T > T circshift(T x, T n)
	{
		return (x << n) | (x >> (sizeof(T) - n));
	}

	//compute id
	uint64 ObjectInfo::compute_id(const std::string& name)
	{
		//buffer
		uint64 buffer = 0;
		//sead
		uint64 id = name.size();
		//hash function 
		for (uint64 i = 0; i < uint64(name.size()); i += sizeof(uint64))
		{
			std::memcpy(&buffer, &name.c_str()[i], std::min<size_t>(sizeof(uint64), name.size() - i));
			buffer = circshift(buffer, i);
			id ^= id * buffer + buffer % id;
		}
		//return
		return id;
	}

	//object info
	ObjectInfo::ObjectInfo(const std::string& name)
	{
		//sead
		m_id = compute_id(name);
		//save name
		m_name = name;
	}

	//object id
	uint64 ObjectInfo::id() const
	{
		return m_id;
	}

	//object name
	const std::string& ObjectInfo::name() const
	{
		return m_name;
	}

	// Object
	std::optional<const VariantRef> Object::variable(const std::string& name) const
	{
		//get var
		auto variable = m_variables.find(name);
		if (variable != m_variables.end()) return &variable->second;
		//none
		return {};
	}

	void Object::set_variable(const std::string& name, const Variant& value)
	{
		m_variables[name] = value;
	}

	//object factory
	ObjectFactory::ObjectFactory(const ObjectFactory&) = default;

	ObjectFactory::ObjectFactory(ObjectFactory&&) = default;

	ObjectFactory::ObjectFactory(const ObjectInfo& info) : m_info(info) { }

	const ObjectInfo& ObjectFactory::info() { return m_info; }


}