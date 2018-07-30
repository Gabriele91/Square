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
			std::string m_path;
			std::string m_reg_exp;
			bool		m_recursive{ false };
			bool		m_filtered{ false };
		};

		struct FileField
		{
			std::string m_path;
			std::string m_asset_name;
			bool		m_use_asset_name{ false };
		};

		struct Context
		{
			std::vector< PathField > m_paths;
			std::vector< FileField > m_files;
			std::list < ErrorField >	    m_errors;
			size_t						    m_line{ 0 };

			std::string errors_to_string() const;
		};

		bool parse(Context& default_context, const std::string& source);

		bool parse(Context& default_context, const char*& ptr);

	protected:

		//////////////////////////////////////////////////////
		Context * m_context{ nullptr };
		//////////////////////////////////////////////////////
		bool parse_resources_block(const char*& ptr);
		//////////////////////////////////////////////////////
		void push_error(const std::string& error);
	};
}
}