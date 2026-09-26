//
//  InputSystem.cpp
//  Square
//
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
#include "Square/Driver/Input.h"
#include "Square/Driver/Window.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/System/InputSystem.h"

namespace Square
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(InputSystem);

	//Registration in context
	void InputSystem::object_registration(Context& ctx)
	{
		//system: at start-up (ring DRIVER)
		ctx.add_system<InputSystem>(SystemStartup::AUTOMATIC);
	}

	InputSystem::InputSystem(Context& context) : System(context)
	{
	}

	InputSystem::~InputSystem()
	{
	}

	bool InputSystem::initialize()
	{
		//no window, no input
		Video::Window* window = context().window();
		if (!window) return false;
		m_input = SQ_NEW(context().allocator(), Video::Input, AllocType::ALCT_DEFAULT) Video::Input(window);
		m_close_requested = false;
		//events to the application
		auto app = [this]() -> AppInterface*
		{
			return context().application() ? context().application()->app_instance() : nullptr;
		};
		m_input->subscrive_keyboard_listener([app](Video::KeyboardEvent key, short mode, Video::ActionEvent action)
		{
			if (auto* instance = app()) instance->key_event(key, mode, action);
		});
		m_input->subscrive_mouse_move_listener([app](double x, double y)
		{
			if (auto* instance = app()) instance->mouse_move_event(DVec2(x, y));
		});
		m_input->subscrive_mouse_button_listener([app](Video::MouseButtonEvent button, Video::ActionEvent action)
		{
			if (auto* instance = app()) instance->mouse_button_event(button, action);
		});
		m_input->subscrive_mouse_scroll_listener([app](double scroll)
		{
			if (auto* instance = app()) instance->mouse_scroll_event(scroll);
		});
		m_input->subscrive_window_listener([this, app](Video::WindowEvent event)
		{
			if (auto* instance = app()) instance->window_event(event);
			if (event == Video::WindowEvent::CLOSE) m_close_requested = true;
		});
		return true;
	}

	void InputSystem::shutdown()
	{
		if (m_input)
		{
			SQ_DELETE_NAMESPACE(context().allocator(), Video, Input, m_input);
			m_input = nullptr;
		}
	}

	void InputSystem::update(double delta_time)
	{
		Video::Input::pull_events();
	}

	Video::Input* InputSystem::input() const
	{
		return m_input;
	}

	bool InputSystem::close_requested() const
	{
		return m_close_requested;
	}
}
