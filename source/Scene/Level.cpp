

//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Scene/Level.h"
#include <algorithm>

namespace Square
{
namespace Scene
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Level);

	//Registration in context
	void Level::object_registration(Context& ctx)
	{
		//factory
		ctx.add_object<Level>();
		//Attributes
		ctx.add_attributes<Level>(attribute_field("name", std::string(), &Level::m_name));
	}

	//constructor
	Level::Level(Context& context) : Object(context)
	{
	}

	//serialize
	void Level::serialize(Data::Archive& archivie)
	{
		//serialize this
		Data::serialize(archivie, this);
		//serialize actors
		{
			uint64 size = m_actors.size();
			archivie % size;
			for (auto actor : m_actors)
			{
				actor->serialize(archivie);
			}
		}
	}
	void Level::serialize_json(Data::Json& archivie)
	{
		//to do
	}
	//deserialize
	void Level::deserialize(Data::Archive& archivie)
	{
		///clear
		m_actors.clear(); //todo: call events
						  //deserialize this
		Data::deserialize(archivie, this);
		//deserialize childs
		{
			uint64 size = 0;
			archivie % size;
			for (uint64 i = 0; i != size; ++i)
			{
				actor()->deserialize(archivie);
			}
		}
	}
	void Level::deserialize_json(Data::Json& archivie)
	{
		//to do
	}
	
	//add an actor
	void Level::add(Shared<Actor> actor)
	{
		if (!actor) return;
		actor->remove_from_parent();
		m_actors.push_back(actor);
		actor->level(this);
	}
	
	//remove an actor
	void Level::remove(Shared<Actor> actor)
	{
		if (!actor->parent())
		{
			//remove child from list
			auto it = std::find(m_actors.begin(), m_actors.end(), actor);
			if (it != m_actors.end()) m_actors.erase(it);
		}
	}

	//get/create actor
	Shared<Actor> Level::actor()
	{
		//create
		auto actor = std::make_shared<Actor>(context());
		//add
		add(actor);
		//return
		return actor;
	}
	Shared<Actor> Level::actor(size_t index)
	{
		return m_actors.size() ? m_actors[index] : nullptr;
	}
	Shared<Actor> Level::actor(const std::string& name)
	{
		//search
		for (auto actor : m_actors) if (actor->name() == name) return actor;
		//create
		auto actor = std::make_shared<Actor>(context(), name);
		//add
		add(actor);
		//return
		return actor;
	}
	const ActorList& Level::actors() const
	{
		return m_actors;
	}

	//search
	Shared<Actor> Level::find(const std::string& path_names)
	{
		//fail
		if (!path_names.size()) return nullptr;
		//ptr to actor
		Shared<Actor> c_actor;
		//ptr to current list
		const ActorList* c_actor_list = &m_actors;
		//ptr
		const char* ptr = path_names.c_str();
		//ptr name
		std::string name;
		//pre alloc
		name.reserve(path_names.size());
		//for all tokens
		while (true)
		{
			//void
			name.clear();
			//cases
			while (*ptr)
			{ 
				if (*ptr == '.') 
				{
					++ptr; 
					break;
				}
				else if (*ptr == '\\' && *(ptr + 1) == '.')
				{
					name += '.';  
					ptr += 2;
				}
				else
				{
					name += *ptr; 
					++ptr;
				}
			}
			//continue
			bool do_continue = false;
			//search
			for (auto actor : *c_actor_list)
			{
				if (actor->name() == name)
				{
					c_actor = actor;
					c_actor_list = &actor->childs();
					do_continue = true;
					break;
				}
			}
			//continue?
			if (!do_continue || !*ptr) return c_actor;
		}
	}

	//contains an actor
	bool Level::contains(Shared<Actor> actor) const
	{
		//local
		if (std::find(m_actors.begin(), m_actors.end(), actor) != m_actors.end()) return true;
		//deph
		for (auto child : m_actors) if (contains(actor)) return true;
		//fail
		return false;
	}

	//name
	const std::string& Level::name() const
	{
		return m_name;
	}
	void Level::name(const std::string& name)
	{
		m_name = name;
	}

	//message to actors
	void Level::send_message(const Variant& variant, bool brodcast)
	{
		//create message
		Message msg(nullptr, variant);
		//send
		send_message(msg, brodcast);
	}
	void Level::send_message(const Message& msg, bool brodcast)
	{
		//to all childs
		for (auto actor : m_actors)
		{
			actor->send_message(msg, brodcast);
		}
	}

}
}