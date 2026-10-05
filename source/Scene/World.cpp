

//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Level.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/World.h"
#include "Square/System/System.h"
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
		ctx.add_attribute_field<World, std::string>("name", std::string(), offsetof(World,m_name));
	}

	World::World(Context& context) : Object(context), SharedObject_t(context.allocator())
	{
		//alive: the systems started from now on give it their instances
		context.add_world(this);
		//the systems already running
		for (const Shared<System>& system : context.systems()) add_instance(*system);
	}
	World::~World()
	{
		context().remove_world(this);
		clear_instances();
	}

	//systems
	void World::add_instance(System& system)
	{
		if (auto new_instance = system.create_instance(*this))
		{
			m_instances.push_back(new_instance);
			//the components already in the levels (the active ones: the others add them when active)
			for (const Shared<Level>& level : m_levels)
			{
				if (!level->active()) continue;
				level->visit([&](Shared<Actor> actor) -> bool
				{
					for (auto& component : actor->components()) new_instance->on_add_component(actor, component.second);
					return true;
				});
			}
		}
	}
	void World::remove_instances(const System& system)
	{
		m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(), [&system](const Shared<SystemInstance>& world_instance)
		{
			return &world_instance->system() == &system;
		}), m_instances.end());
	}
	void World::clear_instances()
	{
		m_instances.clear();
	}
	Shared<SystemInstance> World::instance(uint64 instance_id) const
	{
		for (const Shared<SystemInstance>& world_instance : m_instances)
		{
			if (world_instance->object_id() == instance_id) return world_instance;
		}
		return nullptr;
	}
	const SystemInstanceList& World::instances() const
	{
		return m_instances;
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
	Shared<Level> World::create_level(const std::string& name)
	{
		m_levels.push_back(MakeShared<Level>(context(), weak_from_this(), name));
		return m_levels.back();
	}

	//query
	Shared<Level> World::level(size_t index)
	{
		return index < m_levels.size() ? m_levels[index] : nullptr;
	}
	Shared<Level> World::level(const std::string& name)
	{
		for (const Shared<Level>& level : m_levels)
		{
			if (level->name() == name) return level;
		}
		return nullptr;
	}

	void World::active_levels(const std::vector<std::string>& names)
	{
		//off first: a level that goes on finds the systems free of the ones that went off
		auto named = [&names](const Shared<Level>& level)
		{
			return std::find(names.begin(), names.end(), level->name()) != names.end();
		};
		for (const Shared<Level>& level : m_levels)
		{
			if (!named(level)) level->active(false);
		}
		for (const Shared<Level>& level : m_levels)
		{
			if (named(level)) level->active(true);
		}
	}

	const LevelList& World::levels() const
	{
		return m_levels;
	}
	Shared<Actor> World::find_actor(const std::string& name)
	{
		//search
		for (const Shared<Level>& level : m_levels)
			if (const auto& actor = level->find_actor(name)) 
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
		if (it != m_levels.end())
		{
			//its components leave the systems of this world
			(*it)->remove_components();
			(*it)->m_world.reset();
			m_levels.erase(it);
			return true;
		}
		//return
		return false;
	}

	//every frame
	void World::update(double delta_time)
	{
		for (size_t i = 0; i < m_levels.size(); ++i)
		{
			Shared<Level> level = m_levels[i];
			level->update(delta_time);
		}
	}
	void World::late_update(double delta_time)
	{
		for (size_t i = 0; i < m_levels.size(); ++i)
		{
			Shared<Level> level = m_levels[i];
			level->late_update(delta_time);
		}
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
	void World::serialize(Data::Archive& archive)
	{
		//serialize this
		Data::serialize(archive, this);
		//serialize actors
		{
			uint64 size = m_levels.size();
			archive % size;
			for (auto& level : m_levels)
			{
				level->serialize(archive);
			}
		}
	}
	void  World::serialize_json(Data::JsonValue& archive)
	{
		Data::Json json_data = Data::JsonObject();
		Data::serialize_json(json_data, this);
		archive["data"] = std::move(json_data);
		// Actors
		auto json_levels = Data::JsonArray();
		json_levels.reserve(m_levels.size());
		for (auto level : m_levels)
		{
			Data::Json json_level = Data::JsonObject();
			level->serialize_json(json_level);
			json_levels.emplace_back(std::move(json_level));
		}
		archive["levels"] = std::move(json_levels);
	}
	//deserialize
	void  World::deserialize(Data::Archive& archive)
	{
		///clear
		for (auto& old_level : m_levels) { old_level->remove_components(); old_level->m_world.reset(); }
		m_levels.clear();
		//deserialize this
		Data::deserialize(archive, this);
		//deserialize childs
		{
			uint64 size = 0;
			archive % size;
			for (uint64 i = 0; i != size; ++i)
			{
				create_level()->deserialize(archive);
			}
		}
	}
	void  World::deserialize_json(Data::JsonValue& archive)
	{
		///clear
		for (auto& old_level : m_levels) { old_level->remove_components(); old_level->m_world.reset(); }
		m_levels.clear();
		//deserialize this
		if(archive.contains("data") && archive["data"].is_object())
		{
			Data::deserialize_json(archive["data"], this);
		}
		if (archive.contains("levels") && archive["levels"].is_array())
		{
			for (auto& jlevel : archive["levels"].array())
			{
				create_level()->deserialize_json(jlevel);
			}
		}
	}

}
}