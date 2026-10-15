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
	//a variable bound, its value when it was shown last (dirty_changed)
	struct WatchedVariable
	{
		enum class Type : uint8 { BOOL, INT, UINT, FLOAT, DOUBLE, STRING };
		String m_name;
		Type   m_type{ Type::BOOL };
		void*  m_value{ nullptr };
		union
		{
			bool         m_bool;
			int          m_int;
			unsigned int m_uint;
			float        m_float;
			double       m_double;
		}      m_shown{};
		String m_shown_string;

		//its value now kept: true when it changed
		bool update()
		{
			switch (m_type)
			{
			case Type::BOOL:   return update(m_shown.m_bool, *static_cast<bool*>(m_value));
			case Type::INT:    return update(m_shown.m_int, *static_cast<int*>(m_value));
			case Type::UINT:   return update(m_shown.m_uint, *static_cast<unsigned int*>(m_value));
			case Type::FLOAT:  return update(m_shown.m_float, *static_cast<float*>(m_value));
			case Type::DOUBLE: return update(m_shown.m_double, *static_cast<double*>(m_value));
			case Type::STRING: return update(m_shown_string, *static_cast<String*>(m_value));
			default: return false;
			}
		}

	private:
		template < class T >
		static bool update(T& shown, const T& value)
		{
			if (shown == value) return false;
			shown = value;
			return true;
		}
	};

	//the constructor of the model of RmlUi
	struct DataModel::State
	{
		Rml::DataModelConstructor m_constructor;
		Vector<WatchedVariable>   m_watched;

		//a variable bound: watched by dirty_changed
		template < class T >
		bool watch(bool bound, const String& name, WatchedVariable::Type type, T* value)
		{
			if (!bound) return false;
			WatchedVariable variable;
			variable.m_name = name;
			variable.m_type = type;
			variable.m_value = value;
			variable.update();
			m_watched.push_back(std::move(variable));
			return true;
		}
		//the owner (Context::create_data_model): the model goes with the last DataModel, before
		//the variables bound (of the game) go, the documents must not read them after
		Square::Context*          m_context{ nullptr };
		Rml::Context*             m_owner{ nullptr };
		String               m_name;

		~State()
		{
			if (!m_context || !m_owner) return;
			//RmlUi still alive (it is not after UISystem::shutdown)
			auto* ui_system = System::get<UISystem>(*m_context);
			if (ui_system && ui_system->ui().native() == m_owner) m_owner->RemoveDataModel(m_name);
		}
	};

	//make a DataModel of a constructor of RmlUi, owner of it when made by a context (Context.cpp)
	DataModel make_data_model(Rml::DataModelConstructor constructor, Square::Context* context, Rml::Context* owner, const String& name)
	{
		if (!constructor) return DataModel();
		auto state = MakeSharedPtr<DataModel::State>();
		state->m_constructor = constructor;
		state->m_context = context;
		state->m_owner = owner;
		state->m_name = name;
		return DataModel(state);
	}

	DataModel::DataModel(const Shared<State>& state) : m_state(state)
	{
	}

	Weak<DataModel::State> DataModel::weak() const
	{
		return m_state;
	}

	bool DataModel::valid() const
	{
		return m_state && bool(m_state->m_constructor);
	}

	bool DataModel::bind(const String& name, bool* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::BOOL, value);
	}

	bool DataModel::bind(const String& name, int* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::INT, value);
	}

	bool DataModel::bind(const String& name, unsigned int* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::UINT, value);
	}

	bool DataModel::bind(const String& name, float* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::FLOAT, value);
	}

	bool DataModel::bind(const String& name, double* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::DOUBLE, value);
	}

	bool DataModel::bind(const String& name, String* value)
	{
		return valid() && m_state->watch(m_state->m_constructor.Bind(name, value), name, WatchedVariable::Type::STRING, value);
	}

	bool DataModel::bind_function(const String& name, const std::function<String()>& get, const std::function<void(const String&)>& set)
	{
		if (!valid() || !get) return false;
		Rml::DataSetFunc set_func;
		if (set)
		{
			set_func = [set](const Rml::Variant& variant) { set(variant.Get<Rml::String>()); };
		}
		return m_state->m_constructor.BindFunc(name, [get](Rml::Variant& variant) { variant = get(); }, set_func);
	}

	bool DataModel::bind_event(const String& name, const EventCallback& callback)
	{
		if (!valid() || !callback) return false;
		return m_state->m_constructor.BindEventCallback(name, [callback](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&)
		{
			Event ui_event(event);
			callback(ui_event);
		});
	}

	void DataModel::dirty(const String& name)
	{
		if (valid())
		{ 
			m_state->m_constructor.GetModelHandle().DirtyVariable(name);
		}
	}

	void DataModel::dirty_all()
	{
		if (!valid()) return;
		//all of them shown: their values kept
		for (WatchedVariable& variable : m_state->m_watched)
		{ 
			variable.update();
		}
		m_state->m_constructor.GetModelHandle().DirtyAllVariables();
	}

	void DataModel::dirty_changed()
	{
		if (!valid()) return;
		Rml::DataModelHandle handle = m_state->m_constructor.GetModelHandle();
		for (WatchedVariable& variable : m_state->m_watched)
		{
			if (variable.update()) handle.DirtyVariable(variable.m_name);
		}
	}
}
}
