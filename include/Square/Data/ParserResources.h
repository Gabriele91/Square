#pragma once
#include "Square/Config.h"
#include "Square/Data/ParserUtils.h"

namespace Square
{
namespace Parser
{
	class SQUARE_API Resources
	{
	public:
		struct PathField
		{
			String m_path;
			String m_reg_exp;
			bool		m_recursive{ false };
			bool		m_filtered{ false };
		};

		struct FileField
		{
			String m_path;
			String m_asset_name;
			bool		m_use_asset_name{ false };
		};

		struct Context
		{
			Vector< PathField > m_paths;
			Vector< FileField > m_files;
			List < ErrorField >	    m_errors;
			size_t						    m_line{ 0 };

			String errors_to_string() const;
		};

		bool parse(Context& default_context, const String& source);

		bool parse(Context& default_context, const char*& ptr);

	protected:

		//////////////////////////////////////////////////////
		Context * m_context{ nullptr };
		//////////////////////////////////////////////////////
		bool parse_resources_block(const char*& ptr);
		//////////////////////////////////////////////////////
		void push_error(const String& error);
	};
}
}