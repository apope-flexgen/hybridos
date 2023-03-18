#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>
#include <iostream>
#include <iterator>

#ifndef LOGGER_H
#define LOGGER_H

namespace StaticLogs{

    class Logger
    {
    private:
        //smart pointer no mem leak
        static std::shared_ptr<spdlog::logger> logger;
        static std::shared_ptr<spdlog::logger> console;
    public:
        static void Init(std::string module);
        inline static std::shared_ptr<spdlog::logger>& get_logger() {return logger;}
        inline static std::shared_ptr<spdlog::logger>& get_console() {return console;}
    };

    /**
     * @brief This function removes newlines from user messages to allow for valid json and 
     * generates a single string from a variable number or arguments. Mimics printf formatting.
     * 
     * @tparam Types Variable arguments that correspond with format in printf fashion
     * @param format printf style format <"%s etc. %d">
     * @param args const reference to types
     * @return std::string <- single string with newlines removed.
     */
    template <class ... Types>
    std::string msgString(std::string format, const Types&... args)
    {
        // removing newlines
        std::string formatWithoutNewline; 
        std::copy_if(format.begin(), format.end(), back_inserter(formatWithoutNewline), [](char c){return c!='\n' && c!='\t';});

        // sprintf a string
        std::string out(fmt::sprintf(formatWithoutNewline, args...));

        // finish json needed output
        out.push_back('"');
        out.push_back('}');
        return out;
    }
 
    std::string preString( const char * file, const char* func, const int line); 

    void log_error( std::string pre, std::string post );
    void log_info( std::string pre, std::string post );
    void log_warning( std::string pre, std::string post );
    void log_debug( std::string pre, std::string post );
    void log_test( std::string pre, std::string post );

    void console_error( std::string pre, std::string post );
    void console_info( std::string pre, std::string post );
    void console_warning( std::string pre, std::string post );
    void console_debug( std::string pre, std::string post );
    void console_test( std::string pre, std::string post );

}

// macros for logging

// if ./package_utility/build.sh -m devel
// then also log to console
#ifdef FPS_DEVELOPER_MODE
    #define FPS_INFO_LOG(...)       do{ \
        ::StaticLogs::log_info(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
        ::StaticLogs::console_info(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
    }while(0)
    #define FPS_ERROR_LOG(...)      do{ \
        ::StaticLogs::log_error(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
        ::StaticLogs::console_error(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
    }while (0)
    #define FPS_WARNING_LOG(...)    do{ \
        ::StaticLogs::log_warning(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
        ::StaticLogs::console_warning(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
    }while(0)
    #ifdef FPS_DEBUG_MODE
        #define FPS_DEBUG_LOG(...)      do{ \
            ::StaticLogs::log_debug(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
            ::StaticLogs::console_debug(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); \
        }while(0) 
    #else
        #define FPS_DEBUG_LOG(...)
    #endif
#else 
    #ifdef FPS_TEST_MODE // use fprintf for gtests. The logger is initialized inside of main don't use logger
        #define FPS_INFO_LOG(...)       do{ \
            std::string str = ::StaticLogs::msgString(__VA_ARGS__); \
            std::string pre = ::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__); \
            str.pop_back(); \
            fprintf(stderr, "\nInfo:%s%s\n", pre.c_str(), str.c_str()); \
        }while(0)
        #define FPS_ERROR_LOG(...)      do{ \
            std::string str = ::StaticLogs::msgString(__VA_ARGS__); \
            std::string pre = ::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__); \
            str.pop_back(); \
            fprintf(stderr, "\nError:%s%s\n", pre.c_str(), str.c_str()); \
        }while(0)
        #define FPS_WARNING_LOG(...)    do{ \
            std::string str = ::StaticLogs::msgString(__VA_ARGS__); \
            std::string pre = ::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__); \
            str.pop_back(); \
            fprintf(stderr, "\nWarning:%s%s\n", pre.c_str(), str.c_str()); \
        }while(0)
        #define FPS_TEST_LOG(...)       do{ \
            std::string str = ::StaticLogs::msgString(__VA_ARGS__); \
            std::string pre = ::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__); \
            str.pop_back(); \
            fprintf(stderr, "\nTest:%s%s\n", pre.c_str(), str.c_str()); \
        }while(0)
        #define FPS_DEBUG_LOG(...)      do{ \
            std::string str = ::StaticLogs::msgString(__VA_ARGS__); \
            std::string pre = ::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__); \
            str.pop_back(); \
            fprintf(stderr, "\nDebug:%s%s\n", pre.c_str(), str.c_str()); \
        }while(0)
    #else // only log to file
        #define FPS_INFO_LOG(...)       ::StaticLogs::log_info(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__))
        #define FPS_ERROR_LOG(...)      ::StaticLogs::log_error(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__))
        #define FPS_WARNING_LOG(...)    ::StaticLogs::log_warning(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__))
        #define FPS_DEBUG_LOG(...)
        #define FPS_TEST_LOG(...)
    #endif
#endif

#endif // header guard
