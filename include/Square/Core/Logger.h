#pragma once
#include "Square/Config.h"
#include <string>
#include <vector>

namespace Square
{
	class SQUARE_API Logger
	{
	public:
		Logger() = default;
		virtual void error(const std::string& value) const = 0;
		virtual void warning(const std::string& value) const = 0;
		virtual void debug(const std::string& value) const = 0;
		virtual void info (const std::string& value) const = 0;

		virtual void errors(const std::vector< std::string >& values) const
		{
			for (const auto& value : values) error(value);
		}
		virtual void warnings(const std::vector< std::string >& values) const
		{
			for (const auto& value : values) warning(value);
		}
		virtual void debugs(const std::vector< std::string >& values) const
		{
			for (const auto& value : values) debug(value);
		}
		virtual void infos(const std::vector< std::string >& values) const
		{
			for (const auto& value : values) info(value);
		}
		
		virtual void verbose(bool verbose) = 0;
		virtual bool verbose() const = 0;
	};

	enum class LoggerType
	{
		LOGGER_STD_DEBUG,
		LOGGER_STD_RELEASE,
		LOGGER_STD_DEFAULT,
		LOGGER_OS_DEBUG,
		LOGGER_OS_RELEASE,
		LOGGER_OS_DEFAULT,
		LOGGER_NONE
	};
	
	SQUARE_API Logger* get_logger_intance(LoggerType type);
}