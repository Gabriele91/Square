//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace std
{
	using u8string = std::string;
}

namespace Square
{
	////////////////////////////////////////////////////////////////////////
	//utf
	SQUARE_API std::u8string to_utf8(const std::u16string &s);

	SQUARE_API std::u8string to_utf8(const std::u32string &s);

	SQUARE_API std::u16string to_utf16(const std::u8string &s);

	SQUARE_API std::u16string to_utf16(const std::u32string &s);

	SQUARE_API std::u32string to_utf32(const std::u8string &s);

	SQUARE_API std::u32string to_utf32(const std::u16string &s);
	////////////////////////////////////////////////////////////////////////
	//ucs
	SQUARE_API std::u16string to_ucs2(const std::u8string &s);

	SQUARE_API std::u32string to_ucs4(const std::u8string &s);

	SQUARE_API std::u32string to_ucs4(const std::u16string &s);

}
