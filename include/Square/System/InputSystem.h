//
//  InputSystem.h
//  Square
//
//  The input of the window (Video::Input): reads the events at the start of the frame
//  (update) and sends them to the application (AppInterface).
//  It also keeps the state of the frame, for who asks it (components, systems): keys and mouse
//  buttons held, pressed or released in this frame, the mouse position, and the actions (a
//  name bound to one or more keys: "forward" -> up, W).
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

		//actions: a name bound to keys (any of them)
		void bind(const std::string& action, Video::KeyboardEvent key);
		void unbind(const std::string& action);
		bool action(const std::string& action) const;          //one of its keys held
		bool action_pressed(const std::string& action) const;  //one of its keys went down in this frame
		bool action_released(const std::string& action) const; //one of its keys went up in this frame

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

		//an event (next: the state being built by the events, until the next update)
		static void change(Button& button, Video::ActionEvent action);
		//the state of the frame from the events, then the next one starts from it
		template < size_t N > static void latch(std::array<Button, N>& state, std::array<Button, N>& next);

		Video::Input* m_input{ nullptr };
		bool          m_close_requested{ false };
		std::vector<InputListener*> m_listeners;
		//state of the frame, and the next one
		std::array<Button, KEY_COUNT>    m_keys;
		std::array<Button, KEY_COUNT>    m_keys_next;
		std::array<Button, BUTTON_COUNT> m_buttons;
		std::array<Button, BUTTON_COUNT> m_buttons_next;
		Vec2  m_mouse{ 0.0f };
		Vec2  m_mouse_next{ 0.0f };
		Vec2  m_mouse_delta{ 0.0f };
		float m_scroll{ 0.0f };
		float m_scroll_next{ 0.0f };
		//actions
		std::unordered_map<std::string, std::vector<Video::KeyboardEvent>> m_actions;
	};
}
