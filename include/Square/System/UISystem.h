//
//  UISystem.h
//  Square
//
//  The system of the UI (RmlUi inside, see UI/Context.h): it starts RmlUi with the backend of
//  the engine (the render of Render::Context, the time and the log of the engine), gives it the
//  input (InputSystem) and draws its documents over the frame (RenderSystem overlay), every
//  frame after the worlds.
//
#pragma once
#include <memory>
#include "Square/Config.h"
#include "Square/System/System.h"
#include "Square/System/InputSystem.h"
#include "Square/System/RenderSystem.h"
#include "Square/UI/Context.h"
#include "Square/Core/DebugOptions.h"

namespace Square
{
namespace UI
{
	class Backend;
	class DebugPanel;
	struct DebugProvider;
}

	class SQUARE_API UISystem : public System, public InputListener, public RenderOverlay
	{
	public:
		//A square system, and its ring: after the render and the input
		SQUARE_SYSTEM(UISystem, SystemRing::SERVICE + 50)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		UISystem(Context& context);
		virtual ~UISystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		virtual void update(double delta_time) override;

		//the UI of the window
		UI::Context& ui();

		//it wants the mouse (it is over an element of a document) or the keyboard (an element
		//has the focus): the game can ignore them
		bool wants_mouse() const;
		bool wants_keyboard() const;

		//the debug panel of the engine over the frame (F1: its key, on by default): the render
		//profiler, the pipeline of the worlds, the debug views, the UI, the tabs of the game.
		//Not in Retail (SQUARE_DEBUG_TOOLS not defined): never shown, its sections not asked
		void debug_panel(bool visible);
		bool debug_panel() const;
		//F1 opens and closes it (false: only debug_panel does)
		void debug_panel_key(bool enabled);
		bool debug_panel_key() const;
		//the sections a game gives to a tab of it (an engine tab: "Pipeline", "Debug draw", "UI",
		//or a tab of its own), by a key (given again: replaced); asked when the panel is made
		void debug_sections(const std::string& tab, const std::string& key, DebugSections sections);
		void remove_debug_sections(const std::string& key);

		//the render profiler (Render/Profiler.h) on, the debug panel shown with it. Without
		//RENDER_PROFILER (no profiler) it is never on
		void profiler(bool enable);
		bool profiler() const;
		//the render profiler is compiled in the engine
		bool has_profiler() const;

		//the size of the window the documents are made for: their dp scaled to the window (the
		//smaller of the ratios of its width and its height: the proportions kept, all in the
		//window); 0: a dp a pixel
		void reference_size(const IVec2& size);
		const IVec2& reference_size() const;

	protected:
		//InputListener
		virtual void on_key(Video::KeyboardEvent key, short mode, Video::ActionEvent action) override;
		virtual void on_character(int character, short mode) override;
		virtual void on_mouse_button(Video::MouseButtonEvent button, Video::ActionEvent action) override;
		virtual void on_mouse_move(const Vec2& position) override;
		virtual void on_mouse_scroll(float scroll) override;
		//RenderOverlay
		virtual void draw_overlay(Render::Context& render) override;

		std::unique_ptr<UI::Backend>       m_backend;
		std::unique_ptr<UI::DebugPanel>    m_debug_panel;
		std::vector<UI::DebugProvider>     m_debug_providers;
		bool                               m_debug_panel_key{ true };
		//the debug panel, made the first time it is needed (nullptr: no UI)
		UI::DebugPanel* debug_panel_instance();
		UI::Context                  m_ui;
		int                          m_modifiers{ 0 };
		bool                         m_wants_mouse{ false };
		IVec2                        m_reference_size{ 0, 0 };
	};
}
