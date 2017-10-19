//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Data/Json.h"
#include <cstring>

namespace Square
{
	//get
	JsonString& JsonValue::string() { return *m_string; }
	double		JsonValue::number() { return m_number; }
	bool	    JsonValue::boolean(){ return m_boolean;}
	JsonArray&  JsonValue::array()  { return *m_array; }
	JsonObject& JsonValue::object() { return *m_object;}
	//info
	bool JsonValue::is_null()   const { return m_type == Type::IS_NULL;   }
	bool JsonValue::is_string() const { return m_type == Type::IS_STRING; }
	bool JsonValue::is_number() const { return m_type == Type::IS_NUMBER; }
	bool JsonValue::is_boolean()const { return m_type == Type::IS_BOOL;   }
	bool JsonValue::is_array()  const { return m_type == Type::IS_ARRAY;  }
	bool JsonValue::is_object() const { return m_type == Type::IS_OBJECT; }
	//init
	JsonValue::JsonValue( /* null */ )
	{
		m_type = Type::IS_NULL;
		m_ptr  = nullptr;
	}		
	JsonValue::JsonValue(const JsonValue& v)
	{
		//move type
		m_type = v.m_type;
		//move value
		switch (m_type)
		{
		case Type::IS_NULL:   m_ptr     = v.m_ptr; break;
		case Type::IS_BOOL:   m_boolean = v.m_boolean; break;
		case Type::IS_NUMBER: m_number  = v.m_number; break;
		case Type::IS_STRING: m_string  = new JsonString(*v.m_string); break;
		case Type::IS_ARRAY:  m_array   = new JsonArray(*v.m_array); break;
		case Type::IS_OBJECT: m_object  = new JsonObject(*v.m_object); break;
		}
	}
	JsonValue::JsonValue(const std::string& value)
	{
		m_type   = Type::IS_STRING;
		m_string = new JsonString(value);
	}
	JsonValue::JsonValue(double value)
	{
		m_type = Type::IS_NUMBER;
		m_number = value;
	}
	JsonValue::JsonValue(int value)
	{
		m_type = Type::IS_NUMBER;
		m_number = double(value);
	}
	JsonValue::JsonValue(unsigned int value)
	{
		m_type = Type::IS_NUMBER;
		m_number = double(value);
	}
	JsonValue::JsonValue(bool value)
	{
		m_type   = Type::IS_BOOL;
		m_number = value;
	}
	JsonValue::JsonValue(const JsonArray& value)
	{
		m_type  = Type::IS_ARRAY;
		m_array = new JsonArray(value);
	}
	JsonValue::JsonValue(const JsonObject& value)
	{
		m_type   = Type::IS_OBJECT;
		m_object = new JsonObject(value);
	}
	JsonValue::~JsonValue()
	{
		switch (m_type)
		{
		case Type::IS_STRING: delete m_string; break;
		case Type::IS_ARRAY:  delete m_array; break;
		case Type::IS_OBJECT: delete m_object; break;
		default: /* none */ break;
		}
	}
	// operators
	size_t JsonValue::size() const
	{
		     if (is_array())  return m_array->size();
		else if (is_object()) return m_object->size();
		else return 0;
	}
	JsonValue& JsonValue::operator[] (const size_t& key)       
	{ 
		return array()[key];
	}
	JsonValue& JsonValue::operator[] (const std::string& key)  
	{ 
		return object()[key];
	}
	const JsonValue& JsonValue::operator[] (const size_t& key) const      
	{ 
		return (*m_array)[key]; 
	}
	const JsonValue& JsonValue::operator[] (const std::string& key) const 
	{ 
		return (*m_object).find(key)->second;
	}
	// move
	JsonValue& JsonValue::operator= (JsonValue&& v)
	{
		//move type
		m_type = std::move(v.m_type);
		//move value
		switch (m_type)
		{
		case Type::IS_NULL:   m_ptr = std::move(v.m_ptr); break;
		case Type::IS_BOOL:   m_boolean = std::move(v.m_boolean); break;
		case Type::IS_NUMBER: m_number = std::move(v.m_number); break;
		case Type::IS_STRING: m_string = std::move(v.m_string); break;
		case Type::IS_ARRAY:  m_array = std::move(v.m_array); break;
		case Type::IS_OBJECT: m_object = std::move(v.m_object); break;
		}
		return *this;
	}
}