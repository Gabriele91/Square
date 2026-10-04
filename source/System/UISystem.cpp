//
//  UISystem.cpp
//  Square
//
//  See UISystem.h.
//
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include "Square/Core/Context.h"
#include "Square/Core/Logger.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Driver/Window.h"
#include "Square/System/UISystem.h"
#include "../UI/Backend.h"
#include "../UI/ProfilerPanel.h"

namespace Square
{
	//the key of RmlUi of a key of the engine
	static Rml::Input::KeyIdentifier to_rml_key(Video::KeyboardEvent key)
	{
		using namespace Rml::Input;
		if (key >= Video::KEY_A && key <= Video::KEY_Z)       return KeyIdentifier(KI_A + (key - Video::KEY_A));
		if (key >= Video::KEY_0 && key <= Video::KEY_9)       return KeyIdentifier(KI_0 + (key - Video::KEY_0));
		if (key >= Video::KEY_F1 && key <= Video::KEY_F24)    return KeyIdentifier(KI_F1 + (key - Video::KEY_F1));
		if (key >= Video::KEY_KP_0 && key <= Video::KEY_KP_9) return KeyIdentifier(KI_NUMPAD0 + (key - Video::KEY_KP_0));
		switch (key)
		{
		case Video::KEY_SPACE:         return KI_SPACE;
		case Video::KEY_APOSTROPHE:    return KI_OEM_7;
		case Video::KEY_COMMA:         return KI_OEM_COMMA;
		case Video::KEY_MINUS:         return KI_OEM_MINUS;
		case Video::KEY_PERIOD:        return KI_OEM_PERIOD;
		case Video::KEY_SLASH:         return KI_OEM_2;
		case Video::KEY_SEMICOLON:     return KI_OEM_1;
		case Video::KEY_EQUAL:         return KI_OEM_PLUS;
		case Video::KEY_LEFT_BRACKET:  return KI_OEM_4;
		case Video::KEY_BACKSLASH:     return KI_OEM_5;
		case Video::KEY_RIGHT_BRACKET: return KI_OEM_6;
		case Video::KEY_GRAVE_ACCENT:  return KI_OEM_3;
		case Video::KEY_WORLD_1:
		case Video::KEY_WORLD_2:       return KI_OEM_102;
		case Video::KEY_ESCAPE:        return KI_ESCAPE;
		case Video::KEY_ENTER:         return KI_RETURN;
		case Video::KEY_TAB:           return KI_TAB;
		case Video::KEY_BACKSPACE:     return KI_BACK;
		case Video::KEY_INSERT:        return KI_INSERT;
		case Video::KEY_DELETE:        return KI_DELETE;
		case Video::KEY_RIGHT:         return KI_RIGHT;
		case Video::KEY_LEFT:          return KI_LEFT;
		case Video::KEY_DOWN:          return KI_DOWN;
		case Video::KEY_UP:            return KI_UP;
		case Video::KEY_PAGE_UP:       return KI_PRIOR;
		case Video::KEY_PAGE_DOWN:     return KI_NEXT;
		case Video::KEY_HOME:          return KI_HOME;
		case Video::KEY_END:           return KI_END;
		case Video::KEY_CAPS_LOCK:     return KI_CAPITAL;
		case Video::KEY_SCROLL_LOCK:   return KI_SCROLL;
		case Video::KEY_NUM_LOCK:      return KI_NUMLOCK;
		case Video::KEY_PRINT_SCREEN:  return KI_SNAPSHOT;
		case Video::KEY_PAUSE:         return KI_PAUSE;
		case Video::KEY_KP_DECIMAL:    return KI_DECIMAL;
		case Video::KEY_KP_DIVIDE:     return KI_DIVIDE;
		case Video::KEY_KP_MULTIPLY:   return KI_MULTIPLY;
		case Video::KEY_KP_SUBTRACT:   return KI_SUBTRACT;
		case Video::KEY_KP_ADD:        return KI_ADD;
		case Video::KEY_KP_ENTER:      return KI_NUMPADENTER;
		case Video::KEY_KP_EQUAL:      return KI_OEM_NEC_EQUAL;
		case Video::KEY_LEFT_SHIFT:    return KI_LSHIFT;
		case Video::KEY_LEFT_CONTROL:  return KI_LCONTROL;
		case Video::KEY_LEFT_ALT:      return KI_LMENU;
		case Video::KEY_LEFT_SUPER:    return KI_LWIN;
		case Video::KEY_RIGHT_SHIFT:   return KI_RSHIFT;
		case Video::KEY_RIGHT_CONTROL: return KI_RCONTROL;
		case Video::KEY_RIGHT_ALT:     return KI_RMENU;
		case Video::KEY_RIGHT_SUPER:   return KI_RWIN;
		case Video::KEY_MENU:          return KI_APPS;
		default:                       return KI_UNKNOWN;
		}
	}

	//the modifiers of RmlUi of the ones of the engine
	static int to_rml_modifiers(short mode)
	{
		int modifiers = 0;
		if (mode & Video::SHIFT)   modifiers |= Rml::Input::KM_SHIFT;
		if (mode & Video::CONTROL) modifiers |= Rml::Input::KM_CTRL;
		if (mode & Video::ALT)     modifiers |= Rml::Input::KM_ALT;
		if (mode & Video::SUPER)   modifiers |= Rml::Input::KM_META;
		return modifiers;
	}

	//the size of the window (pixels)
	static IVec2 window_size(Context& context)
	{
		unsigned int width = 0, height = 0;
		if (auto* window = context.window()) window->get_size(width, height);
		return IVec2(int(width), int(height));
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//UISystem
	SQUARE_CLASS_OBJECT_REGISTRATION(UISystem);

	void UISystem::object_registration(Context& ctx)
	{
		//system: at start-up (after the render and the input)
		ctx.add_system<UISystem>(SystemStartup::AUTOMATIC);
	}

	UISystem::UISystem(Context& context) : System(context)
	{
	}

	UISystem::~UISystem()
	{
	}

	bool UISystem::initialize()
	{
		auto* render_system = System::get<RenderSystem>(context());
		auto* input_system = System::get<InputSystem>(context());
		if (!render_system || !render_system->render() || !context().window()) return false;
		//the backend of RmlUi (render, time, log)
		m_backend = std::make_unique<UI::Backend>(context());
		if (!m_backend->initialize())
		{
			context().logger()->warning("UI: unable to start the backend");
			m_backend.reset();
			return false;
		}
		Rml::SetRenderInterface(m_backend.get());
		Rml::SetSystemInterface(m_backend.get());
		Rml::SetFileInterface(m_backend.get());
		if (!Rml::Initialise())
		{
			context().logger()->warning("UI: unable to start RmlUi");
			m_backend.reset();
			return false;
		}
		//the context of the window, its debugger
		const IVec2 size = window_size(context());
		Rml::Context* ui_context = Rml::CreateContext("main", Rml::Vector2i(size.x, size.y));
		if (!ui_context)
		{
			Rml::Shutdown();
			m_backend.reset();
			return false;
		}
		Rml::Debugger::Initialise(ui_context);
		m_ui = UI::Context(context(), ui_context);
		//the input, the frame
		if (input_system) input_system->add_listener(this);
		render_system->add_overlay(this);
		return true;
	}

	void UISystem::shutdown()
	{
		if (auto* input_system = System::get<InputSystem>(context())) input_system->remove_listener(this);
		if (auto* render_system = System::get<RenderSystem>(context())) render_system->remove_overlay(this);
		//its document, before the contexts go
		m_profiler_panel.reset();
		if (m_backend)
		{
			//the documents, the contexts, the fonts: their GPU objects by the backend
			Rml::Shutdown();
			m_backend->release();
			m_backend.reset();
		}
		m_ui = UI::Context();
	}

	void UISystem::update(double delta_time)
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			//the size of the window
			const IVec2 size = window_size(context());
			if (size.x > 0 && size.y > 0 && size != m_ui.size())
				ui_context->SetDimensions(Rml::Vector2i(size.x, size.y));
			//the profiler panel: its rows before the update (their layout)
			if (m_profiler_panel) m_profiler_panel->update();
			// Update
			ui_context->Update();
		}
	}

	void UISystem::draw_overlay(Render::Context& render)
	{
		if (Rml::Context* ui_context = m_ui.native(); ui_context && m_backend)
		{
			//the size of the context (of the window)
			const IVec2 size = m_ui.size();
			if (size.x > 0 && size.y > 0)
			{
				m_backend->begin_frame(size);
				ui_context->Render();
				m_backend->end_frame();
			}
		}
	}

	void UISystem::profiler(bool visible)
	{
		if (!m_profiler_panel)
		{
			if (!visible || !has_profiler() || !m_ui.valid()) return;
			m_profiler_panel = std::make_unique<UI::ProfilerPanel>(context(), m_ui);
		}
		m_profiler_panel->show(visible);
	}

	bool UISystem::profiler() const
	{
		return m_profiler_panel && m_profiler_panel->visible();
	}

	bool UISystem::has_profiler() const
	{
		auto* render_system = System::get<RenderSystem>(context());
		return render_system && render_system->profiler();
	}

	UI::Context& UISystem::ui()
	{
		return m_ui;
	}

	bool UISystem::wants_mouse() const
	{
		return m_wants_mouse;
	}

	bool UISystem::wants_keyboard() const
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			//the body of a document has the focus when nothing else has it
			if (Rml::Element* focus = ui_context->GetFocusElement())
				return focus != focus->GetOwnerDocument();
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//input
	void UISystem::on_key(Video::KeyboardEvent key, short mode, Video::ActionEvent action)
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			m_modifiers = to_rml_modifiers(mode);
			const Rml::Input::KeyIdentifier rml_key = to_rml_key(key);
			if (rml_key != Rml::Input::KI_UNKNOWN)
			{
				if (action == Video::RELEASE) ui_context->ProcessKeyUp(rml_key, m_modifiers);
				else                          ui_context->ProcessKeyDown(rml_key, m_modifiers);
			}
		}
	}

	void UISystem::on_character(int character, short mode)
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			//text only (the control characters are keys)
			if (character >= 32 && character != 127)
				ui_context->ProcessTextInput(Rml::Character(character));
		}
	}

	void UISystem::on_mouse_button(Video::MouseButtonEvent button, Video::ActionEvent action)
	{
		if (Rml::Context* ui_context = m_ui.native(); ui_context && button >= 0)
		{
			//buttons: 0 left, 1 right, 2 middle in both
			if (action == Video::RELEASE)
				ui_context->ProcessMouseButtonUp(int(button), m_modifiers);
			else if (action == Video::PRESS)
				ui_context->ProcessMouseButtonDown(int(button), m_modifiers);
		}
	}

	void UISystem::on_mouse_move(const Vec2& position)
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			//false: over an element of a document
			m_wants_mouse = !ui_context->ProcessMouseMove(int(position.x), int(position.y), m_modifiers);
		}
	}

	void UISystem::on_mouse_scroll(float scroll)
	{
		if (Rml::Context* ui_context = m_ui.native())
		{
			//RmlUi: positive down, the engine: positive up
			ui_context->ProcessMouseWheel(-scroll, m_modifiers);
		}
	}
}
