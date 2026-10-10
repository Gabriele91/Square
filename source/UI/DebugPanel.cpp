//
//  DebugPanel.cpp
//  Square
//
//  See DebugPanel.h for the high level description.
//
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Elements/ElementTabSet.h>
#include "Square/Core/Context.h"
#include "Square/Core/Logger.h"
#include "Square/System/RenderSystem.h"
#include "Square/Render/Pipeline/DrawerPassDebug.h"
#include "Square/Render/PostEffect/PostEffect.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/DirectionLight.h"
#include "Square/Driver/RenderInspector.h"
#include "ProfilerPanel.h"
#include "TexturePreviews.h"
#include "DebugPanel.h"

namespace Square
{
namespace UI
{
	namespace AuxDebugPanel
	{
		//the document of the panel (its style: debug.rcss, next to it)
		static const char* s_debug_rml = "common/ui/debug.rml";

		//the tabs of the engine, in their order (the ones of a game after them)
		static const char* s_profiler = "Profiler";
		static const char* s_pipeline = "Pipeline";
		static const char* s_draw = "Debug draw";
		static const char* s_textures = "Textures";
		static const char* s_ui = "UI";

		//the name of a format of a texture
		static std::string format_name(Render::TextureFormat format)
		{
			switch (format)
			{
			case Render::TF_RGBA8: return "RGBA8";
			case Render::TF_RGBA16F: return "RGBA16F";
			case Render::TF_RGBA32F: return "RGBA32F";
			case Render::TF_RGB8: return "RGB8";
			case Render::TF_RGB16F: return "RGB16F";
			case Render::TF_RGB32F: return "RGB32F";
			case Render::TF_RG8: return "RG8";
			case Render::TF_RG16F: return "RG16F";
			case Render::TF_RG32F: return "RG32F";
			case Render::TF_R8: return "R8";
			case Render::TF_R16F: return "R16F";
			case Render::TF_R32F: return "R32F";
			case Render::TF_DEPTH16_STENCIL8: return "D16S8";
			case Render::TF_DEPTH24_STENCIL8: return "D24S8";
			case Render::TF_DEPTH32_STENCIL8: return "D32S8";
			case Render::TF_DEPTH_COMPONENT16: return "D16";
			case Render::TF_DEPTH_COMPONENT24: return "D24";
			case Render::TF_DEPTH_COMPONENT32: return "D32";
			case Render::TF_BC1: return "BC1";
			case Render::TF_BC3: return "BC3";
			case Render::TF_BC4: return "BC4";
			case Render::TF_BC5: return "BC5";
			case Render::TF_ASTC_4x4: return "ASTC 4x4";
			default: return "format " + std::to_string(int(format));
			}
		}

		//the textures of the driver of a kind (0: render targets, 1: shadow maps: depth, arrays,
		//cubes, 2: images), not the ones of the thumbnails; none without the registry
		static std::vector<TexturePreviews::Item> textures_of(Square::Context& context, int kind, const TexturePreviews& previews)
		{
			std::vector<TexturePreviews::Item> items;
			auto* render_system = System::get<RenderSystem>(context);
			Render::RenderInspector* inspector = render_system && render_system->render() ? render_system->render()->inspector() : nullptr;
			if (inspector)
			{
				for (const auto& texture : inspector->textures())
				{
					bool attached = false;
					for (const auto& target : inspector->targets())
					{
						for (const auto& field : target.second.m_fields)
						{
							attached = attached || field.m_texture == texture.first;
						}
					}
					const Render::TextureInfo& info = texture.second;
					const bool special = info.is_depth() || info.m_shape != Render::TS_TEXTURE_2D;
					const int of = special ? 1 : attached ? 0 : 2;
					if (of == kind && !previews.owns(texture.first))
					{
						items.push_back({ texture.first, info, attached });
					}
				}
			}
			return items;
		}

		//a text in RML (its markup characters escaped)
		static std::string escape(const std::string& text)
		{
			std::string out;
			out.reserve(text.size());
			for (char c : text)
			{
				switch (c)
				{
				case '&': out += "&amp;"; break;
				case '<': out += "&lt;"; break;
				case '>': out += "&gt;"; break;
				case '"': out += "&quot;"; break;
				default:  out += c; break;
				}
			}
			return out;
		}

		//the markup of an option, its control "o<id>"
		static std::string option_rml(const DebugOption& option, size_t id)
		{
			const std::string control = "o" + std::to_string(id);
			const std::string name = "<span class=\"name\">" + escape(option.m_name) + "</span>";
			std::string rml;
			switch (option.m_type)
			{
			case DebugOption::Type::TOGGLE:
				rml = name + "<input type=\"checkbox\" id=\"" + control + "\"/>";
			break;
			case DebugOption::Type::CHOICE:
			{
				rml = name + "<select id=\"" + control + "\">";
				for (size_t i = 0; i != option.m_choices.size(); ++i)
				{
					rml += "<option value=\"" + std::to_string(i) + "\">" + escape(option.m_choices[i]) + "</option>";
				}
				rml += "</select>";
			}
			break;
			case DebugOption::Type::BUTTON:
				rml = "<button id=\"" + control + "\">" + escape(option.m_name) + "</button>";
			break;
			case DebugOption::Type::TEXT:
			default:
				rml = name + "<span class=\"text\" id=\"" + control + "\"></span>";
			break;
			}
			return "<div class=\"option\">" + rml + "</div>";
		}

		//an option that does nothing once its owner is gone
		template < typename T >
		static void guard(DebugOption& option, const Weak<T>& owner)
		{
			if (option.m_get)
			{
				auto get = option.m_get;
				option.m_get = [owner, get]() { return owner.lock() ? get() : 0; };
			}
			if (option.m_set)
			{
				auto set = option.m_set;
				option.m_set = [owner, set](int value) { if (owner.lock()) set(value); };
			}
			if (option.m_text)
			{
				auto text = option.m_text;
				option.m_text = [owner, text]() { return owner.lock() ? text() : std::string(); };
			}
		}

		//the name of a world among more of them ("" if it is the only one)
		static std::string world_prefix(size_t index, size_t count)
		{
			return count > 1 ? "World " + std::to_string(index + 1) + ": " : std::string();
		}

		//the render system's worlds alive
		static std::vector< Shared<RenderInstance> > worlds(Square::Context& context)
		{
			std::vector< Shared<RenderInstance> > instances;
			if (auto* render_system = System::get<RenderSystem>(context))
			{
				instances = render_system->instances();
			}
			return instances;
		}

		//the direction lights of a world
		static std::vector< Shared<Scene::DirectionLight> > direction_lights(const RenderInstance& world)
		{
			std::vector< Shared<Scene::DirectionLight> > lights;
			for (const Weak<Render::Light>& weak_light : world.collection().m_lights)
			{
				if (auto light = DynamicPointerCast<Scene::DirectionLight>(weak_light.lock()))
				{
					lights.push_back(light);
				}
			}
			return lights;
		}
	}

	DebugPanel::DebugPanel(Square::Context& context, UI::Context& ui, const std::vector<DebugProvider>& providers)
	: m_context(context)
	, m_ui(ui)
	, m_providers(providers)
	, m_profiler(std::make_unique<ProfilerPanel>(context))
	, m_previews(std::make_unique<TexturePreviews>(context))
	{
	}

	DebugPanel::~DebugPanel()
	{
		m_controls.clear();
		m_profiler->detach();
		if (m_document.valid()) m_document.close();
	}

	bool DebugPanel::create()
	{
		bool made = m_document.valid();
		if (!made && m_ui.valid())
		{
			m_document = m_ui.load(AuxDebugPanel::s_debug_rml);
			m_content = m_document.valid() ? m_document.find("content") : Element();
			made = m_content.valid();
			if (!made)
			{
				m_context.logger()->warning("UI: unable to load the debug panel " + std::string(AuxDebugPanel::s_debug_rml));
			}
			m_dirty = true;
		}
		return made;
	}

	void DebugPanel::show(bool visible)
	{
		if (visible && create())
		{
			m_visible = true;
			m_dirty = true;
			update();
			m_document.show();
		}
		else if (!visible)
		{
			m_visible = false;
			m_previews->active(false);
			if (m_document.valid()) m_document.hide();
		}
	}

	bool DebugPanel::visible() const
	{
		return m_visible;
	}

	void DebugPanel::profile(bool enable)
	{
		m_profiler->enable(enable);
	}

	bool DebugPanel::profiling() const
	{
		return m_profiler->enabled();
	}

	void DebugPanel::providers_changed()
	{
		m_dirty = true;
	}

	std::string DebugPanel::signature() const
	{
		using namespace AuxDebugPanel;
		std::ostringstream out;
		for (const Shared<RenderInstance>& world : worlds(m_context))
		{
			out << world.get() << '/' << world->debug_pass().get() << '/';
			for (const auto& effect : world->post_effects()) out << effect.get() << ',';
			for (const auto& light : direction_lights(*world)) out << light.get() << ',';
			out << ';';
		}
		//the pages of the textures shown
		out << textures_of(m_context, m_texture_filter, *m_previews).size();
		return out.str();
	}

	std::vector<DebugPanel::Tab> DebugPanel::tabs()
	{
		using namespace AuxDebugPanel;
		std::vector<Tab> list;
		list.push_back({ s_profiler, {} });
		list.push_back({ s_pipeline, {} });
		list.push_back({ s_draw, {} });
		list.push_back({ s_textures, {} });
		list.push_back({ s_ui, {} });
		pipeline_sections(list[1].m_sections);
		draw_sections(list[2].m_sections);
		textures_sections(list[3].m_sections);
		ui_sections(list[4].m_sections);
		//the sections of the game: in a tab of the engine, or in one of its own
		for (const DebugProvider& provider : m_providers)
		{
			auto tab = std::find_if(list.begin(), list.end(), [&](const Tab& other) { return other.m_name == provider.m_tab; });
			if (tab == list.end())
			{
				list.push_back({ provider.m_tab, {} });
				tab = list.end() - 1;
			}
			if (provider.m_sections)
			{
				provider.m_sections(tab->m_sections);
			}
		}
		return list;
	}

	void DebugPanel::rebuild()
	{
		using namespace AuxDebugPanel;
		//the tab shown (again after)
		int active = 0;
		if (Element old_tabs = m_document.find("debug_tabs"))
		{
			if (auto* tabset = dynamic_cast<Rml::ElementTabSet*>(old_tabs.native()))
			{
				active = tabset->GetActiveTab();
			}
		}
		//the markup: a tab each, its sections, their options
		m_controls.clear();
		m_captions.clear();
		m_profiler->detach();
		const std::vector<Tab> list = tabs();
		std::string rml = "<tabset id=\"debug_tabs\">";
		m_textures_tab = -1;
		for (const Tab& tab : list)
		{
			if (tab.m_name == s_textures)
			{
				m_textures_tab = int(&tab - list.data());
			}
			rml += "<tab>" + escape(tab.m_name) + "</tab><panel>";
			if (tab.m_name == s_profiler)
			{
				rml += ProfilerPanel::rml();
			}
			for (const DebugSection& section : tab.m_sections)
			{
				rml += "<div class=\"section\">";
				if (!section.m_title.empty())
				{
					rml += "<h3>" + escape(section.m_title) + "</h3>";
				}
				for (const DebugOption& option : section.m_options)
				{
					rml += option_rml(option, m_controls.size());
					m_controls.push_back({ option, Element(), -1, std::string() });
				}
				rml += "</div>";
			}
			if (tab.m_name == s_textures)
			{
				//the thumbnails: an image a slot, its caption
				rml += "<div class=\"thumbs\">";
				for (size_t slot = 0; slot != TexturePreviews::slots; ++slot)
				{
					rml += "<div class=\"thumb\"><img src=\"" + TexturePreviews::source(slot) + "\"/>"
					       "<div class=\"caption\" id=\"tc" + std::to_string(slot) + "\"></div></div>";
				}
				rml += "</div>";
				//the pages: back and forth
				rml += "<div class=\"pages\"><button id=\"tp_back\">&lt; Back</button>"
				       "<span id=\"tp_page\"></span><button id=\"tp_next\">Next &gt;</button></div>";
			}
			if (tab.m_name != s_profiler && tab.m_sections.empty())
			{
				rml += "<div class=\"empty\">nothing here</div>";
			}
			rml += "</panel>";
		}
		rml += "</tabset>";
		m_content.set_html(rml);
		//the controls, their events
		for (size_t i = 0; i != m_controls.size(); ++i)
		{
			m_controls[i].m_element = m_document.find("o" + std::to_string(i));
			bind(m_controls[i]);
		}
		m_profiler->attach(m_document);
		for (size_t slot = 0; slot != TexturePreviews::slots; ++slot)
		{
			m_captions.push_back(m_document.find("tc" + std::to_string(slot)));
		}
		//the arrows of the pages of the textures
		m_page_label = m_document.find("tp_page");
		if (Element back = m_document.find("tp_back"))
		{
			back.on(EventType::CLICK, [this](Event&) { m_texture_page = std::max(m_texture_page - 1, 0); });
		}
		if (Element next = m_document.find("tp_next"))
		{
			next.on(EventType::CLICK, [this](Event&) { m_texture_page = std::min(m_texture_page + 1, m_texture_pages - 1); });
		}
		if (Element new_tabs = m_document.find("debug_tabs"))
		{
			if (auto* tabset = dynamic_cast<Rml::ElementTabSet*>(new_tabs.native()))
			{
				tabset->SetActiveTab(std::clamp(active, 0, std::max(int(list.size()) - 1, 0)));
			}
		}
		m_signature = signature();
		m_dirty = false;
	}

	void DebugPanel::bind(Control& control)
	{
		if (control.m_element.valid())
		{
			const size_t index = size_t(&control - m_controls.data());
			Element element = control.m_element;
			switch (control.m_option.m_type)
			{
			case DebugOption::Type::TOGGLE:
				element.on(EventType::CHANGE, [this, index, element](Event&)
				{
					Control& changed = m_controls[index];
					const int value = element.has_attribute("checked") ? 1 : 0;
					if (value != changed.m_shown && changed.m_option.m_set)
					{
						changed.m_option.m_set(value);
						changed.m_shown = value;
					}
				});
			break;
			case DebugOption::Type::CHOICE:
				element.on(EventType::CHANGE, [this, index](Event& event)
				{
					Control& changed = m_controls[index];
					const std::string text = event.value();
					const int value = text.empty() ? changed.m_shown : std::atoi(text.c_str());
					if (value != changed.m_shown && changed.m_option.m_set)
					{
						changed.m_option.m_set(value);
						changed.m_shown = value;
					}
				});
			break;
			case DebugOption::Type::BUTTON:
				element.on(EventType::CLICK, [this, index](Event&)
				{
					Control& pressed = m_controls[index];
					if (pressed.m_option.m_set)
					{
						pressed.m_option.m_set(0);
					}
				});
			break;
			case DebugOption::Type::TEXT:
			default:
			break;
			}
		}
	}

	void DebugPanel::refresh(Control& control)
	{
		if (control.m_element.valid())
		{
			switch (control.m_option.m_type)
			{
			case DebugOption::Type::TOGGLE:
			{
				const int value = control.m_option.m_get ? control.m_option.m_get() : 0;
				if (value != control.m_shown)
				{
					control.m_shown = value;
					if (value) control.m_element.set_attribute("checked", "");
					else control.m_element.remove_attribute("checked");
				}
			}
			break;
			case DebugOption::Type::CHOICE:
			{
				const int value = control.m_option.m_get ? control.m_option.m_get() : 0;
				if (value != control.m_shown)
				{
					control.m_shown = value;
					control.m_element.set_attribute("value", std::to_string(value));
				}
			}
			break;
			case DebugOption::Type::TEXT:
			{
				const std::string text = control.m_option.m_text ? control.m_option.m_text() : std::string();
				if (text != control.m_text)
				{
					control.m_text = text;
					control.m_element.set_text(text);
				}
			}
			break;
			case DebugOption::Type::BUTTON:
			default:
			break;
			}
		}
	}

	void DebugPanel::update()
	{
		if (m_visible && m_document.valid())
		{
			if (m_dirty || signature() != m_signature)
			{
				rebuild();
			}
			for (Control& control : m_controls)
			{
				refresh(control);
			}
			m_profiler->update();
			update_textures();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//the sections of the engine
	void DebugPanel::pipeline_sections(std::vector<DebugSection>& sections) const
	{
		using namespace AuxDebugPanel;
		const std::vector< Shared<RenderInstance> > instances = worlds(m_context);
		for (size_t w = 0; w != instances.size(); ++w)
		{
			const Shared<RenderInstance>& world = instances[w];
			const Weak<RenderInstance> weak_world = world;
			RenderInstance* raw = world.get();
			const std::string prefix = world_prefix(w, instances.size());
			//its render
			DebugSection render{ prefix + "Render", {} };
			render.m_options.push_back(DebugOption::toggle("Shadows",
				[raw]() { return raw->shadows(); },
				[raw](bool value) { raw->shadows(value); }));
			render.m_options.push_back(DebugOption::toggle("Occlusion culling",
				[raw]() { return raw->occlusion().enabled; },
				[raw](bool value) { auto settings = raw->occlusion(); settings.enabled = value; raw->occlusion(settings); }));
			render.m_options.push_back(DebugOption::choice("Occlusion buffer", { "128", "256", "512" },
				[raw]() { return raw->occlusion().width <= 128 ? 0 : raw->occlusion().width <= 256 ? 1 : 2; },
				[raw](int value) { auto settings = raw->occlusion(); settings.width = 128 << std::clamp(value, 0, 2); raw->occlusion(settings); }));
			render.m_options.push_back(DebugOption::choice("Level of detail", { "By the camera", "0", "1", "2", "3" },
				[raw]() { return std::clamp(raw->levels_of_detail().m_force_level + 1, 0, 4); },
				[raw](int value) { auto settings = raw->levels_of_detail(); settings.m_force_level = value - 1; raw->levels_of_detail(settings); }));
			for (DebugOption& option : render.m_options) guard(option, weak_world);
			sections.push_back(render);
			//its direction lights
			for (const Shared<Scene::DirectionLight>& light : direction_lights(*world))
			{
				const Weak<Scene::DirectionLight> weak_light = light;
				Scene::DirectionLight* sun = light.get();
				std::string name = "Light";
				if (auto actor = light->actor().lock())
				{
					name = "Light " + actor->name();
				}
				DebugSection section{ prefix + name, {} };
				section.m_options.push_back(DebugOption::toggle("Visible",
					[sun]() { return sun->visible(); },
					[sun](bool value) { sun->visible(value); }));
				section.m_options.push_back(DebugOption::choice("Shadow filter", { "None", "PCF", "PCSS" },
					[sun]() { return int(sun->shadow_filter()); },
					[sun](int value) { sun->shadow_filter(Render::ShadowFilter(value)); }));
				section.m_options.push_back(DebugOption::choice("Cascades", { "1", "2", "3", "4", "5", "6", "7", "8" },
					[sun]() { return std::clamp(sun->cascades() - 1, 0, 7); },
					[sun](int value) { sun->cascades(value + 1); }));
				section.m_options.push_back(DebugOption::choice("Cascade fit", { "Follow", "Stable" },
					[sun]() { return int(sun->cascade_fit()); },
					[sun](int value) { sun->cascade_fit(Scene::CascadeFit(value)); }));
				for (DebugOption& option : section.m_options) guard(option, weak_light);
				sections.push_back(section);
			}
			//its post effects: on or off, their options
			for (const Shared<Render::PostEffect>& effect : world->post_effects())
			{
				const Weak<Render::PostEffect> weak_effect = effect;
				Render::PostEffect* post = effect.get();
				DebugSection section{ prefix + "Post " + effect->object_name(), {} };
				section.m_options.push_back(DebugOption::toggle("Enabled",
					[post]() { return post->enabled(); },
					[post](bool value) { post->enabled(value); }));
				effect->debug_options(section.m_options);
				for (DebugOption& option : section.m_options) guard(option, weak_effect);
				sections.push_back(section);
			}
		}
	}

	void DebugPanel::draw_sections(std::vector<DebugSection>& sections) const
	{
		using namespace AuxDebugPanel;
		const std::vector< Shared<RenderInstance> > instances = worlds(m_context);
		for (size_t w = 0; w != instances.size(); ++w)
		{
			const Shared<RenderInstance>& world = instances[w];
			const std::string prefix = world_prefix(w, instances.size());
			const Shared<Render::DrawerPassDebug> pass = world->debug_pass();
			DebugSection draw{ prefix + "Draw", {} };
			if (pass)
			{
				const Weak<Render::DrawerPassDebug> weak_pass = pass;
				Render::DrawerPassDebug* raw = pass.get();
				auto flag = [&](const std::string& name, unsigned short flags)
				{
					draw.m_options.push_back(DebugOption::toggle(name,
						[raw, flags]() { return (raw->draw_flags() & flags) == flags; },
						[raw, flags](bool value) { raw->draw_flags((unsigned short)(value ? raw->draw_flags() | flags : raw->draw_flags() & ~flags)); }));
				};
				flag("Bounding boxes", Render::DF_DRAW_OBB);
				flag("Camera frustum", Render::DF_DRAW_FUSTRUM);
				flag("Spot light volumes", Render::DF_DRAW_SPOT_LIGHT);
				flag("Point light volumes", Render::DF_DRAW_POINT_LIGHT);
				flag("Occlusion (CPU raster, hidden boxes)", Render::DF_DRAW_OCCLUSION);
				for (DebugOption& option : draw.m_options) guard(option, weak_pass);
			}
			else
			{
				draw.m_options.push_back(DebugOption::text("Debug pass", []() { return std::string("none (the pipeline without RP_DEBUG)"); }));
			}
			//the shadows of the direction lights: a color a cascade
			for (const Shared<Scene::DirectionLight>& light : direction_lights(*world))
			{
				const Weak<Scene::DirectionLight> weak_light = light;
				Scene::DirectionLight* sun = light.get();
				std::string name = "Shadow cascades (colors)";
				if (auto actor = light->actor().lock())
				{
					name += " " + actor->name();
				}
				DebugOption option = DebugOption::toggle(name,
					[sun]() { return sun->cascade_colors(); },
					[sun](bool value) { sun->cascade_colors(value); });
				guard(option, weak_light);
				draw.m_options.push_back(option);
			}
			sections.push_back(draw);
			//what the occlusion did
			const Weak<RenderInstance> weak_world = world;
			RenderInstance* raw_world = world.get();
			DebugSection occlusion{ prefix + "Occlusion", {} };
			occlusion.m_options.push_back(DebugOption::text("Occluders", [raw_world]()
			{
				const auto stats = raw_world->occlusion_stats();
				return std::to_string(stats.m_occluders) + " (" + std::to_string(stats.m_triangles) + " triangles)";
			}));
			occlusion.m_options.push_back(DebugOption::text("Hidden", [raw_world]()
			{
				const auto stats = raw_world->occlusion_stats();
				return std::to_string(stats.m_hidden) + " of " + std::to_string(stats.m_tested) + " tested";
			}));
			occlusion.m_options.push_back(DebugOption::text("Instances hidden", [raw_world]()
			{
				const auto stats = raw_world->occlusion_stats();
				return std::to_string(stats.m_instances_hidden) + " of " + std::to_string(stats.m_instances_tested) + " tested";
			}));
			for (DebugOption& option : occlusion.m_options) guard(option, weak_world);
			sections.push_back(occlusion);
		}
	}

	bool DebugPanel::textures_shown() const
	{
		bool shown = false;
		if (m_visible && m_textures_tab >= 0)
		{
			if (Element tabs = m_document.find("debug_tabs"))
			{
				if (auto* tabset = dynamic_cast<Rml::ElementTabSet*>(tabs.native()))
				{
					shown = tabset->GetActiveTab() == m_textures_tab;
				}
			}
		}
		return shown;
	}

	void DebugPanel::update_textures()
	{
		const bool shown = textures_shown();
		m_previews->active(shown);
		if (shown)
		{
			//the page of the kind shown, a caption each
			const std::vector<TexturePreviews::Item> all = AuxDebugPanel::textures_of(m_context, m_texture_filter, *m_previews);
			const size_t first = size_t(std::max(m_texture_page, 0)) * TexturePreviews::slots;
			std::vector<TexturePreviews::Item> page;
			for (size_t i = first; i < all.size() && page.size() != TexturePreviews::slots; ++i)
			{
				page.push_back(all[i]);
			}
			m_previews->items(page);
			const std::string label = "page " + std::to_string(m_texture_page + 1) + " of " + std::to_string(m_texture_pages);
			if (m_page_label.valid() && m_page_label.text() != label)
			{
				m_page_label.set_text(label);
			}
			for (size_t slot = 0; slot != m_captions.size(); ++slot)
			{
				std::string caption;
				if (slot < page.size())
				{
					const Render::TextureInfo& info = page[slot].m_info;
					caption = AuxDebugPanel::format_name(info.m_format) + " " + std::to_string(info.m_width) + " x " + std::to_string(info.m_height);
					if (info.m_shape == Render::TS_TEXTURE_ARRAY) caption += ", " + std::to_string(info.m_layers) + " layers";
					if (info.m_shape == Render::TS_TEXTURE_CUBE) caption += ", cube";
				}
				if (m_captions[slot].valid() && m_captions[slot].text() != caption)
				{
					m_captions[slot].set_text(caption);
				}
			}
		}
	}

	void DebugPanel::draw(Render::Context& render)
	{
		m_previews->draw(render);
	}

	Render::Texture* DebugPanel::preview(const std::string& source, IVec2& size)
	{
		Render::Texture* texture = nullptr;
		size_t slot = 0;
		if (TexturePreviews::slot_of(source, slot))
		{
			texture = m_previews->texture(slot);
			size = IVec2(TexturePreviews::width, TexturePreviews::height);
		}
		return texture;
	}

	void DebugPanel::textures_sections(std::vector<DebugSection>& sections)
	{
		DebugSection section{ "Textures of the driver", {} };
		const size_t count = AuxDebugPanel::textures_of(m_context, m_texture_filter, *m_previews).size();
		const size_t pages = std::max<size_t>((count + TexturePreviews::slots - 1) / TexturePreviews::slots, 1);
		m_texture_pages = int(pages);
		m_texture_page = std::min(m_texture_page, m_texture_pages - 1);
		std::vector<std::string> numbers;
		for (size_t page = 0; page != pages; ++page)
		{
			numbers.push_back(std::to_string(page + 1) + " of " + std::to_string(pages));
		}
		section.m_options.push_back(DebugOption::choice("Show", { "Render targets", "Shadow maps (depth, arrays, cubes)", "Images" },
			[this]() { return m_texture_filter; },
			[this](int value) { m_texture_filter = value; m_texture_page = 0; m_dirty = true; }));
		section.m_options.push_back(DebugOption::choice("Page", numbers,
			[this]() { return m_texture_page; },
			[this](int value) { m_texture_page = value; }));
		section.m_options.push_back(DebugOption::text("Count", [count]()
		{
			return std::to_string(count) + " textures";
		}));
		auto* render_system = System::get<RenderSystem>(m_context);
		if (!render_system || !render_system->render() || !render_system->render()->inspector())
		{
			section.m_options.push_back(DebugOption::text("Registry", []() { return std::string("none (TEXTURE_INTROSPECTION off)"); }));
		}
		sections.push_back(section);
	}

	void DebugPanel::ui_sections(std::vector<DebugSection>& sections) const
	{
		UI::Context* ui = &m_ui;
		DebugSection section{ "RmlUi", {} };
		section.m_options.push_back(DebugOption::toggle("Debugger",
			[ui]() { return ui->debugger(); },
			[ui](bool value) { ui->debugger(value); }));
		section.m_options.push_back(DebugOption::text("Size", [ui]()
		{
			const IVec2 size = ui->size();
			return std::to_string(size.x) + " x " + std::to_string(size.y);
		}));
		sections.push_back(section);
	}
}
}
