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
		String tag() const;
		String id() const;
		void set_id(const String& id);

		//the tree: an element by id, the first/all by a selector (".item > span"), the parent,
		//the children, its document
		Element find(const String& id) const;
		Element query(const String& selector) const;
		Vector<Element> query_all(const String& selector) const;
		Element parent() const;
		Vector<Element> children() const;
		Document document() const;

		//the content: as text (escaped), as RML
		void set_text(const String& text);
		String text() const;
		void set_html(const String& rml);
		String html() const;

		//classes, pseudo classes (":hover"), attributes, style properties ("color", "red")
		void set_class(const String& name, bool enable = true);
		bool has_class(const String& name) const;
		void set_pseudo_class(const String& name, bool enable = true);
		bool has_pseudo_class(const String& name) const;
		void set_attribute(const String& name, const String& value);
		String attribute(const String& name) const;
		bool has_attribute(const String& name) const;
		void remove_attribute(const String& name);
		bool set_property(const String& name, const String& value);
		void remove_property(const String& name);

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

		//its scroll (an element with overflow: its content higher than it): how far down (pixels),
		//the height of its content, the height shown of it
		float scroll_top() const;
		void  scroll_top(float top);
		float scroll_height() const;
		float client_height() const;

		//children: a new one of a tag at the end, removed from its parent
		Element create_child(const String& tag);
		void remove();

		//events: a callback (in the capture phase: before the children get it); a custom
		//event by name; dispatch one
		Listener on(EventType type, const EventCallback& callback, bool capture = false);
		Listener on(const String& type, const EventCallback& callback, bool capture = false);
		bool dispatch(const String& type);

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
		String value() const;
		void set_value(const String& value);
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
		String title() const;
		void set_title(const String& title);

		//the document of RmlUi
		Rml::ElementDocument* native_document() const;
	};
}
}
