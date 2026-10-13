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
#include <algorithm>

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
		//events to the state of the frame and to the application
		auto app = [this]() -> AppInterface*
		{
			return context().application() ? context().application()->app_instance() : nullptr;
		};
		m_input->subscrive_keyboard_listener([this, app](Video::KeyboardEvent key, short mode, Video::ActionEvent action)
		{
			if (key >= 0 && size_t(key) < KEY_COUNT) change(m_keys_next[size_t(key)], action);
			for (auto* listener : m_listeners) listener->on_key(key, mode, action);
			if (auto* instance = app()) instance->key_event(key, mode, action);
		});
		m_input->subscrive_character_listener([this](int character, short mode, int /*plain*/)
		{
			for (auto* listener : m_listeners) listener->on_character(character, mode);
		});
		m_input->subscrive_mouse_move_listener([this, app](double x, double y)
		{
			m_mouse_next = Vec2(float(x), float(y));
			for (auto* listener : m_listeners) listener->on_mouse_move(m_mouse_next);
			if (auto* instance = app()) instance->mouse_move_event(DVec2(x, y));
		});
		m_input->subscrive_mouse_button_listener([this, app](Video::MouseButtonEvent button, Video::ActionEvent action)
		{
			if (button >= 0 && size_t(button) < BUTTON_COUNT) change(m_buttons_next[size_t(button)], action);
			for (auto* listener : m_listeners) listener->on_mouse_button(button, action);
			if (auto* instance = app()) instance->mouse_button_event(button, action);
		});
		m_input->subscrive_mouse_scroll_listener([this, app](double scroll)
		{
			m_scroll_next += float(scroll);
			for (auto* listener : m_listeners) listener->on_mouse_scroll(float(scroll));
			if (auto* instance = app()) instance->mouse_scroll_event(scroll);
		});
		m_input->subscrive_window_listener([this, app](Video::WindowEvent event)
		{
			for (auto* listener : m_listeners) listener->on_window(event);
			if (auto* instance = app()) instance->window_event(event);
			if (event == Video::WindowEvent::CLOSE) m_close_requested = true;
			//no focus: no key is held any more (their release goes to another window)
			if (event == Video::WindowEvent::LOST_FOCUS)
			{
				for (auto& key : m_keys_next) if (key.down) change(key, Video::ActionEvent::RELEASE);
				for (auto& button : m_buttons_next) if (button.down) change(button, Video::ActionEvent::RELEASE);
			}
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
		m_actions.clear();
	}

	void InputSystem::update(double delta_time)
	{
		//the events of the frame
		Video::Input::pull_events();
		//the state of this frame
		latch(m_keys, m_keys_next);
		latch(m_buttons, m_buttons_next);
		m_mouse_delta = m_mouse_next - m_mouse;
		m_mouse = m_mouse_next;
		m_scroll = m_scroll_next;
		m_scroll_next = 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//state
	void InputSystem::change(Button& button, Video::ActionEvent action)
	{
		switch (action)
		{
		case Video::ActionEvent::PRESS:
			if (!button.down) button.pressed = true;
			button.down = true;
			break;
		case Video::ActionEvent::RELEASE:
			if (button.down) button.released = true;
			button.down = false;
			break;
		default: break; //repeat: still held
		}
	}

	template < size_t N >
	void InputSystem::latch(std::array<Button, N>& state, std::array<Button, N>& next)
	{
		//pressed and released in the same frame: both seen
		state = next;
		for (auto& button : next) button.pressed = button.released = false;
	}

	void InputSystem::add_listener(InputListener* listener)
	{
		if (listener && std::find(m_listeners.begin(), m_listeners.end(), listener) == m_listeners.end())
		{
			m_listeners.push_back(listener);
		}
	}

	void InputSystem::remove_listener(InputListener* listener)
	{
		m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
	}

	Video::Input* InputSystem::input() const
	{
		return m_input;
	}

	bool InputSystem::close_requested() const
	{
		return m_close_requested;
	}

	bool InputSystem::down(Video::KeyboardEvent key) const
	{
		return key >= 0 && size_t(key) < KEY_COUNT && m_keys[size_t(key)].down;
	}

	bool InputSystem::pressed(Video::KeyboardEvent key) const
	{
		return key >= 0 && size_t(key) < KEY_COUNT && m_keys[size_t(key)].pressed;
	}

	bool InputSystem::released(Video::KeyboardEvent key) const
	{
		return key >= 0 && size_t(key) < KEY_COUNT && m_keys[size_t(key)].released;
	}

	bool InputSystem::down(Video::MouseButtonEvent button) const
	{
		return button >= 0 && size_t(button) < BUTTON_COUNT && m_buttons[size_t(button)].down;
	}

	bool InputSystem::pressed(Video::MouseButtonEvent button) const
	{
		return button >= 0 && size_t(button) < BUTTON_COUNT && m_buttons[size_t(button)].pressed;
	}

	bool InputSystem::released(Video::MouseButtonEvent button) const
	{
		return button >= 0 && size_t(button) < BUTTON_COUNT && m_buttons[size_t(button)].released;
	}

	const Vec2& InputSystem::mouse() const
	{
		return m_mouse;
	}

	const Vec2& InputSystem::mouse_delta() const
	{
		return m_mouse_delta;
	}

	float InputSystem::scroll() const
	{
		return m_scroll;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//actions
	void InputSystem::bind(const std::string& action, Video::KeyboardEvent key)
	{
		auto& keys = m_actions[action];
		if (std::find(keys.begin(), keys.end(), key) == keys.end()) keys.push_back(key);
	}

	void InputSystem::unbind(const std::string& action)
	{
		m_actions.erase(action);
	}

	bool InputSystem::action(const std::string& action) const
	{
		auto it = m_actions.find(action);
		if (it == m_actions.end()) return false;
		for (auto key : it->second) if (down(key)) return true;
		return false;
	}

	bool InputSystem::action_pressed(const std::string& action) const
	{
		auto it = m_actions.find(action);
		if (it == m_actions.end()) return false;
		for (auto key : it->second) if (pressed(key)) return true;
		return false;
	}

	bool InputSystem::action_released(const std::string& action) const
	{
		auto it = m_actions.find(action);
		if (it == m_actions.end()) return false;
		for (auto key : it->second) if (released(key)) return true;
		return false;
	}
}
