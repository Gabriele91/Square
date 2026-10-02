//
//  Element.cpp
//  Square
//
//  See Square/UI/Element.h.
//
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/StringUtilities.h>
#include "Square/UI/Element.h"
#include "EventBridge.h"

namespace Square
{
namespace UI
{
	//////////////////////////////////////////////////////////////////////////////////////////
	//Element
	Element::Element(Rml::Element* element) : m_element(element)
	{
	}

	bool Element::valid() const
	{
		return m_element != nullptr;
	}

	std::string Element::tag() const
	{
		return m_element ? m_element->GetTagName() : std::string();
	}

	std::string Element::id() const
	{
		return m_element ? m_element->GetId() : std::string();
	}

	void Element::set_id(const std::string& id)
	{
		if (m_element) m_element->SetId(id);
	}

	//tree
	Element Element::find(const std::string& id) const
	{
		return Element(m_element ? m_element->GetElementById(id) : nullptr);
	}

	Element Element::query(const std::string& selector) const
	{
		return Element(m_element ? m_element->QuerySelector(selector) : nullptr);
	}

	std::vector<Element> Element::query_all(const std::string& selector) const
	{
		std::vector<Element> elements;
		if (m_element)
		{
			Rml::ElementList list;
			m_element->QuerySelectorAll(list, selector);
			for (Rml::Element* element : list)
			{ 
				elements.emplace_back(element);
			}
		}
		return elements;
	}

	Element Element::parent() const
	{
		return Element(m_element ? m_element->GetParentNode() : nullptr);
	}

	std::vector<Element> Element::children() const
	{
		std::vector<Element> elements;
		if (!m_element) return elements;
		for (int i = 0, count = m_element->GetNumChildren(); i != count; ++i)
		{
			elements.emplace_back(m_element->GetChild(i));
		}
		return elements;
	}

	Document Element::document() const
	{
		return Document(m_element ? m_element->GetOwnerDocument() : nullptr);
	}

	//content
	void Element::set_text(const std::string& text)
	{
		if (m_element) m_element->SetInnerRML(Rml::StringUtilities::EncodeRml(text));
	}

	std::string Element::text() const
	{
		return m_element ? Rml::StringUtilities::DecodeRml(m_element->GetInnerRML()) : std::string();
	}

	void Element::set_html(const std::string& rml)
	{
		if (m_element) m_element->SetInnerRML(rml);
	}

	std::string Element::html() const
	{
		return m_element ? m_element->GetInnerRML() : std::string();
	}

	//classes, attributes, properties
	void Element::set_class(const std::string& name, bool enable)
	{
		if (m_element) m_element->SetClass(name, enable);
	}

	bool Element::has_class(const std::string& name) const
	{
		return m_element && m_element->IsClassSet(name);
	}

	void Element::set_pseudo_class(const std::string& name, bool enable)
	{
		if (m_element) m_element->SetPseudoClass(name, enable);
	}

	bool Element::has_pseudo_class(const std::string& name) const
	{
		return m_element && m_element->IsPseudoClassSet(name);
	}

	void Element::set_attribute(const std::string& name, const std::string& value)
	{
		if (m_element) m_element->SetAttribute(name, value);
	}

	std::string Element::attribute(const std::string& name) const
	{
		return m_element ? m_element->GetAttribute<Rml::String>(name, "") : std::string();
	}

	bool Element::has_attribute(const std::string& name) const
	{
		return m_element && m_element->GetAttribute(name) != nullptr;
	}

	void Element::remove_attribute(const std::string& name)
	{
		if (m_element) m_element->RemoveAttribute(name);
	}

	bool Element::set_property(const std::string& name, const std::string& value)
	{
		return m_element && m_element->SetProperty(name, value);
	}

	void Element::remove_property(const std::string& name)
	{
		if (m_element) m_element->RemoveProperty(name);
	}

	//state and actions
	bool Element::visible() const
	{
		return m_element && m_element->IsVisible(true);
	}

	void Element::set_enabled(bool enable)
	{
		if (!m_element) return;
		if (enable) m_element->RemoveAttribute("disabled");
		else        m_element->SetAttribute("disabled", Rml::String());
	}

	bool Element::enabled() const
	{
		return m_element && m_element->GetAttribute("disabled") == nullptr;
	}

	bool Element::focus()
	{
		return m_element && m_element->Focus();
	}

	void Element::blur()
	{
		if (m_element) m_element->Blur();
	}

	void Element::click()
	{
		if (m_element) m_element->Click();
	}

	void Element::scroll_into_view(bool align_with_top)
	{
		if (m_element) m_element->ScrollIntoView(align_with_top);
	}

	Vec2 Element::size() const
	{
		if (!m_element) return Vec2(0.0f);
		const Rml::Vector2f size = m_element->GetBox().GetSize(Rml::BoxArea::Border);
		return Vec2(size.x, size.y);
	}

	Vec2 Element::position() const
	{
		if (!m_element) return Vec2(0.0f);
		const Rml::Vector2f offset = m_element->GetAbsoluteOffset(Rml::BoxArea::Border);
		return Vec2(offset.x, offset.y);
	}

	//children
	Element Element::create_child(const std::string& tag)
	{
		if (!m_element || !m_element->GetOwnerDocument()) return Element();
		return Element(m_element->AppendChild(m_element->GetOwnerDocument()->CreateElement(tag)));
	}

	void Element::remove()
	{
		if (!m_element || !m_element->GetParentNode()) return;
		m_element->GetParentNode()->RemoveChild(m_element);
		m_element = nullptr;
	}

	//events
	Listener Element::on(EventType type, const EventCallback& callback, bool capture)
	{
		return on(event_name(type), callback, capture);
	}

	Listener Element::on(const std::string& type, const EventCallback& callback, bool capture)
	{
		if (!m_element || type.empty()) return Listener();
		auto* bridge = new ListenerBridge(m_element, type, callback, capture);
		Listener listener(bridge->link());
		m_element->AddEventListener(type, bridge, capture);
		return listener;
	}

	bool Element::dispatch(const std::string& type)
	{
		return m_element && m_element->DispatchEvent(type, Rml::Dictionary());
	}

	Rml::Element* Element::native() const
	{
		return m_element;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//FormControl
	FormControl::FormControl(const Element& element) : Element(element)
	{
	}

	Rml::ElementFormControl* FormControl::control() const
	{
		return dynamic_cast<Rml::ElementFormControl*>(m_element);
	}

	bool FormControl::valid() const
	{
		return control() != nullptr;
	}

	std::string FormControl::value() const
	{
		auto* form_control = control();
		return form_control ? form_control->GetValue() : std::string();
	}

	void FormControl::set_value(const std::string& value)
	{
		if (auto* form_control = control()) form_control->SetValue(value);
	}

	bool FormControl::disabled() const
	{
		auto* form_control = control();
		return form_control && form_control->IsDisabled();
	}

	void FormControl::set_disabled(bool disable)
	{
		if (auto* form_control = control()) form_control->SetDisabled(disable);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//Document
	Document::Document(Rml::ElementDocument* document) : Element(document)
	{
	}

	Rml::ElementDocument* Document::native_document() const
	{
		return dynamic_cast<Rml::ElementDocument*>(m_element);
	}

	bool Document::valid() const
	{
		return native_document() != nullptr;
	}

	void Document::show(bool modal)
	{
		if (auto* document = native_document()) 
			document->Show(modal ? Rml::ModalFlag::Modal : Rml::ModalFlag::None);
	}

	void Document::hide()
	{
		if (auto* document = native_document()) 
			document->Hide();
	}

	void Document::close()
	{
		if (auto* document = native_document()) 
			document->Close();
		m_element = nullptr;
	}

	std::string Document::title() const
	{
		auto* document = native_document();
		return document ? document->GetTitle() : std::string();
	}

	void Document::set_title(const std::string& title)
	{
		if (auto* document = native_document()) document->SetTitle(title);
	}
}
}
