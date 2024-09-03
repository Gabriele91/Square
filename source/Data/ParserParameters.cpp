//
//  ParserParameters.cpp
//  Square
//
//  Created by Gabriele Di Bari on 03/05/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserParameters.h"

namespace Square
{
namespace Parser
{
	// // // // // // // // // // // // // // // // // // // // // // // // // //
    // ParameterField
    bool  Parameters::ParameterField::alloc(Allocator* allocator, ParameterType type)
    {
        //type
        m_type = type;
        //alloc
        m_paramter = ::Square::Render::EffectParameter::create(allocator, m_type);
        m_allocator = allocator;
        //if
        if(!m_paramter) return false;
        //else
        return true;
    }
    bool  Parameters::ParameterField::alloc(Allocator* allocator, const std::string& name,ParameterType type)
    {
        //name
        m_name = name;
        //type
        m_type = type;
        //alloc
        m_paramter = ::Square::Render::EffectParameter::create(allocator, m_type);
        m_allocator = allocator;
        //if
        if(!m_paramter) return false;
        //else
        return true;
    }
 	//default
	Parameters::ParameterField::ParameterField(){}
	//copy / move
	Parameters::ParameterField::ParameterField(ParameterField&& value)
	{
		//move (std)
		m_name     = std::move(value.m_name);
		m_resources = std::move(value.m_resources);
		//move value type
		m_type = value.m_type;
		value.m_type = ParameterType::PT_NONE;
		//move paramter
		if (value.m_paramter)
		{
			//move
			m_paramter = std::move(value.m_paramter);
            m_allocator = value.m_allocator;
			value.m_paramter = nullptr;
            value.m_allocator = nullptr;
		}
	}
	Parameters::ParameterField::ParameterField(const ParameterField& value)
	{
		m_type     = value.m_type;
		m_name     = value.m_name;
		m_resources = value.m_resources;
		//copy paramter
		if (value.m_paramter)
		{
			m_paramter = value.m_paramter->copy();
		}
	}
	Parameters::ParameterField::~ParameterField()
    {
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    // Context
    std::string Parameters::Context::errors_to_string() const
    {
        std::stringstream sbuffer;
        
        for (auto& error : m_errors)
        {
            sbuffer << "Error:" << error.m_line << ": " << error.m_error << "\n";
        }
        
        return sbuffer.str();
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    // Parser
    bool Parameters::parse(Allocator* allocator, Context& default_context, const std::string& source)
    {
        const char* c_ptr = source.c_str();
        return parse(allocator, default_context, c_ptr);
    }
    
    bool Parameters::parse(Allocator* allocator, Context& default_context, const char*& ptr)
    {
		//set
        m_context = &default_context;
        m_allocator = allocator;
        //skip line and comments
        skip_space_and_comments(m_context->m_line, ptr);
        //parsing a block
		if (!parse_parameters_block(ptr)) return false; 
        //skip line and comments
        skip_space_and_comments(m_context->m_line, ptr);
		//ok
        return true;
    }

    bool Parameters::parse_parameters_block(const char*& ptr)
    {
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse table
        if (is_start_table(*ptr))
        {
            //skip '{'
            ++ptr;
            //skip spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //alloc uniform field
                ParameterField field;
                //parse
                if (!parse_name(ptr, field.m_name))
                {
                    push_error("Not valid uniform name");
                    return false;
                }
                //skip "line" space
                skip_line_space(m_context->m_line, ptr);
                //value
                if (!parse_value(ptr, field))
                {
                    push_error("Not valid uniform field");
                    return false;
                }
                //push
                m_context->m_parameters.push_back(std::move(field));
                //skip spaces
                skip_space_and_comments(m_context->m_line, ptr);
            }
            //end while
            if (!is_end_table(*ptr))
            {
                push_error("Not found }");
                return false;
            }
            //skip }
            ++ptr;
        }
        return true;
    }
    
    bool Parameters::parse_value(const char*& ptr, ParameterField& field)
    {
        if (!parse_type(ptr, field.m_type)) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump '('
        ++ptr;
        //space
        skip_space_and_comments(m_context->m_line, ptr);
        //alloc
        field.alloc(m_allocator, field.m_type);
        //parse by type
        switch (field.m_type)
        {
            case ParameterType::PT_INT:     if(!parse_int_values(ptr, field.m_paramter->value_ptr<int>(), 1))  return false;  break;
            case ParameterType::PT_FLOAT:   if(!parse_float_values(ptr, field.m_paramter->value_ptr<float>(), 1))  return false; break;
            case ParameterType::PT_DOUBLE:  if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 1))  return false; break;

            case ParameterType::PT_IVEC2:   if(!parse_int_values(ptr, field.m_paramter->value_ptr<int>(), 2)) return false; break;
            case ParameterType::PT_IVEC3:   if(!parse_int_values(ptr, field.m_paramter->value_ptr<int>(), 3)) return false; break;
            case ParameterType::PT_IVEC4:   if(!parse_int_values(ptr, field.m_paramter->value_ptr<int>(), 4)) return false; break;
                
            case ParameterType::PT_VEC2:    if(!parse_float_values(ptr, field.m_paramter->value_ptr<float>(), 2)) return false; break;
            case ParameterType::PT_VEC3:    if(!parse_float_values(ptr, field.m_paramter->value_ptr<float>(), 3)) return false; break;
            case ParameterType::PT_VEC4:    if(!parse_float_values(ptr, field.m_paramter->value_ptr<float>(), 4)) return false; break;
            case ParameterType::PT_MAT3:    if(!parse_mat3(ptr, *field.m_paramter->value_ptr<Mat3>())) return false; break;
            case ParameterType::PT_MAT4:    if(!parse_mat4(ptr, *field.m_paramter->value_ptr<Mat4>())) return false;  break;
                
            case ParameterType::PT_DVEC2:    if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 2)) return false; break;
            case ParameterType::PT_DVEC3:    if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 3)) return false; break;
            case ParameterType::PT_DVEC4:    if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 4)) return false; break;
            case ParameterType::PT_DMAT3:    if(!parse_dmat3(ptr, *field.m_paramter->value_ptr<DMat3>())) return false; break;
            case ParameterType::PT_DMAT4:    if(!parse_dmat4(ptr, *field.m_paramter->value_ptr<DMat4>())) return false;  break;

			case ParameterType::PT_TEXTURE: 
			{
				std::string value_name;
				if (!parse_string(m_context->m_line, ptr, value_name))  return false;
				field.add(value_name);
			}
			break;
            default: return false; break;
        }
        //jump space
        skip_space_and_comments(m_context->m_line, ptr);
        //parse ')'
        if (!is_end_arg(*ptr))  return false;
        //jump ')'
        ++ptr;
        //...
        return true;
    }
    
    bool Parameters::parse_int_values(const char*& ptr, int* values, size_t n)
    {
        //first
        if (!Parser::parse_int(ptr, *values)) return false;
        //next
        while(--n)
        {
            //jmp space
            skip_space_and_comments(m_context->m_line, ptr);
            //jump ,
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            //jump space
            skip_space_and_comments(m_context->m_line, ptr);
            //get float
            if (!Parser::parse_int(ptr, *(++values))) return false;
        }
        return true;
    }
    
    bool Parameters::parse_float_values(const char*& ptr, float* values, size_t n)
    {
        //first
        if (!Parser::parse_float(ptr, *values)) return false;
        //next
        while(--n)
        {
            //jmp space
            skip_space_and_comments(m_context->m_line, ptr);
            //jump ,
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            //jump space
            skip_space_and_comments(m_context->m_line, ptr);
            //get float
            if (!Parser::parse_float(ptr, *(++values))) return false;
        }
        return true;
    }
    
    bool Parameters::parse_double_values(const char*& ptr, double* values, size_t n)
    {
        //first
        if (!Parser::parse_double(ptr, *values)) return false;
        //next
        while(--n)
        {
            //jmp space
            skip_space_and_comments(m_context->m_line, ptr);
            //jump ,
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            //jump space
            skip_space_and_comments(m_context->m_line, ptr);
            //get float
            if (!Parser::parse_double(ptr, *(++values))) return false;
        }
        return true;
    }
    
    bool Parameters::parse_texture(const char*& ptr, ParameterField& field)
    {
        //texture name
        std::string texture_name;
        //parse
        if (!parse_string(m_context->m_line, ptr, texture_name))
        {
            return false;
        }
        //save texture name
        field.m_resources.push_back(texture_name);
        //ok
        return true;
    }
    
    bool Parameters::parse_mat3(const char*& ptr, Mat3& m3)
    {
        //first
        if (!parse_float(ptr, m3[0][0])) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        // if one param, call mat4 constructor
        if (!is_comm_arg(*ptr))
        {
            // is 'mat3('<float>')'
            m3 = Mat3(m3[0][0]);
            //success
            return true;
        }
        //for all
        for (int y = 0; y != 3; ++y)
        for (int x = 0; x != 3; ++x)
        {
            //jmp first
            if ((x + y) == 0) continue;
            //parse ','
            skip_space_and_comments(m_context->m_line, ptr);
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            skip_space_and_comments(m_context->m_line, ptr);
            //parse value
            if (!parse_float(ptr, m3[x][y])) return false;
        }
        
        return true;
    }
    
    bool Parameters::parse_mat4(const char*& ptr, Mat4& m4)
    {
        //first
        if (!parse_float(ptr, m4[0][0])) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        // if one param, call mat4 constructor
        if (!is_comm_arg(*ptr))
        {
            // is 'mat4('<float>')'
            m4 = Mat4(m4[0][0]);
            //success
            return true;
        }
        //for all
        for (int y = 0; y != 4; ++y)
        for (int x = 0; x != 4; ++x)
        {
            //jmp first
            if ((x + y) == 0) continue;
            //parse ','
            skip_space_and_comments(m_context->m_line, ptr);
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            skip_space_and_comments(m_context->m_line, ptr);
            //parse value
            if (!parse_float(ptr, m4[x][y])) return false;
        }
        
        return true;
    }
    
    bool Parameters::parse_dmat3(const char*& ptr, DMat3& m3)
    {
        //first
        if (!parse_double(ptr, m3[0][0])) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        // if one param, call mat4 constructor
        if (!is_comm_arg(*ptr))
        {
            // is 'mat3('<float>')'
            m3 = DMat3(m3[0][0]);
            //success
            return true;
        }
        //for all
        for (int y = 0; y != 3; ++y)
        for (int x = 0; x != 3; ++x)
        {
            //jmp first
            if ((x + y) == 0) continue;
            //parse ','
            skip_space_and_comments(m_context->m_line, ptr);
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            skip_space_and_comments(m_context->m_line, ptr);
            //parse value
            if (!parse_double(ptr, m3[x][y])) return false;
        }
        
        return true;
    }
    
    bool Parameters::parse_dmat4(const char*& ptr, DMat4& m4)
    {
        //first
        if (!parse_double(ptr, m4[0][0])) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        // if one param, call mat4 constructor
        if (!is_comm_arg(*ptr))
        {
            // is 'mat4('<float>')'
            m4 = DMat4(m4[0][0]);
            //success
            return true;
        }
        //for all
        for (int y = 0; y != 4; ++y)
        for (int x = 0; x != 4; ++x)
        {
            //jmp first
            if ((x + y) == 0) continue;
            //parse ','
            skip_space_and_comments(m_context->m_line, ptr);
            if (!is_comm_arg(*ptr)) return false; else ++ptr;
            skip_space_and_comments(m_context->m_line, ptr);
            //parse value
            if (!parse_double(ptr, m4[x][y])) return false;
        }
        
        return true;
    }
    //////////////////////////////////////////////////////
	bool Parameters::parse_type(const char*& inout, ParameterType& type)
	{
		struct { const char* m_key; ParameterType m_type; } map_key_type[]
		{
        //Primitive
		{ "int", ParameterType::PT_INT },
		{ "float", ParameterType::PT_FLOAT },
		{ "double", ParameterType::PT_DOUBLE },
		//3 name for texture
		{ "texture", ParameterType::PT_TEXTURE },   //like primitive type
		{ "Texture", ParameterType::PT_TEXTURE },   //like class object
		{ "texture2D", ParameterType::PT_TEXTURE }, //like shader (GLSL 450, DX10/11/12)
        //like GLSL
		{ "IVec2", ParameterType::PT_IVEC2 },
		{ "IVec3", ParameterType::PT_IVEC3 },
		{ "IVec4", ParameterType::PT_IVEC4 },

		{ "Vec2", ParameterType::PT_VEC2 },
		{ "Vec3", ParameterType::PT_VEC3 },
		{ "Vec4", ParameterType::PT_VEC4 },
		{ "Mat3", ParameterType::PT_MAT3 },
		{ "Mat4", ParameterType::PT_MAT4 },

		{ "DVec2", ParameterType::PT_DVEC2 },
		{ "DVec3", ParameterType::PT_DVEC3 },
		{ "DVec4", ParameterType::PT_DVEC4 },
		{ "DMat3", ParameterType::PT_DMAT3 },
		{ "DMat4", ParameterType::PT_DMAT4 },
        //like DirectX
        { "int2", ParameterType::PT_IVEC2 },
        { "int3", ParameterType::PT_IVEC3 },
        { "int4", ParameterType::PT_IVEC4 },
        
        { "float2", ParameterType::PT_VEC2 },
        { "float3", ParameterType::PT_VEC3 },
        { "float4", ParameterType::PT_VEC4 },
        { "float3x3", ParameterType::PT_MAT3 },
        { "float4x4", ParameterType::PT_MAT4 },
        
        { "double2", ParameterType::PT_DVEC2 },
        { "double3", ParameterType::PT_DVEC3 },
        { "double4x4", ParameterType::PT_DVEC4 },
        { "double3x3", ParameterType::PT_DMAT3 },
        { "double4x4", ParameterType::PT_DMAT4 },
		};

		for (auto map : map_key_type)
		{
			if (cstr_cmp_skip(inout, map.m_key))
			{
				type = map.m_type;
				return true;
			}
		}

		return false;
	}
	//////////////////////////////////////////////////////
    void Parameters::push_error(const std::string& error)
    {
        m_context->m_errors.push_front(ErrorField{ m_context->m_line,error });
    }
}
}
