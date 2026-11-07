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
#include <cmath>

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
		m_finger_places.fill(Vec2(0.0f));
		m_finger_places_next.fill(Vec2(0.0f));
		m_input->subscrive_finger_listener([this](Video::FingerEvent finger, Video::ActionEvent action, double x, double y)
		{
			const Vec2 position{ float(x), float(y) };
			if (size_t(finger) < FINGER_COUNT)
			{
				change(m_fingers_next[size_t(finger)], action);
				m_finger_places_next[size_t(finger)] = position;
			}
			for (auto* listener : m_listeners) listener->on_finger(finger, action, position);
		});
		m_input->subscrive_tilt_listener([this](double x, double y, double z)
		{
			m_tilt = Vec3(float(x), float(y), float(z));
			m_tilted = true;
		});
		m_input->subscrive_gamepad_listener([this, app](int pad, bool connected)
		{
			if (pad >= 0 && size_t(pad) < PAD_COUNT)
			{
				m_pads_next[size_t(pad)].connected = connected;
			}
			for (auto* listener : m_listeners) listener->on_gamepad(pad, connected);
			if (auto* instance = app()) instance->gamepad_event(pad, connected);
		});
		m_input->subscrive_gamepad_button_listener([this, app](int pad, Video::GamepadButton button, Video::ActionEvent action)
		{
			if (pad >= 0 && size_t(pad) < PAD_COUNT && button >= 0 && size_t(button) < PAD_BUTTONS)
			{
				change(m_pads_next[size_t(pad)].buttons[size_t(button)], action);
			}
			for (auto* listener : m_listeners) listener->on_gamepad_button(pad, button, action);
			if (auto* instance = app()) instance->gamepad_button_event(pad, button, action);
		});
		m_input->subscrive_gamepad_axis_listener([this](int pad, Video::GamepadAxis axis, double value)
		{
			if (pad >= 0 && size_t(pad) < PAD_COUNT && axis >= 0 && size_t(axis) < PAD_AXES)
			{
				m_pads_next[size_t(pad)].axes[size_t(axis)] = float(value);
			}
			for (auto* listener : m_listeners) listener->on_gamepad_axis(pad, axis, float(value));
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
				for (auto& finger : m_fingers_next) if (finger.down) change(finger, Video::ActionEvent::RELEASE);
				for (auto& pad : m_pads_next)
				{
					for (auto& button : pad.buttons) if (button.down) change(button, Video::ActionEvent::RELEASE);
					pad.axes.fill(0.0f);
				}
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
		m_pad_actions.clear();
	}

	void InputSystem::update(double delta_time)
	{
		//the events of the frame
		Video::Input::pull_events();
		//the state of this frame
		latch(m_keys, m_keys_next);
		latch(m_buttons, m_buttons_next);
		latch(m_fingers, m_fingers_next);
		for (size_t pad = 0; pad != PAD_COUNT; ++pad)
		{
			m_pads[pad].connected = m_pads_next[pad].connected;
			m_pads[pad].axes = m_pads_next[pad].axes;
			latch(m_pads[pad].buttons, m_pads_next[pad].buttons);
		}
		m_finger_places = m_finger_places_next;
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
	void InputSystem::latch(Array<Button, N>& state, Array<Button, N>& next)
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

	bool InputSystem::down(Video::FingerEvent finger) const
	{
		return size_t(finger) < FINGER_COUNT && m_fingers[size_t(finger)].down;
	}

	bool InputSystem::pressed(Video::FingerEvent finger) const
	{
		return size_t(finger) < FINGER_COUNT && m_fingers[size_t(finger)].pressed;
	}

	bool InputSystem::released(Video::FingerEvent finger) const
	{
		return size_t(finger) < FINGER_COUNT && m_fingers[size_t(finger)].released;
	}

	const Vec2& InputSystem::finger(Video::FingerEvent finger) const
	{
		static const Vec2 s_none(0.0f);
		return size_t(finger) < FINGER_COUNT ? m_finger_places[size_t(finger)] : s_none;
	}

	void InputSystem::tilt(bool enable)
	{
		if (m_input) m_input->enable_tilt(enable);
		if (!enable) m_tilted = false;
	}

	bool InputSystem::tilt(Vec3& gravity) const
	{
		if (!m_tilted) return false;
		gravity = m_tilt;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//gamepads
	int InputSystem::gamepads() const
	{
		return int(std::count_if(m_pads.begin(), m_pads.end(), [](const Pad& pad) { return pad.connected; }));
	}

	bool InputSystem::gamepad(int pad) const
	{
		return pad >= 0 && size_t(pad) < PAD_COUNT && m_pads[size_t(pad)].connected;
	}

	bool InputSystem::pad_button(Video::GamepadButton button, int pad, bool Button::* state) const
	{
		if (button < 0 || size_t(button) >= PAD_BUTTONS) return false;
		if (pad >= 0)
		{
			return size_t(pad) < PAD_COUNT && m_pads[size_t(pad)].buttons[size_t(button)].*state;
		}
		for (const Pad& any : m_pads) if (any.buttons[size_t(button)].*state) return true;
		return false;
	}

	bool InputSystem::down(Video::GamepadButton button, int pad) const
	{
		return pad_button(button, pad, &Button::down);
	}

	bool InputSystem::pressed(Video::GamepadButton button, int pad) const
	{
		return pad_button(button, pad, &Button::pressed);
	}

	bool InputSystem::released(Video::GamepadButton button, int pad) const
	{
		return pad_button(button, pad, &Button::released);
	}

	float InputSystem::axis(Video::GamepadAxis axis, int pad) const
	{
		if (axis < 0 || size_t(axis) >= PAD_AXES) return 0.0f;
		if (pad >= 0)
		{
			return size_t(pad) < PAD_COUNT ? m_pads[size_t(pad)].axes[size_t(axis)] : 0.0f;
		}
		//(any: the one most moved)
		float most = 0.0f;
		for (const Pad& any : m_pads)
		{
			if (std::abs(any.axes[size_t(axis)]) > std::abs(most)) 
				most = any.axes[size_t(axis)];
		}
		return most;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//actions
	void InputSystem::bind(const String& action, Video::KeyboardEvent key)
	{
		auto& keys = m_actions[action];
		if (std::find(keys.begin(), keys.end(), key) == keys.end()) keys.push_back(key);
	}

	void InputSystem::bind(const String& action, Video::GamepadButton button)
	{
		auto& buttons = m_pad_actions[action];
		if (std::find(buttons.begin(), buttons.end(), button) == buttons.end()) buttons.push_back(button);
	}

	void InputSystem::unbind(const String& action)
	{
		m_actions.erase(action);
		m_pad_actions.erase(action);
	}

	bool InputSystem::action(const String& action) const
	{
		auto it = m_actions.find(action);
		if (it != m_actions.end()) for (auto key : it->second) if (down(key)) return true;
		auto pad = m_pad_actions.find(action);
		if (pad != m_pad_actions.end()) for (auto button : pad->second) if (down(button)) return true;
		return false;
	}

	bool InputSystem::action_pressed(const String& action) const
	{
		auto it = m_actions.find(action);
		if (it != m_actions.end()) for (auto key : it->second) if (pressed(key)) return true;
		auto pad = m_pad_actions.find(action);
		if (pad != m_pad_actions.end()) for (auto button : pad->second) if (pressed(button)) return true;
		return false;
	}

	bool InputSystem::action_released(const String& action) const
	{
		auto it = m_actions.find(action);
		if (it != m_actions.end()) for (auto key : it->second) if (released(key)) return true;
		auto pad = m_pad_actions.find(action);
		if (pad != m_pad_actions.end()) for (auto button : pad->second) if (released(button)) return true;
		return false;
	}
}
