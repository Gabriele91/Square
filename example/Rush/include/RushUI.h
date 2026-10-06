//
//  RushUI.h
//  Rush
//
//  The UI of the game (documents of assets/ui.sqz, the data model "rush"):
//   - the title (over the 3D hovercraft of TitleScreen): Play Game (the modes: Races and Battle
//     to come, Arena: the maps turning on a wheel, the one in front played), Settings (saved at
//     every change: GameSettings), About, Exit; the arrows and enter, Esc back, or the mouse;
//   - the HUD of a race: the score cards, the speed (0 to 100), the start (3, 2, 1, GO!), the
//     end (win or lose, press a key);
//   - the Esc menu of a race: with the demo (RUSH_DEMO, Debug) the full one of the engine
//     demo, else the pause (resume, exit to the title).
//  The demo binds its own variables to the same model (DemoTools), before the documents are
//  loaded.
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>
#include <UIOption.h>
#include <GameSettings.h>

class Race;
class Graphics;

class RushUI
{
public:

	using Callback = std::function<void()>;

	RushUI(Square::Context& context);

	//the fonts, the data model with the variables of the game (false: no UI)
	bool create();
	//the title, the HUD, the menu (after every binding: the documents read the model when loaded)
	void load_documents();

	Square::UI::DataModel& model();
	Square::UI::Document&  menu_document();

	//the title shown (the HUD and the menu hidden), or the HUD
	void title(bool show);
	//a key in the title: left/right (up/down) the item (the map), enter its action (play it)
	void title_key(Square::Video::KeyboardEvent key);
	//a map chosen to play, Exit of the title (the game)
	void on_play(const Callback& callback);
	void on_quit(const Callback& callback);
	//the map chosen in the title (s_race_maps), or chosen by name (false: none of that name)
	const RaceMap& race_map() const;
	bool race_map(const std::string& name);

	//the Esc menu of a race, Exit of it (to the title)
	void menu(bool show);
	bool menu_visible() const;
	void on_exit(const Callback& callback);

	//the settings of the game (loaded at the start), applied to the window and the effects
	const GameSettings& settings() const;
	void load_settings(Graphics& graphics);

	//the HUD of a race (none: the title), the options of the game (a change of the settings
	//applied and saved), every frame
	void update(const Race* race, Graphics& graphics, float fps);

private:

	void setup_title();
	void update_hud(const Race& race);
	void update_options(Graphics& graphics);

	//the screens of the title
	enum class Screen : int
	{
		MAIN,
		MODES,
		ARENA,
		SETTINGS,
		ABOUT
	};
	//the items of the main screen, in order
	enum MainItem : int
	{
		MAIN_PLAY,
		MAIN_SETTINGS,
		MAIN_ABOUT,
		MAIN_EXIT,
		MAIN_COUNT
	};
	//the modes of Play, in order (only Arena plays now)
	enum Mode : int
	{
		MODE_RACES,
		MODE_ARENA,
		MODE_BATTLE,
		MODE_COUNT
	};
	void screen(Screen screen);
	void main_select(int item);
	void main_activate(int item);
	void mode_select(int mode);
	void mode_activate(int mode);
	//the wheel of the maps: the map in front, the others around it; played
	void map_select(size_t map);
	void map_play(size_t map);

	struct State
	{
		//HUD
		int            m_winning_score{ s_winning_score };
		int            m_scores[s_racers]{ 0, 0, 0, 0 };
		int            m_speed{ 0 };
		std::string    m_speed_bar{ "0%" }; //the width of its bar (of the HUD)
		std::string    m_message;               //the start: 3, 2, 1, GO!
		bool           m_message_visible{ false };
		std::string    m_result;                //the end: win or lose
		bool           m_result_visible{ false };
		float          m_fps{ 0.0f };
		//options of the game (the menu of the demo)
		UIOption<bool> m_ssr;
		UIOption<bool> m_bloom;
		UIOption<bool> m_ssao;
		UIOption<bool> m_fullscreen;
		//the settings (the controls of Settings), the ones applied
		GameSettings   m_settings;
		GameSettings   m_applied;
	};

	Square::Context&                 m_context;
	State                            m_state;
	Square::UI::DataModel            m_model;
	Square::UI::Document             m_title;
	Square::UI::Document             m_hud;
	Square::UI::Document             m_menu;
	Screen                           m_screen{ Screen::MAIN };
	std::vector<Square::UI::Element> m_main_items;
	int                              m_main_selected{ MAIN_PLAY };
	std::vector<Square::UI::Element> m_modes;
	int                              m_mode{ MODE_ARENA };
	std::vector<Square::UI::Element> m_map_cards; //map_<n>, s_race_maps
	std::vector<Square::UI::Element> m_map_infos; //info_<n>
	size_t                           m_map{ 0 };  //the selected one
	Callback                         m_on_play;
	Callback                         m_on_quit;
	Callback                         m_on_exit;
};
