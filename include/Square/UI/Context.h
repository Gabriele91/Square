//
//  Context.h
//  Square
//
//  The UI of the engine, RmlUi inside: documents in RML (HTML like) styled by RCSS (CSS like),
//  drawn over the frame. A Context holds the documents of the window (UISystem::context()).
//
#pragma once
#include <string>
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/UI/Element.h"
#include "Square/UI/DataModel.h"

namespace Rml
{
	class Context;
}

namespace Square
{
	class Context;
}

namespace Square
{
namespace UI
{
	class SQUARE_API Context
	{
	public:
		Context() = default;
		Context(Square::Context& context, Rml::Context* ui_context);

		//a context
		bool valid() const;

		//a document (.rml): its path, from the working folder or the resource folder; not
		//shown (Document::show)
		Document load(const std::string& path);
		//a document from its RML
		Document load_from_memory(const std::string& rml);
		//a loaded document by its id (the id of its body)
		Document document(const std::string& id) const;
		//the element with the focus, the one under the mouse
		Element focused() const;
		Element hovered() const;

		//data models (before the documents that use them)
		DataModel create_data_model(const std::string& name);
		DataModel data_model(const std::string& name) const;
		bool remove_data_model(const std::string& name);

		//a font (.ttf, .otf), for every document; fallback: for the glyphs the others have not
		static bool load_font(const std::string& path, bool fallback = false);

		//the debugger of RmlUi over the documents
		void debugger(bool visible);
		bool debugger() const;

		//size of the context (pixels)
		IVec2 size() const;

		//the context of RmlUi (needs the headers of RmlUi)
		Rml::Context* native() const;

	private:
		Square::Context* m_context{ nullptr };
		Rml::Context*    m_ui_context{ nullptr };
	};
}
}
