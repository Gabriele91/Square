//
//  EventBridge.h
//  Square
//
//  A callback of the UI as an Rml::EventListener of its element (inside Square): see Event.cpp.
//
#pragma once
#include <memory>
#include <string>
#include <RmlUi/Core/EventListener.h>
#include "Square/UI/Event.h"

namespace Square
{
namespace UI
{
	class ListenerBridge;

	//what a Listener sees of its bridge (null: detached)
	struct Listener::Link
	{
		ListenerBridge* m_bridge{ nullptr };
	};

	class ListenerBridge : public Rml::EventListener
	{
	public:
		ListenerBridge(Rml::Element* element, const std::string& type, const EventCallback& callback, bool capture);
		virtual ~ListenerBridge();

		//Rml::EventListener
		void ProcessEvent(Rml::Event& event) override;
		void OnDetach(Rml::Element* element) override;

		//remove it from its element (it is deleted)
		void detach();
		const std::shared_ptr<Listener::Link>& link() const { return m_link; }

	private:
		std::shared_ptr<Listener::Link> m_link;
		Rml::Element*  m_element{ nullptr };
		std::string    m_type;
		EventCallback  m_callback;
		bool           m_capture{ false };
	};
}
}
