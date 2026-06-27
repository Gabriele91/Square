//
//  ParserMaterial.cpp
//  Square
//
//  Created by Gabriele Di Bari on 03/05/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserMaterial.h"

namespace Square
{
namespace Parser
{
	// // // // // // // // // // // // // // // // // // // // // // // // // //
	// Keyword constants for the .material grammar. Centralised so the spelling
	// lives in one place (like ParserEffect).
	namespace keyword
	{
		constexpr std::string_view effect     = "effect";
		constexpr std::string_view parameters = "parameters";
		constexpr std::string_view material   = "material";
	}
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
	bool Material::parse(Allocator* allocator, Context& default_context, const std::string& source)
	{
		const char* source_ptr = source.c_str();
		return parse(allocator, default_context, source_ptr);
	}
	bool Material::parse(Allocator* allocator, Context& default_context, const char*& ptr)
	{
		//set context
		m_context = &default_context;
		m_allocator = allocator;
		//top-level commands: the three keywords are synonyms that introduce the same
		//'<keyword> "name" { parameters }' block.
		struct Command { std::string_view keyword; bool (Material::*handle)(const char*&); };
		static const Command commands[]
		{
			{ keyword::effect,     &Material::parse_effect },
			{ keyword::material,   &Material::parse_effect },
			{ keyword::parameters, &Material::parse_effect },
		};
		//get type
		while (*ptr != EOF && *ptr != '\0')
		{
			//skip line and comments
			skip_space_and_comments(m_context->m_line, ptr);
			//nothing left but trailing space/comments
			if (*ptr == EOF || *ptr == '\0') break;
			//dispatch to the command that matches the keyword
			bool handled = false;
			for (const auto& command : commands)
			{
				if (!cstr_cmp_skip(ptr, command.keyword)) continue;
				if (!(this->*command.handle)(ptr)) return false;
				handled = true;
				break;
			}
			if (!handled)
			{
				push_error("Not found a valid command");
				return false;
			}
			//skip line and comments
			skip_space_and_comments(m_context->m_line, ptr);
		}
		return true;
	}
	//////////////////////////////////////////////////////
	bool Material::parse_effect(const char*& ptr)
	{
		//skip spaces
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
	void Material::push_error(const std::string& error)
	{
		m_context->m_errors.push_front(ErrorField{ m_context->m_line,error });
	}
	//////////////////////////////////////////////////////
}
}