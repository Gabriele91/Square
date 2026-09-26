//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Application.h"
#include "Square/Data/ParserResources.h"
#include "Square/System/System.h"
#include "Square/Scene/World.h"
#include <iostream>
#include <algorithm>

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
    const std::vector < Attribute >* BaseContext::attributes(const std::string& name) const
    {
        auto attributes = m_attributes.find(ObjectInfo::compute_id(name));
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* BaseContext::attributes(uint64 object_id) const
    {
        auto attributes = m_attributes.find(object_id);
        if (attributes != m_attributes.end()) return &attributes->second;
        return nullptr;
    }
    const std::vector < Attribute >* BaseContext::attributes(const Object& object) const
    {
        return attributes(object.object_id());
    }
    const std::vector < Attribute >* BaseContext::attributes(const ObjectInfo& info) const
    {
        return attributes(info.id());
    }
    
    //Get resource
    Shared<ResourceObject> BaseContext::resource(const std::string& name)
    {
        //referenced while a resource of a folder loads: look in that folder first
        if (!m_resource_scopes.empty() && !m_resource_scopes.back().empty())
        {
            const size_t colon = name.find(':');
            if (colon != std::string::npos)
            {
                const std::string scoped = name.substr(0, colon + 1) + m_resource_scopes.back() + "/" + name.substr(colon + 1);
                if (m_resources.count(scoped) || m_resources_file.count(scoped))
                {
                    return load_resource(scoped);
                }
            }
        }
        return load_resource(name);
    }

    Shared<ResourceObject> BaseContext::load_resource(const std::string& name)
    {
        //olready loaded
        auto resource_it = m_resources.find(name);
        if (resource_it != m_resources.end()) return resource_it->second;
        //else find resource from file
        auto resource_file_it = m_resources_file.find(name);
        if (resource_file_it  == m_resources_file.end()) return nullptr;
        //copy: loading can register/load other resources (and invalidate the iterators)
        const ResourceFile resource_file = resource_file_it->second;
        //create resource
        auto resource = DynamicPointerCast<ResourceObject>(create(resource_file.m_resouce_id));
        if(!resource) return nullptr;
		//insert
		m_resources.insert({ name, resource });
		//set resource name (the key string lives as long as the map entry)
		resource->resource_name(m_resources.find(name)->first.c_str());
        //folder of "Class:folder/name": the resources it references are looked up there first
        const size_t colon = name.find(':');
        const size_t name_start = colon == std::string::npos ? 0 : colon + 1;
        const size_t slash = name.rfind('/');
        m_resource_scopes.push_back(slash != std::string::npos && slash > name_start
                                    ? name.substr(name_start, slash - name_start)
                                    : std::string());
        //load
        const bool loaded = resource->load(resource_file.m_filepath);
        m_resource_scopes.pop_back();
        if (loaded) return resource;
		//fail
		m_resources.erase(name);
		//wrong
		logger()->warning("Resource: unable to load " + name);
		//end
        return nullptr;
    }
    
    const std::string& BaseContext::resource_path(const std::string& name)
    {
        //find resource from file
        auto resource_file_it = m_resources_file.find(name);
		if (resource_file_it != m_resources_file.end()) return resource_file_it->second.m_filepath;
        //fail
		static const std::string s_void_name;
		return s_void_name;
        
    }
    //Get variable
    std::optional<const VariantRef> BaseContext::variable(const std::string& name) const
    {
        //get var
        auto variable = m_variables.find(name);
        if (variable != m_variables.end()) return &variable->second;
        //none
		return {};
    }
    
    //Object fectory
    void BaseContext::add_object(Shared<ObjectFactory> object_fectory)
    {
        m_object_factories[object_fectory->info().id()] = object_fectory;
    }
    void BaseContext::add_resource(Shared<ObjectFactory> object_fectory,const std::vector< std::string >& exts)
    {
        add_object(object_fectory);
        m_resources_info[object_fectory->info().id()] = exts;
    }
    
    //Resource
	bool BaseContext::add_resources(const std::string& file_of_resources)
	{
		//parser
		Parser::Resources			r_parser;
		Parser::Resources::Context  r_context;
		//do parsing
		if (!r_parser.parse(r_context, Filesystem::text_file_read_all(file_of_resources)))
		{
			logger()->warning("Faild to read resources file: " + file_of_resources);
			logger()->warning(r_context.errors_to_string());
			return false;
		}
		//Base path
		std::string directory = Filesystem::get_directory(file_of_resources);
		//add all paths
		for (const Parser::Resources::PathField& path : r_context.m_paths)
		{
			if (path.m_filtered) add_resource_path(Filesystem::join(directory, path.m_path), path.m_reg_exp, path.m_recursive);
			else				 add_resource_path(Filesystem::join(directory, path.m_path), path.m_recursive);
		}
		//add all files
		for (const Parser::Resources::FileField& files : r_context.m_files)
		{
			if (files.m_use_asset_name) add_resource_file(files.m_asset_name, Filesystem::join(directory, files.m_path));
			else				        add_resource_file(Filesystem::join(directory, files.m_path));
		}
		//end
		return true;
	}
    void BaseContext::add_resource_path(const std::string& path, bool recursive, const std::string& name_prefix)
    {
        //for all sub path: their files are named "sub/name"
		if (recursive)
		{
			for (const std::string& directoryname : Filesystem::get_sub_directories(path))
			{
				std::string subdirfullpath = Filesystem::join(path, directoryname);
				if (auto canonical_path = Filesystem::get_canonical(subdirfullpath); canonical_path.m_success)
				{
					subdirfullpath = canonical_path.m_path;
				}
				add_resource_path(subdirfullpath, recursive, name_prefix + directoryname + "/");
			}
		}
        //for all files
        for(const std::string& filename : Filesystem::get_files(path))
        {
			add_resource_file(name_prefix + Filesystem::get_basename(filename), Filesystem::join(path, filename));
        }
        //end
    }
	void BaseContext::add_resource_path(const std::string& path, const std::string& filter, bool recursive, const std::string& name_prefix)
    {
		//get all files
		Filesystem::FilesList files = Filesystem::get_files(path);
		//success?
		if (!files.m_success) return;
		//put into table
		try
		{
			std::regex reg_exp(filter, std::regex::ECMAScript);
			add_resource_path(path, reg_exp, recursive, name_prefix);
		}
		catch (std::regex_error& e)
		{
			logger()->warning("Faild to generate regexp \""+ filter +"\" of resources file: " + path);
			logger()->warning(e.what());
		}
        //end
    }
	void BaseContext::add_resource_path(const std::string& path, const std::regex& filter, bool recursive, const std::string& name_prefix)
    {
		//get all files
		Filesystem::FilesList files = Filesystem::get_files(path);
		//success?
		if (!files.m_success) return;
		//put into table
		for (const std::string& filename : files.m_fields)
		{
			if (std::regex_match(filename, filter))
			{
				//add
				add_resource_file(name_prefix + Filesystem::get_basename(filename), Filesystem::join(path, filename));
			}
		}
		//sub directories
		if (recursive)
		{
			//get all directories
			Filesystem::DirectoriesList directories = Filesystem::get_sub_directories(path);
			//success?
			if (!directories.m_success) return;
			//push dir into table
			for (const std::string& directoryname : directories.m_fields)
			{
				add_resource_path(Filesystem::join(path, directoryname), filter, true, name_prefix + directoryname + "/");
			}
		}
        //end
    }

	bool BaseContext::add_resource_file(const std::string& filepath)
	{
		return add_resource_file(Filesystem::get_basename(filepath), filepath);
	}
	bool BaseContext::add_resource_file(const std::string& resource_name, const std::string& filepath)
	{
		//get extension
		auto    f_ext = Filesystem::get_extension(filepath);
		//special case
		if (f_ext == ".rs" || f_ext == ".resources")
		{
			add_resources(filepath);
			return true;
		}
		//if is supported
		for (auto r_info : m_resources_info)
		for (auto r_ext : r_info.second)
		{
			if (r_ext == f_ext)
			{
				auto class_name = m_object_factories[r_info.first]->info().name();
				auto name = class_name + ":" + resource_name;
				//two files with the same name and type: only one can be loaded
				if (auto it = m_resources_file.find(name); it != m_resources_file.end() && it->second.m_filepath != filepath)
				{
					logger()->warning("Resource " + name + " found twice: " + it->second.m_filepath + " and " + filepath + " (the last one is used)");
				}
				m_resources_file[name] = ResourceFile(r_info.first, filepath);
				return true;
			}
		}
		return false;
	}	
	bool BaseContext::add_resource_map(uint64 object_factory_id,const std::unordered_map<std::string, std::string>& name_files)
	{
		for (const auto& name_file : name_files)
		{
			auto class_name = m_object_factories[object_factory_id]->info().name();
			auto name = class_name + ":" + name_file.first;
			m_resources_file[name] = ResourceFile(object_factory_id, name_file.second);
		}
		return false;
	}
    
    //Add an attrbute
    void BaseContext::add_attribute(const std::string& name, Attribute&& attribute)
    {
        m_attributes[ObjectInfo::compute_id(name)].push_back(std::forward<Attribute>(attribute));
    }
    void BaseContext::add_attribute(uint64 object_id, Attribute&& attribute)
    {
        m_attributes[object_id].push_back(std::forward<Attribute>(attribute));
    }
    void BaseContext::add_attribute(const Object& object, Attribute&& attribute)
    {
        m_attributes[object.object_id()].push_back(std::forward<Attribute>(attribute));
    }
    void BaseContext::add_attribute(const ObjectInfo& info, Attribute&& attribute)
    {
        m_attributes[info.id()].push_back(std::forward<Attribute>(attribute));
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

	//get application
	Application* BaseContext::application()
	{
		return m_application;
	}
	//get allocator
	Allocator* BaseContext::allocator()
	{
		return m_allocator;
	}
	//get logger
	Logger* BaseContext::logger()
	{
		return m_logger;
	}
	//get window
	Video::Window* BaseContext::window()
	{
		return application() ? application()->window() : nullptr;
	}
    //get application
    const Application* BaseContext::application() const
    {
        return m_application;
    }
	//get allocator
	Allocator* BaseContext::allocator() const
	{
		return m_allocator;
	}
	//get logger
	Logger* BaseContext::logger() const
	{
		return m_logger;
	}
    //get window
    const Video::Window* BaseContext::window() const
    {
        return application() ? application()->window() : nullptr;
    }

	//System class
	void BaseContext::add_system(Shared<ObjectFactory> object_fectory, SystemStartup startup, unsigned int ring)
	{
		add_object(object_fectory);
		m_systems_info[object_fectory->info().id()] = SystemInfo{ startup, ring };
	}
	const SystemInfo* BaseContext::system_info(const std::string& name) const
	{
		return system_info(ObjectInfo::compute_id(name));
	}
	const SystemInfo* BaseContext::system_info(uint64 id) const
	{
		auto info = m_systems_info.find(id);
		return info != m_systems_info.end() ? &info->second : nullptr;
	}

	//Systems
	System* BaseContext::start_system(const std::string& name)
	{
		return start_system(ObjectInfo::compute_id(name));
	}
	System* BaseContext::start_system(uint64 id)
	{
		//once
		if (auto* existing = system(id)) return existing;
		//a system class
		const SystemInfo* info = system_info(id);
		if (!info)
		{
			logger()->warning("Context: " + std::to_string(id) + " is not a registered system");
			return nullptr;
		}
		auto new_system = DynamicPointerCast<System>(create(id));
		if (!new_system)
		{
			logger()->warning("Context: unable to create the system " + std::to_string(id));
			return nullptr;
		}
		//running, by ring; already in the list while it initializes (what it creates can
		//look for it, e.g. the drawer for the render device)
		auto position = std::upper_bound(m_systems.begin(), m_systems.end(), new_system->system_ring(), [](unsigned int ring, const Shared<System>& running)
		{
			return ring < running->system_ring();
		});
		m_systems.insert(position, new_system);
		if (!new_system->initialize())
		{
			logger()->warning("Context: unable to start the system " + new_system->object_name());
			m_systems.erase(std::find(m_systems.begin(), m_systems.end(), new_system));
			return nullptr;
		}
		//the worlds already there
		for (Scene::World* alive : m_worlds) alive->add_instance(*new_system);
		//the application is already running: the second phase too
		if (m_systems_post_initialized) new_system->post_initialize();
		return new_system.get();
	}
	void BaseContext::start_systems()
	{
		//the AUTOMATIC ones, from the lower ring up (then by name, to always start them the same way)
		std::vector< std::pair<unsigned int, std::string> > automatic;
		for (const auto& info : m_systems_info)
		{
			if (info.second.m_startup != SystemStartup::AUTOMATIC) continue;
			auto factory = m_object_factories.find(info.first);
			if (factory == m_object_factories.end()) continue;
			automatic.emplace_back(info.second.m_ring, factory->second->info().name());
		}
		std::sort(automatic.begin(), automatic.end());
		for (const auto& system : automatic) start_system(system.second);
	}
	System* BaseContext::system(const std::string& name) const
	{
		return system(ObjectInfo::compute_id(name));
	}
	System* BaseContext::system(uint64 id) const
	{
		for (const Shared<System>& system : m_systems)
		{
			if (system->object_id() == id) return system.get();
		}
		return nullptr;
	}
	const SystemList& BaseContext::systems() const
	{
		return m_systems;
	}
	void BaseContext::shutdown_system(const Shared<System>& system)
	{
		//the application is still running: the first phase too
		if (m_systems_post_initialized) system->pre_shutdown();
		//the worlds first, then the system
		for (Scene::World* alive : m_worlds) alive->remove_instances(*system);
		system->shutdown();
	}
	bool BaseContext::stop_system(const std::string& name)
	{
		return stop_system(ObjectInfo::compute_id(name));
	}
	bool BaseContext::stop_system(uint64 id)
	{
		auto it = std::find_if(m_systems.begin(), m_systems.end(), [id](const Shared<System>& system) { return system->object_id() == id; });
		if (it == m_systems.end()) return false;
		//still in the list while it shuts down (what it releases can look for it,
		//e.g. the resources for the render device), then out
		auto running = *it;
		shutdown_system(running);
		m_systems.erase(std::find(m_systems.begin(), m_systems.end(), running));
		return true;
	}
	void BaseContext::stop_systems()
	{
		//the last started first
		while (!m_systems.empty())
		{
			auto running = m_systems.back();
			shutdown_system(running);
			m_systems.erase(std::find(m_systems.begin(), m_systems.end(), running));
		}
	}
	void BaseContext::post_initialize_systems()
	{
		if (m_systems_post_initialized) return;
		m_systems_post_initialized = true;
		//lower ring up (by index: a system can start/stop others)
		for (size_t i = 0; i < m_systems.size(); ++i)
		{
			Shared<System> running = m_systems[i];
			running->post_initialize();
		}
	}
	void BaseContext::pre_shutdown_systems()
	{
		if (!m_systems_post_initialized) return;
		m_systems_post_initialized = false;
		//upper ring down
		for (size_t i = m_systems.size(); i > 0; --i)
		{
			if (i > m_systems.size()) continue;
			Shared<System> running = m_systems[i - 1];
			running->pre_shutdown();
		}
	}
	//worlds
	const std::vector<Scene::World*>& BaseContext::worlds() const
	{
		return m_worlds;
	}
	void BaseContext::add_world(Scene::World* world)
	{
		m_worlds.push_back(world);
	}
	void BaseContext::remove_world(Scene::World* world)
	{
		m_worlds.erase(std::remove(m_worlds.begin(), m_worlds.end(), world), m_worlds.end());
	}
	void BaseContext::clear_resources()
	{
		m_resources.clear();
		m_resource_scopes.clear();
	}
	void BaseContext::update_systems(double delta_time)
	{
		//lower ring up (by index: a system can start/stop others)
		for (size_t i = 0; i < m_systems.size(); ++i)
		{
			Shared<System> running = m_systems[i];
			running->update(delta_time);
		}
	}
	void BaseContext::late_update_systems(double delta_time)
	{
		//upper ring down: the render device is the last one
		for (size_t i = m_systems.size(); i > 0; --i)
		{
			if (i > m_systems.size()) continue;
			Shared<System> running = m_systems[i - 1];
			running->late_update(delta_time);
		}
	}

	void BaseContext::clear()
	{
		//systems
		stop_systems();
		m_systems_info.clear();
        //
		m_variables.clear();
		m_attributes.clear();
		m_object_factories.clear();
        //
		m_resources.clear();
        m_resources_file.clear();
        m_resources_info.clear();
	}
}
