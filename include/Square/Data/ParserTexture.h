#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Resource/Texture.h"
#include "Square/Data/ParserUtils.h"
#include <variant>

namespace Square
{
namespace Parser
{
	class SQUARE_API Texture
	{
	public:

		struct Context
		{
			Resource::Texture::Attributes                         m_attributes;
			std::variant<std::string, std::vector<unsigned char>> m_image;
			std::list< ErrorField >	                              m_errors;
			size_t						                          m_line{ 0 };

			std::string errors_to_string() const;
		};

		static bool parse(Context& default_context, const std::string& source);

		static bool parse(Context& default_context, const char* ptr, size_t size);

	};
}
}