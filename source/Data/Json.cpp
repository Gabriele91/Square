//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Data/Json.h"
#include <cstring>
#include <sstream>
#include <stack>

namespace Square
{
	//get
	JsonString& JsonValue::string() { return *m_string; }
	double&		JsonValue::number() { return m_number; }
	bool&	    JsonValue::boolean(){ return m_boolean;}
	JsonArray&  JsonValue::array()  { return *m_array; }
	JsonObject& JsonValue::object() { return *m_object;}
    
    const JsonString& JsonValue::string() const { return *m_string; }
    double            JsonValue::number() const { return m_boolean; }
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

    ////////////////////////////////////////////////////////////////////////////////////
    //parse
    JSon::JSon(){}
    JSon::JSon(const std::string& source){ parser(source); }
    
    //parsing
    static inline bool json_is_space(char c)
    {
        return c == ' ' || (c >= '\t' && c <= '\r');
    }
    
    static inline bool json_is_delim(char c)
    {
        return c == ',' || c == ':' || c == ']' || c == '}' || isspace(c) || !c;
    }
    
    static inline bool json_is_digit(char c)
    {
        return c >= '0' && c <= '9';
    }
    
    static inline bool json_is_digit_or_point(char c)
    {
        return c == '.'  || (c >= '0' && c <= '9');
    }
    
    
    static inline bool json_is_xdigit(char c)
    {
        return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
    }
    
    static inline int json_char_to_int(char c)
    {
        return (c <= '9') ?  c - '0' : (c & ~' ') - 'A' + 10;
    }
    
    static double json_string_to_double(const char*& source)
    {
        //+/-
        char ch = *source;
        if (ch == '-') ++source;
        //integer part
        double result = 0;
        while (json_is_digit(*source)) result = (result * 10) + (*source++ - '0');
        //fraction
        if (*source == '.')
        {
            ++source;
            double fraction = 1;
            while (json_is_digit(*source))
            {
                fraction *= 0.1;
                result += (*source++ - '0') * fraction;
            }
        }
        //exponent
        if (*source == 'e' || *source == 'E')
        {
            ++source;
            //base of exp
            double base = 10;
            //+/- exp
            if (*source == '+')
            {
                ++source;
            }
            else if (*source == '-')
            {
                ++source;
                base = 0.1;
            }
            //parsing exponent
            unsigned int exponent = 0;
            while (json_is_digit(*source)) exponent = (exponent * 10) + (*source++ - '0');
            //compute exponent
            double power = 1;
            for (; exponent; exponent >>= 1, base *= base) if (exponent & 1) power *= base;
            //save result
            result *= power;
        }
        //result
        return ch == '-' ? -result : result;
    }
    
    static void json_skip_space(size_t& line, const char*& source)
    {
        while (json_is_space(*source))
        {
            //count line
            if(*source == '\n') ++line;
            //jump
            ++source;
            //exit
            if (!*source) break;
        }
    }
    struct json_string_out { std::string m_str; bool m_success; };
    static json_string_out json_string(size_t& line,  const char*& source)
    {
        //init
        std::string out;
        //start parse
        if ((*source) == '\"')  //["...."]
        {
            ++source;  //[...."]
            while ((*source) != '\"' && (*source) != '\n')
            {
                if ((*source) == '\\') //[\.]
                {
                    ++source;  //[.]
                    switch (*source)
                    {
                        case 'n':
                            out += '\n';
                            break;
                        case 't':
                            out += '\t';
                            break;
                        case 'b':
                            out += '\b';
                            break;
                        case 'r':
                            out += '\r';
                            break;
                        case 'f':
                            out += '\f';
                            break;
                        case 'a':
                            out += '\a';
                            break;
                        case '\\':
                            out += '\\';
                            break;
                        case '?':
                            out += '\?';
                            break;
                        case '\'':
                            out += '\'';
                            break;
                        case '\"':
                            out += '\"';
                            break;
                        case 'u':
                        {
                            int c = 0;
                            //comput u
                            for (int i = 0; i < 4; ++i)
                            {
                                if (json_is_xdigit(*++source))
                                {
                                    c = c * 16 + json_char_to_int(*source);
                                }
                                else
                                {
                                    return { out, false };
                                }
                            }
                            //ascii
                            if (c < 0x80)
                            {
                                out += c;
                            }
                            //utf 2 byte
                            else if (c < 0x800)
                            {
                                out += 0xC0 | (c >> 6);
                                out += 0x80 | (c & 0x3F);
                            }
                            //utf 3 byte
                            else
                            {
                                out += 0xE0 | (c >> 12);
                                out += 0x80 | ((c >> 6) & 0x3F);
                                out += 0x80 | (c & 0x3F);
                            }
                        }
                        break;
                        case '\n': /* jump unix */
                            ++line;
                            break;
                        case '\r': /* jump mac */
                            ++line;
                            if ((*(source + 1)) == '\n') ++source; /* jump window (\r\n)*/
                            break;
                        default:
                            return { out, false };
                            break;
                    }
                }
                else
                {
                    if ((*source) != '\0') out += (*source);
                    else return { out, false };
                }
                ++source;//next char
            }
            //exit cases
            if ((*source) == '\n')
            {
                return { out, false };
            }
            else if ((*source) == '\"')
            {
                ++source;
                return { out, true };
            }
        }
        return { out, false };
    }

    
    bool JSon::parser(const std::string& cppsource)
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
            //skip
            json_skip_space(line,source);
            //switch
            switch (*source)
            {
                // // // // // // // // // // // // // // // // // // // // // // // //
                // number
                case '-':
                if (!json_is_digit_or_point(*source))
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
                push(json_string_to_double(source));
                if (!json_is_delim(*source))
                {
                    m_list_errors.push_back({line,"JSON_BAD_NUMBER"});
                    return false;
                }
                break;
                // // // // // // // // // // // // // // // // // // // // // // // //
                // string
                case '"':
                {
                    auto jstr_pret = json_string(line, source);
                    if (!jstr_pret.m_success)
                    {
                        m_list_errors.push_back({line,"JSON_BAD_STRING"});
                        return false;
                    }
                    push(JsonValue{ jstr_pret.m_str });
                }
                break;
                // // // // // // // // // // // // // // // // // // // // // // // //
                // null/true/false
                case 'n':
                    if(std::strncmp(source, "null", 4) == 0)
                    {
                        source += 4;
                        push({});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_NULL"});
                        return false;
                    }
                break;
                case 't':
                    if(std::strncmp(source, "true", 4) == 0)
                    {
                        source += 4;
                        push({true});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_TRUE"});
                        return false;
                    }
                break;
                case 'f':
                    if(std::strncmp(source, "false", 4) == 0)
                    {
                        source += 4;
                        push({false});
                    }
                    else
                    {
                        m_list_errors.push_back({line,"JSON_BAD_FALSE"});
                        return false;
                    }
                break;
                // // // // // // // // // // // // // // // // // // // // // // // //
                // table
                case '{':
                    push(JsonObject());
                    separator = true;
                    ++source;
                continue;
                case '}':
                    if(separator || !in_object())
                    {
                        m_list_errors.push_back({line,"JSON_MISMATCH_BRACKET"});
                        return false;
                    }
                    pop();
                    ++source;
                break;
                // // // // // // // // // // // // // // // // // // // // // // // //
                // array
                case '[':
                    push(JsonArray());
                    separator = true;
                    ++source;
                continue;
                case ']':
                    if(separator || !in_array())
                    {
                        m_list_errors.push_back({line,"JSON_MISMATCH_BRACKET"});
                        return false;
                    }
                    pop();
                    ++source;
                break;
                // // // // // // // // // // // // // // // // // // // // // // // //
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
                // // // // // // // // // // // // // // // // // // // // // // // //
                // EOF
                case '\0':
                    continue;
                // // // // // // // // // // // // // // // // // // // // // // // //
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
    std::string JSon::errors()
    {
        std::stringstream ssout;
        for(auto& error : m_list_errors)
        {
            ssout << error.m_line << ": " << error.m_error << std::endl;
        }
        return ssout.str();
    }
    
    //get document
    JsonValue& JSon::document()
    {
        return m_document;
    }
}
