//
//  ParserEffect.cpp
//  Square
//
//  Created by Gabriele Di Bari on 03/05/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserEffect.h"

namespace Square
{
namespace Parser
{
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    std::string Effect::blend_to_string(Render::BlendType blend)
    {
        using namespace Render;
        switch (blend)
        {
            case BLEND_ONE: return "one";
            case BLEND_ZERO: return "zero";
                
            case BLEND_ONE_MINUS_DST_COLOR: return "one_minus_dst_color";
            case BLEND_ONE_MINUS_DST_ALPHA: return "one_minus_dst_alpha";
            case BLEND_ONE_MINUS_SRC_COLOR: return "one_minus_src_color";
            case BLEND_ONE_MINUS_SRC_ALPHA: return "one_minus_src_alpha";
                
                
            case BLEND_DST_COLOR: return "dst_color";
            case BLEND_DST_ALPHA: return "dst_alpha";
                
            case BLEND_SRC_COLOR: return "src_color";
            case BLEND_SRC_ALPHA: return "src_alpha";
            case BLEND_SRC_ALPHA_SATURATE: return "src_apha_sature";
            default: return "";
        }
    }
    
    Render::BlendType Effect::blend_from_string(const std::string& blend, Render::BlendType blend_default)
    {
        using namespace Render;
        //
        if (blend == "one") return BLEND_ONE;
        if (blend == "zero") return BLEND_ZERO;
        //
        if (blend == "one_minus_dst_color") return BLEND_ONE_MINUS_DST_COLOR;
        if (blend == "one_minus_dst_alpha") return BLEND_ONE_MINUS_DST_ALPHA;
        if (blend == "one_minus_src_color") return BLEND_ONE_MINUS_SRC_COLOR;
        if (blend == "one_minus_src_alpha") return BLEND_ONE_MINUS_SRC_ALPHA;
        //
        if (blend == "dst_color") return BLEND_DST_COLOR;
        if (blend == "dst_alpha") return BLEND_DST_ALPHA;
        //
        if (blend == "src_color") return BLEND_SRC_COLOR;
        if (blend == "src_alpha") return BLEND_SRC_ALPHA;
        if (blend == "src_apha_sature") return BLEND_SRC_ALPHA_SATURATE;
        
        return blend_default;
    }
    
    std::string Effect::depth_to_string(Render::DepthFuncType depth)
    {
        using namespace Render;
        switch (depth)
        {
            case DT_NEVER: return "never";
            case DT_LESS: return "less";
            case DT_GREATER:  return "greater";
            case DT_EQUAL:  return "equal";
            case DT_LESS_EQUAL:  return "less_equal";
            case DT_GREATER_EQUAL:  return "greater_equal";
            case DT_NOT_EQUAL:  return "not_equal";
            case DT_ALWAYS: return "always";
            default: break;
        }
    }

    Render::DepthFuncType Effect::depth_from_string(const std::string& depth, Render::DepthFuncType depth_default)
    {
        using namespace Render;
        //
        if (depth == "never") return DT_NEVER;
        if (depth == "less") return DT_LESS;
        if (depth == "greater") return DT_GREATER;
        //
        if (depth == "equal") return DT_EQUAL;
        if (depth == "less_equal") return DT_LESS_EQUAL;
        if (depth == "greater_equal") return DT_GREATER_EQUAL;
        if (depth == "not_equal") return DT_NOT_EQUAL;
        //
        if (depth == "always") return DT_ALWAYS;
        //
        return depth_default;
    }
    
    std::string Effect::cullface_to_string(Render::CullfaceType cullface)
    {
        //
        using namespace Render;
        //
        switch (cullface)
        {
            case CF_BACK: return "back";
            case CF_FRONT: return "front";
            case CF_FRONT_AND_BACK: return "front_and_back";
            default: return "";
        }
    }
    
    Render::CullfaceType Effect::cullface_from_string(const std::string& cullface, Render::CullfaceType cullface_default)
    {
        //
        using namespace Render;
        //
        if (cullface == "back")           return CF_BACK;
        if (cullface == "front")          return CF_FRONT;
        if (cullface == "front_and_back") return CF_FRONT_AND_BACK;
        return cullface_default;
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    //RequirementField
    bool Effect::RequirementField::test(Render::Context* render) const
    {
        //get info
        Render::RenderDriverInfo info = render->get_render_driver_info();
        //test name
        if (m_driver_name != Render::render_driver_str[info.m_render_driver]) return false;
        if (m_shader_name != info.m_shader_language)                          return false;
        //test version
        if (m_driver_major_version <= info.m_major_version &&
            m_driver_minor_version <= info.m_minor_version &&
            m_shader_version <= info.m_shader_version)
        {
            return true;
        }
        return false;
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    // ParameterField
    bool  Effect::ParameterField::alloc(ParameterType type)
    {
        //type
        m_type = type;
        //alloc
        m_paramter = ::Square::Resource::Effect::create_parameter(m_type);
        //if
        if(!m_paramter) return false;
        //else
        return true;
    }
    bool  Effect::ParameterField::alloc(const std::string& name,ParameterType type)
    {
        //name
        m_name = name;
        //type
        m_type = type;
        //alloc
        m_paramter = ::Square::Resource::Effect::create_parameter(m_type);
        //if
        if(!m_paramter) return false;
        //else
        return true;
    }
    Effect::ParameterField::~ParameterField()
    {
        if ( m_paramter ) delete m_paramter;
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    // Context
    std::string Effect::Context::errors_to_string() const
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
    bool Effect::parse(Context& default_context, const std::string& source)
    {
        const char* c_ptr = source.c_str();
        return parse(default_context, c_ptr);
    }
    
    bool Effect::parse(Context& default_context, const char*& ptr)
    {
        m_context = &default_context;
        //restart
        m_context->m_sub_effect.clear();
        m_context->m_parameters.clear();
        m_context->m_errors.clear();
        m_context->m_line = 1;
        //get type
        while (*ptr != EOF && *ptr != '\0')
        {
            //skeep line and comments
            skip_space_and_comments(m_context->m_line, ptr);
            //parsing a block
            if (cstr_cmp_skip(ptr, "parameters")) { if (!parse_parameters_block(ptr)) return false; }
            else  if (cstr_cmp_skip(ptr, "sub_effect")) { if (!parse_sub_effect_block(ptr)) return false; }
            else { push_error("Not found a valid command"); return false; }
            //skeep line and comments
            skip_space_and_comments(m_context->m_line, ptr);
        }
        return true;
    }

    bool Effect::parse_parameters_block(const char*& ptr)
    {
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
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
                //skeep "line" space
                skip_line_space(m_context->m_line, ptr);
                //value
                if (!parse_value(ptr, field))
                {
                    push_error("Not valid uniform field");
                    return false;
                }
                //push
                m_context->m_parameters.push_back(field);
                //skeep spaces
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
    
    bool Effect::parse_value(const char*& ptr, ParameterField& field)
    {
        if (!parse_type(ptr, field.m_type)) return false;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump '('
        ++ptr;
        //space
        skip_space_and_comments(m_context->m_line, ptr);
        //alloc
        field.alloc(field.m_type);
        //parse by type
        switch (field.m_type)
        {
            case ParameterType::PT_INT:     if(!parse_int_values(ptr, field.m_paramter->value_ptr<int>(), 1))  return false;  break;
            case ParameterType::PT_FLOAT:   if(!parse_float_values(ptr, field.m_paramter->value_ptr<float>(), 1))  return false; break;
            case ParameterType::PT_DOUBLE:  if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 1))  return false; break;
            case ParameterType::PT_TEXTURE: /*if(!parse_double_values(ptr, field.m_paramter->value_ptr<double>(), 1))  return false; */ break;

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
    
    bool Effect::parse_int_values(const char*& ptr, int* values, size_t n)
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
    
    bool Effect::parse_float_values(const char*& ptr, float* values, size_t n)
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
    
    bool Effect::parse_double_values(const char*& ptr, double* values, size_t n)
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
    
    bool Effect::parse_texture(const char*& ptr, ParameterField& field)
    {
        //texture name
        std::string texture_name;
        //parse
        if (!parse_string(m_context->m_line, ptr, texture_name))
        {
            return false;
        }
        //save texture name
        field.m_resource.push_back(texture_name);
        //ok
        return true;
    }
    
    bool Effect::parse_mat3(const char*& ptr, Mat3& m3)
    {
        //first
        if (!parse_float(ptr, m3[0][0])) return false;
        //skeep spaces
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
    
    bool Effect::parse_mat4(const char*& ptr, Mat4& m4)
    {
        //first
        if (!parse_float(ptr, m4[0][0])) return false;
        //skeep spaces
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
    
    bool Effect::parse_dmat3(const char*& ptr, DMat3& m3)
    {
        //first
        if (!parse_double(ptr, m3[0][0])) return false;
        //skeep spaces
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
    
    bool Effect::parse_dmat4(const char*& ptr, DMat4& m4)
    {
        //first
        if (!parse_double(ptr, m4[0][0])) return false;
        //skeep spaces
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
    bool Effect::parse_driver_type(const char*& ptr, RequirementField& field)
    {
        if (!parse_name(ptr, field.m_driver_name)) return false;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump  '('
        ++ptr;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //major
        if (!parse_int(ptr, field.m_driver_major_version)) return false;
        //commond
        skip_space_and_comments(m_context->m_line, ptr);
        if (!is_comm_arg(*ptr)) return false; else ++ptr;
        skip_space_and_comments(m_context->m_line, ptr);
        //minor
        if (!parse_int(ptr, field.m_driver_minor_version)) return false;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse ')'
        if (!is_end_arg(*ptr)) return false;
        //jump  ')'
        ++ptr;
        //ok
        return true;
    }
    
    bool Effect::parse_shader_type(const char*& ptr, RequirementField& field)
    {
        if (!parse_name(ptr, field.m_shader_name)) return false;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump  '('
        ++ptr;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //major
        if (!parse_int(ptr, field.m_shader_version)) return false;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse ')'
        if (!is_end_arg(*ptr)) return false;
        //jump  ')'
        ++ptr;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //ok
        return true;
    }
    //////////////////////////////////////////////////////
    bool Effect::parse_sub_effect_block(const char*& ptr)
    {
        //shader parser state
        enum
        {
            NOT_READ_REQUIREMENT,
            READ_REQUIREMENT
        }
        state = NOT_READ_REQUIREMENT;
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //add technique
        m_context->m_sub_effect.resize(m_context->m_sub_effect.size() + 1);
        //ref
        SubEffectField& t_field = m_context->m_sub_effect[m_context->m_sub_effect.size() - 1];
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //search source attribute
                if (cstr_cmp_skip(ptr, "requirement"))
                {
                    //test
                    if (state == READ_REQUIREMENT)
                    {
                        push_error("Requirement already declared");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_requirement_block(ptr, t_field.m_requirement)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //change state
                    state = READ_REQUIREMENT;
                }
                if (cstr_cmp_skip(ptr, "queue"))
                {
                    //test
                    if (state == NOT_READ_REQUIREMENT)
                    {
                        push_error("Requirement must to be declared before");
                        return false;
                    }
                    //..
                    ParameterQueue p_queue;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_queue_block(ptr, t_field.m_queue)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "technique"))
                {
                    //test
                    if (state == NOT_READ_REQUIREMENT)
                    {
                        push_error("Requirement must to be declared before");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //techniques
                    if (!parse_techniques_block(ptr, t_field)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
                //skeep spaces
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
        //end
        return true;
    }
    //////////////////////////////////////////////////////
    bool Effect::parse_requirement_block(const char*& ptr, RequirementField& r_field)
    {
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //all casses
                if (cstr_cmp_skip(ptr, "driver"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_driver_type(ptr, r_field))
                    {
                        push_error("Requirement: driver value not valid");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "shader"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_shader_type(ptr, r_field))
                    {
                        push_error("Requirement: shader value not valid");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
                //skeep spaces
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
    //////////////////////////////////////////////////////
    bool Effect::parse_techniques_block(const char*& ptr, SubEffectField& subeffect)
    {
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //name technique
        TechniqueField new_t_field;
        //get name
        if (!parse_string(m_context->m_line, ptr, new_t_field.m_name))
        {
            push_error("Not valid technique name");
            return false;
        }
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //add technique
        subeffect.m_techniques.push_back(new_t_field);
        //ref
        TechniqueField& t_field = subeffect.m_techniques[subeffect.m_techniques.size() - 1];
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //search source attribute
                if (cstr_cmp_skip(ptr, "pass"))
                {
                    PassField pass;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_pass_block(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //append
                    t_field.m_pass.push_back(pass);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
                //skeep spaces
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
        //end
        return true;
    }
    //////////////////////////////////////////////////////
    bool Effect::parse_pass_block(const char*& ptr, PassField& pass)
    {
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //all casses
                if (cstr_cmp(ptr, "blend"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_blend(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp(ptr, "depth") || cstr_cmp(ptr, "zbuffer"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_depth(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp(ptr, "cullface"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_cullface(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp(ptr, "lights"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_lights(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp(ptr, "shader"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_shader(ptr, pass)) return false;
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
                //skeep spaces
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
    bool Effect::parse_queue_block(const char*& ptr, ParameterQueue& p_queue)
    {
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            //skeep spaces
            skip_space_and_comments(m_context->m_line, ptr);
            //read all values
            while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
            {
                //all casses
                if (cstr_cmp_skip(ptr, "type"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_queue_type(ptr, p_queue.m_type))
                    {
                        push_error("Queue type not valid");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "order"))
                {
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_int(ptr, p_queue.m_order))
                    {
                        push_error("Queue order value not valid");
                        return false;
                    }
                    //skeep spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
                //skeep spaces
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
    //////////////////////////////////////////////////////
    bool Effect::parse_blend(const char*& ptr, PassField& pass)
    {
        //search source attribute
        if (!cstr_cmp_skip(ptr, "blend"))
        {
            push_error("Blend not found");
            return false;
        }
        //skeep "line" space
        skip_line_space(m_context->m_line, ptr);
        //string
        std::string param1, param2;
        //parse name
        if (!parse_name(ptr, param1))
        {
            push_error("First blend parameter not valid");
            return false;
        }
        //param test
        if (param1 == "off")
        {
            pass.m_blend = Render::BlendState();
            return true;
        }
        //skeep "line" space
        skip_line_space(m_context->m_line, ptr);
        //parse name
        if (!parse_name(ptr, param2))
        {
            push_error("Second blend parameter not valid");
            return false;
        }
        //parse
        pass.m_blend.m_enable = true;
        pass.m_blend.m_src = blend_from_string(param1, Render::BLEND_ONE);
        pass.m_blend.m_dst = blend_from_string(param2, Render::BLEND_ZERO);
        return true;
    }
    bool Effect::parse_depth(const char*& ptr, PassField& pass)
    {
        //search source attribute
        if (!(cstr_cmp_skip(ptr, "depth") || cstr_cmp_skip(ptr, "zbuffer")))
        {
            push_error("Depth not found");
            return false;
        }
        //skeep "line" space
        skip_line_space(m_context->m_line, ptr);
        //string
        std::string param1;
        //parse name
        if (!parse_name(ptr, param1))
        {
            push_error("Depth parameter not valid");
            return false;
        }
        //param test
        if (param1 == "off")
        {
            pass.m_depth = Render::DepthBufferState({ Render::DM_DISABLE });
            return true;
        }
        //parse
        pass.m_depth.m_mode = Render::DM_ENABLE_AND_WRITE;
        pass.m_depth.m_type = depth_from_string(param1,  Render::DT_LESS);
        return true;
    }
    bool Effect::parse_cullface(const char*& ptr, PassField& pass)
    {
        //search source attribute
        if (!(cstr_cmp_skip(ptr, "cullface")))
        {
            push_error("Cullface not found");
            return false;
        }
        //skeep "line" space
        skip_line_space(m_context->m_line, ptr);
        //string
        std::string param1;
        //parse name
        if (!parse_name(ptr, param1))
        {
            push_error("Cullface parameter not valid");
            return false;
        }
        //param test
        if (param1 == "off")
        {
            pass.m_cullface = Render::CullfaceState(Render::CF_DISABLE);
            return true;
        }
        //parse
        pass.m_cullface.m_cullface = cullface_from_string(param1, Render::CF_BACK);
        return true;
    }
    bool Effect::parse_lights(const char*& ptr, PassField& pass)
    {
        //search source attribute
        if (!(cstr_cmp_skip(ptr, "lights")))
        {
            push_error("Lights not found");
            return false;
        }
        //skeep "line" space
        skip_line_space(m_context->m_line, ptr);
        //is off?
        if ((cstr_cmp_skip(ptr, "off")))
        {
            //default pass.m_lights
            return true;
        }
        //is only_ambient?
        if ((cstr_cmp_skip(ptr, "only_ambient")))
        {
            pass.m_lights = LF_AMBIENT;
            return true;
        }
        //spot point and direction
        if ((cstr_cmp_skip(ptr, "spot_point_direction")))
        {
            pass.m_lights = LF_SPOT_POINT_DIRECTION;
            return true;
        }
        //spot and point
        if ((cstr_cmp_skip(ptr, "spot_point")))
        {
            pass.m_lights = LF_SPOT_POINT;
            return true;
        }
        //spot and direction
        if ((cstr_cmp_skip(ptr, "spot_direction")))
        {
            pass.m_lights = LF_SPOT_DIRECTION;
            return true;
        }
        //point and direction
        if ((cstr_cmp_skip(ptr, "point_direction")))
        {
            pass.m_lights = LF_POINT_DIRECTION;
            return true;
        }
        //spot
        if ((cstr_cmp_skip(ptr, "spot")))
        {
            pass.m_lights = LF_SPOT;
            return true;
        }
        //spot
        if ((cstr_cmp_skip(ptr, "point")))
        {
            pass.m_lights = LF_POINT;
            return true;
        }
        //spot
        if ((cstr_cmp_skip(ptr, "direction")))
        {
            pass.m_lights = LF_DIRECTION;
            return true;
        }
        //error
        push_error("Lights parameter not valid");
        //end
        return false;
    }
    bool Effect::parse_shader(const char*& ptr, PassField& pass)
    {
        //search shader attribute
        if (!cstr_cmp_skip(ptr, "shader"))
        {
            push_error("Shader not found");
            return false;
        }
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //is a shader name?
        if (is_start_string(*ptr))
        {
            //shader name
            std::string shader_name;
            //get
            if (!parse_string(m_context->m_line, ptr, shader_name))
            {
                push_error("Not valid shader name");
                return false;
            }
            pass.m_shader.m_name = true;
            pass.m_shader.m_text = shader_name;
            return true;
        }
        //else is a shader source
        if (!cstr_cmp_skip(ptr, "source"))
        {
            push_error("Shader source not found");
            return false;
        }
        //skeep spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse source
        pass.m_shader.m_name = false;
        //count parenthesis
        size_t p_count = 0;
        //inside a comment
        bool   c_inside = false;
        //parse table
        if (is_start_table(*ptr))
        {
            //skeep '{'
            ++ptr;
            ++p_count;
            //save start line
            pass.m_shader.m_line = m_context->m_line;
            //read all values
            while (*ptr != EOF && *ptr != '\0')
            {
                //comment test
                if (is_start_multy_line_comment(ptr)) { c_inside = true; }
                else if (is_end_multy_line_comment(ptr)) { c_inside = false; }
                //inc case
                if (is_start_table(*ptr) && !c_inside) ++p_count;
                //dec case
                if (is_end_table(*ptr) && !c_inside) --p_count;
                //exit case
                if (!p_count) break;
                //inc count line
                if (*ptr == '\n') ++m_context->m_line;
                //append
                pass.m_shader.m_text += *ptr;
                //next
                ++ptr;
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
    //////////////////////////////////////////////////////
    bool Effect::parse_type(const char*& inout, ParameterType& type)
    {
        struct { const char* m_key; ParameterType m_type; } map_key_type []
        {
            { "int", ParameterType::PT_INT  },
            { "float", ParameterType::PT_FLOAT  },
            { "double", ParameterType::PT_DOUBLE  },
            { "Texture", ParameterType::PT_TEXTURE  },
            
            { "IVec2", ParameterType::PT_IVEC2  },
            { "IVec3", ParameterType::PT_IVEC3  },
            { "IVec4", ParameterType::PT_IVEC4  },
            
            { "Vec2", ParameterType::PT_VEC2  },
            { "Vec3", ParameterType::PT_VEC3  },
            { "Vec4", ParameterType::PT_VEC4  },
            { "Mat3", ParameterType::PT_MAT3  },
            { "Mat4", ParameterType::PT_MAT4  },
            
            { "DVec2", ParameterType::PT_DVEC2  },
            { "DVec3", ParameterType::PT_DVEC3  },
            { "DVec4", ParameterType::PT_DVEC4  },
            { "DMat3", ParameterType::PT_DMAT3  },
            { "DMat4", ParameterType::PT_DMAT4  },
        };
        
        for(auto map : map_key_type)
        {
            if (cstr_cmp_skip(inout,  map.m_key))
            {
                type = map.m_type;
                return true;
            }
        }
        
        return false;
    }
    bool Effect::parse_queue_type(const char*& inout, Render::QueueType& type)
    {
        
        struct { const char* m_key; Render::QueueType m_type; } map_key_type []
        {
            { "opaque", Render::RQ_OPAQUE },
            { "translucent", Render::RQ_TRANSLUCENT },
            { "ui", Render::RQ_UI },
            { "background", Render::RQ_BACKGROUND },
        };
        
        for(auto map : map_key_type)
        {
            if (cstr_cmp_skip(inout,  map.m_key))
            {
                type = map.m_type;
                return true;
            }
        }
        
        return false;
    }
    //////////////////////////////////////////////////////
    void Effect::push_error(const std::string& error)
    {
        m_context->m_errors.push_front(ErrorField{ m_context->m_line,error });
    }
}
}
