//
//  Square
//
//  Created by Gabriele Di Bari on 10/03/18.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Object.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"

namespace Square
{
	class System;
	class SystemInstance;
	using SystemInstanceList = Vector< Shared<SystemInstance> >;
namespace Scene
{
	//..................
	//declaretion
	class Actor;
	class Level;
	struct Message;
	using ActorList = Vector< Shared<Actor> >;
	using LevelList = Vector< Shared<Level> >;
	//..................
	class SQUARE_API World :  public Object
							, public SharedObject<World>
							, public Uncopyable
	{
	public:
		//Init object
		SQUARE_OBJECT(World)

		//Registration in context
		static void object_registration(Context& ctx);

		//constructor
		World(Context& context);
		virtual ~World();

		//name        
		const String& name() const;
		void name(const String&);

		//a new level of this world, named or not (the world is created with MakeShared: its levels
		//keep it weak)
		Shared<Level> create_level(const String& name = String());
		//query (nullptr: no such level)
		Shared<Level> level(size_t index);
		Shared<Level> level(const String& name);
		const LevelList& levels() const;
		//the levels that run (updated, in the systems: render, collisions...): the ones named,
		//the others not. The levels of a game are made once and switched (the menu, a race...)
		void active_levels(const Vector<String>& names);
		Shared<Actor> find_actor(const String& name);

		//contains an actor
		bool contains(const Shared<Actor>& child) const;
		bool contains(const Shared<Level>& child) const;

		//remove an actor
		bool remove(const Shared<Actor>& child);
		bool remove(const Shared<Level>& child);
			
		//every frame (SceneSystem): the levels, and the components of their actors
		void update(double delta_time);
		void late_update(double delta_time);

		//message
		void send_message(const VariantRef& value, bool brodcast = false);
		void send_message(const Message& msg, bool brodcast = false);

		//systems: the state of every system in this world (created by the systems)
		void add_instance(System& system);
		void remove_instances(const System& system);
		void clear_instances();
		Shared<SystemInstance> instance(uint64 instance_id) const;
		const SystemInstanceList& instances() const;
		template< class T > Shared<T> instance() const
		{
			return DynamicPointerCast<T>(instance(T::static_object_id()));
		}

		//serialize
		void serialize(Data::Archive& archive);
		void serialize_json(Data::JsonValue& archive);
		//deserialize
		void deserialize(Data::Archive& archive);
		void deserialize_json(Data::JsonValue& archive);


	protected:
		//name
		String m_name;
		//actor list
		LevelList m_levels;
		//system instances
		SystemInstanceList m_instances;
	};
}
}