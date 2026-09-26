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
#include <cctype>

namespace Square
{
	//pipeline mask <-> "forward|deferred|debug" (case insensitive)
	static const std::pair<RenderPipeline, const char*> s_pipeline_names[]
	{
		{ RP_FORWARD,  "forward"  },
		{ RP_DEFERRED, "deferred" },
		{ RP_DEBUG,    "debug"    }
	};

	static std::string pipeline_to_string(unsigned int pipeline)
	{
		std::string names;
		for (const auto& flag : s_pipeline_names)
		{
			if (!(pipeline & flag.first)) continue;
			if (!names.empty()) names += "|";
			names += flag.second;
		}
		return names;
	}

	static unsigned int pipeline_from_string(const std::string& names)
	{
		unsigned int pipeline = 0;
		size_t start = 0;
		while (start <= names.size())
		{
			size_t end = names.find('|', start);
			if (end == std::string::npos) end = names.size();
			//trim
			size_t first = start, last = end;
			while (first < last && std::isspace((unsigned char)names[first])) ++first;
			while (last > first && std::isspace((unsigned char)names[last - 1])) --last;
			const std::string name = names.substr(first, last - first);
			for (const auto& flag : s_pipeline_names)
			{
				if (case_insensitive_equal(name, flag.second)) pipeline |= flag.first;
			}
			start = end + 1;
		}
		return pipeline;
	}

	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(RenderSystem);
	SQUARE_CLASS_OBJECT_REGISTRATION(RenderInstance);

	//Registration in context
	void RenderSystem::object_registration(Context& ctx)
	{
		//system: at start-up
		ctx.add_system<RenderSystem>(SystemStartup::AUTOMATIC);
	}

	void RenderInstance::object_registration(Context& ctx)
	{
		//Attributes: the render settings of a world
		ctx.add_attribute_function<RenderInstance, std::string>
		("pipeline"
		, std::string("deferred")
		, [](const RenderInstance* instance) -> std::string { return pipeline_to_string(instance->pipeline()); }
		, [](RenderInstance* instance, const std::string& pipeline) { instance->pipeline(pipeline_from_string(pipeline)); });
		ctx.add_attribute_function<RenderInstance, bool>
		("shadows"
		, true
		, [](const RenderInstance* instance) -> bool { return instance->shadows(); }
		, [](RenderInstance* instance, const bool& enable) { instance->shadows(enable); });
		ctx.add_attribute_function<RenderInstance, Vec4>
		("clear_color"
		, Vec4(0.25f, 0.5f, 1.0f, 1.0f)
		, [](const RenderInstance* instance) -> Vec4 { return instance->clear_color(); }
		, [](RenderInstance* instance, const Vec4& color) { instance->clear_color(color); });
		ctx.add_attribute_function<RenderInstance, Vec4>
		("ambient_color"
		, Vec4(0.1f, 0.1f, 0.1f, 1.0f)
		, [](const RenderInstance* instance) -> Vec4 { return instance->ambient_color(); }
		, [](RenderInstance* instance, const Vec4& color) { instance->ambient_color(color); });
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
		//the drawers of the worlds: post_initialize
		return true;
	}

	void RenderSystem::post_initialize()
	{
		//the passes load their effects: AppInterface::start has added the resources
		m_ready = true;
		for (auto& instance : instances()) instance->build_drawer();
	}

	void RenderSystem::pre_shutdown()
	{
		for (auto& instance : instances()) instance->release_drawer();
		m_ready = false;
	}

	void RenderSystem::shutdown()
	{
		for (auto& instance : instances()) instance->release_drawer();
		m_instances.clear();
		m_ready = false;
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
		//a world created while the application runs
		if (m_ready) instance->build_drawer();
		return instance;
	}

	Render::Context* RenderSystem::render() const
	{
		return m_render;
	}

	bool RenderSystem::ready() const
	{
		return m_ready;
	}

	std::vector< Shared<RenderInstance> > RenderSystem::instances()
	{
		//worlds gone
		m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(), [](const Weak<RenderInstance>& instance) { return instance.expired(); }), m_instances.end());
		std::vector< Shared<RenderInstance> > alive;
		for (const Weak<RenderInstance>& weak_instance : m_instances)
		{
			if (auto instance = weak_instance.lock()) alive.push_back(instance);
		}
		return alive;
	}

	void RenderSystem::draw()
	{
		for (auto& instance : instances())
		{
			if (instance->visible()) instance->draw();
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

	unsigned int RenderInstance::pipeline() const
	{
		return m_pipeline;
	}

	void RenderInstance::pipeline(unsigned int pipeline)
	{
		if (pipeline == m_pipeline) return;
		m_pipeline = pipeline;
		rebuild_drawer();
	}

	bool RenderInstance::shadows() const
	{
		return m_shadows;
	}

	void RenderInstance::shadows(bool enable)
	{
		if (enable == m_shadows) return;
		m_shadows = enable;
		rebuild_drawer();
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

	Shared<Render::Drawer> RenderInstance::drawer() const
	{
		return m_drawer;
	}

	Shared<Render::DrawerPassDebug> RenderInstance::debug_pass() const
	{
		return m_debug_pass;
	}

	void RenderInstance::build_drawer()
	{
		//the debug flags survive a rebuild
		const unsigned char debug_flags = m_debug_pass ? m_debug_pass->draw_flags() : 0;
		m_drawer = MakeShared<Render::Drawer>(context());
		if ((m_pipeline & RP_FORWARD) && !(m_pipeline & RP_DEFERRED))
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
		m_debug_pass = nullptr;
		if (m_pipeline & RP_DEBUG)
		{
			m_debug_pass = m_drawer->create<Render::DrawerPassDebug>();
			m_debug_pass->draw_flags(debug_flags);
		}
	}

	void RenderInstance::release_drawer()
	{
		m_debug_pass.reset();
		m_drawer.reset();
	}

	void RenderInstance::rebuild_drawer()
	{
		//only while the render system is ready (else post_initialize builds them)
		if (m_drawer) build_drawer();
	}

	void RenderInstance::draw()
	{
		if (!m_drawer) return;
		//one collection with the cameras, lights and renderables of all the levels
		m_collection.clear();
		for (const Shared<Scene::Level>& level : m_world.levels())
		{
			const Render::Collection& collection = level->randerable_collection();
			m_collection.m_cameras.insert(m_collection.m_cameras.end(), collection.m_cameras.begin(), collection.m_cameras.end());
			m_collection.m_lights.insert(m_collection.m_lights.end(), collection.m_lights.begin(), collection.m_lights.end());
			m_collection.m_renderables.insert(m_collection.m_renderables.end(), collection.m_renderables.begin(), collection.m_renderables.end());
		}
		m_drawer->draw(m_clear_color, m_ambient_color, m_collection);
	}
}
