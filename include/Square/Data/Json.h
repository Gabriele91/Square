//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include <unordered_map>

namespace Square
{
	//list of types
	class JsonValue;
	using JsonString = std::string;
	using JsonArray  = std::vector< JsonValue >;
	using JsonObject = std::unordered_map< JsonString, JsonValue >;
	//value
	class JsonValue
	{
	public:
		//type
		enum class Type : unsigned char
		{
			IS_NULL,
			IS_BOOL,
			IS_NUMBER,
			IS_STRING,
			IS_ARRAY,
			IS_OBJECT
		};
		//get
		JsonString& string();
		double      number();
		bool        boolean();
		JsonArray&  array();
		JsonObject& object();
		//info		
		bool is_null() const;
		bool is_string() const;
		bool is_number() const;
		bool is_boolean() const;
		bool is_array() const;
		bool is_object() const;
		//init
		JsonValue( /* null */);
		JsonValue(const JsonValue& v);
		JsonValue(const std::string& value);
		JsonValue(double);
		JsonValue(int);
		JsonValue(unsigned int);
		JsonValue(bool);
		JsonValue(const JsonArray&);
		JsonValue(const JsonObject&);
		~JsonValue();
		// operator
		size_t size() const;
		JsonValue& operator[] (const size_t& key);
		JsonValue& operator[] (const std::string& key);
		const JsonValue& operator[] (const size_t& key) const;
		const JsonValue& operator[] (const std::string& key) const;
		// move assignment 
		JsonValue& operator= (JsonValue&&);

	private:

		//Value
		union
		{
			bool        m_boolean;
			JsonString* m_string;
			double      m_number;
			JsonArray*  m_array;
			JsonObject* m_object;
			void*       m_ptr;
		};
		//Type
		Type m_type;
	};
}