

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
#include "Square/Scene/Actor.h"
#include "Square/Scene/Level.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/World.h"
#include <algorithm>

namespace Square
{
namespace Scene
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(World);

	//Registration in context
	void World::object_registration(Context& ctx)
	{
		//factory
		ctx.add_object<World>();
		//Attributes
		ctx.add_attribute_field<World, std::string>("name", std::string(), &World::m_name);
	}

	World::World(Context& context) : Object(context), SharedObject_t(context.allocator())
	{

	}
	//name        
	const std::string& World::name() const
	{
		return m_name;
	}
	void World::name(const std::string& name)
	{
		m_name = name;
	}

	//add an level	
	Shared<Level> World::level()
	{
		m_levels.push_back(MakeShared<Level>(context()));
		return m_levels.back();
	}
	void  World::add(Shared<Level> level)
	{
		m_levels.push_back(level);
	}

	//query
	Shared<Level> World::level(size_t index)
	{
		return m_levels[index];
	}
	Shared<Level> World::level(const std::string& name)
	{
		//search
		for (const Shared<Level>& level : m_levels) if (level->name() == name) return level;
		//create
		auto level = MakeShared<Level>(context(), name);
		//add
		add(level);
		//return
		return level;
	}		
	const LevelList& World::levels() const
	{
		return m_levels;
	}
	Shared<Actor> World::find_actor(const std::string& name)
	{
		//search
		for (const Shared<Level>& level : m_levels)
			if (auto& actor = level->find_actor(name)) 
				return actor;
		//return
		return nullptr;
	}

	//contains an actor
	bool World::contains(Shared<Actor> child) const
	{
		//search
		for (const Shared<Level>& level : m_levels)
			if (level->contains(child))
				return true;
		//return
		return false;
	}
	bool World::contains(Shared<Level> level_) const
	{
		//search
		for (const Shared<Level>& level : m_levels)
			if (level == level_)
				return true;
		//return
		return false;
	}

	//remove an actor
	bool World::remove(Shared<Actor> child)
	{
		//search
		for (Shared<Level>& level : m_levels)
			if (level->remove(child))
				return true;
		//return
		return false;
	}
	bool World::remove(Shared<Level> level_)
	{
		//remove child from list
		auto it = std::find(m_levels.begin(), m_levels.end(), level_);
		if (it != m_levels.end()) { m_levels.erase(it); return true; }
		//return
		return false;
	}

	//message
	void World::send_message(const VariantRef& value, bool brodcast)
	{
		for (Shared<Level>& level : m_levels)
			level->send_message(value, brodcast);
	}
	void World::send_message(const Message& msg, bool brodcast)
	{
		for (Shared<Level>& level : m_levels)
			level->send_message(msg, brodcast);
	}

	//serialize
	void World::serialize(Data::Archive& archivie)
	{
		//serialize this
		Data::serialize(archivie, this);
		//serialize actors
		{
			uint64 size = m_levels.size();
			archivie % size;
			for (auto& level : m_levels)
			{
				level->serialize(archivie);
			}
		}
	}
	void  World::serialize_json(Data::Json& archivie)
	{
		//todo
	}
	//deserialize
	void  World::deserialize(Data::Archive& archivie)
	{
		///clear
		m_levels.clear(); //todo: call events
						  //deserialize this
		Data::deserialize(archivie, this);
		//deserialize childs
		{
			uint64 size = 0;
			archivie % size;
			for (uint64 i = 0; i != size; ++i)
			{
				level()->deserialize(archivie);
			}
		}
	}
	void  World::deserialize_json(Data::Json& archivie)
	{
		//todo
	}

}
}