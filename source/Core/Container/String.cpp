//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <cstdio>
#include <charconv>
#include "Square/Core/Container/String.h"

namespace Square
{
namespace AuxString
{
	//an integer: its digits (as std::to_string)
	template < class T >
	static String integer_to_string(T value)
	{
		char buffer[24];
		const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
		return String(buffer, result.ptr);
	}

	//a real: "%f" (as std::to_string)
	template < class T >
	static String real_to_string(const char* format, T value)
	{
		const int size = std::snprintf(nullptr, 0, format, value);
		String out(size_t(size > 0 ? size : 0), '\0');
		if (size > 0) std::snprintf(out.data(), out.size() + 1, format, value);
		return out;
	}
}

	String to_string(int value)                { return AuxString::integer_to_string(value); }
	String to_string(long value)               { return AuxString::integer_to_string(value); }
	String to_string(long long value)          { return AuxString::integer_to_string(value); }
	String to_string(unsigned value)           { return AuxString::integer_to_string(value); }
	String to_string(unsigned long value)      { return AuxString::integer_to_string(value); }
	String to_string(unsigned long long value) { return AuxString::integer_to_string(value); }
	String to_string(float value)              { return AuxString::real_to_string("%f", double(value)); }
	String to_string(double value)             { return AuxString::real_to_string("%f", value); }
	String to_string(long double value)        { return AuxString::real_to_string("%Lf", value); }

	//the ones of the STL (their errors, their exceptions) on a copy of its characters
	int                stoi(const String& str, size_t* pos, int base)   { return std::stoi(std::string(str), pos, base); }
	long               stol(const String& str, size_t* pos, int base)   { return std::stol(std::string(str), pos, base); }
	long long          stoll(const String& str, size_t* pos, int base)  { return std::stoll(std::string(str), pos, base); }
	unsigned long      stoul(const String& str, size_t* pos, int base)  { return std::stoul(std::string(str), pos, base); }
	unsigned long long stoull(const String& str, size_t* pos, int base) { return std::stoull(std::string(str), pos, base); }
	float              stof(const String& str, size_t* pos)             { return std::stof(std::string(str), pos); }
	double             stod(const String& str, size_t* pos)             { return std::stod(std::string(str), pos); }
	long double        stold(const String& str, size_t* pos)            { return std::stold(std::string(str), pos); }
}
