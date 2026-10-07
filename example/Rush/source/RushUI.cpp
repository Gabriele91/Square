//
//  RushUI.cpp
//  Rush
//
//  See RushUI.h.
//
#include <algorithm>
#include <cctype>
#include <cmath>
#include <RushUI.h>
#include <Race.h>
#include <Graphics.h>
#include <RushConfig.h>

using namespace Rush;

namespace AuxRushUI
{
	//the Esc menu of a race: the demo one, or the pause
#if defined(RUSH_DEMO)
	const char* s_menu_document = "example/Rush/assets/ui.sqz/menu.rml";
#else
	const char* s_menu_document = "example/Rush/assets/ui.sqz/pause.rml";
#endif
}

RushUI::RushUI(Square::Context& context)
: m_context(context)
{
}

bool RushUI::create()
{
	using namespace Square;
	auto* ui_system = System::get<UISystem>(m_context);
	if (!ui_system || !ui_system->ui().valid()) return false;
	//the documents made for 1920x1080: scaled to the window, their proportions kept
	ui_system->reference_size({ 1920, 1080 });
	UI::Context::load_font("common/ui/LatoLatin-Regular.ttf");
	UI::Context::load_font("common/ui/LatoLatin-Bold.ttf");
	//the model (before the documents)
	m_model = ui_system->ui().create_data_model("rush");
	m_state.m_winning_score = Config::get().rules().m_winning_score;
	m_model.bind("winning_score", &m_state.m_winning_score);
	m_model.bind("score_red", &m_state.m_scores[0]);
	m_model.bind("score_blue", &m_state.m_scores[1]);
	m_model.bind("score_green", &m_state.m_scores[2]);
	m_model.bind("score_yellow", &m_state.m_scores[3]);
	m_model.bind("speed", &m_state.m_speed);
	m_model.bind("speed_bar", &m_state.m_speed_bar);
	m_model.bind("circuit", &m_state.m_circuit);
	m_model.bind("lap", &m_state.m_lap);
	m_model.bind("place", &m_state.m_place);
	m_model.bind("place_suffix", &m_state.m_place_suffix);
	m_model.bind("race_time", &m_state.m_race_time);
	m_model.bind("wrong_way", &m_state.m_wrong_way);
	for (size_t row = 0; row != s_racers; ++row)
	{
		m_model.bind("standing_" + std::to_string(row + 1), &m_state.m_standing_colors[row]);
		m_model.bind("standing_name_" + std::to_string(row + 1), &m_state.m_standing_names[row]);
	}
	m_model.bind("message", &m_state.m_message);
	m_model.bind("message_visible", &m_state.m_message_visible);
	m_model.bind("result", &m_state.m_result);
	m_model.bind("result_visible", &m_state.m_result_visible);
	m_model.bind("fps", &m_state.m_fps);
	m_model.bind("ssr", &m_state.m_ssr.m_value);
	m_model.bind("bloom", &m_state.m_bloom.m_value);
	m_model.bind("ssao", &m_state.m_ssao.m_value);
	m_model.bind("fullscreen", &m_state.m_fullscreen.m_value);
	//the settings
	m_model.bind("set_fullscreen", &m_state.m_settings.m_fullscreen);
	m_model.bind("set_resolution", &m_state.m_settings.m_resolution);
	m_model.bind("set_fps", &m_state.m_settings.m_show_fps);
	m_model.bind("show_fps", &m_state.m_applied.m_show_fps);
	m_model.bind("set_reflections", &m_state.m_settings.m_reflections);
	m_model.bind("set_occlusion", &m_state.m_settings.m_occlusion);
	m_model.bind("set_shadows", &m_state.m_settings.m_shadows);
	m_model.bind("set_bloom", &m_state.m_settings.m_bloom);
	m_model.bind("set_weather", &m_state.m_settings.m_weather);
	m_model.bind("set_antialiasing", &m_state.m_settings.m_antialiasing);
	m_model.bind("set_motion_blur", &m_state.m_settings.m_motion_blur);
	m_model.bind("set_god_rays", &m_state.m_settings.m_god_rays);
	return true;
}

void RushUI::load_documents()
{
	using namespace Square;
	auto* ui_system = System::get<UISystem>(m_context);
	if (!ui_system || !m_model.valid()) return;
	UI::Context& ui = ui_system->ui();
	m_title = ui.load("example/Rush/assets/ui.sqz/title.rml");
	m_hud = ui.load("example/Rush/assets/ui.sqz/hud.rml");
	m_menu = ui.load(AuxRushUI::s_menu_document);
	setup_title();
	m_menu.find("resume").on(UI::EventType::CLICK, [this](UI::Event&) { menu(false); });
	m_menu.find("exit").on(UI::EventType::CLICK, [this](UI::Event&) { if (m_on_exit) m_on_exit(); });
}

Square::UI::DataModel& RushUI::model()
{
	return m_model;
}

Square::UI::Document& RushUI::menu_document()
{
	return m_menu;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//title
void RushUI::setup_title()
{
	using namespace Square;
	static const char* s_main_ids[MAIN_COUNT]{ "item_play", "item_settings", "item_about", "item_exit" };
	m_main_items.clear();
	m_main_items.reserve(MAIN_COUNT);
	for (int item = 0; item != MAIN_COUNT; ++item)
	{
		UI::Element element = m_title.find(s_main_ids[item]);
		element.on(UI::EventType::MOUSEOVER, [this, item](UI::Event&) { main_select(item); });
		element.on(UI::EventType::CLICK, [this, item](UI::Event&) { main_activate(item); });
		m_main_items.push_back(element);
	}
	//the modes
	m_modes.clear();
	m_modes.reserve(MODE_COUNT);
	for (int mode = 0; mode != MODE_COUNT; ++mode)
	{
		UI::Element element = m_title.find("mode_" + std::to_string(mode));
		element.on(UI::EventType::MOUSEOVER, [this, mode](UI::Event&) { mode_select(mode); });
		element.on(UI::EventType::CLICK, [this, mode](UI::Event&) { mode_activate(mode); });
		m_modes.push_back(element);
	}
	//the maps: a card on the wheel (a click: in front, again: played), its words
	m_map_cards.clear();
	m_map_infos.clear();
	const size_t arenas = Config::get().arenas().size();
	m_map_cards.reserve(arenas);
	m_map_infos.reserve(arenas);
	for (size_t map = 0; map != arenas; ++map)
	{
		UI::Element element = m_title.find("map_" + std::to_string(map));
		element.on(UI::EventType::CLICK, [this, map](UI::Event&)
		{
			if (m_map == map) map_play(map);
			else              map_select(map);
		});
		m_map_cards.push_back(element);
		m_map_infos.push_back(m_title.find("info_" + std::to_string(map)));
	}
	m_title.find("map_play").on(UI::EventType::CLICK, [this](UI::Event&) { map_play(m_map); });
	m_title.find("settings_back").on(UI::EventType::CLICK, [this](UI::Event&) { screen(Screen::MAIN); });
	m_title.find("about_back").on(UI::EventType::CLICK, [this](UI::Event&) { screen(Screen::MAIN); });
	main_select(MAIN_PLAY);
	mode_select(MODE_ARENA);
	map_select(m_map);
}

void RushUI::title(bool show)
{
	if (show)
	{
		m_menu.hide();
		m_hud.hide();
		m_title.show();
		screen(Screen::MAIN);
		main_select(MAIN_PLAY);
		return;
	}
	m_title.hide();
	m_hud.show();
}

void RushUI::hud(bool show)
{
	if (show) m_hud.show();
	else      m_hud.hide();
}

void RushUI::screen(Screen screen)
{
	m_screen = screen;
	m_title.find("screen_main").set_class("hidden", screen != Screen::MAIN);
	m_title.find("screen_modes").set_class("hidden", screen != Screen::MODES);
	m_title.find("screen_arena").set_class("hidden", screen != Screen::ARENA);
	m_title.find("screen_settings").set_class("hidden", screen != Screen::SETTINGS);
	m_title.find("screen_about").set_class("hidden", screen != Screen::ABOUT);
}

void RushUI::title_key(Square::Video::KeyboardEvent key)
{
	using namespace Square;
	const bool previous = key == Video::KEY_LEFT || key == Video::KEY_UP;
	const bool next = key == Video::KEY_RIGHT || key == Video::KEY_DOWN;
	const bool enter = key == Video::KEY_ENTER || key == Video::KEY_KP_ENTER;
	const bool back = key == Video::KEY_ESCAPE || key == Video::KEY_BACKSPACE;
	switch (m_screen)
	{
	case Screen::MAIN:
		if (previous) main_select((m_main_selected + MAIN_COUNT - 1) % MAIN_COUNT);
		if (next)     main_select((m_main_selected + 1) % MAIN_COUNT);
		if (enter)    main_activate(m_main_selected);
	break;
	case Screen::MODES:
		if (previous) mode_select((m_mode + MODE_COUNT - 1) % MODE_COUNT);
		if (next)     mode_select((m_mode + 1) % MODE_COUNT);
		if (enter)    mode_activate(m_mode);
		if (back)     screen(Screen::MAIN);
	break;
	case Screen::ARENA:
	{
		const size_t arenas = Config::get().arenas().size();
		if (previous) map_select((m_map + arenas - 1) % arenas);
		if (next)     map_select((m_map + 1) % arenas);
		if (enter)    map_play(m_map);
		if (back)     screen(Screen::MODES);
	}
	break;
	case Screen::SETTINGS:
	case Screen::ABOUT:
	default:
		if (back) screen(Screen::MAIN);
	break;
	}
}

void RushUI::main_select(int item)
{
	m_main_selected = item;
	for (int id = 0; id != int(m_main_items.size()); ++id)
	{
		m_main_items[id].set_class("selected", id == item);
	}
}

void RushUI::main_activate(int item)
{
	switch (item)
	{
	case MAIN_PLAY:     screen(Screen::MODES); break;
	case MAIN_SETTINGS: screen(Screen::SETTINGS); break;
	case MAIN_ABOUT:    screen(Screen::ABOUT); break;
	case MAIN_EXIT:     if (m_on_quit) m_on_quit(); break;
	default: break;
	}
}

void RushUI::mode_select(int mode)
{
	m_mode = mode;
	for (int id = 0; id != int(m_modes.size()); ++id)
	{
		m_modes[id].set_class("selected", id == mode);
	}
}

void RushUI::mode_activate(int mode)
{
	mode_select(mode);
	//races: the circuit at once; the arena: its maps (battle: to come)
	switch (mode)
	{
	case MODE_RACES: circuit_play(0); break;
	case MODE_ARENA: screen(Screen::ARENA); break;
	default: break;
	}
}

void RushUI::map_select(size_t map)
{
	const auto& arenas = Config::get().arenas();
	if (map >= arenas.size()) return;
	m_map = map;
	m_race_map = &arenas[map];
	//the wheel: the one in front, the one before over it, the one after under it, the rest
	//behind (hidden)
	const size_t count = m_map_cards.size();
	for (size_t id = 0; id != count; ++id)
	{
		const size_t slot = (id + count - map) % count;
		const bool front = slot == 0;
		const bool after = slot == 1;
		const bool before = slot == count - 1 && count > 2;
		m_map_cards[id].set_class("slot_cur", front);
		m_map_cards[id].set_class("slot_next", after);
		m_map_cards[id].set_class("slot_prev", before);
		m_map_cards[id].set_class("slot_far", !front && !after && !before);
	}
	for (size_t id = 0; id != m_map_infos.size(); ++id)
	{
		m_map_infos[id].set_class("shown", id == map);
	}
}

void RushUI::map_play(size_t map)
{
	map_select(map);
	if (m_on_play) m_on_play();
}

void RushUI::circuit_play(size_t circuit)
{
	const auto& circuits = Config::get().circuits();
	if (circuits.empty()) return;
	m_race_map = &circuits[circuit % circuits.size()];
	if (m_on_play) m_on_play();
}

void RushUI::on_play(const Callback& callback)
{
	m_on_play = callback;
}

void RushUI::on_quit(const Callback& callback)
{
	m_on_quit = callback;
}

const RaceMap& RushUI::race_map() const
{
	return *m_race_map;
}

bool RushUI::race_map(const std::string& name)
{
	const auto& arenas = Config::get().arenas();
	for (size_t map = 0; map != arenas.size(); ++map)
	{
		if (name != arenas[map].m_name) continue;
		map_select(map);
		return true;
	}
	for (const RaceMap& circuit : Config::get().circuits())
	{
		if (name != circuit.m_name) continue;
		m_race_map = &circuit;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//menu
void RushUI::menu(bool show)
{
	if (show) m_menu.show(true);
	else      m_menu.hide();
}

bool RushUI::menu_visible() const
{
	return m_menu.visible();
}

void RushUI::on_exit(const Callback& callback)
{
	m_on_exit = callback;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//update
const GameSettings& RushUI::settings() const
{
	return m_state.m_applied;
}

void RushUI::load_settings(Graphics& graphics)
{
	m_state.m_settings.load();
	//the levels of the effects: the options of their selects (config/graphics.json, "Off" first),
	//made now, the one read selected (a select sets its value to the model as it is made)
	const std::pair<const char*, int> chosen[]
	{
		  { "reflections", m_state.m_settings.m_reflections }, { "occlusion", m_state.m_settings.m_occlusion }
		, { "shadows", m_state.m_settings.m_shadows }, { "bloom", m_state.m_settings.m_bloom }
		, { "motion_blur", m_state.m_settings.m_motion_blur }, { "god_rays", m_state.m_settings.m_god_rays }
	};
	for (const auto& effect : chosen)
	{
		Square::UI::Element holder = m_title.find(std::string("levels_") + effect.first);
		if (!holder) continue;
		const auto& levels = Config::get().levels(effect.first);
		std::string rml = std::string("<select data-value=\"set_") + effect.first + "\">";
		for (size_t level = 0; level <= levels.size(); ++level)
		{
			const std::string title = level ? levels[level - 1].m_title : std::string("Off");
			const std::string selected = int(level) == effect.second ? " selected" : "";
			rml += "<option value=\"" + std::to_string(level) + "\"" + selected + ">" + title + "</option>";
		}
		rml += "</select>";
		holder.set_html(rml);
	}
	m_model.dirty_all();
	m_state.m_applied = m_state.m_settings;
	m_state.m_applied.apply_window();
	m_state.m_applied.apply_effects(graphics);
}

void RushUI::update(const Race* race, Graphics& graphics, float fps)
{
	if (!m_model.valid()) return;
	m_state.m_fps = fps;
	if (race) update_hud(*race);
	update_options(graphics);
	m_model.dirty_all();
}

namespace AuxRushUI
{
	//a name of a racer in the standings (upper case)
	static std::string upper(std::string name)
	{
		for (char& c : name)
		{
			c = char(std::toupper((unsigned char)c));
		}
		return name;
	}

	//1st, 2nd, 3rd, 4th
	static const char* suffix(size_t place)
	{
		switch (place)
		{
		case 1:  return "ST";
		case 2:  return "ND";
		case 3:  return "RD";
		default: return "TH";
		}
	}

	//m:ss.d
	static std::string time(double seconds)
	{
		const int tenths = int(seconds * 10.0);
		const int minutes = tenths / 600;
		const int rest = tenths % 600;
		std::string text = std::to_string(minutes) + ":";
		if (rest < 100) text += "0";
		return text + std::to_string(rest / 10) + "." + std::to_string(rest % 10);
	}
}

void RushUI::update_circuit(const Race& race)
{
	const auto& player = race.racers().front();
	const size_t place = race.place(0);
	m_state.m_lap = std::to_string(std::min(player.m_lap, race.laps())) + "/" + std::to_string(race.laps());
	m_state.m_place = std::to_string(place);
	m_state.m_place_suffix = AuxRushUI::suffix(place);
	m_state.m_race_time = AuxRushUI::time(player.m_finished ? player.m_finish_time : race.race_time());
	const auto& standings = race.standings();
	for (size_t row = 0; row != s_racers; ++row)
	{
		const size_t id = row < standings.size() ? standings[row] : row;
		const Racer& racer = Config::get().racer(id);
		m_state.m_standing_colors[row] = racer.m_color;
		m_state.m_standing_names[row] = AuxRushUI::upper(racer.m_name);
	}
	m_state.m_wrong_way = race.wrong_way() && race.phase() == Race::Phase::PLAY;
}

void RushUI::update_hud(const Race& race)
{
	//scores, speed of the player
	const auto& racers = race.racers();
	for (size_t id = 0; id < racers.size() && id < s_racers; ++id)
	{
		m_state.m_scores[id] = racers[id].m_score;
	}
	m_state.m_speed = race.player_speed();
	//(the bar full at the top speed: a circuit's 130)
	const int top = race.circuit() ? int(std::round(Config::get().circuit().m_speed * 100.0f)) : 100;
	m_state.m_speed_bar = std::to_string(std::clamp(m_state.m_speed * 100 / top, 0, 100)) + "%";
	//a circuit: the lap, the place, the time, the order of the racers
	m_state.m_circuit = race.circuit();
	if (m_state.m_circuit) update_circuit(race);
	//the phase: the countdown of the start, GO! for a while, the result of the end
	m_state.m_message_visible = false;
	m_state.m_result_visible = false;
	switch (race.phase())
	{
	case Race::Phase::START:
	{
		const double left = Config::get().rules().m_start_time - race.phase_time();
		m_state.m_message = std::to_string(int(std::ceil(left)));
		m_state.m_message_visible = true;
	}
	break;
	case Race::Phase::PLAY:
		m_state.m_message = "GO!";
		m_state.m_message_visible = race.phase_time() < Config::get().rules().m_go_time;
	break;
	case Race::Phase::END:
		if (race.circuit()) m_state.m_result = m_state.m_place + m_state.m_place_suffix + " PLACE";
		else                m_state.m_result = race.winner() == 0 ? "YOU WIN!" : "YOU LOSE!";
		m_state.m_result_visible = true;
	break;
	default: break;
	}
}

void RushUI::update_options(Graphics& graphics)
{
	using namespace Square;
	//the settings changed (a control of Settings): applied, saved
	if (m_state.m_settings != m_state.m_applied)
	{
		const bool window = m_state.m_settings.m_fullscreen != m_state.m_applied.m_fullscreen
		                 || m_state.m_settings.m_resolution != m_state.m_applied.m_resolution;
		m_state.m_applied = m_state.m_settings;
		if (window) m_state.m_applied.apply_window();
		m_state.m_applied.apply_effects(graphics);
		m_state.m_applied.save();
	}
	auto ssr = graphics.ssr();
	auto bloom = graphics.bloom();
	auto ssao = graphics.ssao();
	auto* app = Application::instance();
	sync_option(m_state.m_ssr, ssr && ssr->enabled(), [ssr](bool value) { if (ssr) ssr->enabled(value); });
	sync_option(m_state.m_bloom, bloom && bloom->enabled(), [bloom](bool value) { if (bloom) bloom->enabled(value); });
	sync_option(m_state.m_ssao, ssao && ssao->enabled(), [ssao](bool value) { if (ssao) ssao->enabled(value); });
	sync_option(m_state.m_fullscreen, app && app->fullscreen(), [app](bool value) { if (app) app->fullscreen(value); });
}
