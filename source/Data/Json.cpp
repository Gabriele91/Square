//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Data/Json.h"
#include "Square/Data/ParserUtils.h"
#include <cstring>
#include <sstream>
#include <stack>

namespace Square
{
//parser
namespace Parser
{
	static inline bool is_delim(char c)
	{
		return c == ',' || c == ':' || c == ']' || c == '}' || isspace(c) || !c;
	}
}
namespace Data
{

	//get
	JsonString& JsonValue::string() { return *m_string; }
	double&		JsonValue::number() { return m_number; }
	bool&	    JsonValue::boolean(){ return m_boolean;}
	JsonArray&  JsonValue::array()  { return *m_array; }
	JsonObject& JsonValue::object() { return *m_object;}
    
    const JsonString& JsonValue::string() const { return *m_string; }
    double            JsonValue::number() const { return m_number; }
    bool              JsonValue::boolean() const{ return m_boolean; }
    const JsonArray&  JsonValue::array() const  { return *m_array; }
    const JsonObject& JsonValue::object() const { return *m_object; }
    JsonValue::Type   JsonValue::type() const   { return m_type; }
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
    JsonValue::JsonValue(const char* value)
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
		m_boolean = value;
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
		//leave ref
		v.m_type = Type::IS_NULL;
		v.m_ptr = nullptr;
		//return this
		return *this;
	}
    JsonValue& JsonValue::operator= (const JsonValue& v)
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
        return *this;
    }

	//stream
	inline std::string json_string_dump(const std::string& str)
	{
		//copy ptr
		const char *tmp = str.c_str();
		//start '"'
		std::string out = "\"";
		//push all chars
		while (*tmp)
		{
			switch (*tmp)
			{
			case '\n':
				out += "\\n";
				break;
			case '\t':
				out += "\\t";
				break;
			case '\b':
				out += "\\b";
				break;
			case '\r':
				out += "\\r";
				break;
			case '\f':
				out += "\\f";
				break;
			case '\a':
				out += "\\a";
				break;
			case '\\':
				out += "\\\\";
				break;
			case '\?':
				out += "\\?";
				break;
			case '\'':
				out += "\\\'";
				break;
			case '\"':
				out += "\\\"";
				break;
			default:
				out += *tmp;
				break;
			}
			++tmp;
		}
		return out + "\"";
	}
	std::ostream& operator<< (std::ostream& stream, const Json& value)
	{
		return (stream << value.document());
	}
	std::ostream& operator<< (std::ostream& stream, const JsonValue& value)
	{
		switch (value.type())
		{
			case JsonValue::Type::IS_NULL:   stream << "null"; break;
			case JsonValue::Type::IS_BOOL:   stream << (value.boolean() ? "true" : "false"); break;
			case JsonValue::Type::IS_NUMBER: stream << value.number(); break;
			case JsonValue::Type::IS_STRING: stream << json_string_dump(value.string()); break;
			case JsonValue::Type::IS_ARRAY:
			{
				stream << "[";
				for (size_t i = 0; i < value.size(); ++i)
				{
					stream << value.operator[](i) << (i != value.size() - 1 ? "," : "");
				}
				stream << "]";
			}
			break;
			case JsonValue::Type::IS_OBJECT:
			{
				stream << "{";
				{
					size_t i = 0;
					for (auto& k_v : value.object())
					{ 
						stream << json_string_dump(k_v.first) << ":" << k_v.second << (++i != value.object().size() ? "," : "");
					}
				}
				stream << "}";
			}
			break;
		}
		return stream;
	}
    ////////////////////////////////////////////////////////////////////////////////////
    //parse
	Json::Json(){}
	Json::Json(const JsonValue& document) :m_document(document) {}
	Json::Json(const std::string& source){ parser(source); }
    
	
	bool Json::parser(const std::string& cppsource)
    {
        //get ptr
        const char* source = cppsource.c_str();
        //temp values
        size_t line = 1;
        //ref to conteiner
        bool                   separator = false;
        bool                   added_key = false;
        JsonString             key;
        std::stack<JsonValue*> stack;
        //info
        auto in_object = [&]() -> bool
        {
            return stack.size() && stack.top()->is_object();
        };
        auto in_array = [&]() -> bool
        {
            return stack.size() && stack.top()->is_array();
        };
        //helps
        auto push = [&] (const JsonValue& value) -> JsonValue*
        {
            if(!stack.size())
            {
                if(value.is_object() || value.is_array())
                {
                    m_document = value; //copy
                    stack.push(&m_document);
                    return stack.top();
                }
                m_list_errors.push_back({line, "JSON_BAD_DOCUMENT"});
                return nullptr;
            }
            else if( stack.top()->is_object() && !added_key)
            {
                if(!value.is_string())
                {
                    m_list_errors.push_back({line, "JSON_BAD_KEY"});
                    return nullptr;
                }
                key = value.string();
                added_key = true;
                return (JsonValue*)&value;
            }
            else switch (stack.top()->type())
            {
                default:
                    m_list_errors.push_back({line, "JSON_UNKNOW"});
                    return nullptr;
                break;
                case JsonValue::Type::IS_OBJECT:
                {
                    //---
                    stack.top()->object()[key] = value;
                    //--
                    auto& ref = stack.top()->object()[key];
                    //--- into stack
                    if(ref.is_object() || ref.is_array())
                    {
                        stack.push(&ref);
                    }
                    //--- remove key
                    added_key = false;
                    //--- return
                    return &ref;
                }
                break;
                case JsonValue::Type::IS_ARRAY:
                    //--
                    stack.top()->array().push_back(value);
                    //--
                    auto& ref = stack.top()->array().back();
                    //--- into stack
                    if(ref.is_object() || ref.is_array())
                    {
                        stack.push(&ref);
                    }
                    //--- remove key
                    added_key = false;
                    //--- return
                    return &ref;
                break;
            }
        };
        //pop
        auto pop = [&]() -> bool
        {
            if(!stack.size()) return false;
            stack.pop();
            return true;
        };
        //parsing
        while(*source)
        {
			//parse
			using namespace Square::Parser;
            //skip
			skip_space_and_comments(line, source);
            //switch
            switch (*source)
            {
				////////////////////////////////////////////////////////////////////////////////////
				// number
                case '-':
                if (!is_float_digit(*(source+1)))
                {
                    m_list_errors.push_back({line,"JSON_BAD_NUMBER"});
                    return false;
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
				{
					double value = 0.0;
					parse_double2(source, value);
					push(value);
				}
                if (!is_delim(*source))
                {
                    m_list_errors.push_back({line,"JSON_BAD_NUMBER"});
                    return false;
                }
                break;
				////////////////////////////////////////////////////////////////////////////////////
                // string
                case '"':
                {
                    std::string str_out;
                    if (!parse_string(line, source, str_out))
                    {
                        m_list_errors.push_back({line,"JSON_BAD_STRING"});
                        return false;
                    }
                    push(JsonValue{ str_out });
                }
                break;
				////////////////////////////////////////////////////////////////////////////////////
                // null/true/false
                case 'n':
                    if(cstr_cmp_skip(source, "null"))
                    {
                        push({});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_NULL"});
                        return false;
                    }
                break;
                case 't':
                    if(cstr_cmp_skip(source, "true"))
                    {
                        push({true});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_TRUE"});
                        return false;
                    }
                break;
                case 'f':
                    if(cstr_cmp_skip(source, "false"))
                    {
                        push({false});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_FALSE"});
                        return false;
                    }
                break;
				////////////////////////////////////////////////////////////////////////////////////
                // table
                case '{':
                    push(JsonObject());
                    separator = true;
                    ++source;
                continue;
                case '}':
					if (!in_object())
					{
                        m_list_errors.push_back({line,"JSON_MISMATCH_BRACKET"});
                        return false;
                    }
                    pop();
                    ++source;
                break;
				////////////////////////////////////////////////////////////////////////////////////
                // array
                case '[':
                    push(JsonArray());
                    separator = true;
                    ++source;
                continue;
                case ']':
					if (!in_array())
					{
                        m_list_errors.push_back({line,"JSON_MISMATCH_BRACKET"});
                        return false;
                    }
                    pop();
                    ++source;
                break;
				////////////////////////////////////////////////////////////////////////////////////
                // separator
                case ':':
                    if(separator || !added_key || !in_object())
                    {
                        m_list_errors.push_back({line,"JSON_UNEXPECTED_CHARACTER"});
                        return false;
                    }
                    separator = true;
                    ++source;
                continue;
                case ',':
                    if(separator || (!in_object() && !in_array()))
                    {
                        m_list_errors.push_back({line,"JSON_UNEXPECTED_CHARACTER"});
                        return false;
                    }
                    separator = true;
                    ++source;
                continue;
				////////////////////////////////////////////////////////////////////////////////////
                // EOF
                case '\0':
                    continue;
				////////////////////////////////////////////////////////////////////////////////////
                default:
                    m_list_errors.push_back({line,"JSON_UNEXPECTED_CHARACTER"});
                    return false;
                break;
            }
            //to false
            separator = false;
        }
        //end
        return true;
    }
	
	//get error
    std::string Json::errors() const
    {
        std::stringstream ssout;
        for(auto& error : m_list_errors)
        {
            ssout << error.m_line << ": " << error.m_error << std::endl;
        }
        return ssout.str();
    }
    
    //get document
	JsonValue& Json::document(){ return m_document; }
	const JsonValue& Json::document() const{ return m_document; }
	//operators
    Json::operator JsonValue& ()                                    { return document(); }
    Json::operator const JsonValue& () const                        { return document(); }
    JsonValue& Json::operator[] (const size_t& key)                 { return document()[key]; }
    JsonValue& Json::operator[] (const std::string& key)            { return document()[key]; }
    const JsonValue& Json::operator[] (const size_t& key) const     { return document()[key]; }
    const JsonValue& Json::operator[] (const std::string& key) const{ return document()[key]; }
}
}
