//
//  Square
//
//  Created by Gabriele Di Bari on 10/03/18.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Object.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"

namespace Square
{
namespace Scene
{
	//..................
	//declaretion
	class Actor;
	class Component;
	class Level;
	class World;
	using ActorList = Vector< Shared<Actor> >;
	using LevelList = Vector< Shared<Level> >;
	//..................
    class SQUARE_API Level : public Object
                           , public SharedObject<Level>
                           , public Uncopyable
    {
    public:
		//Init object
		SQUARE_OBJECT(Level)

		//Registration in context
		static void object_registration(Context& ctx);

		//constructor: a level is always of a world (World::create_level creates it)
		Level(Context& context, Weak<World> world, const String& name = String());
		virtual ~Level();

		//add an actor
		void add(const Shared<Actor>& child);

		// Load a new actor from a file; an instance is cread at every call,
		Shared<Actor> load_actor(const String& resource_name);

		//query
		Shared<Actor> actor();
		Shared<Actor> actor(size_t index);
		Shared<Actor> actor(const String& name);
		const ActorList& actors() const;
		Shared<Actor> find_actor(const String& name);

		//contains an actor
		bool contains(const Shared<Actor>& child) const;

		//remove an actor
		bool remove(const Shared<Actor>& child);

		// Visit all actors DSF (each one by reference, see Actor::visit)
		void visit(const std::function<bool(const Shared<Actor>&)>& callback);

		//name        
		const String& name() const;
		void name(const String&);

		//active (the default): its components updated and in the systems of the world (render,
		//collisions...); not active: out of them, its actors as they are (a menu, a level not
		//played now), back as they were when active again
		void active(bool active);
		bool active() const;

		//paused: its components not updated, still in the systems of the world (drawn as they
		//are: a pause menu over it)
		void paused(bool paused) { m_paused = paused; }
		bool paused() const { return m_paused; }

		//message
		void send_message(const VariantRef& value, bool brodcast = false);
		void send_message(const Message& msg, bool brodcast = false);

		//serialize
		void serialize(Data::Archive& archive);
		void serialize_json(Data::JsonValue& archive);
		//deserialize
		void deserialize(Data::Archive& archive);
		void deserialize_json(Data::JsonValue& archive);

		//its world (empty if it is gone, or the level was removed from it)
		Weak<World> world() const;

		//every frame (SceneSystem): the events of the components of its actors
		void update(double delta_time);
		void late_update(double delta_time);

	protected:
		
		//name
		String m_name;
		bool        m_active{ true };
		bool        m_paused{ false };

		//actor list
		ActorList m_actors;

		//Call by an actor when is add into level 
		void on_add_a_actor(const Shared<Actor>& actor);
		void on_remove_a_actor(const Shared<Actor>& actor);

		//Call by component add into level / actor: sent to the systems of the world (their
		//collections: render, collisions...)
		void on_add_a_component(const Shared<Actor>& actor, const Shared<Component>& component);
		void on_remove_a_component(const Shared<Actor>& actor, const Shared<Component>& component);

		//every component of its actors leaves the systems of the world (the actors are cleared,
		//the level leaves the world, or it is not active), or comes back to them (active again)
		void remove_components();
		void add_components();

		//world, given by the world that creates it (reset only when it removes the level)
		Weak<World> m_world;
		friend class World;

		//firend class
		friend class Actor;
	};
}
}