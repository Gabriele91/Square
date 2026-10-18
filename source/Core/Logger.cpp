#include <ctime>
#include <iomanip>
#include <sstream>
#include "Square/Core/Logger.h"

#if defined(__ANDROID__)
    #include <android/log.h>
#endif
#if defined(_WIN32)
    #include <Windows.h>   
    #include <debugapi.h>
    #include <thread>
    #include <mutex>
    #include <queue>
    #include <condition_variable>
    #include <atomic>
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
        NONELogger() = default;

        void error(const String& value) const override { }
        void warning(const String& value) const override { }
        void debug(const String& value) const override { }
        void info(const String& value) const override { }

        void errors(const Vector< String >& values) const override { }
        void warnings(const Vector< String >& values) const override { }
        void debugs(const Vector< String >& values) const override { }
        void infos(const Vector< String >& values) const override { }

        virtual void verbose(bool verbose) override { }
        virtual bool verbose() const override { return false; }
    
    protected:
    };

    template < bool enable_debug >
    class STDLogger : public Logger
    {
    public:
        void error(const String& value) const override
        {
            print(stderr, TAG_ERROR, value);
        }

        void warning(const String& value) const override
        {
            print(stderr, TAG_WARNING, value);
        }

        void debug(const String& value) const override
        {
            if (enable_debug && m_verbose)
            {
                print(stdout, TAG_DEBUG, value);
            }
        }

        void info(const String& value) const override
        {
            if (m_verbose)
            {
                print(stdout, TAG_INFO, value);
            }
        }

        virtual void debugs(const Vector< String >& values) const override
        {
            if (enable_debug && m_verbose)
            {
                for (const auto& value : values) debug(value);
            }
        }

        virtual void infos(const Vector< String >& values) const override
        {
            if (m_verbose)
            {
                for (const auto& value : values) info(value);
            }
        }

        virtual void verbose(bool verbose) override 
        { 
            m_verbose = verbose;
        }

        virtual bool verbose() const override 
        { 
            return m_verbose; 
        }

    protected:
#if defined(SQUARE_RETAIL)
        bool m_verbose{ false };
#else
        bool m_verbose{ true };
#endif

    private:
        void print(FILE* const stream, const char* tag, const String& value) const
        {
            StringStream output;
            std::time_t  current_type = std::time(nullptr);
            std::tm      local_time = *std::localtime(&current_type);
            output << std::put_time(&local_time, TIMESTAMP_FORMAT);
            output << " [" << tag << "] ";
            output << value;
            output << std::endl;
        #if defined(__ANDROID__)
            //Android: no console, the log of the system (logcat, tag Square)
            const int priority = stream == stderr ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO;
            __android_log_write(priority, "Square", output.str().c_str());
        #else
            fprintf(stream, "%s", output.str().c_str());
        #endif
        }
    };

#if defined(_WIN32)
    template < bool enable_debug >
    class WIN32Logger : public Logger
    {
    public:

        WIN32Logger() 
        {
            run_thread();
        }

        ~WIN32Logger() 
        {
            stop_thread();
        }

        void error(const String& value) const override
        {
            print(TAG_ERROR, value);
        }

        void warning(const String& value) const override
        {
            print(TAG_WARNING, value);
        }

        void debug(const String& value) const override
        {
            if (enable_debug && m_verbose)
            {
                print(TAG_DEBUG, value);
            }
        }

        void info(const String& value) const override
        {
            if (m_verbose)
            {
                print(TAG_INFO, value);
            }
        }

        virtual void debugs(const Vector< String >& values) const override
        {
            if (enable_debug && m_verbose)
            {
                for (const auto& value : values) debug(value);
            }
        }

        virtual void infos(const Vector< String >& values) const override
        {
            if (m_verbose)
            {
                for (const auto& value : values) info(value);
            }
        }

        virtual void verbose(bool verbose) override
        {
            m_verbose = verbose;
        }

        virtual bool verbose() const override
        {
            return m_verbose;
        }

    protected:
#if defined(SQUARE_RETAIL)
        bool m_verbose{ false };
#else
        bool m_verbose{ true };
#endif

    private:
        // Multi thread
        mutable Square::Queue<String> m_queue;
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_condition;
        std::atomic<bool> m_running{ false };
        std::thread m_thread;

        void run_thread()
        {
            m_running = true;
            m_thread = std::thread(&WIN32Logger<enable_debug>::run, this);
        }

        void enqueue(const String& value) const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(value);
            m_condition.notify_one();
        }

        void run()
        {
            while (true)
            {
                String msg;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condition.wait(lock, [this]() { return !m_queue.empty() || !m_running; });
                    if (m_queue.empty() && !m_running)   { break; }
                    msg = m_queue.front();
                    m_queue.pop();
                }
                OutputDebugStringA(msg.c_str());
            }
        }

        void stop_thread()
        {
            // Stop running
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_running = false;
                m_condition.notify_one();
            }
            // Wait
            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }

        void print(const char* tag, const String& value) const
        {
            StringStream output;
            std::time_t  current_type = std::time(nullptr);
            std::tm      local_time = *std::localtime(&current_type);
            output << std::put_time(&local_time, TIMESTAMP_FORMAT);
            output << " [" << tag << "] ";
            output << value;
            output << std::endl;
            enqueue(output.str());
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