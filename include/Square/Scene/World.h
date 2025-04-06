//
//  Square
//
//  Created by Gabriele Di Bari on 10/03/18.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
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
namespace Scene
{
	//..................
	//declaretion
	class Actor;
	class Level;
	struct Message;
	using ActorList = std::vector< Shared<Actor> >;
	using LevelList = std::vector< Shared<Level> >;
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
		const std::string& name() const;
		void name(const std::string&);

		//add an level	
		Shared<Level> level();
		void add(Shared<Level> level);

		//query
		Shared<Level> level(size_t index);
		Shared<Level> level(const std::string& name);
		const LevelList& levels() const;
		Shared<Actor> find_actor(const std::string& name);

		//contains an actor
		bool contains(Shared<Actor> child) const;
		bool contains(Shared<Level> child) const;

		//remove an actor
		bool remove(Shared<Actor> child);
		bool remove(Shared<Level> child);
			
		//message
		void send_message(const VariantRef& value, bool brodcast = false);
		void send_message(const Message& msg, bool brodcast = false);

		//serialize
		void serialize(Data::Archive& archive);
		void serialize_json(Data::JsonValue& archive);
		//deserialize
		void deserialize(Data::Archive& archive);
		void deserialize_json(Data::JsonValue& archive);


	protected:
		//name
		std::string m_name;
		//actor list
		LevelList m_levels;
	};
}
}