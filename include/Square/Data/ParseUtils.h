#pragma once
#include <string>
#include "Square/Config.h"
#include "Square/Core/Variant.h"

namespace Square
{
namespace Parser 
{
	//////////////////////////////////////////////////////
	// KEYWORD UTILS
	static inline bool cstr_cmp(const char* x, const char key[])
	{
		return std::strncmp(x, key, std::strlen(key)) == 0;
	}

	static inline bool cstr_cmp_skip(const char*& x, const char key[])
	{
		if (cstr_cmp(x, key))
		{ 
			x += std::strlen(key);
			return true; 
		} 
		return false;
	}

	//////////////////////////////////////////////////////
	// WHO IS UTILS
	static bool is_line_space(char c)
	{
		return 	 c == ' ' || c == '\t';
	}

	static bool is_space(char c)
	{
		return 	 c == ' ' || c == '\t' || c == '\r' || c == '\n';
	}

	static bool is_start_name(char c)
	{
		return 	 (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	}

	static bool is_char_name(char c)
	{
		return 	 (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
	}

	static bool is_digit(char c)
	{
		return 	 (c >= '0' && c <= '9');
	}

	static bool is_xdigit(char c)
	{
		return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
	}

	static bool is_int_digit(char c)
	{
		return 	 (c >= '0' && c <= '9') || c == '-';
	}

	static bool is_float_digit(char c)
	{
		return 	 (c >= '0' && c <= '9') || c == '-' || c == '.';
	}

	static bool is_start_string(char c)
	{
		return c == '\"';
	}

	static bool is_start_table(char c)
	{
		return (c == '{');
	}

	static bool is_end_table(char c)
	{
		return (c == '}');
	}

	static bool is_start_arg(char c)
	{
		return (c == '(');
	}

	static bool is_comm_arg(char c)
	{
		return (c == ',');
	}

	static bool is_end_arg(char c)
	{
		return (c == ')');
	}

	static bool is_start_obj_list(char c)
	{
		return (c == '|');
	}

	static bool is_line_comment(const char* c)
	{
		return (*c) == '/' && (*(c + 1)) == '/';
	}

	static bool is_start_multy_line_comment(const char* c)
	{
		return (*c) == '/' && (*(c + 1)) == '*';
	}

	static bool is_end_multy_line_comment(const char* c)
	{
		return (*c) == '*' && (*(c + 1)) == '/';
	}
	//////////////////////////////////////////////////////
	// COMMENTS UTILS
	static bool skip_line_comment(size_t& line, const char*& inout)
	{
		//not a line comment
		if ((*inout) != '/' || *(inout + 1) != '/') return false;
		//skeep
		while (*(inout) != EOF && *(inout) != '\0'&& *(inout) != '\n') ++(inout);
		//jump endline
		if ((*(inout)) == '\n')
		{
			++line;  ++inout;
		}
		//ok
		return true;
	}

	static bool skip_multy_lines_comment(size_t& line,const char*& inout)
	{
		//not a multilines comment
		if (is_start_multy_line_comment(inout)) return false;
		//jump
		while
		(
			*(inout) != EOF &&
			*(inout) != '\0' &&
			!is_end_multy_line_comment(inout)
		)
		{
			line += (*(inout)) == '\n';
			++inout;
		}
		//jmp last one
		if (is_end_multy_line_comment(inout)) inout += 2;
		//ok
		return true;
	}
	
	static bool skip_line_space(size_t& line, const char*& source)
	{
		bool a_space_is_skipped = false;
		while (is_line_space(*source))
		{
			//to true
			a_space_is_skipped = true;
			//jump
			++source;
			//exit
			if (!*source) break;
		}
		return a_space_is_skipped;
	}

	static bool skip_space(size_t& line, const char*& source)
	{
		bool a_space_is_skipped = false;
		while (is_space(*source))
		{
			//to true
			a_space_is_skipped = true;
			//count line
			if (*source == '\n') ++line;
			//jump
			++source;
			//exit
			if (!*source) break;
		}
		return a_space_is_skipped;
	}

	static void skip_space_and_comments(size_t& line, const char*& source)
	{
		while (skip_space(line, source)
			|| skip_line_comment(line, source)
			|| skip_multy_lines_comment(line, source));
	}
	//////////////////////////////////////////////////////
	// TYPE UTILS
	static bool parse_bool(const char*& inout, bool& out)
	{
		if (cstr_cmp_skip(inout, "true")) { out = true;  return true; }
		if (cstr_cmp_skip(inout, "false")){ out = false; return true; };
		return false;
	}

	static bool parse_short(const char*& inout, short& out)
	{
		const char*  strin  = inout;
		const char** strout = &inout;
		out = (short)std::strtol(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_int(const char*& inout, int& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = (int)std::strtol(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_long(const char*& inout, long& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtol(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_long_long(const char*& inout, long long& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtoll(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_unsigned_short(const char*& inout, unsigned short& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = (unsigned short)std::strtoul(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_unsigned_int(const char*& inout, unsigned int& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = (unsigned int)std::strtoul(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_unsigned_long(const char*& inout, unsigned long& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtoul(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_unsigned_long_long(const char*& inout, unsigned long long& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtoull(strin, (char**)strout, 10);
		return strin != (*strout);
	}

	static bool parse_float(const char*& inout, float& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtof(strin, (char**)strout);
		return strin != (*strout);
	}

	static bool parse_double(const char*& inout, double& out)
	{
		const char*  strin = inout;
		const char** strout = &inout;
		out = std::strtod(strin, (char**)strout);
		return strin != (*strout);
	}
	
	static bool parse_double2(const char*& inout, double& out)
	{ 
		//+/-
		char ch = *inout;
		if (ch == '-') ++inout;
		//integer part
		double result = 0;
		while (is_digit(*inout)) result = (result * 10) + (*inout++ - '0');
		//fraction
		if (*inout == '.')
		{
			++inout;
			double fraction = 1;
			while (is_digit(*inout))
			{
				fraction *= 0.1;
				result += (*inout++ - '0') * fraction;
			}
		}
		//exponent
		if (*inout == 'e' || *inout == 'E')
		{
			++inout;
			//base of exp
			double base = 10;
			//+/- exp
			if (*inout == '+')
			{
				++inout;
			}
			else if (*inout == '-')
			{
				++inout;
				base = 0.1;
			}
			//parsing exponent
			unsigned int exponent = 0;
			while (is_digit(*inout)) exponent = (exponent * 10) + (*inout++ - '0');
			//compute exponent
			double power = 1;
			for (; exponent; exponent >>= 1, base *= base) if (exponent & 1) power *= base;
			//save result
			result *= power;
		}
		//result
		out = (ch == '-' ? -result : result);
		//ok
		return true;
	}

	static bool parse_char_as_int(const char*& inout, int& out)
	{
		return out = (((*inout) <= '9') ? (*inout) - '0' : ((*inout) & ~' ') - 'A' + 10), true;
	}

	static bool parse_name(const char*& inout, std::string& out)
	{
		const char*  strin = inout;
		//test first
		if (!is_start_name(*strin)) return false;
		//first char
		out = *strin;
		++strin;
		//next
		while (is_char_name(*strin))
		{
			out += *strin;
			++strin;
		}
		//save next point
		inout = strin;
		//ok?
		return true;
	}

	static bool parse_string(size_t& line,  const char*& inout, std::string& out, const char open_string = '\"', const char close_string = '\"')
	{
		//temp ptr
		const char *tmp = inout;
		out = "";
		//start parse
		if ((*tmp) == open_string)  //["...."]
		{
			++tmp;  //[...."]
			while ((*tmp) != close_string && (*tmp) != '\n')
			{
				if ((*tmp) == '\\') //[\.]
				{
					++tmp;  //[.]
					switch (*tmp)
					{
					case 'n':
						out += '\n';
						break;
					case 't':
						out += '\t';
						break;
					case 'b':
						out += '\b';
						break;
					case 'r':
						out += '\r';
						break;
					case 'f':
						out += '\f';
						break;
					case 'a':
						out += '\a';
						break;
					case '\\':
						out += '\\';
						break;
					case '?':
						out += '\?';
						break;
					case '\'':
						out += '\'';
						break;
					case '\"':
						out += '\"';
						break;
					case '\n': /* jump unix */
						++line;
						break;
					case '\r': /* jump mac */
						++line;
						if ((*(tmp + 1)) == '\n') ++tmp; /* jump window (\r\n)*/
						break;
					case 'u':
					{
						int c = 0;
						//comput u
						for (int i = 0; i < 4; ++i)
						{
							if (is_xdigit(*++tmp))
							{
								int value = 0;
								parse_char_as_int(tmp, value);
								c = c * 16 + value;
							}
							else
							{
								return false;
							}
						}
						//ascii
						if (c < 0x80)
						{
							out += c;
						}
						//utf 2 byte
						else if (c < 0x800)
						{
							out += 0xC0 | (c >> 6);
							out += 0x80 | (c & 0x3F);
						}
						//utf 3 byte
						else
						{
							out += 0xE0 | (c >> 12);
							out += 0x80 | ((c >> 6) & 0x3F);
							out += 0x80 | (c & 0x3F);
						}
					}
					break;
					default:
						return true;
						break;
					}
				}
				else
				{
					if ((*tmp) != '\0') out += (*tmp);
					else return false;
				}
				++tmp;//next char
			}
			if ((*tmp) == '\n') return false;
			//ptr to next
			inout = tmp + 1;
			//ok
			return true;
		}
		return false;
	}
}
}
