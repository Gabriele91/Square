//
//  RushUI.h
//  Rush
//
//  The UI of the game (documents of assets/ui.sqz, the data model "rush"):
//   - the title: Play Game (the chooser of the maps: their views, arrows and enter or a click
//     plays, Esc back), Instructions, About, Exit (arrows and enter, or the mouse);
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

	//the HUD of a race (none: the title), the options of the game, every frame
	void update(const Race* race, const Graphics& graphics, float fps);

private:

	void setup_title();
	void update_hud(const Race& race);
	void update_options(const Graphics& graphics);

	//the items of the title, in order
	enum TitleItem : int
	{
		TITLE_PLAY,
		TITLE_INSTRUCTIONS,
		TITLE_ABOUT,
		TITLE_EXIT,
		TITLE_COUNT
	};
	void title_select(int item);
	void title_activate(int item);
	//a panel of the title (Instructions, About) shown, the others hidden (none: TITLE_COUNT)
	void title_panel(int item);
	//the chooser of the maps (instead of the items), the map selected, played
	void maps(bool show);
	void maps_key(Square::Video::KeyboardEvent key);
	void map_select(size_t map);
	void map_play(size_t map);

	struct State
	{
		//HUD
		int            m_winning_score{ s_winning_score };
		int            m_scores[s_racers]{ 0, 0, 0, 0 };
		int            m_speed{ 0 };
		std::string    m_message;               //the start: 3, 2, 1, GO!
		bool           m_message_visible{ false };
		std::string    m_result;                //the end: win or lose
		bool           m_result_visible{ false };
		float          m_fps{ 0.0f };
		//options of the game
		UIOption<bool> m_ssr;
		UIOption<bool> m_bloom;
		UIOption<bool> m_ssao;
		UIOption<bool> m_fullscreen;
	};

	Square::Context&                 m_context;
	State                            m_state;
	Square::UI::DataModel            m_model;
	Square::UI::Document             m_title;
	Square::UI::Document             m_hud;
	Square::UI::Document             m_menu;
	std::vector<Square::UI::Element> m_title_items;
	int                              m_title_selected{ TITLE_PLAY };
	std::vector<Square::UI::Element> m_map_cards; //map_<n>, s_race_maps
	size_t                           m_map{ 0 };  //the selected one
	bool                             m_maps_shown{ false };
	Callback                         m_on_play;
	Callback                         m_on_quit;
	Callback                         m_on_exit;
};
