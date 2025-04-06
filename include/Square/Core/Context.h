//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator.h"
#include "Square/Core/Object.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Resource.h"
#include "Square/Core/Attribute.h"
#include "Square/Core/Logger.h"
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
        const std::vector < Attribute >* attributes(const std::string& name) const;
        const std::vector < Attribute >* attributes(uint64 object_id) const;
        const std::vector < Attribute >* attributes(const Object& object) const;
        const std::vector < Attribute >* attributes(const ObjectInfo& info) const;

		//Get resource
        Shared<ResourceObject> resource(const std::string& name);
        const std::string& resource_path(const std::string& name);

		//Get variable
        std::optional<const VariantRef> variable(const std::string& name) const;
        
		//Object fectory
        void add_object(Shared<ObjectFactory> object_fectory);
        void add_resource(Shared<ObjectFactory> object_fectory,const std::vector< std::string >& exts);
        
		//Resource
		bool add_resources(const std::string& file_of_resources);
		void add_resource_path(const std::string& path, bool recursive = false);
		void add_resource_path(const std::string& path, const std::string& filter, bool recursive = true);
		void add_resource_path(const std::string& path, const std::regex& filter, bool recursive = true);
		bool add_resource_file(const std::string& filepath);
		bool add_resource_file(const std::string& name, const std::string& path);
		template< typename T >
		bool add_resource_map(const std::unordered_map<std::string, std::string>& name_files)
		{
			return add_resource_map(T::static_object_id(), name_files);
		}

		//Add an attrbute
        void add_attribute(const std::string& name, Attribute&& attribute);
        void add_attribute(uint64 object_id, Attribute&& attribute);
        void add_attribute(const Object& object, Attribute&& attribute);
        void add_attribute(const ObjectInfo& info, Attribute&& attribute);

		//Add variable
        void add_variable(const std::string& name, const Variant& value);
		
		//get application
		Application* application();
		//get allocator
		Allocator* allocator();
        //get render
        Render::Context* render();
		//get logger
		Logger* logger();
        //get window
        Video::Window* window();
        //get window
        Video::Input* input();
		//get world
		Scene::World* world();
        //get application
        const Application* application() const;
		//get allocator
		Allocator* allocator() const;
		//get logger
		Logger* logger() const;
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
        //Object factory
		ObjectFactoryMap m_object_factories;
		//Context refs
		Application*     m_application{ nullptr };
		//Allocator refs
		Allocator*		 m_allocator;
		//Logger refs
		Logger*			 m_logger;
        //Reousce factory
        ResourceFileMap   m_resources_file;
        ResourceInfoMap   m_resources_info;
		ResourceObjectMap m_resources;
		//friend class
		friend class Application;
		//delete all
		void clear();
		//help function
		bool add_resource_map(uint64 object_factory_id, const std::unordered_map<std::string, std::string>& name_files);
	};

	// Specialization
	template< class T, class U, class... Args >
	static inline std::enable_if_t<std::is_base_of<U, BaseContext>::value, Shared<T> >
	MakeShared(const U& base_context, Args&&... args) {
		return std::move(Shared<T>(SQ_NEW(base_context.allocator(), T, AllocType::ALCT_DEFAULT) T(std::forward<Args>(args)...),
			                       DefaultDelete(base_context.allocator())));
	}

	template< class T, class U, class... Args >
	static inline std::enable_if_t<std::is_base_of<U, BaseContext>::value, Unique<T> >
	MakeUnique(const U& base_context, Args&&... args) {
		return std::move(Unique<T>(SQ_NEW(base_context.allocator(), T, AllocType::ALCT_DEFAULT) T(std::forward<Args>(args)...),
			                       DefaultDelete(base_context.allocator())));
	}
	
	//Template help context
	class SQUARE_API Context : public BaseContext
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
		using BaseContext::add_attribute;
		using BaseContext::add_resource;
		using BaseContext::add_resources;
		using BaseContext::add_resource_path;
		using BaseContext::add_resource_file;
		using BaseContext::add_variable;

		using BaseContext::application;
		using BaseContext::render;
		using BaseContext::window;
		using BaseContext::input;
		using BaseContext::world;
		using BaseContext::allocator;

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
			BaseContext::add_object(MakeShared< ObjectFactoryItem<T> >(*this));
		}

		template< class T > inline void add_resource(const std::vector< std::string >& exts)
		{
			BaseContext::add_resource(MakeShared< ObjectFactoryItem<T> >(*this), exts);
		}

		template < class T > inline void add_attribute(Attribute& attribute)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute));
		}

		template < class T > inline void add_attribute(Attribute&& attribute)
		{
			BaseContext::add_attribute(T::static_object_id(), std::forward<Attribute>(attribute));
		}
		///
#if 0
		template < class T, class U, typename... Args > inline void add_attribute_field(Args&&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_field<T,U>(this->allocator(), std::forward(args)...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_method(Args&&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_method<T, U>(this->allocator(), std::forward(args)...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_function(Args&&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_function<T, U>(this->allocator(), std::forward(args)...)));
		}
#endif		
		///
#if 1
		template < class T, class U, typename... Args > inline void add_attribute_field(const Args&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_field<T, U>(this->allocator(), args...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_method(const Args&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_method<T, U>(this->allocator(), args...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_function(const Args&... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_function<T, U>(this->allocator(), args...)));
		}
#endif
		///
#if 0
		template < class T, class U, typename... Args > inline void add_attribute_field(Args... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_field<T, U>(this->allocator(), args...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_method(Args... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_method<T, U>(this->allocator(), args...)));
		}

		template < class T, class U, typename... Args > inline void add_attribute_function(Args... args)
		{
			BaseContext::add_attribute(T::static_object_id(), std::move<Attribute>(attribute_function<T, U>(this->allocator(), args...)));
		}
#endif
		///
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
	// Specialization
	template< class T, class... Args >
	static inline Shared<T>
	MakeShared(Context& context, Args&&... args) {
		return std::move(Shared<T>(SQ_NEW(context.allocator(), T, AllocType::ALCT_DEFAULT) T(context, std::forward<Args>(args)...),
			                       DefaultDelete(context.allocator())));
	}

	template< class T, class U, class... Args >
	static inline Unique<T>
	MakeUnique(Context& context, Args&&... args) {
		return std::move(Unique<T>(SQ_NEW(context.allocator(), T, AllocType::ALCT_DEFAULT) T(context, std::forward<Args>(args)...),
			                       DefaultDelete(context.allocator())));
	}

	// Specialization
	template< class T, class... Args >
	static inline Shared<T>
	MakeShared(const Context& context, Args&&... args) {
		return std::move(Shared<T>(SQ_NEW(context.allocator(), T, AllocType::ALCT_DEFAULT) T(context, std::forward<Args>(args)...),
			                       DefaultDelete(context.allocator())));
	}

	template< class T, class U, class... Args >
	static inline Unique<T>
	MakeUnique(const Context& context, Args&&... args) {
		return std::move(Unique<T>(SQ_NEW(context.allocator(), T, AllocType::ALCT_DEFAULT) T(context, std::forward<Args>(args)...),
			                       DefaultDelete(context.allocator())));
	}
}
