#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Data/ParserParameters.h"

namespace Square
{
namespace Parser
{
	class SQUARE_API Material
	{
		using  ParameterField = Parser::Parameters::ParameterField;

	public:

		struct Context
		{
			String						m_effect;
			Vector< ParameterField >   m_parameters;
			List< ErrorField >	        m_errors;
			size_t						    m_line{ 0 };

			String errors_to_string() const;
		};

		bool parse(Allocator* allocator, Context& default_context, const String& source);

		bool parse(Allocator* allocator, Context& default_context, const char*& ptr);
		
	protected:
		//////////////////////////////////////////////////////
		Context * m_context{ nullptr };
		Allocator* m_allocator{ nullptr };
		//////////////////////////////////////////////////////
		bool parse_effect(const char*& ptr);
		//////////////////////////////////////////////////////
		bool parse_parameters_block(const char*& ptr);
		//////////////////////////////////////////////////////
		void push_error(const String& error);
		//////////////////////////////////////////////////////
	};
}
}