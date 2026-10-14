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
			std::variant<String, Vector<unsigned char>> m_image;
			String                                           m_fallback; //an image for a GPU without the format of m_image (DDS/KTX)
			List< ErrorField >	                              m_errors;
			size_t						                          m_line{ 0 };

			String errors_to_string() const;
		};

		static bool parse(Context& default_context, const String& source);

		static bool parse(Context& default_context, const char* ptr, size_t size);

	};
}
}