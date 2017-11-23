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
        Shared<Object> create(const std::string& name);
        Shared<Object> create(uint64 id);
        
		//Get attrbutes
        const std::vector < Attribute >* attributes(const std::string& name);
        const std::vector < Attribute >* attributes(uint64 object_id);
        const std::vector < Attribute >* attributes(const Object& object);
        const std::vector < Attribute >* attributes(const ObjectInfo& info);
        template< class T > const std::vector < Attribute >* attributes();

		//Get resource
        Shared<ResourceObject> resource(const std::string& name);

		//Get variable
        const Variant& variable(const std::string& name);
        
		//Object fectory
        void add_object(ObjectFactory* object_fectory);
        void add_resource(ObjectFactory* object_fectory,const std::vector< std::string >& exts);
        
		//Resource
        void add_resource_path(const std::string& path, bool recursive = false);
		void add_resource_file(const std::string& filepath);
		void add_resource_file(const std::string& name, const std::string& path);

		//Add an attrbute
        void add_attributes(const std::string& name, const Attribute& attribute);
        void add_attributes(uint64 object_id, const Attribute& attribute);
        void add_attributes(const Object& object, const Attribute& attribute);
        void add_attributes(const ObjectInfo& info, const Attribute& attribute);

		//Add variable
        void add_variable(const std::string& name, const Variant& value);
		
        //template utils
        template< class T > Shared<T> create()
        { return DynamicPointerCast<T>(create(T::static_object_id())); }
        
        template< class T > void add_object()
        { add_object(new ObjectFactoryItem<T>()); }
        
        template< class T > void add_resource(const std::vector< std::string >& exts)
        { add_resource(new ObjectFactoryItem<T>(), exts); }
        
        template < class T >
        void add_attributes(const Attribute& attribute)
        { add_attributes(T::static_object_id(),attribute); }
        
        template< class T >  Shared<T> resource(const std::string& name)
        { return DynamicPointerCast<T>(resource(T::static_object_name() + ":" + name)); }
        
	protected:
	
		//type
		using VariantMap = std::unordered_map< std::string, Variant >;
        using StringMap = std::unordered_map< std::string, std::string >;
        //Attributes
		using AttributeMap = std::unordered_map< uint64, std::vector < Attribute > >;
        //Factory
		using ObjectFactoryMap = std::unordered_map< uint64, Unique<ObjectFactory> >;
        //Resource
        struct ResourceFile
        {
            uint64      m_resouce_id;
            std::string m_filepath;
            //...
            ResourceFile()
            {
            }
            ResourceFile(const ResourceFile& in)
            : m_filepath(in.m_filepath)
            , m_resouce_id(in.m_resouce_id)
            {
            }
            ResourceFile(uint64 id, const std::string filepath)
            : m_filepath(filepath)
            , m_resouce_id(id)
            {
            }
        };
        using ResourceFileMap   = std::unordered_map< std::string, ResourceFile >;
        using ResourceInfoMap   = std::unordered_map< uint64, std::vector<std::string> >;
        using ResourceObjectMap = std::unordered_map< std::string, Shared<ResourceObject> >;
		//context
		VariantMap       m_variables;
		AttributeMap     m_attributes;
        //Object factory
		ObjectFactoryMap m_object_factories;
        //Reousce factory
        ResourceFileMap   m_resources_file;
        ResourceInfoMap   m_resources_info;
		ResourceObjectMap m_resources;
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
