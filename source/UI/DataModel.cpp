//
//  DataModel.cpp
//  Square
//
//  See Square/UI/DataModel.h.
//
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Context.h>
#include "Square/Core/Context.h"
#include "Square/System/UISystem.h"
#include "Square/UI/DataModel.h"

namespace Square
{
namespace UI
{
	//the constructor of the model of RmlUi
	struct DataModel::State
	{
		Rml::DataModelConstructor m_constructor;
		//the owner (Context::create_data_model): the model goes with the last DataModel, before
		//the variables bound (of the game) go, the documents must not read them after
		Square::Context*          m_context{ nullptr };
		Rml::Context*             m_owner{ nullptr };
		std::string               m_name;

		~State()
		{
			if (!m_context || !m_owner) return;
			//RmlUi still alive (it is not after UISystem::shutdown)
			auto* ui_system = System::get<UISystem>(*m_context);
			if (ui_system && ui_system->ui().native() == m_owner) m_owner->RemoveDataModel(m_name);
		}
	};

	//make a DataModel of a constructor of RmlUi, owner of it when made by a context (Context.cpp)
	DataModel make_data_model(Rml::DataModelConstructor constructor, Square::Context* context, Rml::Context* owner, const std::string& name)
	{
		if (!constructor) return DataModel();
		auto state = std::make_shared<DataModel::State>();
		state->m_constructor = constructor;
		state->m_context = context;
		state->m_owner = owner;
		state->m_name = name;
		return DataModel(state);
	}

	DataModel::DataModel(const std::shared_ptr<State>& state) : m_state(state)
	{
	}

	bool DataModel::valid() const
	{
		return m_state && bool(m_state->m_constructor);
	}

	bool DataModel::bind(const std::string& name, bool* value)         { return valid() && m_state->m_constructor.Bind(name, value); }
	bool DataModel::bind(const std::string& name, int* value)          { return valid() && m_state->m_constructor.Bind(name, value); }
	bool DataModel::bind(const std::string& name, unsigned int* value) { return valid() && m_state->m_constructor.Bind(name, value); }
	bool DataModel::bind(const std::string& name, float* value)        { return valid() && m_state->m_constructor.Bind(name, value); }
	bool DataModel::bind(const std::string& name, double* value)       { return valid() && m_state->m_constructor.Bind(name, value); }
	bool DataModel::bind(const std::string& name, std::string* value)  { return valid() && m_state->m_constructor.Bind(name, value); }

	bool DataModel::bind_function(const std::string& name, const std::function<std::string()>& get, const std::function<void(const std::string&)>& set)
	{
		if (!valid() || !get) return false;
		Rml::DataSetFunc set_func;
		if (set)
		{
			set_func = [set](const Rml::Variant& variant) { set(variant.Get<Rml::String>()); };
		}
		return m_state->m_constructor.BindFunc(name, [get](Rml::Variant& variant) { variant = get(); }, set_func);
	}

	bool DataModel::bind_event(const std::string& name, const EventCallback& callback)
	{
		if (!valid() || !callback) return false;
		return m_state->m_constructor.BindEventCallback(name, [callback](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&)
		{
			Event ui_event(event);
			callback(ui_event);
		});
	}

	void DataModel::dirty(const std::string& name)
	{
		if (valid()) m_state->m_constructor.GetModelHandle().DirtyVariable(name);
	}

	void DataModel::dirty_all()
	{
		if (valid()) m_state->m_constructor.GetModelHandle().DirtyAllVariables();
	}
}
}
