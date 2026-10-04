//
//  RushUI.cpp
//  Rush
//
//  See RushUI.h.
//
#include <cmath>
#include <RushUI.h>
#include <Race.h>
#include <Graphics.h>

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
	UI::Context::load_font("common/ui/LatoLatin-Regular.ttf");
	UI::Context::load_font("common/ui/LatoLatin-Bold.ttf");
	//the model (before the documents)
	m_model = ui_system->ui().create_data_model("rush");
	m_model.bind("winning_score", &m_state.m_winning_score);
	m_model.bind("score_red", &m_state.m_scores[0]);
	m_model.bind("score_blue", &m_state.m_scores[1]);
	m_model.bind("score_green", &m_state.m_scores[2]);
	m_model.bind("score_yellow", &m_state.m_scores[3]);
	m_model.bind("speed", &m_state.m_speed);
	m_model.bind("message", &m_state.m_message);
	m_model.bind("message_visible", &m_state.m_message_visible);
	m_model.bind("result", &m_state.m_result);
	m_model.bind("result_visible", &m_state.m_result_visible);
	m_model.bind("fps", &m_state.m_fps);
	m_model.bind("ssr", &m_state.m_ssr.m_value);
	m_model.bind("bloom", &m_state.m_bloom.m_value);
	m_model.bind("ssao", &m_state.m_ssao.m_value);
	m_model.bind("fullscreen", &m_state.m_fullscreen.m_value);
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
	static const char* s_item_ids[TITLE_COUNT]{ "title_play", "title_instructions", "title_about", "title_exit" };
	m_title_items.clear();
	m_title_items.reserve(TITLE_COUNT);
	for (int item = 0; item != TITLE_COUNT; ++item)
	{
		UI::Element element = m_title.find(s_item_ids[item]);
		element.on(UI::EventType::MOUSEOVER, [this, item](UI::Event&) { title_select(item); });
		element.on(UI::EventType::CLICK, [this, item](UI::Event&) { title_activate(item); });
		m_title_items.push_back(element);
	}
	title_select(TITLE_PLAY);
	//the cards of the maps
	m_map_cards.clear();
	m_map_cards.reserve(s_race_maps_count);
	for (size_t map = 0; map != s_race_maps_count; ++map)
	{
		UI::Element element = m_title.find("map_" + std::to_string(map));
		element.on(UI::EventType::MOUSEOVER, [this, map](UI::Event&) { map_select(map); });
		element.on(UI::EventType::CLICK, [this, map](UI::Event&) { map_play(map); });
		m_map_cards.push_back(element);
	}
}

void RushUI::title(bool show)
{
	if (show)
	{
		m_menu.hide();
		m_hud.hide();
		m_title.show();
		title_panel(TITLE_COUNT);
		maps(false);
		title_select(TITLE_PLAY);
		return;
	}
	m_title.hide();
	m_hud.show();
}

void RushUI::title_key(Square::Video::KeyboardEvent key)
{
	using namespace Square;
	if (m_maps_shown)
	{
		maps_key(key);
		return;
	}
	switch (key)
	{
	case Video::KEY_LEFT:
	case Video::KEY_UP:
		title_select((m_title_selected + TITLE_COUNT - 1) % TITLE_COUNT);
	break;
	case Video::KEY_RIGHT:
	case Video::KEY_DOWN:
		title_select((m_title_selected + 1) % TITLE_COUNT);
	break;
	case Video::KEY_ENTER:
	case Video::KEY_KP_ENTER:
		title_activate(m_title_selected);
	break;
	default: break;
	}
}

void RushUI::title_select(int item)
{
	m_title_selected = item;
	for (int id = 0; id != int(m_title_items.size()); ++id)
	{
		m_title_items[id].set_class("selected", id == item);
	}
}

void RushUI::title_activate(int item)
{
	switch (item)
	{
	case TITLE_PLAY:
		maps(true);
	break;
	case TITLE_INSTRUCTIONS:
	case TITLE_ABOUT:
		title_panel(item);
	break;
	case TITLE_EXIT:
		if (m_on_quit) m_on_quit();
	break;
	default: break;
	}
}

void RushUI::title_panel(int item)
{
	m_title.find("panel_instructions").set_class("hidden", item != TITLE_INSTRUCTIONS);
	m_title.find("panel_about").set_class("hidden", item != TITLE_ABOUT);
}

void RushUI::maps(bool show)
{
	m_maps_shown = show;
	m_title.find("title_maps").set_class("hidden", !show);
	m_title.find("title_items").set_class("hidden", show);
	if (show) title_panel(TITLE_COUNT);
	map_select(m_map);
}

void RushUI::maps_key(Square::Video::KeyboardEvent key)
{
	using namespace Square;
	switch (key)
	{
	case Video::KEY_LEFT:
	case Video::KEY_UP:
		map_select((m_map + s_race_maps_count - 1) % s_race_maps_count);
	break;
	case Video::KEY_RIGHT:
	case Video::KEY_DOWN:
		map_select((m_map + 1) % s_race_maps_count);
	break;
	case Video::KEY_ENTER:
	case Video::KEY_KP_ENTER:
		map_play(m_map);
	break;
	case Video::KEY_ESCAPE:
		maps(false);
	break;
	default: break;
	}
}

void RushUI::map_select(size_t map)
{
	m_map = map;
	for (size_t id = 0; id != m_map_cards.size(); ++id)
	{
		m_map_cards[id].set_class("selected", id == map);
	}
}

void RushUI::map_play(size_t map)
{
	map_select(map);
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
	return s_race_maps[m_map];
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
void RushUI::update(const Race* race, const Graphics& graphics, float fps)
{
	if (!m_model.valid()) return;
	m_state.m_fps = fps;
	if (race) update_hud(*race);
	update_options(graphics);
	m_model.dirty_all();
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
	//the phase: the countdown of the start, GO! for a while, the result of the end
	m_state.m_message_visible = false;
	m_state.m_result_visible = false;
	switch (race.phase())
	{
	case Race::Phase::START:
		m_state.m_message = std::to_string(int(std::ceil(s_start_time - race.phase_time())));
		m_state.m_message_visible = true;
	break;
	case Race::Phase::PLAY:
		m_state.m_message = "GO!";
		m_state.m_message_visible = race.phase_time() < s_go_time;
	break;
	case Race::Phase::END:
		m_state.m_result = race.winner() == 0 ? "YOU WIN!" : "YOU LOSE!";
		m_state.m_result_visible = true;
	break;
	default: break;
	}
}

void RushUI::update_options(const Graphics& graphics)
{
	using namespace Square;
	auto ssr = graphics.ssr();
	auto bloom = graphics.bloom();
	auto ssao = graphics.ssao();
	auto* app = Application::instance();
	sync_option(m_state.m_ssr, ssr && ssr->enabled(), [ssr](bool value) { if (ssr) ssr->enabled(value); });
	sync_option(m_state.m_bloom, bloom && bloom->enabled(), [bloom](bool value) { if (bloom) bloom->enabled(value); });
	sync_option(m_state.m_ssao, ssao && ssao->enabled(), [ssao](bool value) { if (ssao) ssao->enabled(value); });
	sync_option(m_state.m_fullscreen, app && app->fullscreen(), [app](bool value) { if (app) app->fullscreen(value); });
}
