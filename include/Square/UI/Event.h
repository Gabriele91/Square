//
//  Event.h
//  Square
//
//  The events of the UI (RmlUi inside: see UI/Element.h): their types, the event given to a
//  callback, the handle of a callback (Listener).
//
#pragma once
#include <memory>
#include <string>
#include <functional>
#include "Square/Config.h"
#include "Square/Math/Linear.h"

namespace Rml
{
	class Event;
}

namespace Square
{
namespace UI
{
	class Element;

	//all the events of RmlUi (Element::on with a name: the custom ones)
	enum class EventType : unsigned char
	{
		//mouse
		CLICK,
		DBLCLICK,
		MOUSEDOWN,
		MOUSEUP,
		MOUSEOVER,
		MOUSEOUT,
		MOUSEMOVE,
		MOUSESCROLL,
		//keyboard
		KEYDOWN,
		KEYUP,
		TEXTINPUT,
		//focus
		FOCUS,
		BLUR,
		//form
		CHANGE,
		SUBMIT,
		//drag
		DRAGSTART,
		DRAG,
		DRAGEND,
		DRAGOVER,
		DRAGOUT,
		DRAGMOVE,
		DRAGDROP,
		//scroll and layout
		SCROLL,
		RESIZE,
		//document
		LOAD,
		UNLOAD,
		SHOW,
		HIDE,
		//animations
		ANIMATIONEND,
		TRANSITIONEND,
		//tab set
		TABCHANGE
	};

	//the name of an event type in RmlUi ("click")
	SQUARE_API const char* event_name(EventType type);

	//the phase of an event: going down to its target (capture), on it, going up (bubble)
	enum class EventPhase : unsigned char
	{
		NONE,
		CAPTURE,
		TARGET,
		BUBBLE
	};

	//an event, valid in its callback
	class SQUARE_API Event
	{
	public:
		Event(Rml::Event& event);

		//its name ("click") and the element it is for, and the one of the callback
		const std::string& type() const;
		Element target() const;
		Element current() const;
		EventPhase phase() const;

		//no more elements get it; neither the other callbacks of this one
		void stop();
		void stop_immediate();

		//mouse: position (pixels of the window), button (0 left, 1 right, 2 middle), wheel
		Vec2 mouse_position() const;
		int  mouse_button() const;
		Vec2 scroll_delta() const;
		//keyboard: the key (Rml::Input::KeyIdentifier), the modifiers (Rml::Input::KeyModifier
		//bits), the text of a textinput
		int  key() const;
		int  modifiers() const;
		std::string text() const;
		//form: the value of a change, the values of a submit ("name=value&...")
		std::string value() const;
		std::string form_values() const;
		//drag: the element dragged
		Element drag_element() const;
		//animations: the property animated
		std::string property() const;
		//tab set: the tab
		int tab_index() const;
		//any parameter of the event, as text ("" when it has not it)
		std::string parameter(const std::string& name) const;

		//the event of RmlUi
		Rml::Event& native() const;

	private:
		Rml::Event* m_event{ nullptr };
	};

	//the callback of an event
	using EventCallback = std::function<void(Event& event)>;

	//the handle of a callback: remove() detaches it (else it lives with its element)
	class SQUARE_API Listener
	{
	public:
		struct Link;

		Listener() = default;
		Listener(const std::shared_ptr<Link>& link);

		//it is still attached
		bool valid() const;
		//detach it
		void remove();

	private:
		std::weak_ptr<Link> m_link;
	};
}
}
