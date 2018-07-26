//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Application.h"
#include <iostream>

namespace Square
{
	//disable public allocation
	BaseContext::BaseContext()
	{
		//none
	}
    //Create
    Shared<Object> BaseContext::create(const std::string& name)
    {
        return create(ObjectInfo::compute_id(name));
    }
    Shared<Object> BaseContext::create(uint64 id)
    {
        auto factory = m_object_factories.find(id);
        if (factory != m_object_factories.end()) return factory->second->create();
        return nullptr;
    }
    //Get attrbutes
    const std::vector < Attribute >* BaseContext::attributes(const std::string& name)
    {
        auto attributes = m_attributes.find(ObjectInfo::compute_id(name));
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* BaseContext::attributes(uint64 object_id)
    {
        auto attributes = m_attributes.find(object_id);
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* BaseContext::attributes(const Object& object)
    {
        return attributes(object.object_id());
    }
    const std::vector < Attribute >* BaseContext::attributes(const ObjectInfo& info)
    {
        return attributes(info.id());
    }
    
    //Get resource
    Shared<ResourceObject> BaseContext::resource(const std::string& name)
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
		//insert 
		m_resources.insert({ name, resource });
		//iterator
		resource_it = m_resources.find(name);
		//set resource name
		resource->resource_name(resource_it->first.c_str());
        //load
        if(resource->load(resource_file_it->second.m_filepath)) return resource;
		//fail
		m_resources.erase(resource_it);
		//end
        return nullptr;
    }
    
    const std::string& BaseContext::resource_path(const std::string& name)
    {
        //find resource from file
        auto resource_file_it = m_resources_file.find(name);
        //fail
        if (resource_file_it  == m_resources_file.end()) return "";
        //ok
        return resource_file_it->second.m_filepath;
        
    }
    //Get variable
    const Variant& BaseContext::variable(const std::string& name)
    {
        //get var
        auto variable = m_variables.find(name);
        if (variable != m_variables.end()) return &variable->second;
        //none
        static Variant none;
        return none;
    }
    
    //Object fectory
    void BaseContext::add_object(ObjectFactory* object_fectory)
    {
        m_object_factories[object_fectory->info().id()] = Unique<ObjectFactory>(object_fectory);
    }
    void BaseContext::add_resource(ObjectFactory* object_fectory,const std::vector< std::string >& exts)
    {
        add_object(object_fectory);
        m_resources_info[object_fectory->info().id()] = exts;
    }
    
    //Resource
    void BaseContext::add_resource_path(const std::string& path, bool recursive)
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
	void BaseContext::add_resource_file(const std::string& filepath)
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
	void BaseContext::add_resource_file(const std::string& resource_name, const std::string& filepath)
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
    void BaseContext::add_attributes(const std::string& name, const Attribute& attribute)
    {
        m_attributes[ObjectInfo::compute_id(name)].push_back(attribute);
    }
    void BaseContext::add_attributes(uint64 object_id, const Attribute& attribute)
    {
        m_attributes[object_id].push_back(attribute);
    }
    void BaseContext::add_attributes(const Object& object, const Attribute& attribute)
    {
        m_attributes[object.object_id()].push_back(attribute);
    }
    void BaseContext::add_attributes(const ObjectInfo& info, const Attribute& attribute)
    {
        m_attributes[info.id()].push_back(attribute);
    }
    
    //Add variable
    void BaseContext::add_variable(const std::string& name, const Variant& value)
    {
        m_variables[name] = value;
    }

	//Resource info
	BaseContext::ResourceFile::ResourceFile()
	{
	}
	BaseContext::ResourceFile::ResourceFile(const ResourceFile& in)
	: m_filepath(in.m_filepath)
	, m_resouce_id(in.m_resouce_id)
	{
	}
	BaseContext::ResourceFile::ResourceFile(uint64 id, const std::string filepath)
	: m_filepath(filepath)
	, m_resouce_id(id)
	{
	}


	//context errors/wrongs
	void BaseContext::add_wrong(const std::string& wrong)
	{
		m_wrongs.push_back(wrong);
	}

	void BaseContext::add_wrongs(const BaseContext::StringList& wrongs)
	{
		for (const std::string& wrong : wrongs)
		{
			m_wrongs.push_back(wrong);
		}
	}

	const BaseContext::StringList& BaseContext::wrongs() const
	{
		return m_wrongs;
	}

	void BaseContext::show_wrongs() const
	{
		show_wrongs(std::cerr);
	}

	void BaseContext::show_wrongs(std::ostream& output) const
	{
		for (const std::string& wrong : wrongs())
		{
			output << wrong << std::endl;
		}
	}

	//get application
	Application* BaseContext::application()
	{
		return m_application;
	}
	//get render
	Render::Context* BaseContext::render()
	{
		return application() ? application()->render() : nullptr;
	}
	//get window
	Video::Window* BaseContext::window()
	{
		return application() ? application()->window() : nullptr;
	}
	//get window
	Video::Input* BaseContext::input()
	{
		return application() ? application()->input() : nullptr;
	}
	//get world
	Scene::World* BaseContext::world()
	{
		return application() ? application()->world() : nullptr;
	}
    //get application
    const Application* BaseContext::application() const
    {
        return m_application;
    }
    //get render
    const Render::Context* BaseContext::render() const
    {
        return application() ? application()->render() : nullptr;
    }
    //get window
    const Video::Window* BaseContext::window() const
    {
        return application() ? application()->window() : nullptr;
    }
	//get window
	const Video::Input* BaseContext::input() const
	{
		return application() ? application()->input() : nullptr;
	}
	//get world
	const Scene::World* BaseContext::world() const
	{
		return application() ? application()->world() : nullptr;
	}

	void BaseContext::clear()
	{
        //
        m_wrongs.clear();
		m_variables.clear();
		m_attributes.clear();
		m_object_factories.clear();
        //
        m_resources_file.clear();
        m_resources_info.clear();
        m_resources.clear();
	}
}
