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
#include "Square/Scene/Component.h"
#include "Square/Scene/Level.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"
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
		ctx.add_attribute_field<Level, std::string>("name", std::string(), offsetof(Level,m_name));
	}

	//constructor
	Level::Level(Context& context) : Object(context), SharedObject_t(context.allocator())
	{
	}
	Level::Level(Context& context, const std::string& name) : Object(context), SharedObject_t(context.allocator()), m_name(name)
	{
	}
	Level::~Level()
	{
	}
	//serialize
	void Level::serialize(Data::Archive& archive)
	{
		//serialize this
		Data::serialize(archive, this);
		//serialize actors
		{
			uint64 size = m_actors.size();
			archive % size;
			for (auto& actor : m_actors)
			{
				actor->serialize(archive);
			}
		}
	}
	void Level::serialize_json(Data::JsonValue& archive)
	{
		Data::Json json_data = Data::JsonObject();
		Data::serialize_json(json_data, this);
		archive["data"] = std::move(json_data);
		// Actors
		auto json_actors = Data::JsonArray();
		json_actors.reserve(m_actors.size());
		for (auto actor : m_actors)
		{
			Data::Json json_actor = Data::JsonObject();
			actor->serialize_json(json_actor);
			json_actors.emplace_back(std::move(json_actor));
		}
		archive["actors"] = std::move(json_actors);
	}
	//deserialize
	void Level::deserialize(Data::Archive& archive)
	{
		///clear
		m_rander_collection.clear();
		m_actors.clear(); //todo: call events
		//deserialize this
		Data::deserialize(archive, this);
		//deserialize childs
		{
			uint64 size = 0;
			archive % size;
			for (uint64 i = 0; i != size; ++i)
			{
				actor()->deserialize(archive);
			}
		}
	}
	void Level::deserialize_json(Data::JsonValue& archive)
	{
		///clear
		m_rander_collection.clear();
		m_actors.clear(); //todo: call events
		//
		if (archive.contains("data") && archive["data"].is_object())
		{
			Data::deserialize_json(archive["data"], this);
		}
		if (archive.contains("actors") && archive["actors"].is_array())
		{
			for (auto& jactor : archive["actors"].array())
			{
				actor()->deserialize_json(jactor);
			}
		}
	}
	
	//add an actor
	void Level::add(Shared<Actor> actor)
	{
		if (!actor) return;
		actor->remove_from_parent();
		m_actors.push_back(actor);
		actor->level(this->shared_from_this());
	}
	Shared<Actor> Level::load_actor(const std::string& resource_name)
	{
		Shared<Actor> new_actor = context().resource<Actor>(resource_name);
		if (new_actor)
		{
			add(new_actor);
		}
		return new_actor;
	}
	
	void Level::visit(const std::function<bool(Shared<Actor>)>& callback)
	{
		for (auto actor : m_actors)
		{
			actor->visit(callback);
		}
	}

	//remove an actor
	bool Level::remove(Shared<Actor> actor)
	{
		if (auto parent = actor->parent().lock(); !parent)
		{
			//remove child from list
			auto it = std::find(m_actors.begin(), m_actors.end(), actor);
			if (it != m_actors.end()) 
			{
				actor->level(Weak<Level>());
				m_actors.erase(it); 
				return true; 
			}
		}
		return false;
	}

	//get/create actor
	Shared<Actor> Level::actor()
	{
		//create
		auto actor = MakeShared<Actor>(context());
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
		auto actor = MakeShared<Actor>(context(), name);
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
	Shared<Actor> Level::find_actor(const std::string& path_names)
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
		//bool found!
		bool actor_found = false;
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
					do_continue = !!*ptr;
					actor_found = !*ptr;
					break;
				}
			}
			//continue?
			if (!do_continue) break;
		}
		// Return iff it is the right one
		if (actor_found)
			return c_actor;
		else
			return false;
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
	void Level::send_message(const VariantRef& variant, bool brodcast)
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
	//get randerable collection
	const Render::Collection& Level::randerable_collection() const
	{
		return m_rander_collection;
	}

	//added an actor
	void Level::on_add_a_actor(Shared<Actor> actor)
	{
		for (auto component : actor->components())
		{
			on_add_a_component(actor, component.second);
		}
	}
	//remove an actor
	void Level::on_remove_a_actor(Shared<Actor> actor)
	{
		for (auto component : actor->components())
		{
			on_remove_a_component(actor, component.second);
		}
	}

	//added a component
	void Level::on_add_a_component(Shared<Actor> actor, Shared<Component> component)
	{
		if (auto renderable = DynamicPointerCast<Render::Renderable, Component>(component);  renderable) 
		{ 
			m_rander_collection.m_renderables.push_back(renderable);
		}
		else if (auto light = DynamicPointerCast<Render::Light, Component>(component);  light)
		{
			m_rander_collection.m_lights.push_back(light);
		}
		else if (auto camera = DynamicPointerCast<Render::Camera, Component>(component); camera) 
		{ 
			m_rander_collection.m_cameras.push_back(camera);
		}
	}
	//remove a component
	void Level::on_remove_a_component(Shared<Actor> actor, Shared<Component> component)
	{
		auto& renderables = m_rander_collection.m_renderables;
		auto renderable = DynamicPointerCast<Render::Renderable, Component>(component);
		if (renderable)
		{
			renderables.erase(
				std::remove_if( renderables.begin(), renderables.end(), 
								[&](Weak<Render::Renderable> n_renderable)
								{ 
									return renderable == n_renderable.lock();
								}),
				renderables.end()
			);
			return;
		}

		auto& lights = m_rander_collection.m_lights;
		auto light = DynamicPointerCast<Render::Light, Component>(component);
		if (light)
		{
			lights.erase(
				std::remove_if( lights.begin(), lights.end(),
								[&](Weak<Render::Light> n_light)
								{ 
									return light == n_light.lock();
								}),
				lights.end()
			); 
			return;
		}
        
        auto& cameras = m_rander_collection.m_cameras;
        auto camera = DynamicPointerCast<Render::Camera, Component>(component);
        if (camera)
        {
            cameras.erase(
                 std::remove_if( cameras.begin(), cameras.end(),
                                [&](Weak<Render::Camera> n_camera)
                                {
                                    return camera == n_camera.lock();
                                }),
                 cameras.end()
            );
            return;
        }

	}
}
}
