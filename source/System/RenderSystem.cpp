//
//  RenderSystem.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/Window.h"
#include "Square/Render/RegistryInspector.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/System/RenderSystem.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/DrawerPassDebug.h"
#include "Square/Render/DrawerPassForward.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Render/DrawerPassShadow.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/World.h"
#include "Square/Scene/Level.h"
#include <algorithm>
#include <cstdlib>

namespace Square
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(RenderSystem);

	//Registration in context
	void RenderSystem::object_registration(Context& ctx)
	{
		//system: at start-up (ring CORE: the device the other systems use)
		ctx.add_system<RenderSystem>(SystemStartup::AUTOMATIC);
		//Attributes
		ctx.add_attribute_function<RenderSystem, std::string>
		("pipeline"
		, std::string("deferred")
		, [](const RenderSystem* system) -> std::string { return system->pipeline(); }
		, [](RenderSystem* system, const std::string& pipeline) { system->pipeline(pipeline); });
		ctx.add_attribute_function<RenderSystem, bool>
		("shadows"
		, true
		, [](const RenderSystem* system) -> bool { return system->shadows(); }
		, [](RenderSystem* system, const bool& enable) { system->shadows(enable); });
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//RenderSystem
	RenderSystem::RenderSystem(Context& context) : System(context)
	{
	}

	RenderSystem::~RenderSystem()
	{
	}

	bool RenderSystem::initialize()
	{
		// The device, on the window of the application
		Application*   application = context().application();
		Video::Window* window = context().window();
		if (!application || !window)
		{
			context().logger()->error("RenderSystem: unable to get window or application instance");
			return false;
		} 
		// Render driver
		const WindowRenderDriver& driver = application->render_driver();
		m_render = Render::create_render_driver(context().allocator(), context().logger(), driver.m_type);
		if (!m_render || !m_render->init(window->device()))
		{
			context().logger()->error("RenderSystem: unable to load render driver");
			if (m_render) Render::delete_render_driver(m_render);
			m_render = nullptr;
			return false;
		}
		//textures inspector, in debug
		#if defined(TEXTURE_INTROSPECTION)
		if (driver.m_debug)
		{
			m_inspector = SQ_NEW(context().allocator(), Render::RegistryInspector, AllocType::ALCT_DEFAULT) Render::RegistryInspector();
			m_render->set_inspector(m_inspector);
		}
		#endif
		//flush errors, show info
		m_render->print_errors();
		m_render->print_info();
		//SQUARE_RENDERING=forward|deferred
		if (const char* rendering_type = std::getenv("SQUARE_RENDERING"))
		{
			m_pipeline = case_insensitive_equal(rendering_type, "forward") ? "forward" : "deferred";
		}
		//the drawer: post_initialize
		return true;
	}

	void RenderSystem::post_initialize()
	{
		//the passes load their effects: AppInterface::start has added the resources
		build_drawer();
	}

	void RenderSystem::pre_shutdown()
	{
		m_debug.reset();
		m_drawer.reset();
	}

	void RenderSystem::shutdown()
	{
		m_instances.clear();
		m_debug.reset();
		m_drawer.reset();
		//the loaded resources hold objects of the device: they go before it
		context().clear_resources();
		if (m_render)
		{
			m_render->close();
			Render::delete_render_driver(m_render);
			m_render = nullptr;
		}
		#if defined(TEXTURE_INTROSPECTION)
		if (m_inspector)
		{
			auto* texture_inspector = dynamic_cast<Render::RegistryInspector*>(m_inspector);
			SQ_DELETE_NAMESPACE(context().allocator(), Render, RegistryInspector, texture_inspector);
			m_inspector = nullptr;
		}
		#endif
	}

	void RenderSystem::late_update(double delta_time)
	{
		draw();
		present();
	}

	Shared<SystemInstance> RenderSystem::create_instance(Scene::World& world)
	{
		auto instance = MakeShared<RenderInstance>(context(), *this, world);
		m_instances.push_back(instance);
		return instance;
	}

	Render::Context* RenderSystem::render() const
	{
		return m_render;
	}

	const std::string& RenderSystem::pipeline() const
	{
		return m_pipeline;
	}

	void RenderSystem::pipeline(const std::string& pipeline)
	{
		const std::string name = case_insensitive_equal(pipeline, "forward") ? "forward" : "deferred";
		if (name == m_pipeline) return;
		m_pipeline = name;
		//already running: new passes
		if (m_drawer) build_drawer();
	}

	bool RenderSystem::shadows() const
	{
		return m_shadows;
	}

	void RenderSystem::shadows(bool enable)
	{
		if (enable == m_shadows) return;
		m_shadows = enable;
		if (m_drawer) build_drawer();
	}

	Shared<Render::Drawer> RenderSystem::drawer() const
	{
		return m_drawer;
	}

	Shared<Render::DrawerPassDebug> RenderSystem::debug() const
	{
		return m_debug;
	}

	void RenderSystem::build_drawer()
	{
		//the debug flags survive a rebuild
		const unsigned char debug_flags = m_debug ? m_debug->draw_flags() : 0;
		m_drawer = MakeShared<Render::Drawer>(context());
		if (m_pipeline == "forward")
		{
			context().logger()->info("Rendering: forward");
			m_drawer->create<Render::DrawerPassForward>();
		}
		else
		{
			context().logger()->info("Rendering: deferred");
			m_drawer->create<Render::DrawerPassDeferred>();
		}
		if (m_shadows) m_drawer->create<Render::DrawerPassShadow>();
		m_debug = m_drawer->create<Render::DrawerPassDebug>();
		m_debug->draw_flags(debug_flags);
	}

	void RenderSystem::draw()
	{
		if (!m_drawer) return;
		//worlds gone
		m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(), [](const Weak<RenderInstance>& instance) { return instance.expired(); }), m_instances.end());
		for (const Weak<RenderInstance>& weak_instance : m_instances)
		{
			if (auto instance = weak_instance.lock())
			{
				if (instance->visible()) instance->draw(*m_drawer);
			}
		}
	}

	void RenderSystem::present()
	{
		if (m_render) m_render->print_errors();
		if (auto* window = context().window()) window->swap();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//RenderInstance
	RenderInstance::RenderInstance(Context& context, System& system, Scene::World& world)
	: SystemInstance(context, system, world)
	{
	}

	RenderInstance::~RenderInstance()
	{
	}

	const Vec4& RenderInstance::clear_color() const
	{
		return m_clear_color;
	}

	void RenderInstance::clear_color(const Vec4& color)
	{
		m_clear_color = color;
	}

	const Vec4& RenderInstance::ambient_color() const
	{
		return m_ambient_color;
	}

	void RenderInstance::ambient_color(const Vec4& color)
	{
		m_ambient_color = color;
	}

	bool RenderInstance::visible() const
	{
		return m_visible;
	}

	void RenderInstance::visible(bool visible)
	{
		m_visible = visible;
	}

	void RenderInstance::draw(Render::Drawer& drawer)
	{
		//one collection with the cameras, lights and renderables of all the levels
		m_collection.clear();
		for (const Shared<Scene::Level>& level : m_world.levels())
		{
			const Render::Collection& collection = level->randerable_collection();
			m_collection.m_cameras.insert(m_collection.m_cameras.end(), collection.m_cameras.begin(), collection.m_cameras.end());
			m_collection.m_lights.insert(m_collection.m_lights.end(), collection.m_lights.begin(), collection.m_lights.end());
			m_collection.m_renderables.insert(m_collection.m_renderables.end(), collection.m_renderables.begin(), collection.m_renderables.end());
		}
		drawer.draw(m_clear_color, m_ambient_color, m_collection);
	}
}
