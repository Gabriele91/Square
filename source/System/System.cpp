//
//  System.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/System/System.h"

namespace Square
{
	//System
	System::System(Context& context) : Object(context), SharedObject_t(context.allocator())
	{
	}

	System::~System()
	{
	}

	Shared<SystemInstance> System::create_instance(Scene::World& world)
	{
		return nullptr;
	}

	//SystemInstance
	SystemInstance::SystemInstance(Context& context, System& system, Scene::World& world)
	: Object(context)
	, SharedObject_t(context.allocator())
	, m_system(system)
	, m_world(world)
	{
	}

	SystemInstance::~SystemInstance()
	{
	}

	System& SystemInstance::system()
	{
		return m_system;
	}

	const System& SystemInstance::system() const
	{
		return m_system;
	}

	Scene::World& SystemInstance::world()
	{
		return m_world;
	}

	const Scene::World& SystemInstance::world() const
	{
		return m_world;
	}
}
