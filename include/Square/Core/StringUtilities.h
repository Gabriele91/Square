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
#include <tuple>
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

	SQUARE_API bool to_short(const char* arg, short& svalue);

	SQUARE_API bool to_int(const char* arg, int& ivalue);

	SQUARE_API bool to_long(const char* arg, long& lvalue);

	SQUARE_API bool to_long_long(const char* arg, long long& llvalue);

	SQUARE_API bool to_unsigned_short(const char* arg, unsigned short& usvalue);

	SQUARE_API bool to_unsigned_int(const char* arg, unsigned int& uivalue);

	SQUARE_API bool to_unsigned_long(const char* arg, unsigned long& ulvalue);

	SQUARE_API bool to_unsigned_long_long(const char* arg, unsigned long long& ullvalue);

	SQUARE_API bool to_float(const char* arg, float& fvalue);

	SQUARE_API bool to_double(const char* arg, double& dvalue);

	SQUARE_API bool to_boolean(const char* arg, bool& bvalue);

	template < typename T > 
	inline std::tuple< bool, T > to_(const char* arg){ assert(0); return std::make_tuple(false, T()); }

	template <> 
	inline std::tuple< bool, std::string > to_<std::string>(const char* arg)
	{
		std::string v{arg};
		return std::make_tuple(!!v.size(), v);
	}

	template <> 
	inline std::tuple< bool, short > to_<short>(const char* arg)
	{
		short v{0};
		return std::make_tuple(to_short(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, int > to_<int>(const char* arg)
	{
		int v{0};
		return std::make_tuple(to_int(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, long > to_<long>(const char* arg)
	{
		long v{0};
		return std::make_tuple(to_long(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, long long > to_<long long>(const char* arg)
	{
		long long v{0};
		return std::make_tuple(to_long_long(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, unsigned short > to_<unsigned short>(const char* arg)
	{
		unsigned short v{0};
		return std::make_tuple(to_unsigned_short(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, unsigned int > to_< unsigned int >(const char* arg)
	{
		unsigned int v{0};
		return std::make_tuple(to_unsigned_int(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, unsigned long > to_< unsigned long >(const char* arg)
	{
		unsigned long v{0};
		return std::make_tuple(to_unsigned_long(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, unsigned long long > to_<unsigned long long>(const char* arg)
	{
		unsigned long long v{0};
		return std::make_tuple(to_unsigned_long_long(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, float > to_<float>(const char* arg)
	{
		float v{0};
		return std::make_tuple(to_float(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, double > to_<double>(const char* arg)
	{
		double v{0};
		return std::make_tuple(to_double(arg, v), v);
	}

	template <> 
	inline std::tuple< bool, bool > to_<bool>(const char* arg)
	{
		bool v{0};
		return std::make_tuple(to_boolean(arg, v), v);
	}

}
