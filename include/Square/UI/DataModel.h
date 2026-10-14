//
//  DataModel.h
//  Square
//
//  A data model of the UI (RmlUi inside): variables of the game shown by the documents
//  ({{score}}, data-if, data-for...) and their events (data-event-click="start"). Made by
//  Context::create_data_model before the documents that use it are loaded; the model is
//  removed with the last DataModel made by it (not by Context::data_model), so keep it with
//  the variables bound: the documents never read them after they go.
//
#pragma once
#include <memory>
#include <string>
#include <functional>
#include "Square/Config.h"
#include "Square/UI/Event.h"

namespace Square
{
namespace UI
{
	class SQUARE_API DataModel
	{
	public:
		struct State;

		DataModel() = default;
		DataModel(const Shared<State>& state);

		//a model
		bool valid() const;

		//a variable of the game (by pointer: the document reads it when it is dirty)
		bool bind(const String& name, bool* value);
		bool bind(const String& name, int* value);
		bool bind(const String& name, unsigned int* value);
		bool bind(const String& name, float* value);
		bool bind(const String& name, double* value);
		bool bind(const String& name, String* value);
		//a value computed by a function (and set by one: an input of a form)
		bool bind_function(const String& name, const std::function<String()>& get, const std::function<void(const String&)>& set = {});
		//an event of the documents (data-event-click="name")
		bool bind_event(const String& name, const EventCallback& callback);

		//a variable changed: the documents update it (all of them)
		void dirty(const String& name);
		void dirty_all();

	private:
		Shared<State> m_state;
	};
}
}
