//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/Allocator.h"
#include "Square/Core/Allocator/FrameAllocator.h"
#include "Square/Core/Object.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Resource.h"
#include "Square/Core/Attribute.h"
#include "Square/Core/Logger.h"
#include "Square/System/System.h"
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
	//Systems
	class System;
	using SystemList = Vector< Shared<System> >;
	//Context without template (dll)
	class SQUARE_API BaseContext
	{
	protected:

		//type
		using VariantMap = UnorderedMap< String, Variant >;
		using StringMap = UnorderedMap< String, String >;
		using StringList = Vector< String >;
		//Attributes
		using AttributeMap = UnorderedMap< uint64, Vector < Attribute > >;
		//Factory
		using ObjectFactoryMap = UnorderedMap< uint64, Shared<ObjectFactory> >;
		
	public:

		//Create
        Shared<Object> create(const String& name);
        Shared<Object> create(uint64 id);
        
		//Get attrbutes
        const Vector < Attribute >* attributes(const String& name) const;
        const Vector < Attribute >* attributes(uint64 object_id) const;
        const Vector < Attribute >* attributes(const Object& object) const;
        const Vector < Attribute >* attributes(const ObjectInfo& info) const;

		//Get resource, name = "Class:path". While a resource "Class:folder/x" loads, the names it
		//references are looked up first inside "folder/" and then as they are (global).
        Shared<ResourceObject> resource(const String& name);
		//a new object of a resource every call (not kept: e.g. an actor placed in a scene), loaded
		//as resource() loads it (same lookup of the names it references)
		Shared<ResourceObject> resource_instance(const String& name);
		// Get resource path 
        const String& resource_path(const String& name);
		//the names of the resources of a type registered ("Effect": "PBR", "sanctuary/fx_glow"...;
		//loaded or not), sorted
		Vector<String> resource_names(const String& type) const;

		//Get variable
        std::optional<const VariantRef> variable(const String& name) const;
        
		//Object fectory
        void add_object(Shared<ObjectFactory> object_fectory);
        void add_resource(Shared<ObjectFactory> object_fectory,const Vector< String >& exts);
        
		//The folder of the engine (its common/: its effects, its shaders, the UI of the debug panel),
		//relative to the resource folder or absolute; empty: the resource folder itself (the engine
		//is the project). A game with the engine in a folder of its own sets it ("engine")
		void engine_dir(const String& dir) { m_engine_dir = dir; }
		const String& engine_dir() const { return m_engine_dir; }
		//a path of the engine ("common/ui/debug.rml"): in its folder
		String engine_path(const String& path) const;

		//Resource
		bool add_resources(const String& file_of_resources);
		//name_prefix is put before the file names: a recursive path gives every sub folder
		//its own prefix ("sub/"), so files are named by their path relative to the root
		void add_resource_path(const String& path, bool recursive = false, const String& name_prefix = "");
		void add_resource_path(const String& path, const String& filter, bool recursive = true, const String& name_prefix = "");
		void add_resource_path(const String& path, const std::regex& filter, bool recursive = true, const String& name_prefix = "");
		bool add_resource_file(const String& filepath);
		bool add_resource_file(const String& name, const String& path);
		template< typename T >
		bool add_resource_map(const UnorderedMap<String, String>& name_files)
		{
			return add_resource_map(T::static_object_id(), name_files);
		}

		//Add an attrbute
        void add_attribute(const String& name, Attribute&& attribute);
        void add_attribute(uint64 object_id, Attribute&& attribute);
        void add_attribute(const Object& object, Attribute&& attribute);
        void add_attribute(const ObjectInfo& info, Attribute&& attribute);

		//Add variable
        void add_variable(const String& name, const Variant& value);

		//System class: in the object factory (like add_resource), with when it starts (start-up
		//or on demand) and its ring; called by the object_registration of a system
		void add_system(Shared<ObjectFactory> object_fectory, SystemStartup startup, unsigned int ring);
		const SystemInfo* system_info(const String& name) const;
		const SystemInfo* system_info(uint64 id) const;

		//Systems running: created by the object factory, initialized and given to the worlds;
		//a system is started once
		System* start_system(const String& name);
		System* start_system(uint64 id);
		//start the AUTOMATIC systems, from the lower ring up (the Application calls it at start-up)
		void start_systems();
		//a running system, nullptr if it is not running
		System* system(const String& name) const;
		System* system(uint64 id) const;
		//running, from the lower ring up
		const SystemList& systems() const;
		//shut down a system (its world instances first)
		bool stop_system(const String& name);
		bool stop_system(uint64 id);
		//shut down all the systems, from the upper ring down
		void stop_systems();
		//the phases around the application: post_initialize after AppInterface::start, from the
		//lower ring up; pre_shutdown before AppInterface::end, from the upper ring down (the
		//Application calls them). Between the two, a system started/stopped gets them at once
		void post_initialize_systems();
		void pre_shutdown_systems();
		//a frame of the systems: before AppInterface::run from the lower ring up, after it
		//from the upper ring down (the Application calls them)
		void update_systems(double delta_time);
		void late_update_systems(double delta_time);

		//unload all the loaded resources (their files stay registered)
		void clear_resources();

		//the worlds alive (a world adds/removes itself), they get the instances of the systems
		const Vector<Scene::World*>& worlds() const;

		//get application
		Application* application();
		//get allocator
		Allocator* allocator();
		//get the allocator of the frames (its blocks live until the end of the next frame)
		FrameAllocator* frame_allocator();
		//get logger
		Logger* logger();
        //get window
        Video::Window* window();
        //get application
        const Application* application() const;
		//get allocator
		Allocator* allocator() const;
		//get the allocator of the frames (its blocks live until the end of the next frame)
		FrameAllocator* frame_allocator() const;
		//get logger
		Logger* logger() const;
        //get window
        const Video::Window* window() const;

#if defined(SQUARE_DEBUG_TOOLS)
		//the time of the loads of the resources (only with the debug tools), by the extension of
		//their files (".mat", ".sqfx", ".dds"...): how many, their own time (milliseconds, the
		//resources they load inside out), the slowest one; cleared when asked (e.g. before a
		//level is loaded)
		struct ResourceLoadStat
		{
			size_t      m_count{ 0 };
			double      m_ms{ 0.0 };
			String m_slowest;
			double      m_slowest_ms{ 0.0 };
		};
		using ResourceLoadStats = UnorderedMap<String, ResourceLoadStat>;
		const ResourceLoadStats& resource_load_stats() const { return m_load_stats; }
		//details inside the loads (e.g. "shader: translate", "shader: driver"): their time, apart
		//(already in the own time of the resource that has them)
		void add_load_detail(const String& kind, const String& name, double ms);
		const ResourceLoadStats& resource_load_details() const { return m_load_details; }
		void clear_resource_load_stats() { m_load_stats.clear(); m_load_details.clear(); }
#endif

	protected:
		//Can't alloc a BaseContext
		BaseContext();
        //Resource
        struct SQUARE_API ResourceFile
        {
            uint64      m_resouce_id;
            String m_filepath;
            //...
			ResourceFile();
			ResourceFile(const ResourceFile& in);
			ResourceFile(uint64 id, const String filepath);
        };
        using ResourceFileMap   = UnorderedMap< String, ResourceFile >;
        using ResourceInfoMap   = UnorderedMap< uint64, Vector<String> >;
        using ResourceObjectMap = UnorderedMap< String, Shared<ResourceObject> >;
		//context
		VariantMap       m_variables;
		AttributeMap     m_attributes;
        //Object factory
		ObjectFactoryMap m_object_factories;
		//Context refs
		Application*     m_application{ nullptr };
		//Allocator refs
		Allocator*		 m_allocator;
		FrameAllocator*  m_frame_allocator{ nullptr };
		//Logger refs
		Logger*			 m_logger;
        //Reousce factory
        ResourceFileMap   m_resources_file;
        ResourceInfoMap   m_resources_info;
		ResourceObjectMap m_resources;
		//folders ("arena", "a/b") of the resources being loaded, innermost last
		Vector<String> m_resource_scopes;
		//the folder of the engine (empty: the resource folder)
		String m_engine_dir;
#if defined(SQUARE_DEBUG_TOOLS)
		//the times of the loads; the time of the loads inside each one open (out of its own)
		ResourceLoadStats   m_load_stats;
		ResourceLoadStats   m_load_details;
		Vector<double> m_load_inner_ms;
#endif
		//a resource loaded from its file, in the scope of its folder (with the debug tools: timed)
		bool load_in_scope(ResourceObject& resource, const String& name, const String& path);
		//worlds alive
		Vector<Scene::World*> m_worlds;
		void add_world(Scene::World* world);
		void remove_world(Scene::World* world);
		friend class Scene::World;
		//systems
		SystemInfoMap    m_systems_info;
		SystemList       m_systems;
		//shut down a running system
		void shutdown_system(const Shared<System>& system);
		//between post_initialize_systems and pre_shutdown_systems
		bool m_systems_post_initialized{ false };
		//find/load a resource by its full name, no scope lookup
		Shared<ResourceObject> load_resource(const String& name);
		//the full name of a resource, looked up first in the folder of the resource loading
		String scoped_resource_name(const String& name) const;
		//create and load the object of a resource file (its folder is the scope while it loads)
		Shared<ResourceObject> load_resource_file(const String& name);
		//friend class
		friend class Application;
		//delete all
		void clear();
		//help function
		bool add_resource_map(uint64 object_factory_id, const UnorderedMap<String, String>& name_files);
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
		using BaseContext::resource_instance;
		using BaseContext::resource_path;
		using BaseContext::variable;
		
		using BaseContext::add_object;
		using BaseContext::add_attribute;
		using BaseContext::add_resource;
		using BaseContext::add_resources;
		using BaseContext::add_resource_path;
		using BaseContext::add_resource_file;
		using BaseContext::add_variable;
		using BaseContext::add_system;
		using BaseContext::system_info;
		using BaseContext::start_system;
		using BaseContext::start_systems;
		using BaseContext::system;
		using BaseContext::systems;
		using BaseContext::stop_system;
		using BaseContext::stop_systems;
		using BaseContext::post_initialize_systems;
		using BaseContext::pre_shutdown_systems;
		using BaseContext::worlds;
		using BaseContext::update_systems;
		using BaseContext::late_update_systems;

		using BaseContext::application;
		using BaseContext::window;
		using BaseContext::allocator;

		//template utils
		template< class T > inline Shared<T> create()
		{
			return DynamicPointerCast<T>(BaseContext::create(T::static_object_id()));
		}

		template< class T > inline const Vector < Attribute >* attributes()
		{
			return attributes(T::static_object_id());
		}

		template< class T > inline void add_object()
		{
			BaseContext::add_object(MakeShared< ObjectFactoryItem<T> >(*this));
		}

		template< class T > inline void add_system(SystemStartup startup = SystemStartup::ON_DEMAND)
		{
			BaseContext::add_system(MakeShared< ObjectFactoryItem<T> >(*this), startup, T::static_system_ring());
		}

		template< class T > inline const SystemInfo* system_info() const
		{
			return BaseContext::system_info(T::static_object_id());
		}

		template< class T > inline T* start_system()
		{
			return static_cast<T*>(BaseContext::start_system(T::static_object_id()));
		}

		template< class T > inline bool stop_system()
		{
			return BaseContext::stop_system(T::static_object_id());
		}

		template< class T > inline T* system() const
		{
			return static_cast<T*>(BaseContext::system(T::static_object_id()));
		}

		template< class T > inline void add_resource(const Vector< String >& exts)
		{
			BaseContext::add_resource(MakeShared< ObjectFactoryItem<T> >(*this), exts);
		}

		template < class T > inline void add_attribute(Attribute& attribute)
		{
			BaseContext::add_attribute(T::static_object_id(), std::forward<Attribute>(attribute));
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
		template< class T >  inline Shared<T> resource(const String& name)
		{
			return DynamicPointerCast<T>(BaseContext::resource(T::static_object_name() + ":" + name));
		}

		template< class T >  inline Shared<T> resource_instance(const String& name)
		{
			return DynamicPointerCast<T>(BaseContext::resource_instance(T::static_object_name() + ":" + name));
		}

		template< class T >  inline Shared<T> reinterpret_resource(const String& name)
		{
			return DynamicPointerCast<T>(BaseContext::resource(name));
		}

		template< class T >  inline const String& resource_path(const String& name)
		{
			return BaseContext::resource_path(T::static_object_name() + ":" + name);
		}

		template< class T >  inline Vector<String> resource_names() const
		{
			return BaseContext::resource_names(T::static_object_name());
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

	//System::get, here: it needs the Context
	template< class T >
	inline T* System::get(const Context& context)
	{
		return context.system<T>();
	}
}
