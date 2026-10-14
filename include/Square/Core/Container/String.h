//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include <string>
#include <string_view>
#include <sstream>
#include <functional>
#include "Square/Core/Allocator/STLHeapAllocator.h"

namespace Square
{
	//The strings of the engine: the ones of the STL on the heap of the engine (STLHeapAllocator).
	//From/to a std::string (a library out of the engine): explicit, String(std_string) and
	//std::string(string); assigned, appended, compared by their std::string_view
	template < class C, class Traits = std::char_traits<C> >
	using BasicString = std::basic_string<C, Traits, STLHeapAllocator<C>>;
	using String    = BasicString<char>;
	using WString   = BasicString<wchar_t>;
	using U16String = BasicString<char16_t>;
	using U32String = BasicString<char32_t>;
	using StringView = std::string_view;

	//its streams
	template < class C, class Traits = std::char_traits<C> >
	using BasicStringStream = std::basic_stringstream<C, Traits, STLHeapAllocator<C>>;
	template < class C, class Traits = std::char_traits<C> >
	using BasicOStringStream = std::basic_ostringstream<C, Traits, STLHeapAllocator<C>>;
	template < class C, class Traits = std::char_traits<C> >
	using BasicIStringStream = std::basic_istringstream<C, Traits, STLHeapAllocator<C>>;
	using StringStream  = BasicStringStream<char>;
	using OStringStream = BasicOStringStream<char>;
	using IStringStream = BasicIStringStream<char>;

	//the conversions of the STL (std::to_string, std::stoi...) to/from a String
	SQUARE_API String to_string(int value);
	SQUARE_API String to_string(long value);
	SQUARE_API String to_string(long long value);
	SQUARE_API String to_string(unsigned value);
	SQUARE_API String to_string(unsigned long value);
	SQUARE_API String to_string(unsigned long long value);
	SQUARE_API String to_string(float value);
	SQUARE_API String to_string(double value);
	SQUARE_API String to_string(long double value);

	SQUARE_API int                stoi(const String& str, size_t* pos = nullptr, int base = 10);
	SQUARE_API long               stol(const String& str, size_t* pos = nullptr, int base = 10);
	SQUARE_API long long          stoll(const String& str, size_t* pos = nullptr, int base = 10);
	SQUARE_API unsigned long      stoul(const String& str, size_t* pos = nullptr, int base = 10);
	SQUARE_API unsigned long long stoull(const String& str, size_t* pos = nullptr, int base = 10);
	SQUARE_API float              stof(const String& str, size_t* pos = nullptr);
	SQUARE_API double             stod(const String& str, size_t* pos = nullptr);
	SQUARE_API long double        stold(const String& str, size_t* pos = nullptr);
}

//the hash of the strings of the engine: the one of their characters (as a std::string); the STL of
//MSVC, libc++ and libstdc++ 12+ have it for any allocator
#if defined(__GLIBCXX__) && (!defined(_GLIBCXX_RELEASE) || _GLIBCXX_RELEASE < 12)
namespace std
{
	template < class C, class Traits >
	struct hash< std::basic_string<C, Traits, Square::STLHeapAllocator<C>> >
	{
		size_t operator()(const std::basic_string<C, Traits, Square::STLHeapAllocator<C>>& str) const noexcept
		{
			return std::hash< std::basic_string_view<C, Traits> >{}(str);
		}
	};
}
#endif
