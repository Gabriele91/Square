//
//  SceneSystem.h
//  Square
//
//  The scene: owns the main World (Application::world()). Ring SERVICE: it starts after the devices,
//  so the world gets their instances when it is created, and it shuts down before them, so
//  the world is gone before the render device.
//
#pragma once
#include "Square/Config.h"
#include "Square/System/System.h"

namespace Square
{
	//..................
	//declaretion
	namespace Scene
	{
		class World;
	}
	//..................
	class SQUARE_API SceneSystem : public System
	{
	public:
		//A square system, and its ring
		SQUARE_SYSTEM(SceneSystem, SystemRing::SERVICE + 1)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		SceneSystem(Context& context);
		virtual ~SceneSystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		
		//the components of the actors of every world (Component::on_update / on_late_update)
		virtual void update(double delta_time) override;
		virtual void late_update(double delta_time) override;

		//the world
		Shared<Scene::World> world() const;

	protected:
		Shared<Scene::World> m_world;
	};
}
