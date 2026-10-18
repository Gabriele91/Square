//
//  InputSystem.h
//  Square
//
//  The input of the window (Video::Input): reads the events at the start of the frame
//  (update) and sends them to the application (AppInterface).
//  It also keeps the state of the frame, for who asks it (components, systems): keys and mouse
//  buttons held, pressed or released in this frame, the mouse position, the fingers on a touch
//  screen and the tilt of the device (a phone), and the actions (a name bound to one or more
//  keys: "forward" -> up, W).
//  Ring DRIVER: the state is ready before the scene updates.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Driver/Input.h"
#include "Square/System/System.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace Square
{
	//..................
	//who wants the events of the input as they come, before the state of the frame (the UI):
	//InputSystem::add_listener
	class SQUARE_API InputListener
	{
	public:
		virtual ~InputListener() {}
		virtual void on_key(Video::KeyboardEvent key, short mode, Video::ActionEvent action) {}
		virtual void on_character(int character, short mode) {}
		virtual void on_mouse_button(Video::MouseButtonEvent button, Video::ActionEvent action) {}
		virtual void on_mouse_move(const Vec2& position) {}
		virtual void on_mouse_scroll(float scroll) {}
		//a finger on the screen: down (PRESS), moved (REPEAT), up (RELEASE), where (pixels of the
		//window); its first one is also the mouse (its events after the down, before the up)
		virtual void on_finger(Video::FingerEvent finger, Video::ActionEvent action, const Vec2& position) {}
		virtual void on_window(Video::WindowEvent event) {}
	};

	//..................
	class SQUARE_API InputSystem : public System
	{
	public:
		//A square system, and its ring
		SQUARE_SYSTEM(InputSystem, SystemRing::DRIVER)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		InputSystem(Context& context);
		virtual ~InputSystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		virtual void update(double delta_time) override;

		//the input of the window
		Video::Input* input() const;

		//the window was asked to close
		bool close_requested() const;

		//the listeners of the events (not owned: removed before they go)
		void add_listener(InputListener* listener);
		void remove_listener(InputListener* listener);

		//keys: held, went down in this frame, went up in this frame
		bool down(Video::KeyboardEvent key) const;
		bool pressed(Video::KeyboardEvent key) const;
		bool released(Video::KeyboardEvent key) const;

		//mouse buttons: held, went down in this frame, went up in this frame
		bool down(Video::MouseButtonEvent button) const;
		bool pressed(Video::MouseButtonEvent button) const;
		bool released(Video::MouseButtonEvent button) const;

		//mouse: position in the window and its motion in this frame (pixels)
		const Vec2& mouse() const;
		const Vec2& mouse_delta() const;
		//wheel scroll in this frame
		float scroll() const;

		//the fingers on a touch screen (FINGER1, FINGER2...): held, went down in this frame, went
		//up in this frame, where (pixels of the window: the last place of one up)
		bool down(Video::FingerEvent finger) const;
		bool pressed(Video::FingerEvent finger) const;
		bool released(Video::FingerEvent finger) const;
		const Vec2& finger(Video::FingerEvent finger) const;

		//the tilt of the device: its sensor on while asked (a phone; a computer: none), the gravity
		//in the space of the screen (x right, y up, z out of it, m/s^2); false: none yet
		void tilt(bool enable);
		bool tilt(Vec3& gravity) const;

		//actions: a name bound to keys (any of them)
		void bind(const String& action, Video::KeyboardEvent key);
		void unbind(const String& action);
		bool action(const String& action) const;          //one of its keys held
		bool action_pressed(const String& action) const;  //one of its keys went down in this frame
		bool action_released(const String& action) const; //one of its keys went up in this frame

	protected:
		//state of a key/button: held, and the changes of the frame
		struct Button
		{
			bool down{ false };
			bool pressed{ false };
			bool released{ false };
		};
		static constexpr size_t KEY_COUNT    = size_t(Video::KEY_LAST) + 1;
		static constexpr size_t BUTTON_COUNT = size_t(Video::MOUSE_BUTTON_LAST) + 1;
		static constexpr size_t FINGER_COUNT = size_t(Video::FINGERMAX);

		//an event (next: the state being built by the events, until the next update)
		static void change(Button& button, Video::ActionEvent action);
		//the state of the frame from the events, then the next one starts from it
		template < size_t N > static void latch(Array<Button, N>& state, Array<Button, N>& next);

		Video::Input* m_input{ nullptr };
		bool          m_close_requested{ false };
		Vector<InputListener*> m_listeners;
		//state of the frame, and the next one
		Array<Button, KEY_COUNT>    m_keys;
		Array<Button, KEY_COUNT>    m_keys_next;
		Array<Button, BUTTON_COUNT> m_buttons;
		Array<Button, BUTTON_COUNT> m_buttons_next;
		Vec2  m_mouse{ 0.0f };
		Vec2  m_mouse_next{ 0.0f };
		Vec2  m_mouse_delta{ 0.0f };
		float m_scroll{ 0.0f };
		float m_scroll_next{ 0.0f };
		Array<Button, FINGER_COUNT> m_fingers;
		Array<Button, FINGER_COUNT> m_fingers_next;
		Array<Vec2, FINGER_COUNT>   m_finger_places;
		Array<Vec2, FINGER_COUNT>   m_finger_places_next;
		//the tilt: the last gravity, any yet
		Vec3  m_tilt{ 0.0f };
		bool  m_tilted{ false };
		//actions
		UnorderedMap<String, Vector<Video::KeyboardEvent>> m_actions;
	};
}
