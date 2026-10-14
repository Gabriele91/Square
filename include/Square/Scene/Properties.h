//
//  Properties.h
//  Square
//
//  The properties of an actor for the game (as the metadata of a node): names and values (a
//  boolean, a number, a vector of 2-4 numbers, a string) the engine never reads; the game asks
//  them (e.g. "collision": false, "box"). Saved with its scene: the converter makes it from the
//  custom properties "game_<name>" of a node of glTF (Blender: Object Properties > Custom
//  Properties), the prefix out ("game_collision" is "collision").
//
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API Properties : public Square::Scene::Component
	{
	public:
		SQUARE_OBJECT(Properties)

		enum Type : int
		{
			P_BOOLEAN = 0,
			P_NUMBER  = 1,
			P_VECTOR  = 2,
			P_STRING  = 3
		};

		//a value: its type, its numbers (a boolean: 0 or 1, a number: x, a vector: its
		//components), its text (a string)
		struct Value
		{
			Type         m_type{ P_BOOLEAN };
			Square::Vec4 m_numbers{ 0.0f };
			int          m_components{ 1 };
			String  m_string;
		};

		Properties(Square::Context& context);
		virtual ~Properties();

		//set one (again: replaced)
		void set(const String& name, bool value);
		void set(const String& name, float value);
		void set(const String& name, const Square::Vec4& value, int components);
		void set(const String& name, const String& value);
		void remove(const String& name);

		//there is one by that name
		bool has(const String& name) const;
		//its value as asked (another type: converted where it makes sense, a boolean from a number
		//not 0 or the strings "true"/"false"; none: the fallback)
		bool         boolean(const String& name, bool fallback) const;
		float        number(const String& name, float fallback) const;
		Square::Vec4 vector(const String& name, const Square::Vec4& fallback) const;
		String  string(const String& name, const String& fallback) const;
		//all of them
		const Map<String, Value>& values() const { return m_values; }

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		//the values flat (the attributes of the scene file): by their order
		struct Packed
		{
			Vector< String >  m_names;
			Vector< int >          m_types;      //type * 8 + components
			Vector< Square::Vec4 > m_numbers;
			Vector< String >  m_strings;
		};
		Packed pack() const;
		void unpack(const Packed& packed);
		template < typename T, T Packed::* Field >
		static void packed_attribute(Square::Context& ctx, const char* name);

		Map<String, Value> m_values;
		Packed                       m_packed; //(while deserialized)
	};
}
}
