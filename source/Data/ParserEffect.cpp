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
		//all cases?
		bool all_drivers = m_driver_name == "ALL";
		bool all_shader  = m_shader_name == "ALL";
		//test name
        if (!all_drivers && m_driver_name != Render::render_driver_str[info.m_render_driver]) return false;
        if (!all_shader  && m_shader_name != info.m_shader_language)                          return false;
        //test version
        if (all_drivers || (m_driver_major_version <= info.m_major_version && m_driver_minor_version <= info.m_minor_version))
		if (all_shader || (m_shader_version <= info.m_shader_version))
        {
            return true;
        }
        return false;
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
		//set context
        m_context = &default_context;
        //get type
        while (*ptr != EOF && *ptr != '\0')
        {
            //skip line and comments
            skip_space_and_comments(m_context->m_line, ptr);
            //parsing a block
            if (cstr_cmp_skip(ptr, "parameters")) { if (!parse_parameters_block(ptr)) return false; }
            else  if (cstr_cmp_skip(ptr, "sub_effect")) { if (!parse_sub_effect_block(ptr)) return false; }
            else { push_error("Not found a valid command"); return false; }
            //skip line and comments
            skip_space_and_comments(m_context->m_line, ptr);
        }
        return true;
    }
	//////////////////////////////////////////////////////
    bool Effect::parse_parameters_block(const char*& ptr)
    {
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
		//Parser
		Parser::Parameters::Context params;
		params.m_line = m_context->m_line;
		Parser::Parameters parser_parameters;
		bool success = parser_parameters.parse(params, ptr);
		//add errors
		if (!success)
		{
			for (auto& error : params.m_errors)
				m_context->m_errors.push_back(error);
		}
		//copy all parameters
		m_context->m_parameters = std::move(params.m_parameters);
		//update line
		m_context->m_line = params.m_line;
        //results
        return success;
    }   
    //////////////////////////////////////////////////////
    bool Effect::parse_driver_type(const char*& ptr, RequirementField& field)
    {
        if (!parse_name(ptr, field.m_driver_name)) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump  '('
        ++ptr;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //major
        if (!parse_int(ptr, field.m_driver_major_version)) return false;
        //commond
        skip_space_and_comments(m_context->m_line, ptr);
        if (!is_comm_arg(*ptr)) return false; else ++ptr;
        skip_space_and_comments(m_context->m_line, ptr);
        //minor
        if (!parse_int(ptr, field.m_driver_minor_version)) return false;
        //skip spaces
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
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse '('
        if (!is_start_arg(*ptr)) return false;
        //jump  '('
        ++ptr;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //major
        if (!parse_int(ptr, field.m_shader_version)) return false;
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //parse ')'
        if (!is_end_arg(*ptr)) return false;
        //jump  ')'
        ++ptr;
        //skip spaces
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
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //add technique
        m_context->m_sub_effect.resize(m_context->m_sub_effect.size() + 1);
        //ref
        SubEffectField& t_field = m_context->m_sub_effect[m_context->m_sub_effect.size() - 1];
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
                //search source attribute
                if (cstr_cmp_skip(ptr, "requirement"))
                {
                    //test
                    if (state == READ_REQUIREMENT)
                    {
                        push_error("Requirement already declared");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_requirement_block(ptr, t_field.m_requirement)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //change state
                    state = READ_REQUIREMENT;
                }
                if (cstr_cmp_skip(ptr, "queue"))
                {
                    //..
                    EffectQueueType p_queue;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_queue_block(ptr, t_field.m_queue)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "technique"))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //techniques
                    if (!parse_techniques_block(ptr, t_field)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
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
        //end
        return true;
    }
    //////////////////////////////////////////////////////
    bool Effect::parse_requirement_block(const char*& ptr, RequirementField& r_field)
    {
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
                //all casses
                if (cstr_cmp_skip(ptr, "driver"))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_driver_type(ptr, r_field))
                    {
                        push_error("Requirement: driver value not valid");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "shader"))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_shader_type(ptr, r_field))
                    {
                        push_error("Requirement: shader value not valid");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
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
    //////////////////////////////////////////////////////
    bool Effect::parse_techniques_block(const char*& ptr, SubEffectField& subeffect)
    {
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //name technique
        TechniqueField new_t_field;
        //get name
        if (!parse_string(m_context->m_line, ptr, new_t_field.m_name))
        {
            push_error("Not valid technique name");
            return false;
        }
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //add technique
        subeffect.m_techniques.push_back(new_t_field);
        //ref
        TechniqueField& t_field = subeffect.m_techniques[subeffect.m_techniques.size() - 1];
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
                //search source attribute
                if (cstr_cmp_skip(ptr, "pass"))
                {
                    PassField pass;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_pass_block(ptr, pass)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //append
                    t_field.m_pass.push_back(pass);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
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
        //end
        return true;
    }
    //////////////////////////////////////////////////////
    bool Effect::parse_pass_block(const char*& ptr, PassField& pass)
    {
		//field alias		
		using PKey = const char*;
		using PKeys = std::vector<PKey>;
		using PFunction = bool (Effect::*)(const char*&, PassField&);
		//table
		struct { PKeys m_keys; PFunction m_funtion; } keys_function_table[]
		{
			{ { "blend" },            &Effect::parse_blend },
			{ { "depth", "zbuffer" }, &Effect::parse_depth },
			{ { "cullface" },         &Effect::parse_cullface },
			{ { "lights" },           &Effect::parse_lights },
			{ { "shader" },           &Effect::parse_shader },
		};

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
				//success
				bool success = false;
				//ok
				for (auto& keys_function : keys_function_table)
				for (auto& key : keys_function.m_keys)
				{
					if (!success && cstr_cmp_skip(ptr, key))
					{
						//parse textures
						if (!(this->*keys_function.m_funtion)(ptr, pass)) return false;
						//skip spaces
						skip_space_and_comments(m_context->m_line, ptr);
						//success
						success = true;
					}
				}
                if(!success)
                {
                    push_error("Keyword not valid");
                    return false;
                }
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
    bool Effect::parse_queue_block(const char*& ptr, EffectQueueType& p_queue)
    {
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
                //all casses
                if (cstr_cmp_skip(ptr, "type"))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_queue_type(ptr, p_queue.m_type))
                    {
                        push_error("Queue type not valid");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, "order"))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_int(ptr, p_queue.m_order))
                    {
                        push_error("Queue order value not valid");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else
                {
                    push_error("Keyword not valid");
                    return false;
                }
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
    //////////////////////////////////////////////////////
    bool Effect::parse_blend(const char*& ptr, PassField& pass)
    {
        //skip "line" space
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
        //skip "line" space
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
        //skip "line" space
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
        //skip "line" space
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
        //skip "line" space
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
        //skip spaces
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
        //skip spaces
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
            //skip '{'
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
