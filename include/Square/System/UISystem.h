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

namespace Square
{
namespace UI
{
	class Backend;
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

	protected:
		//InputListener
		virtual void on_key(Video::KeyboardEvent key, short mode, Video::ActionEvent action) override;
		virtual void on_character(int character, short mode) override;
		virtual void on_mouse_button(Video::MouseButtonEvent button, Video::ActionEvent action) override;
		virtual void on_mouse_move(const Vec2& position) override;
		virtual void on_mouse_scroll(float scroll) override;
		//RenderOverlay
		virtual void draw_overlay(Render::Context& render) override;

		std::unique_ptr<UI::Backend> m_backend;
		UI::Context                  m_ui;
		int                          m_modifiers{ 0 };
		bool                         m_wants_mouse{ false };
	};
}
