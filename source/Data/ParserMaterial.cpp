//
//  ParserMaterial.cpp
//  Square
//
//  Created by Gabriele Di Bari on 03/05/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserMaterial.h"

namespace Square
{
namespace Parser
{
	//////////////////////////////////////////////////////
	// Context
	std::string Material::Context::errors_to_string() const
	{
		std::stringstream sbuffer;

		for (auto& error : m_errors)
		{
			sbuffer << "Error:" << error.m_line << ": " << error.m_error << "\n";
		}

		return sbuffer.str();
	}
	//////////////////////////////////////////////////////
	bool Material::parse(Context& default_context, const std::string& source)
	{
		const char* source_ptr = source.c_str();
		return parse(default_context, source_ptr);
	}
	bool Material::parse(Context& default_context, const char*& ptr)
	{
		//set context
		m_context = &default_context;
		//skeep line and comments
		skip_space_and_comments(m_context->m_line, ptr);
		//get type
		if (*ptr != EOF && *ptr != '\0')
		{
			//parsing a block
			     if (cstr_cmp_skip(ptr, "effect"))     { if (!parse_effect(ptr))  return false; }
			else if (cstr_cmp_skip(ptr, "parameters")) { if (!parse_effect(ptr))  return false; }
			else if (cstr_cmp_skip(ptr, "material"))   { if (!parse_effect(ptr))  return false; }
			else { push_error("Not found a valid command"); return false; }
			//skeep line and comments
			skip_space_and_comments(m_context->m_line, ptr);
		}
		return true;
	}	
	//////////////////////////////////////////////////////
	bool Material::parse_effect(const char*& ptr)
	{
		//skeep spaces
		skip_space_and_comments(m_context->m_line, ptr);
		//name technique
		std::string effect_name;
		//get name
		if (!parse_string(m_context->m_line, ptr, effect_name))
		{
			push_error("Not valid effect name");
			return false;
		}
		//save effect name
		m_context->m_effect = effect_name;
		//parse block
		return parse_parameters_block(ptr);
	}
	bool Material::parse_parameters_block(const char*& ptr)
	{
		//skeep spaces
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
	void Material::push_error(const std::string& error)
	{
		m_context->m_errors.push_front(ErrorField{ m_context->m_line,error });
	}
	//////////////////////////////////////////////////////
}
}