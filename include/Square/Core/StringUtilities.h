//
//  Square
//
//  Created by Gabriele on 10/03/2018.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include <cctype>
#include <cstring>
#include <string>
#include <sstream>
#include <regex>
#include <vector>

namespace Square
{
	inline void replace_implace(std::string& source, std::string const& find, std::string const& replace)
	{
		std::string::size_type i = source.find(find, 0);
		if (i != std::string::npos) source.replace(i, find.length(), replace);
	}

	inline void replace_all_implace(std::string& source, std::string const& find, std::string const& replace)
	{
		for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
	}

	inline std::string replace(std::string str, const std::string& old_str, const std::string& new_str)
	{
		std::string::size_type pos = 0u;
		while ((pos = str.find(old_str, pos)) != std::string::npos)
		{
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
		return std::move(str);
	}

	inline std::string replace_all(std::string source, std::string const& find, std::string const& replace)
	{
		for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
		return std::move(source);
	}


	inline bool scase_insensitive_equal(const std::string& lstr, const std::string& rstr)
	{
		//not equal len
		if (lstr.size() != rstr.size()) return false;
		//test
		for (std::string::const_iterator c1 = lstr.begin(), c2 = rstr.begin(); c1 != lstr.end(); ++c1, ++c2)
		{
			if (std::tolower(*c1) != std::tolower(*c2)) return false;
		}
		//..
		return true;
	}

	inline std::vector<std::string> split(const std::string& input, const  std::string& regex_str)
	{
		std::regex regex(regex_str);
		std::sregex_token_iterator first{ input.begin(), input.end(), regex, -1 };
		std::sregex_token_iterator last;
		return { first, last };
	}
}
