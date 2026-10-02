//
//  Event.cpp
//  Square
//
//  See Square/UI/Event.h. A callback is an Rml::EventListener of its element (ListenerBridge),
//  deleted when it is detached (remove, or the element goes); the Listener handle sees it
//  through a Link the bridge owns.
//
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Input.h>
#include "Square/UI/Element.h"
#include "Square/UI/Event.h"
#include "EventBridge.h"

namespace Square
{
namespace UI
{
	//////////////////////////////////////////////////////////////////////////////////////////
	//names
	const char* event_name(EventType type)
	{
		switch (type)
		{
		case EventType::CLICK:         return "click";
		case EventType::DBLCLICK:      return "dblclick";
		case EventType::MOUSEDOWN:     return "mousedown";
		case EventType::MOUSEUP:       return "mouseup";
		case EventType::MOUSEOVER:     return "mouseover";
		case EventType::MOUSEOUT:      return "mouseout";
		case EventType::MOUSEMOVE:     return "mousemove";
		case EventType::MOUSESCROLL:   return "mousescroll";
		case EventType::KEYDOWN:       return "keydown";
		case EventType::KEYUP:         return "keyup";
		case EventType::TEXTINPUT:     return "textinput";
		case EventType::FOCUS:         return "focus";
		case EventType::BLUR:          return "blur";
		case EventType::CHANGE:        return "change";
		case EventType::SUBMIT:        return "submit";
		case EventType::DRAGSTART:     return "dragstart";
		case EventType::DRAG:          return "drag";
		case EventType::DRAGEND:       return "dragend";
		case EventType::DRAGOVER:      return "dragover";
		case EventType::DRAGOUT:       return "dragout";
		case EventType::DRAGMOVE:      return "dragmove";
		case EventType::DRAGDROP:      return "dragdrop";
		case EventType::SCROLL:        return "scroll";
		case EventType::RESIZE:        return "resize";
		case EventType::LOAD:          return "load";
		case EventType::UNLOAD:        return "unload";
		case EventType::SHOW:          return "show";
		case EventType::HIDE:          return "hide";
		case EventType::ANIMATIONEND:  return "animationend";
		case EventType::TRANSITIONEND: return "transitionend";
		case EventType::TABCHANGE:     return "tabchange";
		default:                       return "";
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//Event
	Event::Event(Rml::Event& event) : m_event(&event)
	{
	}

	const std::string& Event::type() const
	{
		return m_event->GetType();
	}

	Element Event::target() const
	{
		return Element(m_event->GetTargetElement());
	}

	Element Event::current() const
	{
		return Element(m_event->GetCurrentElement());
	}

	EventPhase Event::phase() const
	{
		switch (m_event->GetPhase())
		{
		case Rml::EventPhase::Capture: return EventPhase::CAPTURE;
		case Rml::EventPhase::Target:  return EventPhase::TARGET;
		case Rml::EventPhase::Bubble:  return EventPhase::BUBBLE;
		default:                       return EventPhase::NONE;
		}
	}

	void Event::stop()
	{
		m_event->StopPropagation();
	}

	void Event::stop_immediate()
	{
		m_event->StopImmediatePropagation();
	}

	Vec2 Event::mouse_position() const
	{
		return Vec2(m_event->GetParameter<float>("mouse_x", 0.0f), m_event->GetParameter<float>("mouse_y", 0.0f));
	}

	int Event::mouse_button() const
	{
		return m_event->GetParameter<int>("button", -1);
	}

	Vec2 Event::scroll_delta() const
	{
		return Vec2(m_event->GetParameter<float>("wheel_delta_x", 0.0f), m_event->GetParameter<float>("wheel_delta_y", 0.0f));
	}

	int Event::key() const
	{
		return m_event->GetParameter<int>("key_identifier", 0);
	}

	int Event::modifiers() const
	{
		int modifiers = 0;
		if (m_event->GetParameter<int>("ctrl_key", 0))        modifiers |= Rml::Input::KM_CTRL;
		if (m_event->GetParameter<int>("shift_key", 0))       modifiers |= Rml::Input::KM_SHIFT;
		if (m_event->GetParameter<int>("alt_key", 0))         modifiers |= Rml::Input::KM_ALT;
		if (m_event->GetParameter<int>("meta_key", 0))        modifiers |= Rml::Input::KM_META;
		if (m_event->GetParameter<int>("caps_lock_key", 0))   modifiers |= Rml::Input::KM_CAPSLOCK;
		if (m_event->GetParameter<int>("num_lock_key", 0))    modifiers |= Rml::Input::KM_NUMLOCK;
		if (m_event->GetParameter<int>("scroll_lock_key", 0)) modifiers |= Rml::Input::KM_SCROLLLOCK;
		return modifiers;
	}

	std::string Event::text() const
	{
		return m_event->GetParameter<Rml::String>("text", "");
	}

	std::string Event::value() const
	{
		return m_event->GetParameter<Rml::String>("value", "");
	}

	std::string Event::form_values() const
	{
		//the parameters of a submit: its fields (name=value&...)
		std::string values;
		for (const auto& parameter : m_event->GetParameters())
		{
			if (!values.empty()) values += "&";
			values += parameter.first + "=" + parameter.second.Get<Rml::String>();
		}
		return values;
	}

	Element Event::drag_element() const
	{
		return Element(static_cast<Rml::Element*>(m_event->GetParameter<void*>("drag_element", nullptr)));
	}

	std::string Event::property() const
	{
		return m_event->GetParameter<Rml::String>("property", "");
	}

	int Event::tab_index() const
	{
		return m_event->GetParameter<int>("tab_index", -1);
	}

	std::string Event::parameter(const std::string& name) const
	{
		return m_event->GetParameter<Rml::String>(name, "");
	}

	Rml::Event& Event::native() const
	{
		return *m_event;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//ListenerBridge
	ListenerBridge::ListenerBridge(Rml::Element* element, const std::string& type, const EventCallback& callback, bool capture)
	: m_link(std::make_shared<Listener::Link>())
	, m_element(element)
	, m_type(type)
	, m_callback(callback)
	, m_capture(capture)
	{
		m_link->m_bridge = this;
	}

	ListenerBridge::~ListenerBridge()
	{
		m_link->m_bridge = nullptr;
	}

	void ListenerBridge::ProcessEvent(Rml::Event& event)
	{
		if (!m_callback) return;
		Event ui_event(event);
		m_callback(ui_event);
	}

	void ListenerBridge::OnDetach(Rml::Element* /*element*/)
	{
		//removed, or its element goes: no more events
		delete this;
	}

	void ListenerBridge::detach()
	{
		//OnDetach deletes this
		m_element->RemoveEventListener(m_type, this, m_capture);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//Listener
	Listener::Listener(const std::shared_ptr<Link>& link) : m_link(link)
	{
	}

	bool Listener::valid() const
	{
		auto link = m_link.lock();
		return link && link->m_bridge;
	}

	void Listener::remove()
	{
		if (auto link = m_link.lock())
		{
			if (link->m_bridge) link->m_bridge->detach();
		}
		m_link.reset();
	}
}
}
