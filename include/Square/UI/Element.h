//
//  Element.h
//  Square
//
//  The elements of the UI (RmlUi inside): an Element, the controls of a form (FormControl), a
//  document (Document). They are handles of the elements of RmlUi, valid while the element
//  lives (a closed document, a removed element: not valid any more).
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/UI/Event.h"

namespace Rml
{
	class Element;
	class ElementDocument;
	class ElementFormControl;
}

namespace Square
{
namespace UI
{
	class Document;

	class SQUARE_API Element
	{
	public:
		Element() = default;
		Element(Rml::Element* element);

		//an element (not an empty handle)
		bool valid() const;
		explicit operator bool() const { return valid(); }
		bool operator==(const Element& other) const { return m_element == other.m_element; }
		bool operator!=(const Element& other) const { return m_element != other.m_element; }

		//its tag ("div"), its id
		std::string tag() const;
		std::string id() const;
		void set_id(const std::string& id);

		//the tree: an element by id, the first/all by a selector (".item > span"), the parent,
		//the children, its document
		Element find(const std::string& id) const;
		Element query(const std::string& selector) const;
		std::vector<Element> query_all(const std::string& selector) const;
		Element parent() const;
		std::vector<Element> children() const;
		Document document() const;

		//the content: as text (escaped), as RML
		void set_text(const std::string& text);
		std::string text() const;
		void set_html(const std::string& rml);
		std::string html() const;

		//classes, pseudo classes (":hover"), attributes, style properties ("color", "red")
		void set_class(const std::string& name, bool enable = true);
		bool has_class(const std::string& name) const;
		void set_pseudo_class(const std::string& name, bool enable = true);
		bool has_pseudo_class(const std::string& name) const;
		void set_attribute(const std::string& name, const std::string& value);
		std::string attribute(const std::string& name) const;
		bool has_attribute(const std::string& name) const;
		void remove_attribute(const std::string& name);
		bool set_property(const std::string& name, const std::string& value);
		void remove_property(const std::string& name);

		//state and actions
		bool visible() const;
		void set_enabled(bool enable);
		bool enabled() const;
		bool focus();
		void blur();
		void click();
		void scroll_into_view(bool align_with_top = true);

		//size and position (pixels of the window, its border box)
		Vec2 size() const;
		Vec2 position() const;

		//children: a new one of a tag at the end, removed from its parent
		Element create_child(const std::string& tag);
		void remove();

		//events: a callback (in the capture phase: before the children get it); a custom
		//event by name; dispatch one
		Listener on(EventType type, const EventCallback& callback, bool capture = false);
		Listener on(const std::string& type, const EventCallback& callback, bool capture = false);
		bool dispatch(const std::string& type);

		//the element of RmlUi (needs the headers of RmlUi)
		Rml::Element* native() const;

	protected:
		Rml::Element* m_element{ nullptr };
	};

	//a control of a form: input, select, textarea (any: an element without value() is not one)
	class SQUARE_API FormControl : public Element
	{
	public:
		FormControl() = default;
		FormControl(const Element& element);

		//it is a control of a form
		bool valid() const;
		std::string value() const;
		void set_value(const std::string& value);
		bool disabled() const;
		void set_disabled(bool disable);

	private:
		Rml::ElementFormControl* control() const;
	};

	//a document (a .rml): it is shown, hidden, closed
	class SQUARE_API Document : public Element
	{
	public:
		Document() = default;
		Document(Rml::ElementDocument* document);

		//a document
		bool valid() const;
		//show it (modal: only it gets the input), hide it, close it (it goes: not valid any more)
		void show(bool modal = false);
		void hide();
		void close();
		//its title
		std::string title() const;
		void set_title(const std::string& title);

		//the document of RmlUi
		Rml::ElementDocument* native_document() const;
	};
}
}
