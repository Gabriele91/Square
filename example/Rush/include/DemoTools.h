//
//  DemoTools.h
//  Rush
//
//  The engine demo over the game, all its tools in the menu (variables of the data model
//  "rush", bound before the documents are loaded):
//   - lights: the spot lights of the arena on its map (a click switches one), the sun, the
//     filter and the cascades of its shadow;
//   - graphics: the settings of SSR and SSAO (resolution, blur, march, denoise);
//   - debug: the player's hovercraft all mirror, the collisions, the debug views (OBB, light
//     volumes, texture panel, SSR/SSAO/Bloom), the debugger and the profiler of the UI;
//   - level: saved and loaded (binary .sq, json .jsq).
//
#pragma once
#include <string>
#include <vector>
#include <Square/Square.h>
#include <UIOption.h>

class Arena;
class Race;
class Graphics;

class DemoTools
{
public:

	DemoTools(Square::Context& context, Square::Scene::World& world);

	//its variables in the data model (before the documents)
	void bind(Square::UI::DataModel& model);
	//its controls in the menu (after the documents): the level buttons, the sun
	void setup(Square::UI::Document& menu, Square::UI::DataModel& model);
	//a race started: the map of its lights; ended: no map
	void race_started(const Arena& arena);
	void race_ended();

	//its options, every frame (no race: the title)
	void update(Race* race, const Graphics& graphics);

	//the wheel: it scrolls the texture panel
	void mouse_scroll(double scroll);

private:

	//a spot light on the map of the menu
	struct LightMarker
	{
		std::string                            m_name;
		Square::Weak<Square::Scene::SpotLight> m_light;
		Square::UI::Element                    m_element;
	};

	struct State
	{
		//lights
		UIOption<bool> m_sun;
		UIOption<int>  m_shadow_filter; //Render::ShadowFilter of the sun
		UIOption<int>  m_cascades{ DIRECTION_SHADOW_CSM_DEFAULT_FACES, DIRECTION_SHADOW_CSM_DEFAULT_FACES }; //a value of the select from the start
		//graphics
		UIOption<int>  m_ssr_resolution;  //Render::PostEffectResolution
		UIOption<int>  m_ssr_march;     //1: on the screen (DDA), 0: in world space
		UIOption<int>  m_ssr_blur;      //Render::SSR::Settings::BlurQuality
		UIOption<bool> m_ssr_denoise;
		UIOption<int>  m_ssao_resolution; //Render::PostEffectResolution
		UIOption<int>  m_ssao_blur;     //Render::SSAO::Settings::BlurQuality
		//debug
		UIOption<bool> m_mirror;
		UIOption<bool> m_collisions;
		UIOption<bool> m_obb;
		UIOption<bool> m_lights;
		UIOption<bool> m_textures;
		UIOption<int>  m_ssr_debug;
		UIOption<bool> m_bloom_debug;
		UIOption<bool> m_ssao_debug;
		UIOption<bool> m_ui_debugger;
		UIOption<bool> m_profiler;
		bool           m_profiler_available{ false }; //the engine has the profiler
	};

	void update_lights(const Arena& arena);
	void update_effects(const Graphics& graphics);
	void update_debug(Race* race);
	Square::Shared<Square::Scene::SpotLight> spot_light(const std::string& name);

	//the player's hovercraft all mirror (the SSR on it), or back to its skin
	void mirror(Race& race, bool enable);

	//the level, binary (.sq) and json (.jsq)
	static std::string level_path(bool json);
	void save_level(bool json);
	void load_level(bool json);

	//the debug pass of the world (OBB, lights, textures)
	Square::Shared<Square::Render::DrawerPassDebug> render_debug();

	Square::Context&          m_context;
	Square::Scene::World&     m_world;
	State                     m_state;
	Square::UI::Document*     m_menu{ nullptr };
	bool                      m_mirror{ false };
	std::vector<LightMarker>  m_light_markers;
};
