//
//  DebugPanel.h
//  Square
//
//  The debug panel of the engine (UISystem: F1, or UISystem::debug_panel): a document of its own
//  (common/ui/debug.rml), on the right of the frame, its tabs:
//   - Profiler: the render profiler (ProfilerPanel);
//   - Pipeline: of each world, its render (shadows, the occlusion culling, the levels of detail),
//     its direction lights (their shadows), its post effects (on or off, PostEffect::debug_options);
//   - Debug draw: the views of the debug pass of each world (boxes, frustum, light volumes, the
//     occlusion, the shadow cascades in colors), what the occlusion did;
//   - Textures: the textures of the driver (render targets, shadow maps, images) as thumbnails,
//     a page at a time (TexturePreviews; the registry of the driver: not in Retail);
//   - UI: the debugger of RmlUi;
//   - the sections a game adds to these tabs, its tabs (UISystem::debug_sections).
//  Its controls are made again when what they stand for changes (a world, its effects, its
//  lights, the sections of the game); each frame they show the values of what they stand for.
//
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/DebugOptions.h"
#include "Square/UI/Context.h"
#include "Square/Math/Linear.h"

namespace Square
{
	class Context;
namespace Render
{
	class Context;
	class Texture;
}
namespace UI
{
	class ProfilerPanel;
	class TexturePreviews;

	//the sections a game gives to a tab (a key: given again, replaced)
	struct DebugProvider
	{
		std::string   m_tab;
		std::string   m_key;
		DebugSections m_sections;
	};

	class DebugPanel
	{
	public:
		DebugPanel(Square::Context& context, UI::Context& ui, const std::vector<DebugProvider>& providers);
		~DebugPanel();

		//shown or hidden (its profiler stays as it is)
		void show(bool visible);
		bool visible() const;

		//on the tab of the profiler, the profiler on
		void profile(bool enable);
		bool profiling() const;

		//the sections of the game changed: made again
		void providers_changed();

		//its controls as what they stand for (made again if that changed); before the update of
		//RmlUi
		void update();

		//the thumbnails of the tab Textures (it shown), in the frame before the UI
		void draw(Render::Context& render);
		//the texture of a thumbnail of a source of an image (none: nullptr)
		Render::Texture* preview(const std::string& source, IVec2& size);

	private:
		//a control of an option, the value it shows
		struct Control
		{
			DebugOption m_option;
			Element     m_element;
			int         m_shown{ -1 };
			std::string m_text;
		};
		//the sections of a tab
		struct Tab
		{
			std::string               m_name;
			std::vector<DebugSection> m_sections;
		};

		bool create();
		//the tabs, their markup, their controls (the tab shown kept)
		void rebuild();
		std::vector<Tab> tabs();
		//what the controls stand for (when it changes: made again)
		std::string signature() const;
		void bind(Control& control);
		void refresh(Control& control);

		//the sections of the engine
		void pipeline_sections(std::vector<DebugSection>& sections) const;
		void draw_sections(std::vector<DebugSection>& sections) const;
		void ui_sections(std::vector<DebugSection>& sections) const;
		void textures_sections(std::vector<DebugSection>& sections);
		//the tab Textures shown; its thumbnails as the filter and the page
		bool textures_shown() const;
		void update_textures();

		Square::Context&                  m_context;
		UI::Context&                      m_ui;
		const std::vector<DebugProvider>& m_providers;
		std::unique_ptr<ProfilerPanel>    m_profiler;
		std::unique_ptr<TexturePreviews>  m_previews;
		std::vector<Element>              m_captions;      //of the thumbnails
		int                               m_textures_tab{ -1 };
		int                               m_texture_filter{ 0 }; //render targets, shadow maps, images
		int                               m_texture_page{ 0 };
		int                               m_texture_pages{ 1 };
		Element                           m_page_label;    //under the thumbnails, between the arrows
		Document                          m_document;
		Element                           m_content;
		std::vector<Control>              m_controls;
		std::string                       m_signature;
		bool                              m_visible{ false };
		bool                              m_dirty{ true };
	};
}
}
