//
//  Square
//
//  Created by Gabriele Di Bari on 10/03/18.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Object.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"
#include "Square/Render/Collection.h"

namespace Square
{
namespace Scene
{
	//..................
	//declaretion
	class Actor;
	class Level;
	class World;
	using ActorList = std::vector< Shared<Actor> >;
	using LevelList = std::vector< Shared<Level> >;
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

		//constructor
		Level(Context& context);
		Level(Context& context, const std::string& name);

		//add an actor
		void add(Shared<Actor> child);
		Shared<Actor> load_actor(const std::string& resource_name);

		//query
		Shared<Actor> actor();
		Shared<Actor> actor(size_t index);
		Shared<Actor> actor(const std::string& name);
		const ActorList& actors() const;
		Shared<Actor> find_actor(const std::string& name);

		//contains an actor
		bool contains(Shared<Actor> child) const;

		//remove an actor
		bool remove(Shared<Actor> child);

		//name        
		const std::string& name() const;
		void name(const std::string&);

		//message
		void send_message(const VariantRef& value, bool brodcast = false);
		void send_message(const Message& msg, bool brodcast = false);

		//serialize
		void serialize(Data::Archive& archivie);
		void serialize_json(Data::Json& archivie);
		//deserialize
		void deserialize(Data::Archive& archivie);
		void deserialize_json(Data::Json& archivie);

		//get randerable collection
		const Render::Collection& randerable_collection() const;

	protected:
		
		//name
		std::string m_name;

		//actor list
		ActorList m_actors;

		//Call by an actor when is add into level 
		void on_add_a_actor(Shared<Actor> actor);
		void on_remove_a_actor(Shared<Actor> actor);

		//Call by component add into level / actor
		void on_add_a_component(Shared<Actor> actor, Shared<Component> component);
		void on_remove_a_component(Shared<Actor> actor, Shared<Component> component);

		//Collection
		Render::Collection m_rander_collection;

		//firend class
		friend class Actor;
	};
}
}