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
#include <ostream>
#include <regex>

namespace Square
{
	//Application
	class Application;
	//Render context
	namespace Render
	{
		class Context;
	}
	//Window and input
	namespace Video
	{
		class Window;
		class Input;
	}
	//World
	namespace Scene
	{
		class World;
	}
	//Context without template (dll)
	class SQUARE_API BaseContext
	{
	protected:

		//type
		using VariantMap = std::unordered_map< std::string, Variant >;
		using StringMap = std::unordered_map< std::string, std::string >;
		using StringList = std::vector< std::string >;
		//Attributes
		using AttributeMap = std::unordered_map< uint64, std::vector < Attribute > >;
		//Factory
		using ObjectFactoryMap = std::unordered_map< uint64, Shared<ObjectFactory> >;
		
	public:

		//Create
        Shared<Object> create(const std::string& name);
        Shared<Object> create(uint64 id);
        
		//Get attrbutes
        const std::vector < Attribute >* attributes(const std::string& name);
        const std::vector < Attribute >* attributes(uint64 object_id);
        const std::vector < Attribute >* attributes(const Object& object);
        const std::vector < Attribute >* attributes(const ObjectInfo& info);

		//Get resource
        Shared<ResourceObject> resource(const std::string& name);
        const std::string& resource_path(const std::string& name);

		//Get variable
        const Variant& variable(const std::string& name);
        
		//Object fectory
        void add_object(ObjectFactory* object_fectory);
        void add_resource(ObjectFactory* object_fectory,const std::vector< std::string >& exts);
        
		//Resource
		bool add_resources(const std::string& file_of_resources);
		void add_resource_path(const std::string& path, bool recursive = false);
		void add_resource_path(const std::string& path, const std::string& filter, bool recursive = true);
		void add_resource_path(const std::string& path, const std::regex& filter, bool recursive = true);
		bool add_resource_file(const std::string& filepath);
		bool add_resource_file(const std::string& name, const std::string& path);

		//Add an attrbute
        void add_attributes(const std::string& name, Attribute&& attribute);
        void add_attributes(uint64 object_id, Attribute&& attribute);
        void add_attributes(const Object& object, Attribute&& attribute);
        void add_attributes(const ObjectInfo& info, Attribute&& attribute);

		//Add variable
        void add_variable(const std::string& name, const Variant& value);
		
		//context errors/wrongs
		void add_wrong(const std::string& wrong);

		void add_wrongs(const StringList& wrongs);

		const StringList& wrongs() const;

		void show_wrongs() const;

		void show_wrongs(std::ostream& output) const;

		//get application
		Application*  application();
        //get render
        Render::Context* render();
        //get window
        Video::Window* window();
        //get window
        Video::Input* input();
		//get world
		Scene::World* world();
        //get application
        const Application* application() const;
        //get render
        const Render::Context* render() const;
        //get window
        const Video::Window* window() const;
        //get window
        const Video::Input* input() const;
		//get world
		const Scene::World* world() const;

	protected:
		//Can't alloc a BaseContext
		BaseContext();
        //Resource
        struct SQUARE_API ResourceFile
        {
            uint64      m_resouce_id;
            std::string m_filepath;
            //...
			ResourceFile();
			ResourceFile(const ResourceFile& in);
			ResourceFile(uint64 id, const std::string filepath);
        };
        using ResourceFileMap   = std::unordered_map< std::string, ResourceFile >;
        using ResourceInfoMap   = std::unordered_map< uint64, std::vector<std::string> >;
        using ResourceObjectMap = std::unordered_map< std::string, Shared<ResourceObject> >;
		//context
		VariantMap       m_variables;
		AttributeMap     m_attributes;
		//pool of errors
		StringList		 m_wrongs;
        //Object factory
		ObjectFactoryMap m_object_factories;
		//Context refs
		Application*     m_application{ nullptr };
        //Reousce factory
        ResourceFileMap   m_resources_file;
        ResourceInfoMap   m_resources_info;
		ResourceObjectMap m_resources;
		//friend class
		friend class Application;
		//delete all
		void clear();
	};
	
	//Template help context
	class Context : public BaseContext
	{
	protected:

		friend class Application;

	public:
		//using
		using BaseContext::create;
		using BaseContext::attributes;
		using BaseContext::resource;
		using BaseContext::resource_path;
		using BaseContext::variable;
		
		using BaseContext::add_object;
		using BaseContext::add_attributes;
		using BaseContext::add_resource;
		using BaseContext::add_resources;
		using BaseContext::add_resource_path;
		using BaseContext::add_resource_file;
		using BaseContext::add_variable;

		using BaseContext::add_wrong;
		using BaseContext::add_wrongs;
		using BaseContext::wrongs;
		using BaseContext::show_wrongs;

		using BaseContext::application;
		using BaseContext::render;
		using BaseContext::window;
		using BaseContext::input;
		using BaseContext::world;

		//template utils
		template< class T > inline Shared<T> create()
		{
			return DynamicPointerCast<T>(BaseContext::create(T::static_object_id()));
		}

		template< class T > inline const std::vector < Attribute >* attributes()
		{
			return attributes(T::static_object_id());
		}

		template< class T > inline void add_object()
		{
			BaseContext::add_object(new ObjectFactoryItem<T>(*this));
		}

		template< class T > inline void add_resource(const std::vector< std::string >& exts)
		{
			BaseContext::add_resource(new ObjectFactoryItem<T>(*this), exts);
		}

		template < class T > inline void add_attributes(Attribute& attribute)
		{
			BaseContext::add_attributes(T::static_object_id(), std::move<Attribute>(attribute));
		}

		template < class T > inline void add_attributes(Attribute&& attribute)
		{
			BaseContext::add_attributes(T::static_object_id(), std::forward<Attribute>(attribute));
		}

		template< class T >  inline Shared<T> resource(const std::string& name)
		{
			return DynamicPointerCast<T>(BaseContext::resource(T::static_object_name() + ":" + name));
		}

		template< class T >  inline Shared<T> reinterpret_resource(const std::string& name)
		{
			return DynamicPointerCast<T>(BaseContext::resource(name));
		}

		template< class T >  inline const std::string& resource_path(const std::string& name)
		{
			return BaseContext::resource_path(T::static_object_name() + ":" + name);
		}
	};
}
