//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"

namespace Square
{
    //Create
    Shared<Object> Context::create(const std::string& name)
    {
        return create(ObjectInfo::compute_id(name));
    }
    Shared<Object> Context::create(uint64 id)
    {
        auto factory = m_object_factories.find(id);
        if (factory != m_object_factories.end()) return factory->second->create();
        return nullptr;
    }
    //Get attrbutes
    const std::vector < Attribute >* Context::attributes(const std::string& name)
    {
        auto attributes = m_attributes.find(ObjectInfo::compute_id(name));
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* Context::attributes(uint64 object_id)
    {
        auto attributes = m_attributes.find(object_id);
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* Context::attributes(const Object& object)
    {
        return attributes(object.object_id());
    }
    const std::vector < Attribute >* Context::attributes(const ObjectInfo& info)
    {
        return attributes(info.id());
    }
    template< class T > const std::vector < Attribute >* Context::attributes()
    {
        return attributes(T::static_object_id());
    }
    
    //Get resource
    Shared<ResourceObject> Context::resource(const std::string& name)
    {
        //olready loaded
        auto resource_it = m_resources.find(name);
        if (resource_it != m_resources.end()) return resource_it->second;
        //else find resource from file
        auto resource_file_it = m_resources_file.find(name);
        if (resource_file_it  == m_resources_file.end()) return nullptr;
        //create resource
        auto resource = DynamicPointerCast<ResourceObject>(create(resource_file_it->second.m_resouce_id));
        if(!resource) return nullptr;
        //load
        if(resource->load(*this, resource_file_it->second.m_filepath))
        {
            m_resources[name] = resource;
            return resource;
        }
        return nullptr;
    }
    
    //Get variable
    const Variant& Context::variable(const std::string& name)
    {
        //get var
        auto variable = m_variables.find(name);
        if (variable != m_variables.end()) return &variable->second;
        //none
        static Variant none;
        return none;
    }
    
    //Object fectory
    void Context::add_object(ObjectFactory* object_fectory)
    {
        m_object_factories[object_fectory->info().id()] = Unique<ObjectFactory>(object_fectory);
    }
    void Context::add_resource(ObjectFactory* object_fectory,const std::vector< std::string >& exts)
    {
        add_object(object_fectory);
        m_resources_info[object_fectory->info().id()] = exts;
    }
    
    //Resource
    void Context::add_resource_path(const std::string& path, bool recursive)
    {
        //for all sub path
        if(recursive)
        for(const std::string& directorypath : Filesystem::get_sub_directories(path))
        {
            add_resource_path(directorypath, recursive);
        }
        //for all files
        for(const std::string& filepath : Filesystem::get_files(path))
        {
            //get extension
            auto f_ext = Filesystem::get_extension(filepath);
            //if is supported
            for(auto r_info : m_resources_info)
            for(auto r_ext  : r_info.second)
            {
                if(r_ext == f_ext)
                {
					auto class_name = m_object_factories[ r_info.first ]->info().name();
                    auto file_name  = Filesystem::get_basename(filepath);
					auto name       = class_name + ":" + file_name;
                    m_resources_file[name] = ResourceFile( r_info.first, filepath );
                    return;
                }
            }
        }
        //end
    }
	void Context::add_resource_file(const std::string& filepath)
	{
		//get extension
		auto    f_ext = Filesystem::get_extension(filepath);
		//if is supported
		for (auto r_info : m_resources_info)
		for (auto r_ext : r_info.second)
		{
			if (r_ext == f_ext)
			{
				auto class_name = m_object_factories[r_info.first]->info().name();
				auto file_name = Filesystem::get_basename(filepath);
				auto name = class_name + ":" + file_name;
				m_resources_file[name] = ResourceFile(r_info.first, filepath);
				return;
			}
		}
	}
	void Context::add_resource_file(const std::string& resource_name, const std::string& filepath)
	{
		//get extension
		auto    f_ext = Filesystem::get_extension(filepath);
		//if is supported
		for (auto r_info : m_resources_info)
		for (auto r_ext : r_info.second)
		{
			if (r_ext == f_ext)
			{
				auto class_name = m_object_factories[r_info.first]->info().name();
				auto name = class_name + ":" + resource_name;
				m_resources_file[name] = ResourceFile(r_info.first, filepath);
				return;
			}
		}
	}

    
    //Add an attrbute
    void Context::add_attributes(const std::string& name, const Attribute& attribute)
    {
        m_attributes[ObjectInfo::compute_id(name)].push_back(attribute);
    }
    void Context::add_attributes(uint64 object_id, const Attribute& attribute)
    {
        m_attributes[object_id].push_back(attribute);
    }
    void Context::add_attributes(const Object& object, const Attribute& attribute)
    {
        m_attributes[object.object_id()].push_back(attribute);
    }
    void Context::add_attributes(const ObjectInfo& info, const Attribute& attribute)
    {
        m_attributes[info.id()].push_back(attribute);
    }
    
    //Add variable
    void Context::add_variable(const std::string& name, const Variant& value)
    {
        m_variables[name] = value;
    }
}
