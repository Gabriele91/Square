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
#include "Square/Scene/Actor.h"
#include "Square/Scene/Component.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"

namespace Square
{
namespace Scene
{
    class SQUARE_API Level : public Object
                           , public SharedObject<Level>
                           , public Uncopyable
    {
    public:
		//Init object
		SQUARE_OBJECT(Level)
		//Registration in context
		static void object_registration(Context& ctx);

		//add an actor
		void add(Shared<Actor> child);

		//query
		Shared<Actor> actor();
		Shared<Actor> actor(size_t index);
		Shared<Actor> actor(const std::string& name);
		const ActorList& actors() const;
		Shared<Actor> find(const std::string& name);

		//contains an actor
		bool contains(Shared<Actor> child) const;

		//remove an actor
		void remove(Shared<Actor> child);

		//name        
		const std::string& name() const;
		void name(const std::string&);

		//message
		void send_message(const Variant& value, bool brodcast = false);
		void send_message(const Message& msg, bool brodcast = false);

		//serialize
		void serialize(Data::Archive& archivie);
		void serialize_json(Data::Json& archivie);
		//deserialize
		void deserialize(Data::Archive& archivie);
		void deserialize_json(Data::Json& archivie);

	protected:
		
		//name
		std::string m_name;
		//actor list
		ActorList m_actors;

	};
}
}