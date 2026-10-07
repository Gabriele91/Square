//
//  DemoTools.cpp
//  Rush
//
//  See DemoTools.h.
//
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <DemoTools.h>
#include <Arena.h>
#include <Race.h>
#include <Graphics.h>
#include <Collision.h>

using namespace Rush;

DemoTools::DemoTools(Square::Context& context, Square::Scene::World& world)
: m_context(context)
, m_world(world)
{
}

void DemoTools::bind(Square::UI::DataModel& model)
{
	using namespace Square;
	//lights
	model.bind("sun", &m_state.m_sun.m_value);
	model.bind("shadow_filter", &m_state.m_shadow_filter.m_value);
	model.bind("cascades", &m_state.m_cascades.m_value);
	//graphics
	model.bind("ssr_resolution", &m_state.m_ssr_resolution.m_value);
	model.bind("ssr_march", &m_state.m_ssr_march.m_value);
	model.bind("ssr_blur", &m_state.m_ssr_blur.m_value);
	model.bind("ssr_denoise", &m_state.m_ssr_denoise.m_value);
	model.bind("ssao_resolution", &m_state.m_ssao_resolution.m_value);
	model.bind("ssao_blur", &m_state.m_ssao_blur.m_value);
	//debug
	model.bind("mirror", &m_state.m_mirror.m_value);
	model.bind("collisions", &m_state.m_collisions.m_value);
	model.bind("obb", &m_state.m_obb.m_value);
	model.bind("lights", &m_state.m_lights.m_value);
	model.bind("textures", &m_state.m_textures.m_value);
	model.bind("ssr_debug", &m_state.m_ssr_debug.m_value);
	model.bind("bloom_debug", &m_state.m_bloom_debug.m_value);
	model.bind("ssao_debug", &m_state.m_ssao_debug.m_value);
	model.bind("ui_debugger", &m_state.m_ui_debugger.m_value);
	model.bind("profiler", &m_state.m_profiler.m_value);
	model.bind("profiler_available", &m_state.m_profiler_available);
	auto* ui_system = System::get<UISystem>(m_context);
	m_state.m_profiler_available = ui_system && ui_system->has_profiler();
}

void DemoTools::setup(Square::UI::Document& menu, Square::UI::DataModel& model)
{
	using namespace Square;
	m_menu = &menu;
	//the level
	menu.find("save").on(UI::EventType::CLICK, [this](UI::Event&) { save_level(false); });
	menu.find("load").on(UI::EventType::CLICK, [this](UI::Event&) { load_level(false); });
	menu.find("save_json").on(UI::EventType::CLICK, [this](UI::Event&) { save_level(true); });
	menu.find("load_json").on(UI::EventType::CLICK, [this](UI::Event&) { load_level(true); });
	//the sun: the daylight (the option "sun")
	if (UI::Element sun = menu.find("sun_toggle"))
	{
		sun.on(UI::EventType::CLICK, [this, &model](UI::Event&)
		{
			m_state.m_sun.m_value = !m_state.m_sun.m_value;
			model.dirty("sun");
		});
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//update
void DemoTools::update(Race* race, const Graphics& graphics)
{
	if (race) update_lights(race->arena());
	update_effects(graphics);
	update_debug(race);
}

void DemoTools::update_lights(const Arena& arena)
{
	using namespace Square;
	auto sun = arena.sun();
	sync_option(m_state.m_sun, sun && sun->visible(), [sun](bool value) { if (sun) sun->visible(value); });
	sync_option(m_state.m_cascades, sun ? sun->cascades() : DIRECTION_SHADOW_CSM_DEFAULT_FACES, [sun](int value)
	{
		if (sun) sun->cascades(value);
	});
	sync_option(m_state.m_shadow_filter, sun ? int(sun->shadow_filter()) : 0, [sun](int value)
	{
		if (sun) sun->shadow_filter(Render::ShadowFilter(std::clamp(value, 0, int(Render::ShadowFilter::PCSS))));
	});
	//the markers of the spot lights: on or off
	for (auto& marker : m_light_markers)
	{
		auto light = spot_light(marker.m_name);
		const bool off = !light || !light->visible();
		marker.m_element.set_class("off", off);
	}
}

void DemoTools::update_effects(const Graphics& graphics)
{
	using namespace Square;
	auto ssr = graphics.ssr();
	auto ssao = graphics.ssao();
	//a setting of an effect: its settings read, changed, set
	auto set_ssr = [ssr](auto change)
	{
		if (!ssr) return;
		auto settings = ssr->settings();
		change(settings);
		ssr->settings(settings);
	};
	auto set_ssao = [ssao](auto change)
	{
		if (!ssao) return;
		auto settings = ssao->settings();
		change(settings);
		ssao->settings(settings);
	};
	using SSRSettings = Render::SSR::Settings;
	using SSAOSettings = Render::SSAO::Settings;
	sync_option(m_state.m_ssr_resolution, ssr ? int(ssr->settings().resolution) : 0, [&](int value)
	{
		set_ssr([&](SSRSettings& settings) { settings.resolution = Render::PostEffectResolution(std::clamp(value, 0, int(Render::PER_QUARTER))); });
	});
	sync_option(m_state.m_ssr_march, ssr && ssr->settings().screen_march ? 1 : 0, [&](int value)
	{
		set_ssr([&](SSRSettings& settings) { settings.screen_march = value != 0; });
	});
	sync_option(m_state.m_ssr_blur, ssr ? int(ssr->settings().blur) : 0, [&](int value)
	{
		set_ssr([&](SSRSettings& settings) { settings.blur = SSRSettings::BlurQuality(std::clamp(value, 0, int(SSRSettings::BLUR_HIGH))); });
	});
	sync_option(m_state.m_ssr_denoise, ssr && ssr->settings().denoise, [&](bool value)
	{
		set_ssr([&](SSRSettings& settings) { settings.denoise = value; });
	});
	sync_option(m_state.m_ssr_debug, ssr ? ssr->settings().debug : 0, [&](int value)
	{
		set_ssr([&](SSRSettings& settings) { settings.debug = value; });
	});
	sync_option(m_state.m_ssao_resolution, ssao ? int(ssao->settings().resolution) : 0, [&](int value)
	{
		set_ssao([&](SSAOSettings& settings) { settings.resolution = Render::PostEffectResolution(std::clamp(value, 0, int(Render::PER_QUARTER))); });
	});
	sync_option(m_state.m_ssao_blur, ssao ? int(ssao->settings().blur) : 0, [&](int value)
	{
		set_ssao([&](SSAOSettings& settings) { settings.blur = SSAOSettings::BlurQuality(std::clamp(value, 0, int(SSAOSettings::BLUR_HIGH))); });
	});
	sync_option(m_state.m_ssao_debug, ssao && ssao->settings().debug, [&](bool value)
	{
		set_ssao([&](SSAOSettings& settings) { settings.debug = value; });
	});
	auto bloom = graphics.bloom();
	sync_option(m_state.m_bloom_debug, bloom && bloom->settings().debug, [bloom](bool value)
	{
		if (!bloom) return;
		auto settings = bloom->settings();
		settings.debug = value;
		bloom->settings(settings);
	});
}

void DemoTools::update_debug(Race* race)
{
	using namespace Square;
	if (race) sync_option(m_state.m_mirror, m_mirror, [&](bool value) { mirror(*race, value); });
	auto collision = m_world.instance<CollisionWorld>();
	sync_option(m_state.m_collisions, collision && collision->debug(), [collision](bool value) { if (collision) collision->debug(value); });
	//the debug views of the world: flags of the debug pass
	auto debug = render_debug();
	auto has_flags = [&](unsigned char flags) { return debug && (debug->draw_flags() & flags) == flags; };
	auto set_flags = [&](unsigned char flags, bool value)
	{
		if (!debug) return;
		const unsigned char draw_flags = debug->draw_flags();
		debug->draw_flags((unsigned char)(value ? draw_flags | flags : draw_flags & ~flags));
	};
	const unsigned char lights = Render::DF_DRAW_SPOT_LIGHT | Render::DF_DRAW_POINT_LIGHT | Render::DF_DRAW_DIRECTIONAL_LIGHT;
	sync_option(m_state.m_obb, has_flags(Render::DF_DRAW_OBB), [&](bool value) { set_flags(Render::DF_DRAW_OBB, value); });
	sync_option(m_state.m_lights, has_flags(lights), [&](bool value) { set_flags(lights, value); });
	sync_option(m_state.m_textures, has_flags(Render::DB_DRAW_TEXTURES), [&](bool value) { set_flags(Render::DB_DRAW_TEXTURES, value); });
	//the debugger and the profiler of the UI
	auto* ui_system = System::get<UISystem>(m_context);
	sync_option(m_state.m_ui_debugger, ui_system && ui_system->ui().debugger(), [ui_system](bool value) { if (ui_system) ui_system->ui().debugger(value); });
	sync_option(m_state.m_profiler, ui_system && ui_system->profiler(), [ui_system](bool value) { if (ui_system) ui_system->profiler(value); });
	static bool temp_profiler = false; if (!temp_profiler && ui_system && std::getenv("RUSH_PROFILE")) { temp_profiler = true; ui_system->profiler(true); }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//lights
void DemoTools::race_started(const Arena& arena)
{
	using namespace Square;
	//a dot over the map (assets/ui.sqz/arena_map.png, it covers the x/z bounds of the arena, +x
	//right, +z up) for each spot light of the arena, a click switches it
	race_ended();
	if (!m_menu) return;
	UI::Element map = m_menu->find("map");
	auto level = m_world.level(s_race_world_level);
	if (!map || !level) return;
	const Vec3 size = glm::max(arena.max() - arena.min(), Vec3(0.001f));
	level->visit([&](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::SpotLight>()) return true;
		const Vec3  position = node->position(true);
		const float left = (position.x - arena.min().x) / size.x * 100.0f;
		const float top  = (arena.max().z - position.z) / size.z * 100.0f;
		LightMarker marker;
		marker.m_name = node->name();
		marker.m_light = node->component<Scene::SpotLight>();
		marker.m_element = map.create_child("div");
		marker.m_element.set_class("light");
		marker.m_element.set_property("left", std::to_string(left) + "%");
		marker.m_element.set_property("top", std::to_string(top) + "%");
		const std::string name = marker.m_name;
		marker.m_element.on(UI::EventType::CLICK, [this, name](UI::Event&)
		{
			if (auto light = spot_light(name)) light->visible(!light->visible());
		});
		m_light_markers.push_back(std::move(marker));
		return true;
	});
}

void DemoTools::race_ended()
{
	//the dots out of the map, the mirror off (its hovercraft goes)
	for (auto& marker : m_light_markers)
	{
		marker.m_element.remove();
	}
	m_light_markers.clear();
	m_mirror = false;
	m_state.m_mirror.m_value = false;
	m_state.m_mirror.m_shown = false;
}

Square::Shared<Square::Scene::SpotLight> DemoTools::spot_light(const std::string& name)
{
	using namespace Square;
	//found again by name after a load of the level
	auto it = std::find_if(m_light_markers.begin(), m_light_markers.end(), [&](const LightMarker& marker) { return marker.m_name == name; });
	if (it == m_light_markers.end()) return nullptr;
	if (auto light = it->m_light.lock()) return light;
	auto level = m_world.level(s_race_world_level);
	if (!level) return nullptr;
	level->visit([&](Shared<Scene::Actor> node) -> bool
	{
		const bool found = node->name() == name && node->contains<Scene::SpotLight>();
		if (!found) return true;
		it->m_light = node->component<Scene::SpotLight>();
		return false;
	});
	return it->m_light.lock();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//mirror
void DemoTools::mirror(Race& race, bool enable)
{
	using namespace Square;
	m_mirror = enable;
	const auto& racers = race.racers();
	if (racers.empty() || !racers[0].m_actor) return;
	//back: its materials again from the .mat, with the skin
	if (!enable)
	{
		race.paint(0);
		return;
	}
	//chrome: a white metal (the albedo of a metal is its reflected color), perfectly smooth
	auto white = m_context.resource<Resource::Texture>("white");
	racers[0].m_actor->visit([&](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		for (auto& material : node->component<Scene::StaticMesh>()->m_materials)
		{
			if (!material) continue;
			if (auto p = material->parameter_by_name("albedo_map"))    p->set(white);
			if (auto p = material->parameter_by_name("metallic_map"))  p->set(white);
			if (auto p = material->parameter_by_name("roughness_map")) p->set(white);
			if (auto p = material->parameter_by_name("color"))         p->set(Vec4(0.95f, 0.95f, 0.95f, 1.0f));
			if (auto p = material->parameter_by_name("metallic"))      p->set(1.0f);
			if (auto p = material->parameter_by_name("roughness"))     p->set(0.0f);
		}
		return true;
	});
}

//////////////////////////////////////////////////////////////////////////////////////////////
//level
std::string DemoTools::level_path(bool json)
{
	return Square::Filesystem::join(Square::Filesystem::resource_dir(), json ? "level.jsq" : "level.sq");
}

void DemoTools::save_level(bool json)
{
	using namespace Square;
	using namespace Square::Data;
	using namespace Square::Filesystem::Stream;
	if (json)
	{
		Json jout = Json(JsonObject());
		m_world.serialize_json(jout);
		std::ofstream(level_path(true)) << jout;
		return;
	}
	GZOStream ofile(level_path(false));
	ArchiveBinWrite out(m_context, ofile);
	m_world.serialize(out);
}

void DemoTools::load_level(bool json)
{
	using namespace Square;
	using namespace Square::Data;
	using namespace Square::Filesystem::Stream;
	const std::string path = level_path(json);
	if (!Filesystem::exists(path)) return;
	if (json)
	{
		Json jin;
		if (jin.parser(Filesystem::text_file_read_all(path))) m_world.deserialize_json(jin);
		return;
	}
	GZIStream ifile(path);
	ArchiveBinRead in(m_context, ifile);
	m_world.deserialize(in);
}

Square::Shared<Square::Render::DrawerPassDebug> DemoTools::render_debug()
{
	auto render_world = m_world.instance<Square::RenderInstance>();
	return render_world ? render_world->debug_pass() : nullptr;
}

void DemoTools::mouse_scroll(double scroll)
{
	if (auto debug = render_debug()) debug->panel_scroll((float)scroll * 20.0f);
}
