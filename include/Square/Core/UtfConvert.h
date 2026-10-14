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
	using u8string = Square::String;
}

namespace Square
{
	////////////////////////////////////////////////////////////////////////
	//utf
	SQUARE_API std::u8string to_utf8(const U16String &s);

	SQUARE_API std::u8string to_utf8(const U32String &s);

	SQUARE_API U16String to_utf16(const std::u8string &s);

	SQUARE_API U16String to_utf16(const U32String &s);

	SQUARE_API U32String to_utf32(const std::u8string &s);

	SQUARE_API U32String to_utf32(const U16String &s);
	////////////////////////////////////////////////////////////////////////
	//ucs
	SQUARE_API U16String to_ucs2(const std::u8string &s);

	SQUARE_API U32String to_ucs4(const std::u8string &s);

	SQUARE_API U32String to_ucs4(const U16String &s);

}
