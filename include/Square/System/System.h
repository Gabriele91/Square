//
//  System.h
//  Square
//
//  A System is a global part of the engine (render, input, physics...), one per Context:
//  - it declares its ring (SQUARE_SYSTEM), like the rings of an OS: the lower rings are the
//    base the upper ones use (the render device is CORE, the game systems GAME); in the
//    same ring by name (always the same order);
//  - it is an Object like the others: SQUARE_CLASS_OBJECT_REGISTRATION, and in its
//    object_registration ctx.add_system<T>(startup), like ctx.add_resource<T>(exts);
//  - AUTOMATIC systems are started by the Application (Context::start_systems), ON_DEMAND
//    ones when the game asks for them (Context::start_system<T>());
//  - every frame: update() before AppInterface::run, from the lower ring up (input first),
//    late_update() after it, from the upper ring down (render last);
//  - it starts from the lower ring up and shuts down from the upper ring down;
//  - life, around the application:
//      initialize()       before AppInterface::start (devices)
//      post_initialize()  after AppInterface::start (what needs the resources it added)
//      pre_shutdown()     before AppInterface::end (releases what post_initialize created)
//      shutdown()         after AppInterface::end (devices)
//    a system started/stopped while the application runs gets all its phases at once;
//  - it can give every World its own state, a SystemInstance (create_instance).
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include <unordered_map>

namespace Square
{
	//..................
	//declaretion
	namespace Scene
	{
		class World;
	}
	class System;
	class SystemInstance;
	//..................
	//ring of a system (unsigned int): the lower rings are the base of the upper ones; a system
	//can use any value of the range of its ring (e.g. SystemRing::SERVICE + 10)
	namespace SystemRing
	{
		enum : unsigned int
		{
			//0 - 99 reserved
			CORE    = 100,  //100 - 199: the devices the others use (render device)
			DRIVER  = 200,  //200 - 299: the other devices (input, audio)
			SERVICE = 300,  //300 - 999: engine services (physics, network, scene)
			GAME    = 1000  //1000 - ...: systems of the game
		};
	}

	//when a system is started
	enum class SystemStartup : unsigned char
	{
		AUTOMATIC, //at start-up, before AppInterface::start
		ON_DEMAND  //when the game starts it (Context::start_system)
	};

	//a system class in the context
	struct SQUARE_API SystemInfo
	{
		SystemStartup m_startup{ SystemStartup::ON_DEMAND };
		unsigned int  m_ring{ SystemRing::GAME };
	};
	using SystemInfoMap = std::unordered_map< uint64, SystemInfo >;

	//declare a system class: object info and ring
	#define SQUARE_SYSTEM(Class, Ring)\
		SQUARE_OBJECT(Class)\
		static constexpr unsigned int static_system_ring()\
			{ return Ring; }\
		unsigned int system_ring() const override\
			{ return Class::static_system_ring(); }
	//..................
	class SQUARE_API System : public Object
	                        , public SharedObject<System>
	                        , public Uncopyable
	{
	public:
		//A square object
		SQUARE_OBJECT(System)

		//Init
		System(Context& context);
		virtual ~System();

		//a running system of a context, nullptr if it is not running
		//(e.g. System::get<RenderSystem>(context())->render())
		template< class T > static T* get(const Context& context);

		//its ring (SQUARE_SYSTEM)
		virtual unsigned int system_ring() const = 0;

		//life: before AppInterface::start / after AppInterface::end
		virtual bool initialize() = 0;
		virtual void shutdown() = 0;
		//life: after AppInterface::start / before AppInterface::end
		virtual void post_initialize() {}
		virtual void pre_shutdown() {}

		//every frame: before AppInterface::run (lower rings first), after it (upper rings first)
		virtual void update(double delta_time) {}
		virtual void late_update(double delta_time) {}

		//its state in a world, nullptr if it has none
		virtual Shared<SystemInstance> create_instance(Scene::World& world);
	};

	//the state of a System inside a World (e.g. the render settings of a world)
	class SQUARE_API SystemInstance : public Object
	                                , public SharedObject<SystemInstance>
	                                , public Uncopyable
	{
	public:
		//A square object
		SQUARE_OBJECT(SystemInstance)

		//Init
		SystemInstance(Context& context, System& system, Scene::World& world);
		virtual ~SystemInstance();

		//owners
		System& system();
		const System& system() const;
		Scene::World& world();
		const Scene::World& world() const;

	protected:
		System&       m_system;
		Scene::World& m_world;
	};
}
