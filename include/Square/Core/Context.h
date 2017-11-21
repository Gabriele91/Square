//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/Resource.h"
#include "Square/Core/Attribute.h"
#include <unordered_map>

namespace Square
{
	class Context
	{
	public:

		//Create
		Shared<Object> create(const std::string& name)
		{
			return create(ObjectInfo::compute_id(name));
		}
		Shared<Object> create(uint64 id)
		{
			auto factory = m_object_factories.find(id);
			if (factory != m_object_factories.end()) return factory->second->create();
			return nullptr;
		}
		template< class T > Shared<T> create()
		{
			return DynamicPointerCast<T>(create(T::static_object_id()));
		}

		//Get attrbutes
		const std::vector < Attribute >* attributes(const std::string& name)
		{
			auto attributes = m_attributes.find(ObjectInfo::compute_id(name));
			if (attributes != m_attributes.end()) return &attributes->second;
			return nullptr;
		}
		const std::vector < Attribute >* attributes(uint64 object_id)
		{
			auto attributes = m_attributes.find(object_id);
			if (attributes != m_attributes.end()) return &attributes->second;
			return nullptr;
		}
		const std::vector < Attribute >* attributes(const Object& object)
		{
			return attributes(object.object_id());
		}
		const std::vector < Attribute >* attributes(const ObjectInfo& info)
		{
			return attributes(info.id());
		}
		template< class T > const std::vector < Attribute >* attributes()
		{
			return attributes(T::static_object_id());
		}

		//Get resource
		Shared<ResourceObject> resource(const std::string& name);
		template< class T >  Shared<T> resource(const std::string& name)
		{
			return DynamicPointerCast<T>(resource(name));
		}

		//Get variable
		const Variant& variable(const std::string& name)
		{
			auto variable = m_variables.find(name);
			if (variable != m_variables.end()) return &variable->second;
			return Variant();
		}

		//Object fectory
		void add_object(ObjectFactory* object_fectory)
		{
			m_object_factories[object_fectory->info().id()] = Unique<ObjectFactory>(object_fectory);
		}
		template< class T > void add_object()
		{
			add(new ObjectFactoryItem<T>());
		}
		void add_resource(ObjectFactory* object_fectory,const std::vector< std::string >& exts)
		{
			m_object_factories[object_fectory->info().id()] = Unique<ObjectFactory>(object_fectory);
			for (auto& ext : exts) m_resource_factories[ext] = object_fectory->info().id();
		}
		template< class T > void add_resource(const std::vector< std::string >& exts)
		{
			add(new ObjectFactoryItem<T>());
		}
		
		//Resource
		void add_resource_path(const std::string& name, const std::string& path);

		//Add an attrbute
		void add(const std::string& name, const Attribute& attribute)
		{
			m_attributes[ObjectInfo::compute_id(name)].push_back(attribute);
		}
		void add(uint64 object_id, const Attribute& attribute)
		{
			m_attributes[object_id].push_back(attribute);
		}
		void add(const Object& object, const Attribute& attribute)
		{
			m_attributes[object.object_id()].push_back(attribute);
		}
		void add(const ObjectInfo& info, const Attribute& attribute)
		{
			m_attributes[info.id()].push_back(attribute);
		}
		template < class T >
		void add(const Attribute& attribute)
		{
			m_attributes[T::static_object_id()].push_back(attribute);
		}

		//Add variable
		void add(const std::string& name, const Variant& value)
		{
			m_variables[name] = value;
		}
		
	protected:
	
		//type
		using VariantMap = std::unordered_map< std::string, Variant >;
		using AttributeMap = std::unordered_map< uint64, std::vector < Attribute > >;
		using ObjectFactoryMap = std::unordered_map< uint64, Unique<ObjectFactory> >;
		using ResouceExtensionMap = std::unordered_map< std::string, uint64 >;
		using ResouceObjectMap = std::unordered_map< std::string, Shared<ResourceObject> >;

		//context
		std::unordered_map< std::string, Variant >               m_variables;
		std::unordered_map< uint64, std::vector < Attribute > >  m_attributes;

		std::unordered_map< uint64, Unique<ObjectFactory> >      m_object_factories;

		std::unordered_map< std::string, uint64 >				 m_resource_factories;
		std::unordered_map< std::string, Shared<ResourceObject> >m_resources;
		//friend class
		friend class Application;
		//delete all
		void clear()
		{
			m_variables.clear();
			m_attributes.clear();
			m_object_factories.clear();
		}
	};
}
