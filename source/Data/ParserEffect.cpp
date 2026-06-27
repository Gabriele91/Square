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
    // Keyword constants for the .effect grammar. Centralised so the spelling
    // lives in one place (no scattered string literals). NOTE: the light/shadow
    // selectors must still be tested longest-first at the call sites, because
    // cstr_cmp_skip matches a prefix (e.g. "point" also matches "point_xxx").
    namespace keyword
    {
        // top-level commands
        constexpr std::string_view parameters = "parameters";
        constexpr std::string_view sub_effect = "sub_effect";
        // sub_effect block
        constexpr std::string_view requirement = "requirement";
        constexpr std::string_view queue       = "queue";
        constexpr std::string_view technique   = "technique";
        constexpr std::string_view import      = "import";
        constexpr std::string_view include     = "include";
        constexpr std::string_view from        = "from";
        // requirement block
        constexpr std::string_view driver                = "driver";
        constexpr std::string_view shader                = "shader";
        constexpr std::string_view geometry              = "geometry";
        constexpr std::string_view vertex_viewport_index = "vertex_viewport_index";
        constexpr std::string_view instanced_draw        = "instanced_draw";
        // light / shadow selectors
        constexpr std::string_view ambient              = "ambient";
        constexpr std::string_view only_ambient         = "only_ambient";
        constexpr std::string_view spot_point_direction = "spot_point_direction";
        constexpr std::string_view point_direction_spot = "point_direction_spot";
        constexpr std::string_view direction_spot_point = "direction_spot_point";
        constexpr std::string_view spot_point           = "spot_point";
        constexpr std::string_view point_spot           = "point_spot";
        constexpr std::string_view spot_direction       = "spot_direction";
        constexpr std::string_view direction_spot       = "direction_spot";
        constexpr std::string_view point_direction      = "point_direction";
        constexpr std::string_view direction_point      = "direction_point";
        constexpr std::string_view direction            = "direction";
        constexpr std::string_view point                = "point";
        constexpr std::string_view spot                 = "spot";
        constexpr std::string_view area                 = "area";
        // shader block
        constexpr std::string_view source = "source";
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    // Keyword <-> enum tables. Single source of truth: each value's spelling lives
    // in exactly one place, shared by the to_string / from_string / parse_* helpers.
    namespace
    {
        // -- lights / shadows ------------------------------------------------------
        struct LightSelector { std::string_view name; Effect::LightsField value; };
        // Ordered LONGEST-FIRST: cstr_cmp_skip matches a prefix, so a longer keyword
        // must be tested before any keyword that is a prefix of it (e.g. "spot_point"
        // before "spot"). Reordering this table can silently break parsing.
        constexpr LightSelector s_light_selectors[]
        {
            { keyword::spot_point_direction, Effect::LT_SPOT_POINT_DIRECTION },
            { keyword::point_direction_spot, Effect::LT_SPOT_POINT_DIRECTION },
            { keyword::direction_spot_point, Effect::LT_SPOT_POINT_DIRECTION },
            { keyword::spot_point,           Effect::LT_SPOT_POINT },
            { keyword::point_spot,           Effect::LT_SPOT_POINT },
            { keyword::spot_direction,       Effect::LT_SPOT_DIRECTION },
            { keyword::direction_spot,       Effect::LT_SPOT_DIRECTION },
            { keyword::point_direction,      Effect::LT_POINT_DIRECTION },
            { keyword::direction_point,      Effect::LT_POINT_DIRECTION },
            { keyword::direction,            Effect::LT_DIRECTION },
            { keyword::point,                Effect::LT_POINT },
            { keyword::spot,                 Effect::LT_SPOT },
            { keyword::area,                 Effect::LT_AREA },
        };
        // Consume a light/shadow selector keyword; false if none matched.
        inline bool match_light_selector(const char*& ptr, Effect::LightsField& out)
        {
            for (const auto& e : s_light_selectors)
                if (cstr_cmp_skip(ptr, e.name)) { out = e.value; return true; }
            return false;
        }

        // -- requirement capability gates ------------------------------------------
        // Map a requirement keyword to the CapabilityTest field it drives.
        struct CapabilityGate { std::string_view name; Effect::CapabilityTest Effect::RequirementField::* field; };
        constexpr CapabilityGate s_capability_gates[]
        {
            { keyword::geometry,              &Effect::RequirementField::m_geometry_shader },
            { keyword::vertex_viewport_index, &Effect::RequirementField::m_vertex_viewport_index },
            { keyword::instanced_draw,        &Effect::RequirementField::m_draw_instanced },
        };

        // Consume a capability-gate keyword from the stream; nullptr if none matched.
        inline const CapabilityGate* capability_gate(const char*& ptr)
        {
            for (const auto& gate : s_capability_gates)
                if (cstr_cmp_skip(ptr, gate.name)) return &gate;
            return nullptr;
        }
        // on/true/yes -> CAP_REQUIRE, off/false/no -> CAP_EXCLUDE; false if neither.
        inline bool parse_capability_value(const std::string& value, Effect::CapabilityTest& out)
        {
            if (is_true_keyword(value))  { out = Effect::CAP_REQUIRE; return true; }
            if (is_false_keyword(value)) { out = Effect::CAP_EXCLUDE; return true; }
            return false;
        }

        // -- blend / depth / cullface (exact match, order irrelevant) --------------
        struct BlendName    { std::string_view name; Render::BlendType     value; };
        struct DepthName    { std::string_view name; Render::DepthFuncType value; };
        struct CullfaceName { std::string_view name; Render::CullfaceType  value; };

        constexpr BlendName s_blend_table[]
        {
            { "one",                 Render::BLEND_ONE },
            { "zero",                Render::BLEND_ZERO },
            { "one_minus_dst_color", Render::BLEND_ONE_MINUS_DST_COLOR },
            { "one_minus_dst_alpha", Render::BLEND_ONE_MINUS_DST_ALPHA },
            { "one_minus_src_color", Render::BLEND_ONE_MINUS_SRC_COLOR },
            { "one_minus_src_alpha", Render::BLEND_ONE_MINUS_SRC_ALPHA },
            { "dst_color",           Render::BLEND_DST_COLOR },
            { "dst_alpha",           Render::BLEND_DST_ALPHA },
            { "src_color",           Render::BLEND_SRC_COLOR },
            { "src_alpha",           Render::BLEND_SRC_ALPHA },
            { "src_alpha_saturate",  Render::BLEND_SRC_ALPHA_SATURATE },
        };
        constexpr DepthName s_depth_table[]
        {
            { "never",         Render::DT_NEVER },
            { "less",          Render::DT_LESS },
            { "greater",       Render::DT_GREATER },
            { "equal",         Render::DT_EQUAL },
            { "less_equal",    Render::DT_LESS_EQUAL },
            { "greater_equal", Render::DT_GREATER_EQUAL },
            { "not_equal",     Render::DT_NOT_EQUAL },
            { "always",        Render::DT_ALWAYS },
        };
        constexpr CullfaceName s_cullface_table[]
        {
            { "back",           Render::CF_BACK },
            { "front",          Render::CF_FRONT },
            { "front_and_back", Render::CF_FRONT_AND_BACK },
        };
    }
    // // // // // // // // // // // // // // // // // // // // // // // // // //
    std::string Effect::blend_to_string(Render::BlendType blend)
    {
        for (const auto& e : s_blend_table)
            if (e.value == blend) return std::string(e.name);
        return "";
    }

    Render::BlendType Effect::blend_from_string(const std::string& blend, Render::BlendType blend_default)
    {
        for (const auto& e : s_blend_table)
            if (e.name == std::string_view(blend)) return e.value;
        return blend_default;
    }
    
    std::string Effect::depth_to_string(Render::DepthFuncType depth)
    {
        for (const auto& e : s_depth_table)
            if (e.value == depth) return std::string(e.name);
        square_assert_debug(0);
        return "less";
    }

    Render::DepthFuncType Effect::depth_from_string(const std::string& depth, Render::DepthFuncType depth_default)
    {
        for (const auto& e : s_depth_table)
            if (e.name == std::string_view(depth)) return e.value;
        return depth_default;
    }
    
    std::string Effect::cullface_to_string(Render::CullfaceType cullface)
    {
        for (const auto& e : s_cullface_table)
            if (e.value == cullface) return std::string(e.name);
        square_assert_debug(0);
        return "front_and_back";
    }

    Render::CullfaceType Effect::cullface_from_string(const std::string& cullface, Render::CullfaceType cullface_default)
    {
        for (const auto& e : s_cullface_table)
            if (e.name == std::string_view(cullface)) return e.value;
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
        //test capability: geometry shader
        if (m_geometry_shader == CAP_REQUIRE && !info.m_geometry_shader) return false;
        if (m_geometry_shader == CAP_EXCLUDE &&  info.m_geometry_shader) return false;
        //test capability: write render-target-array-index / viewport index from the vertex shader
        if (m_vertex_viewport_index == CAP_REQUIRE && !info.m_vertex_viewport_index) return false;
        if (m_vertex_viewport_index == CAP_EXCLUDE &&  info.m_vertex_viewport_index) return false;
        //test capability: instanced draw calls
        if (m_draw_instanced == CAP_REQUIRE && !info.m_draw_instanced) return false;
        if (m_draw_instanced == CAP_EXCLUDE &&  info.m_draw_instanced) return false;
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
    bool Effect::parse(Allocator* allocator, Context& default_context, const std::string& source)
    {
        const char* c_ptr = source.c_str();
        return parse(allocator, default_context, c_ptr);
    }
    
    bool Effect::parse(Allocator* allocator, Context& default_context, const char*& ptr)
    {
		//set context
        m_context = &default_context;
        m_allocator = allocator;
        //get type
        while (*ptr != EOF && *ptr != '\0')
        {
            //skip line and comments
            skip_space_and_comments(m_context->m_line, ptr);
            //parsing a block
            if (cstr_cmp_skip(ptr, keyword::parameters)) { if (!parse_parameters_block(ptr)) return false; }
            else  if (cstr_cmp_skip(ptr, keyword::sub_effect)) { if (!parse_sub_effect_block(ptr)) return false; }
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
		bool success = parser_parameters.parse(m_allocator, params, ptr);
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
                if (cstr_cmp_skip(ptr, keyword::requirement))
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
                else if (cstr_cmp_skip(ptr, keyword::queue))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse textures
                    if (!parse_queue_block(ptr, t_field.m_queue)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
                else if (cstr_cmp_skip(ptr, keyword::technique))
                {
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //techniques
                    if (!parse_techniques_block(ptr, t_field)) return false;
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                }
				else if (cstr_cmp_skip(ptr, keyword::import) || cstr_cmp_skip(ptr, keyword::include))
				{
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//techniques
					if (!parse_import(ptr, t_field)) return false;
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
				}
				else if (cstr_cmp_skip(ptr, keyword::from))
				{
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//techniques
					if (!parse_from(ptr, t_field)) return false;
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
                if (cstr_cmp_skip(ptr, keyword::driver))
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
                else if (cstr_cmp_skip(ptr, keyword::shader))
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
                    //capability gate keyword (geometry / vertex_viewport_index / instanced_draw)
                    const CapabilityGate* gate = capability_gate(ptr);
                    // test gate
                    if (!gate)
                    {
                        push_error("Keyword not valid");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
                    //parse the on/off value
                    std::string value;
                    if (!parse_name(ptr, value))
                    {
                        push_error("Requirement: " + std::string(gate->name) + " value not valid");
                        return false;
                    }
                    if (!parse_capability_value(value, r_field.*(gate->field)))
                    {
                        push_error("Requirement: " + std::string(gate->name) + " expects on/off");
                        return false;
                    }
                    //skip spaces
                    skip_space_and_comments(m_context->m_line, ptr);
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
	bool Effect::parse_import(const char*& ptr, SubEffectField& subeffect)
	{
		//skip spaces
		skip_space_and_comments(m_context->m_line, ptr);		
		//Import field
		ImportField import_field;
		//case
		switch (*ptr)
		{
		case '\"':
		{
			//path
			std::string effect_path;
			//get
			if (!parse_string(m_context->m_line, ptr, effect_path))
			{
				push_error("Not valid import path");
				return false;
			}
			import_field.m_type = ImportField::I_INCLUDE;
			import_field.m_data = effect_path;
			subeffect.m_imports.push_back(import_field);
			return true;
		}
		case '<':
		{
			//name
			std::string effect_name;
			//get
			if (!parse_string(m_context->m_line, ptr, effect_name, '<', '>'))
			{
				push_error("Not valid import name");
				return false;
			}
			import_field.m_type = ImportField::I_RESOUCE;
			import_field.m_data = effect_name;
			subeffect.m_imports.push_back(import_field);
			return true;
		}
		default:
			push_error("Not valid token after import");
			return false;
		break;
		}
	}
	bool Effect::parse_from(const char*& ptr, SubEffectField& subeffect)
	{
		//Import field
		ImportField import_field;
		//skip spaces
		skip_space_and_comments(m_context->m_line, ptr);
		//case
		switch (*ptr)
		{
		case '\"':
		{
			//path
			std::string effect_path;
			//get
			if (!parse_string(m_context->m_line, ptr, effect_path))
			{
				push_error("Not valid import path");
				return false;
			}
			import_field.m_type = ImportField::I_FROM_INCLUDE;
			import_field.m_data = effect_path;
			break;
		}
		case '<':
		{
			//name
			std::string effect_name;
			//get
			if (!parse_string(m_context->m_line, ptr, effect_name, '<', '>'))
			{
				push_error("Not valid import name");
				return false;
			}
			import_field.m_type = ImportField::I_FROM_RESOUCE;
			import_field.m_data = effect_name;
			break;
		}
		default:
			push_error("Not valid token after import");
			return false;
			break;
		}
		//skip spaces
		skip_space_and_comments(m_context->m_line, ptr);
		//keyword
		if (!cstr_cmp_skip(ptr, "import") && !cstr_cmp_skip(ptr, "include"))
		{
			push_error("Not found import/include token after technique name");
			return false;
		}
		//skip spaces
		skip_space_and_comments(m_context->m_line, ptr);
		//name of technique
		std::string technique_name;
		if (!parse_string(m_context->m_line, ptr, technique_name))
		{
			push_error("Not valid import technique name");
			return false;
		}
		import_field.m_technique_name = technique_name;
		//import
		subeffect.m_imports.push_back(import_field);
        //ok
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
			{ { "blend" },                  &Effect::parse_blend },
			{ { "depth", "zbuffer" },       &Effect::parse_depth },
			{ { "cullface" },               &Effect::parse_cullface },
			{ { "lights" },                 &Effect::parse_lights },
			{ { "shadows" },                &Effect::parse_shadows },
			{ { "draw_count", "ndrawcall" },&Effect::parse_draw_count },
			{ { "instances", "instance_count" }, &Effect::parse_instances },
			{ { "shader" },                 &Effect::parse_shader },
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
        if (is_false_keyword(param1))
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
        if (is_false_keyword(param1))
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
        if (is_false_keyword(param1))
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
        //is off? (lights default to flat colour) — accepts off/false/no
        if (cstr_cmp_skip_false(ptr))
        {
            pass.m_lights = LightsField::LT_COLOR;
            return true;
        }
        //is ambient?
        if (cstr_cmp_skip(ptr, keyword::only_ambient) || cstr_cmp_skip(ptr, keyword::ambient))
        {
            pass.m_lights = LightsField::LT_AMBIENT;
            return true;
        }
        //light selector keyword (spot/point/direction and combinations)
        if (match_light_selector(ptr, pass.m_lights)) return true;
        //error
        push_error("Lights parameter not valid");
        //end
        return false;
    }
	bool Effect::parse_draw_count(const char*& ptr, PassField& pass)
	{
		//skip "line" space
		skip_line_space(m_context->m_line, ptr);
		//parse the integer count (number of times the pass is drawn)
		if (!parse_int(ptr, pass.m_draw_count))
		{
			push_error("draw_count: integer value expected");
			return false;
		}
		if (pass.m_draw_count < 1) pass.m_draw_count = 1;
		return true;
	}

	bool Effect::parse_instances(const char*& ptr, PassField& pass)
	{
		//skip "line" space
		skip_line_space(m_context->m_line, ptr);
		//parse the integer count (number of instances drawn per draw call)
		if (!parse_int(ptr, pass.m_instances))
		{
			push_error("instances: integer value expected");
			return false;
		}
		if (pass.m_instances < 1) pass.m_instances = 1;
		return true;
	}

	bool Effect::parse_shadows(const char*& ptr, PassField& pass)
	{
		//skip "line" space
		skip_line_space(m_context->m_line, ptr);
		//is off? — accepts off/false/no
		if (cstr_cmp_skip_false(ptr))
		{
			pass.m_shadows = LightsField::LT_NONE;
			return true;
		}
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!! is ambient? shadow not supported !!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//light selector keyword (same grammar as lights, minus ambient)
		if (match_light_selector(ptr, pass.m_shadows)) return true;
		//error
		push_error("Shadows parameter not valid");
		//end
		return false;
	}
    bool Effect::parse_shader(const char*& ptr, PassField& pass)
    {
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);    
		//case
		switch (*ptr)
		{
			case '\"':
			{
				//shader name
				std::string shader_path;
				//get
				if (!parse_string(m_context->m_line, ptr, shader_path))
				{
					push_error("Not valid shader include");
					return false;
				}
				pass.m_shader.m_type = ShaderField::S_INCLUDE;
				pass.m_shader.m_data = shader_path;
				return true;
			}
			case '<':
			{
				//shader name
				std::string shader_name;
				//get
				if (!parse_string(m_context->m_line, ptr, shader_name, '<', '>'))
				{
					push_error("Not valid shader name");
					return false;
				}
				pass.m_shader.m_type = ShaderField::S_RESOUCE;
				pass.m_shader.m_data = shader_name;
				return true;
			}
			default: /* is a source */ break;
		}
        //else is a shader source
        if (!cstr_cmp_skip(ptr, keyword::source))
        {
            push_error("Shader source not found");
            return false;
        }		
        //skip spaces
        skip_space_and_comments(m_context->m_line, ptr);
        //count parenthesis
        size_t p_count = 0;
        //inside a comment
        bool c_inside = false;
		//ref to data field
		std::string& output_source = pass.m_shader.m_data;
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
				output_source += *ptr;
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
		//set as source
		pass.m_shader.m_type = ShaderField::S_SOURCE;
		//ok
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
