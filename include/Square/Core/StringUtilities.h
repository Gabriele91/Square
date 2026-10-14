//
//  Square
//
//  Created by Gabriele on 10/03/2018.
//  Copyright � 2016 Gabriele. All rights reserved.
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
	inline void replace_implace(String& source, String const& find, String const& replace)
	{
		String::size_type i = source.find(find, 0);
		if (i != String::npos) source.replace(i, find.length(), replace);
	}

	inline void replace_all_implace(String& source, String const& find, String const& replace)
	{
		for (String::size_type i = 0; (i = source.find(find, i)) != String::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
	}

	inline String replace(String str, const String& old_str, const String& new_str)
	{
		String::size_type pos = 0u;
		while ((pos = str.find(old_str, pos)) != String::npos)
		{
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
		return std::move(str);
	}

	inline String replace_all(String source, String const& find, String const& replace)
	{
		for (String::size_type i = 0; (i = source.find(find, i)) != String::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
		return std::move(source);
	}

	inline bool case_insensitive_equal(const String& lstr, const String& rstr)
	{
		//not equal len
		if (lstr.size() != rstr.size()) return false;
		//test
		for (String::const_iterator c1 = lstr.begin(), c2 = rstr.begin(); c1 != lstr.end(); ++c1, ++c2)
		{
			if (std::tolower(*c1) != std::tolower(*c2)) return false;
		}
		//..
		return true;
	}

	inline Vector<String> split(const String& input, const  String& regex_str)
	{
		std::regex regex(regex_str.c_str());
		std::regex_token_iterator<String::const_iterator> first{ input.begin(), input.end(), regex, -1 };
		std::regex_token_iterator<String::const_iterator> last;
		Vector<String> out;
		for (; first != last; ++first) out.emplace_back(first->first, first->second);
		return out;
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
	inline std::tuple< bool, String > to_<String>(const char* arg)
	{
		String v{arg};
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

namespace std
{
	inline Square::String operator * (const Square::String& value, size_t n)
	{
		Square::String out;
		out.reserve(value.size() * n);
		for(size_t i = 0;i < n; ++i)
		{
			out += value;
		}
		return out;
	}
	
    inline Square::String& operator *= (Square::String& inout_string, size_t n)
	{
	    
		Square::String tmp(inout_string);
		inout_string.reserve(tmp.size() * n);
		for(size_t i = 0;i < n; ++i)
		{
			inout_string += tmp;
		}
		return inout_string;
	}
}