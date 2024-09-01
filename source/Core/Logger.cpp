#include <ctime>
#include <iomanip>
#include <sstream>
#include "Square/Core/Logger.h"

#if defined(_WIN32)
    #include <Windows.h>   
    #include <debugapi.h>
#endif

#define TAG_ERROR "ERR"
#define TAG_WARNING "WRG"
#define TAG_DEBUG "DBG"
#define TAG_INFO "INF"
#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"

namespace Square
{
    class NONELogger : public Logger
    {
    public:

        void error(const std::string& value) const override { }
        void warning(const std::string& value) const override { }
        void debug(const std::string& value) const override { }
        void info(const std::string& value) const override { }

        void errors(const std::vector< std::string >& values) const override { }
        void warnings(const std::vector< std::string >& values) const override { }
        void debugs(const std::vector< std::string >& values) const override { }
        void infos(const std::vector< std::string >& values) const override { }

    private:
    };

    template < bool enable_debug >
    class STDLogger : public Logger
    {
    public:
        void error(const std::string& value) const override
        {
            print(stderr, TAG_ERROR, value);
        }

        void warning(const std::string& value) const override
        {
            print(stderr, TAG_WARNING, value);
        }

        void debug(const std::string& value) const override
        {
            if (enable_debug)
            {
                print(stdout, TAG_DEBUG, value);
            }
        }

        void info(const std::string& value) const override
        {
            print(stdout, TAG_INFO, value);
        }

        virtual void debugs(const std::vector< std::string >& values) const override
        {
            if (enable_debug)
            {
                for (const auto& value : values) debug(value);
            }
        }

    private:
        void print(FILE* const stream, const char* tag, const std::string& value) const
        {
            std::stringstream output;
            std::time_t  current_type = std::time(nullptr);
            std::tm      local_time = *std::localtime(&current_type);
            output << std::put_time(&local_time, TIMESTAMP_FORMAT);
            output << " [" << tag << "] ";
            output << value;
            output << std::endl;
            fprintf(stream, output.str().c_str());
        }
    };

#if defined(_WIN32)
    template < bool enable_debug >
    class WIN32Logger : public Logger
    {
    public:
        void error(const std::string& value) const override
        {
            print(TAG_ERROR, value);
        }

        void warning(const std::string& value) const override
        {
            print(TAG_WARNING, value);
        }

        void debug(const std::string& value) const override
        {
            if (enable_debug)
            {
                print(TAG_DEBUG, value);
            }
        }

        void info(const std::string& value) const override
        {
            print(TAG_INFO, value);
        }

        virtual void debugs(const std::vector< std::string >& values) const override
        {
            if (enable_debug)
            {
                for (const auto& value : values) debug(value);
            }
        }

    private:
        void print(const char* tag, const std::string& value) const
        {
            std::stringstream output;
            std::time_t  current_type = std::time(nullptr);
            std::tm      local_time = *std::localtime(&current_type);
            output << std::put_time(&local_time, TIMESTAMP_FORMAT);
            output << " [" << tag << "] ";
            output << value;
            output << std::endl;
            OutputDebugStringA(output.str().c_str());
        }
    };
#endif

	Logger* get_logger_intance(LoggerType type)
	{
        switch (type)
        {
        case Square::LoggerType::LOGGER_STD_DEBUG:
        {
            static STDLogger<true> s_std_logger_debug;
            return &s_std_logger_debug;
        }
        break;
        case Square::LoggerType::LOGGER_STD_RELEASE:
        {
            static STDLogger<false> s_std_logger_release;
            return &s_std_logger_release;
        }
        break;
        case Square::LoggerType::LOGGER_STD_DEFAULT:
        {
            #if defined(_DEBUG)
            return get_logger_intance(Square::LoggerType::LOGGER_STD_DEBUG);
            #else 
            return get_logger_intance(Square::LoggerType::LOGGER_STD_RELEASE);
            #endif
        }
        break;
        #if defined(_WIN32)
        case Square::LoggerType::LOGGER_OS_DEBUG:
        {
            static WIN32Logger<true> s_win_logger_debug;
            return &s_win_logger_debug;
        }
        break;
        case Square::LoggerType::LOGGER_OS_RELEASE:
        {
            static WIN32Logger<false> s_win_logger_release;
            return &s_win_logger_release;
        }
        break;
        case Square::LoggerType::LOGGER_OS_DEFAULT:
        {
            #if defined(_DEBUG)
            return get_logger_intance(Square::LoggerType::LOGGER_OS_DEBUG);
            #else 
            return get_logger_intance(Square::LoggerType::LOGGER_OS_RELEASE);
            #endif
        }
        break;
        #else
        case Square::LoggerType::LOGGER_OS_DEBUG:  return get_logger_intance(Square::LoggerType::LOGGER_STD_DEBUG);
        case Square::LoggerType::LOGGER_OS_RELEASE:return get_logger_intance(Square::LoggerType::LOGGER_STD_RELEASE);
        case Square::LoggerType::LOGGER_OS_DEFAULT:return get_logger_intance(Square::LoggerType::LOGGER_STD_DEFAULT);
        #endif
        case Square::LoggerType::LOGGER_NONE:
        {
            static NONELogger s_none_logger;
            return &s_none_logger;
        }
        break;
        default:
            return nullptr;
        }
	}
}