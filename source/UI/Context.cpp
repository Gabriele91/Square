//
//  Context.cpp
//  Square
//
//  See Square/UI/Context.h.
//
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger/Debugger.h>
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Logger.h"
#include "Square/UI/Context.h"

namespace Square
{
namespace UI
{
	//DataModel.cpp
	DataModel make_data_model(Rml::DataModelConstructor constructor, Square::Context* context, Rml::Context* owner, const std::string& name);

	//a path: as it is, else in the resource folder
	static std::string find_path(const std::string& path)
	{
		if (Filesystem::exists(path)) return path;
		const std::string resource_path = Filesystem::join(Filesystem::resource_dir(), path);
		if (Filesystem::exists(resource_path)) return resource_path;
		return path;
	}

	Context::Context(Square::Context& context, Rml::Context* ui_context)
	: m_context(&context)
	, m_ui_context(ui_context)
	{
	}

	bool Context::valid() const
	{
		return m_ui_context != nullptr;
	}

	Document Context::load(const std::string& path)
	{
		if (!m_ui_context) return Document();
		Rml::ElementDocument* document = m_ui_context->LoadDocument(find_path(path));
		if (!document && m_context) m_context->logger()->warning("UI: unable to load " + path);
		return Document(document);
	}

	Document Context::load_from_memory(const std::string& rml)
	{
		return Document(m_ui_context ? m_ui_context->LoadDocumentFromMemory(rml) : nullptr);
	}

	Document Context::document(const std::string& id) const
	{
		return Document(m_ui_context ? m_ui_context->GetDocument(id) : nullptr);
	}

	Element Context::focused() const
	{
		return Element(m_ui_context ? m_ui_context->GetFocusElement() : nullptr);
	}

	Element Context::hovered() const
	{
		return Element(m_ui_context ? m_ui_context->GetHoverElement() : nullptr);
	}

	DataModel Context::create_data_model(const std::string& name)
	{
		return m_ui_context ? make_data_model(m_ui_context->CreateDataModel(name), m_context, m_ui_context, name) : DataModel();
	}

	DataModel Context::data_model(const std::string& name) const
	{
		return m_ui_context ? make_data_model(m_ui_context->GetDataModel(name), nullptr, nullptr, name) : DataModel();
	}

	bool Context::remove_data_model(const std::string& name)
	{
		return m_ui_context && m_ui_context->RemoveDataModel(name);
	}

	bool Context::load_font(const std::string& path, bool fallback)
	{
		return Rml::LoadFontFace(find_path(path), fallback);
	}

	void Context::debugger(bool visible)
	{
		Rml::Debugger::SetVisible(visible);
	}

	bool Context::debugger() const
	{
		return Rml::Debugger::IsVisible();
	}

	IVec2 Context::size() const
	{
		if (!m_ui_context) return IVec2(0);
		const Rml::Vector2i size = m_ui_context->GetDimensions();
		return IVec2(size.x, size.y);
	}

	Rml::Context* Context::native() const
	{
		return m_ui_context;
	}
}
}
