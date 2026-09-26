//
//  RenderSystem.h
//  Square
//
//  The render device, and the drawing of every world at the end of the frame
//  (late_update, the last one), then the present (window swap):
//  - RenderSystem, global: the render device (System::get<RenderSystem>(context)->render()),
//    it draws the instances and shows the frame;
//  - RenderInstance, per world: its settings (pipeline, shadows, debug, colors), its drawer
//    with the passes of its pipeline, the renderables of its levels.
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

		//the drawers of the worlds can be built: from post_initialize (their passes load the
		//effects added by AppInterface::start) to pre_shutdown
		bool ready() const;

		//draw all the worlds, and show the frame (the loop calls them)
		void draw();
		void present();

	protected:
		//device
		Render::Context*         m_render{ nullptr };
		Render::RenderInspector* m_inspector{ nullptr };
		//between post_initialize and pre_shutdown
		bool m_ready{ false };
		//worlds
		std::vector< Weak<RenderInstance> > m_instances;
		//the instances alive
		std::vector< Shared<RenderInstance> > instances();
	};

	class SQUARE_API RenderInstance : public SystemInstance
	{
	public:
		//A square object
		SQUARE_OBJECT(RenderInstance)

		//Registration in context (attributes)
		static void object_registration(Context& ctx);

		//Init
		RenderInstance(Context& context, System& system, Scene::World& world);
		virtual ~RenderInstance();

		//pipeline: "deferred" (default) or "forward"
		const std::string& pipeline() const;
		void pipeline(const std::string& pipeline);

		//shadow pass
		bool shadows() const;
		void shadows(bool enable);

		//debug pass (OBB, light volumes, textures; its draw flags are all off at start)
		bool debug() const;
		void debug(bool enable);

		//colors
		const Vec4& clear_color() const;
		void clear_color(const Vec4& color);
		const Vec4& ambient_color() const;
		void ambient_color(const Vec4& color);

		//world drawn or not
		bool visible() const;
		void visible(bool visible);

		//the drawer of the world and its debug pass: while the RenderSystem is ready,
		//nullptr out of it (and the debug pass when debug is off)
		Shared<Render::Drawer> drawer() const;
		Shared<Render::DrawerPassDebug> debug_pass() const;

		//draw the levels of the world
		void draw();

	protected:
		//settings
		std::string m_pipeline{ "deferred" };
		bool        m_shadows{ true };
		bool        m_debug{ true };
		Vec4        m_clear_color{ 0.25f, 0.5f, 1.0f, 1.0f };
		Vec4        m_ambient_color{ 0.1f, 0.1f, 0.1f, 1.0f };
		bool        m_visible{ true };
		//drawer
		Shared<Render::Drawer>          m_drawer;
		Shared<Render::DrawerPassDebug> m_debug_pass;
		//the collections of all the levels (rebuilt every frame)
		Render::Collection m_collection;
		//(re)build the passes of the settings / release them (by the RenderSystem, and by
		//the settings when they change)
		void build_drawer();
		void release_drawer();
		//a setting changed: new passes, if they are there
		void rebuild_drawer();
		friend class RenderSystem;
	};
}
