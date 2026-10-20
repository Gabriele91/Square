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
#include "Square/Core/Filesystem.h"
#include "Square/Render/Pipeline/Drawer.h"
#include "Square/Render/Pipeline/DrawerPassDebug.h"
#include "Square/Render/Pipeline/DrawerPassForward.h"
#include "Square/Render/Pipeline/DrawerPassDeferred.h"
#include "Square/Render/Pipeline/DrawerPassShadow.h"
#include "Square/Render/PostEffect/PostEffect.h"
#include "Square/Render/Profiler.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/LevelOfDetail.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/World.h"
#include "Square/Scene/Level.h"
#include "HLSL2ALL/SourceToSpirv.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace Square
{
	//pipeline mask <-> "forward|deferred|debug" (case insensitive)
	static const std::pair<RenderPipeline, const char*> s_pipeline_names[]
	{
		{ RP_FORWARD,  "forward"  },
		{ RP_DEFERRED, "deferred" },
		{ RP_DEBUG,    "debug"    }
	};

	static String pipeline_to_string(unsigned int pipeline)
	{
		String names;
		for (const auto& flag : s_pipeline_names)
		{
			if (!(pipeline & flag.first)) continue;
			if (!names.empty()) names += "|";
			names += flag.second;
		}
		return names;
	}

	static unsigned int pipeline_from_string(const String& names)
	{
		unsigned int pipeline = 0;
		size_t start = 0;
		while (start <= names.size())
		{
			size_t end = names.find('|', start);
			if (end == String::npos) end = names.size();
			//trim
			size_t first = start, last = end;
			while (first < last && std::isspace((unsigned char)names[first])) ++first;
			while (last > first && std::isspace((unsigned char)names[last - 1])) --last;
			const String name = names.substr(first, last - first);
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
		ctx.add_attribute_function<RenderInstance, String>
		("pipeline"
		, String("deferred")
		, [](const RenderInstance* instance) -> String { return pipeline_to_string(instance->pipeline()); }
		, [](RenderInstance* instance, const String& pipeline) { instance->pipeline(pipeline_from_string(pipeline)); });
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
#if defined(SQUARE_DEBUG_TOOLS)
	namespace AuxRenderSystem
	{
		//a test: some capabilities of the driver off (SQUARE_DRIVER_CAPS_OFF, by ',': geometry,
		//vertex_layer, instanced), the paths of the drivers without them (their sub effects, the
		//layers by target)
		static void capabilities_off(Render::RenderDriverInfo& info, Context& context)
		{
			if (const char* off = std::getenv("SQUARE_DRIVER_CAPS_OFF"))
			{
				StringStream list(off);
				for (String capability; std::getline(list, capability, ',');)
				{
					if (capability == "geometry")
					{
						info.m_geometry_shader = false;
					}
					else if (capability == "vertex_layer")
					{
						info.m_vertex_viewport_index = false;
					}
					else if (capability == "instanced")
					{
						info.m_draw_instanced = false;
					}
				}
				context.logger()->warning(String("RenderSystem: capabilities of the driver off: ") + off);
			}
		}
	}
#endif

	RenderSystem::RenderSystem(Context& context) : System(context)
	{
	}

	bool RenderSystem::layers_by_target() const
	{
		const Render::RenderDriverInfo& info = m_shader_target.m_info;
		return !info.m_geometry_shader && !info.m_vertex_viewport_index;
	}

	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::setup_shader_caches()
	{
		//<app data>/square/shader_cache: the files not used for a month gone
		static constexpr int s_unused_days = 30;
		const String square = Filesystem::join(Filesystem::app_data_dir(), "square");
		const String root = Filesystem::join(square, "shader_cache");
		const bool made = (Filesystem::exists(square) || Filesystem::makedir(square))
		               && (Filesystem::exists(root) || Filesystem::makedir(root));
		if (made)
		{
			if (const size_t removed = Resource::ShaderCache::prune(root, s_unused_days))
			{
				context().logger()->info("RenderSystem: " + to_string(removed) + " files of the shader cache not used for " + to_string(s_unused_days) + " days removed");
			}
			//the translations (by language), the binaries of the driver (its language)
			m_shader_cache.folder(Filesystem::join(root, "translated"));
			const String driver = Filesystem::join(root, m_render->get_render_driver_info().m_shader_language);
			if (Filesystem::exists(driver) || Filesystem::makedir(driver))
			{
				m_render->shader_cache(driver);
			}
		}
		else
		{
			context().logger()->warning("RenderSystem: no shader cache (unable to make " + root + ")");
		}
		//the pack (tools/ShaderPack: the shaders of a Retail build), if there is
		const String pack = Filesystem::join(Filesystem::resource_dir(), "shaders.sqz");
		if (Filesystem::exists(pack))
		{
			m_shader_cache.pack(pack);
		}
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
		//textures inspector (the tab Textures of the debug panel): with TEXTURE_INTROSPECTION (Debug,
		//Release), whatever the debug of the driver
		#if defined(TEXTURE_INTROSPECTION)
		m_inspector = SQ_NEW(context().allocator(), Render::RegistryInspector, AllocType::ALCT_DEFAULT) Render::RegistryInspector();
		m_render->set_inspector(m_inspector);
		#endif
		//render profiler (off until enabled)
		#if defined(RENDER_PROFILER)
		m_profiler = SQ_NEW(context().allocator(), Render::Profiler, AllocType::ALCT_DEFAULT) Render::Profiler(*m_render);
		m_render->set_profiler(m_profiler);
		#endif
		//the target of the shaders (the driver, the gamma correction of the window), their caches
		m_shader_target = Resource::ShaderTarget::of(*m_render, window->device() && window->device()->get_context_info().m_srgb);
		#if defined(SQUARE_DEBUG_TOOLS)
		AuxRenderSystem::capabilities_off(m_shader_target.m_info, context());
		#endif
		setup_shader_caches();
		//the shader converter, for all the shaders (its tables made once)
		HLSL2ALL::initialize();
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
		#if defined(RENDER_PROFILER)
		if (m_profiler)
		{
			if (m_render) m_render->set_profiler(nullptr);
			SQ_DELETE_NAMESPACE(context().allocator(), Render, Profiler, m_profiler);
			m_profiler = nullptr;
		}
		#endif
		if (m_render)
		{
			HLSL2ALL::shutdown();
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
		#if defined(RENDER_PROFILER)
		if (m_profiler) m_profiler->begin_frame();
		#endif
		m_delta_time = delta_time;
		if (m_render) m_render->frame_begin();
		draw();
		if (m_render)
		{
			SQUARE_RENDER_SCOPE(*m_render, "Overlays");
			for (auto* overlay : m_overlays) overlay->draw_overlay(*m_render);
		}
		#if defined(RENDER_PROFILER)
		if (m_profiler) m_profiler->end_frame();
		#endif
		present();
	}

	void RenderSystem::add_overlay(RenderOverlay* overlay)
	{
		if (overlay && std::find(m_overlays.begin(), m_overlays.end(), overlay) == m_overlays.end())
		{
			m_overlays.push_back(overlay);
		}
	}

	void RenderSystem::remove_overlay(RenderOverlay* overlay)
	{
		m_overlays.erase(std::remove(m_overlays.begin(), m_overlays.end(), overlay), m_overlays.end());
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

	Render::Profiler* RenderSystem::profiler() const
	{
		return m_profiler;
	}

	bool RenderSystem::ready() const
	{
		return m_ready;
	}

	Vector< Shared<RenderInstance> > RenderSystem::instances()
	{
		//worlds gone
		m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(), [](const Weak<RenderInstance>& instance) { return instance.expired(); }), m_instances.end());
		Vector< Shared<RenderInstance> > alive;
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
			if (instance->visible()) instance->draw(m_delta_time);
		}
	}

	void RenderSystem::present()
	{
		if (m_render)
		{
			m_render->frame_end();
			m_render->print_errors();
		}
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

	const Render::LevelOfDetailSettings& RenderInstance::levels_of_detail() const
	{
		return m_levels_of_detail;
	}

	const Render::SoftwareOcclusion::Settings& RenderInstance::occlusion() const
	{
		return m_occlusion;
	}

	Render::SoftwareOcclusion::Stats RenderInstance::occlusion_stats() const
	{
		Render::SoftwareOcclusion::Stats stats;
		if (m_drawer)
		{
			stats = m_drawer->occlusion().stats();
		}
		return stats;
	}

	void RenderInstance::occlusion(const Render::SoftwareOcclusion::Settings& settings)
	{
		m_occlusion = settings;
		if (m_drawer)
		{
			m_drawer->occlusion(m_occlusion);
		}
	}

	void RenderInstance::levels_of_detail(const Render::LevelOfDetailSettings& settings)
	{
		m_levels_of_detail = settings;
		if (m_drawer)
		{
			m_drawer->levels_of_detail(m_levels_of_detail);
		}
	}

	const Vec2& RenderInstance::resolution_scale() const
	{
		return m_resolution_scale;
	}

	void RenderInstance::resolution_scale(const Vec2& scale)
	{
		m_resolution_scale = glm::clamp(scale, Vec2(0.1f), Vec2(1.0f));
		if (m_drawer)
		{
			m_drawer->resolution_scale(scene_scale());
		}
	}

	const Render::DynamicResolution::Settings& RenderInstance::dynamic_resolution() const
	{
		return m_dynamic_resolution.settings();
	}

	void RenderInstance::dynamic_resolution(const Render::DynamicResolution::Settings& settings)
	{
		m_dynamic_resolution.settings(settings);
	}

	const Vec2& RenderInstance::dynamic_scale() const
	{
		return m_dynamic_scale;
	}

	const Render::DynamicThermal::Settings& RenderInstance::dynamic_thermal() const
	{
		return m_dynamic_thermal.settings();
	}

	void RenderInstance::dynamic_thermal(const Render::DynamicThermal::Settings& settings)
	{
		m_dynamic_thermal.settings(settings);
	}

	const Vec2& RenderInstance::thermal_scale() const
	{
		return m_thermal_scale;
	}

	float RenderInstance::thermal_level() const
	{
		return m_dynamic_thermal.level();
	}

	Vec2 RenderInstance::scene_scale() const
	{
		return m_resolution_scale * m_dynamic_scale * m_thermal_scale;
	}

	float RenderInstance::dynamic_fps() const
	{
		const float target = m_dynamic_resolution.settings().m_target_fps;
		if (target > 0.0f) return target;
		//the limit of the application, else the refresh of its screen
		auto* app = Application::instance();
		if (!app) return 60.0f;
		const FrameRate& frames = app->frames();
		if (frames.m_mode == FrameMode::LIMIT && frames.m_fps > 0) return float(frames.m_fps);
		auto* window = app->window();
		const unsigned int refresh = window && window->device() ? window->device()->get_refresh() : 0;
		return refresh > 0 ? float(refresh) : 60.0f;
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

	void RenderInstance::add_post_effect(Shared<Render::PostEffect> effect)
	{
		if (!effect || std::find(m_post_effects.begin(), m_post_effects.end(), effect) != m_post_effects.end()) return;
		m_post_effects.push_back(effect);
		if (m_drawer) m_drawer->post_effects(m_post_effects);
	}

	void RenderInstance::remove_post_effect(Shared<Render::PostEffect> effect)
	{
		m_post_effects.erase(std::remove(m_post_effects.begin(), m_post_effects.end(), effect), m_post_effects.end());
		if (m_drawer) m_drawer->post_effects(m_post_effects);
	}

	void RenderInstance::post_effects(const Vector< Shared<Render::PostEffect> >& effects)
	{
		m_post_effects = effects;
		if (m_drawer) m_drawer->post_effects(m_post_effects);
	}

	const Vector< Shared<Render::PostEffect> >& RenderInstance::post_effects() const
	{
		return m_post_effects;
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
		const unsigned short debug_flags = m_debug_pass ? m_debug_pass->draw_flags() : 0;
		m_drawer = MakeShared<Render::Drawer>(context());
		m_drawer->post_effects(m_post_effects);
		m_drawer->levels_of_detail(m_levels_of_detail);
		m_drawer->resolution_scale(scene_scale());
		m_drawer->occlusion(m_occlusion);
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
#if defined(SQUARE_DEBUG_TOOLS)
		const bool debug_tools = true;
#else
		const bool debug_tools = false;
#endif
		if (debug_tools && (m_pipeline & RP_DEBUG))
		{
			m_debug_pass = m_drawer->create<Render::DrawerPassDebug>();
			m_debug_pass->draw_flags(debug_flags);
		}
	}

	void RenderInstance::release_drawer()
	{
		//the GPU objects of the post effects go with the passes (made again when drawn)
		for (auto& effect : m_post_effects) if (effect) effect->release();
		m_debug_pass.reset();
		m_drawer.reset();
	}

	void RenderInstance::rebuild_drawer()
	{
		//only while the render system is ready (else post_initialize builds them)
		if (m_drawer) build_drawer();
	}

	void RenderInstance::draw(double delta_time)
	{
		if (!m_drawer) return;
		//the scene lower while the frames are late
		//(the GPU time of the frames: the driver, when it knows it)
		float gpu_ms = -1.0f;
		if (auto system = System::get<RenderSystem>(context()))
		{
			if (Render::Context* render = system->render())
			{
				gpu_ms = render->gpu_frame_ms();
			}
		}
		const Vec2 dynamic = m_dynamic_resolution.update(float(delta_time), dynamic_fps(), gpu_ms);
		//the scene lower while the device is hot (the device of the window of the application)
		Video::DeviceResources* device = nullptr;
		if (Application* app = Application::instance())
		{
			if (Video::Window* window = app->window())
			{
				device = window->device();
			}
		}
		const Vec2 thermal = m_dynamic_thermal.update(float(delta_time), device);
		const bool dynamic_changed = glm::any(glm::greaterThan(glm::abs(dynamic - m_dynamic_scale), Vec2(0.001f)));
		const bool thermal_changed = glm::any(glm::greaterThan(glm::abs(thermal - m_thermal_scale), Vec2(0.001f)));
		if (dynamic_changed || thermal_changed)
		{
			m_dynamic_scale = dynamic;
			m_thermal_scale = thermal;
			m_drawer->resolution_scale(scene_scale());
		}
		//(the heat in the log at each status, at each step of a tenth of its level)
		static constexpr float s_thermal_log_step = 0.1f;
		const Video::ThermalState& state = m_dynamic_thermal.state();
		const bool status_changed = state.m_status != m_thermal_status_logged;
		if (status_changed || std::abs(m_dynamic_thermal.level() - m_thermal_logged) >= s_thermal_log_step)
		{
			m_thermal_logged = m_dynamic_thermal.level();
			m_thermal_status_logged = state.m_status;
			context().logger()->info("Thermal: status " + to_string(int(state.m_status)) + ", headroom " + to_string(state.m_headroom) + ", level " + to_string(m_thermal_logged) + ", scene " + to_string(m_thermal_scale.x));
		}
		m_time += delta_time;
		m_drawer->time(Vec2(float(m_time), float(delta_time)));
		m_drawer->draw(m_clear_color, m_ambient_color, m_collection);
	}

	//the cameras, lights and renderables of all the levels of the world
	void RenderInstance::on_add_component(const Shared<Scene::Actor>& actor, const Shared<Scene::Component>& component)
	{
		//(a group of levels of detail: its renderables are components of their own)
		if (auto lod = DynamicPointerCast<Render::LevelOfDetail, Scene::Component>(component))
		{
			m_collection.m_levels_of_detail.push_back(lod);
		}
		//(an occluder: never drawn)
		if (auto occluder = DynamicPointerCast<Render::Occluder, Scene::Component>(component))
		{
			m_collection.m_occluders.push_back(occluder);
		}
		if (auto renderable = DynamicPointerCast<Render::Renderable, Scene::Component>(component))
		{
			m_collection.m_renderables.push_back(renderable);
		}
		else if (auto light = DynamicPointerCast<Render::Light, Scene::Component>(component))
		{
			m_collection.m_lights.push_back(light);
		}
		else if (auto camera = DynamicPointerCast<Render::Camera, Scene::Component>(component))
		{
			m_collection.m_cameras.push_back(camera);
		}
	}

	void RenderInstance::on_remove_component(const Shared<Scene::Actor>& actor, const Shared<Scene::Component>& component)
	{
		//it, and the ones gone
		if (auto lod = DynamicPointerCast<Render::LevelOfDetail, Scene::Component>(component))
		{
			auto& lods = m_collection.m_levels_of_detail;
			lods.erase(std::remove_if(lods.begin(), lods.end(), [&](const Weak<Render::LevelOfDetail>& weak_lod)
			{
				auto other = weak_lod.lock();
				return !other || other == lod;
			}), lods.end());
		}
		if (auto occluder = DynamicPointerCast<Render::Occluder, Scene::Component>(component))
		{
			auto& occluders = m_collection.m_occluders;
			occluders.erase(std::remove_if(occluders.begin(), occluders.end(), [&](const Weak<Render::Occluder>& weak_occluder)
			{
				auto other = weak_occluder.lock();
				return !other || other == occluder;
			}), occluders.end());
		}
		if (auto renderable = DynamicPointerCast<Render::Renderable, Scene::Component>(component))
		{
			auto& renderables = m_collection.m_renderables;
			renderables.erase(std::remove_if(renderables.begin(), renderables.end(), [&](const Weak<Render::Renderable>& weak_renderable)
			{
				auto other = weak_renderable.lock();
				return !other || other == renderable;
			}), renderables.end());
		}
		else if (auto light = DynamicPointerCast<Render::Light, Scene::Component>(component))
		{
			auto& lights = m_collection.m_lights;
			lights.erase(std::remove_if(lights.begin(), lights.end(), [&](const Weak<Render::Light>& weak_light)
			{
				auto other = weak_light.lock();
				return !other || other == light;
			}), lights.end());
		}
		else if (auto camera = DynamicPointerCast<Render::Camera, Scene::Component>(component))
		{
			auto& cameras = m_collection.m_cameras;
			cameras.erase(std::remove_if(cameras.begin(), cameras.end(), [&](const Weak<Render::Camera>& weak_camera)
			{
				auto other = weak_camera.lock();
				return !other || other == camera;
			}), cameras.end());
		}
	}
}
