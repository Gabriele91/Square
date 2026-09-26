//
//  RenderSystem.h
//  Square
//
//  The render device (ring 0), and the drawing of every world at the end of the frame
//  (late_update, the last one), then the present (window swap):
//  - RenderSystem, global: the render device (System::get<RenderSystem>(context)->render()), the drawer and its
//    passes (pipeline, shadows, debug);
//  - RenderInstance, per world: its clear and ambient colors, the renderables of its levels.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/System/System.h"
#include "Square/Render/Collection.h"

namespace Square
{
	//..................
	//declaretion
	namespace Render
	{
		class Context;
		class Drawer;
		class DrawerPassDebug;
		class RenderInspector;
	}
	class RenderInstance;
	//..................
	class SQUARE_API RenderSystem : public System
	{
	public:
		//A square system, and its ring
		SQUARE_SYSTEM(RenderSystem, SystemRing::SERVICE)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		RenderSystem(Context& context);
		virtual ~RenderSystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		virtual void post_initialize() override;
		virtual void pre_shutdown() override;
		virtual void late_update(double delta_time) override;
		virtual Shared<SystemInstance> create_instance(Scene::World& world) override;

		//the render device
		Render::Context* render() const;

		//pipeline: "deferred" (default) or "forward"; the environment variable
		//SQUARE_RENDERING overrides the default at start
		const std::string& pipeline() const;
		void pipeline(const std::string& pipeline);

		//shadow pass
		bool shadows() const;
		void shadows(bool enable);

		//the drawer, and its debug pass (OBB, light volumes, textures; all off at start): from
		//post_initialize (its passes load the effects added by AppInterface::start) to
		//pre_shutdown, nullptr out of it
		Shared<Render::Drawer> drawer() const;
		Shared<Render::DrawerPassDebug> debug() const;

		//draw all the worlds, and show the frame (the loop calls them)
		void draw();
		void present();

	protected:
		//device
		Render::Context*         m_render{ nullptr };
		Render::RenderInspector* m_inspector{ nullptr };
		//settings
		std::string m_pipeline{ "deferred" };
		bool        m_shadows{ true };
		//drawer
		Shared<Render::Drawer>          m_drawer;
		Shared<Render::DrawerPassDebug> m_debug;
		//worlds
		std::vector< Weak<RenderInstance> > m_instances;
		//(re)build the drawer passes
		void build_drawer();
	};

	class SQUARE_API RenderInstance : public SystemInstance
	{
	public:
		//A square object
		SQUARE_OBJECT(RenderInstance)

		//Init
		RenderInstance(Context& context, System& system, Scene::World& world);
		virtual ~RenderInstance();

		//colors
		const Vec4& clear_color() const;
		void clear_color(const Vec4& color);
		const Vec4& ambient_color() const;
		void ambient_color(const Vec4& color);

		//world drawn or not
		bool visible() const;
		void visible(bool visible);

		//draw the levels of the world
		void draw(Render::Drawer& drawer);

	protected:
		Vec4 m_clear_color{ 0.25f, 0.5f, 1.0f, 1.0f };
		Vec4 m_ambient_color{ 0.1f, 0.1f, 0.1f, 1.0f };
		bool m_visible{ true };
		//the collections of all the levels (rebuilt every frame)
		Render::Collection m_collection;
	};
}
