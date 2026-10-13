//
//  SceneSystem.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/World.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/System/SceneSystem.h"

namespace Square
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(SceneSystem);

	//Registration in context
	void SceneSystem::object_registration(Context& ctx)
	{
		//system: at start-up (ring SERVICE: after the devices, shut down before them)
		ctx.add_system<SceneSystem>(SystemStartup::AUTOMATIC);
	}

	SceneSystem::SceneSystem(Context& context) : System(context)
	{
	}

	SceneSystem::~SceneSystem()
	{
	}

	bool SceneSystem::initialize()
	{
		//the world, with the instances of the systems already running
		m_world = MakeShared<Scene::World>(context());
		return true;
	}

	void SceneSystem::shutdown()
	{
		m_world.reset();
	}

	void SceneSystem::update(double delta_time)
	{
		//by index: a component can create/destroy worlds
		const auto& worlds = context().worlds();
		for (size_t i = 0; i < worlds.size(); ++i) worlds[i]->update(delta_time);
	}

	void SceneSystem::late_update(double delta_time)
	{
		const auto& worlds = context().worlds();
		for (size_t i = 0; i < worlds.size(); ++i) worlds[i]->late_update(delta_time);
	}

	Shared<Scene::World> SceneSystem::world() const
	{
		return m_world;
	}
}
