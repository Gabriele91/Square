//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include <unordered_map>

namespace Square
{
namespace Data
{
	//list of types
	class Json;
	class JsonValue;
	using JsonString = std::string;
	using JsonArray  = std::vector< JsonValue >;
	using JsonObject = std::unordered_map< JsonString, JsonValue >;
	//stream
	SQUARE_API std::ostream& operator<< (std::ostream& stream, const Json& value);
	SQUARE_API std::ostream& operator<< (std::ostream& stream, const JsonValue& value);
	//value
	class SQUARE_API JsonValue
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
		double&     number();
		bool&       boolean();
		JsonArray&  array();
		JsonObject& object();
        
        const JsonString& string() const;
        double            number() const;
        bool              boolean() const;
        const JsonArray&  array() const;
        const JsonObject& object() const;
        Type              type() const;
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
        JsonValue(const char* value);
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
        JsonValue& operator= (const JsonValue&);

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
    
    class SQUARE_API Json
    {
    public:
        
        //parse
		Json();
		Json(const JsonValue& document);
		Json(const std::string& source);

        //parsing
        bool parser(const std::string& source);
        
        //get error
        std::string errors() const;
        
        //get document
		JsonValue& document();
		const JsonValue& document() const;
        
        //operators
        operator JsonValue& ();
        operator const JsonValue& () const;
        JsonValue& operator[] (const size_t& key);
        JsonValue& operator[] (const std::string& key);
        const JsonValue& operator[] (const size_t& key) const;
        const JsonValue& operator[] (const std::string& key) const;
    
    protected:
        
        struct ErrorLine
        {
            ErrorLine()
            {
            }
            
            ErrorLine(size_t line, const std::string& error)
            : m_line(line)
            , m_error(error)
            {
            }
            
            size_t m_line{ 0 };
            std::string m_error;
        };
        
        JsonValue                m_document;
        std::vector< ErrorLine > m_list_errors;
    };

	//string illiterals
	inline Json operator "" _json(const char* s, std::size_t n)
	{
		return std::string(s);
	}
}
}
