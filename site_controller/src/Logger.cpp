#include <Logger.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

// Using a namespace to make thinks easy
namespace StaticLogs{
    // smart pointers so no need to free memory
    std::shared_ptr<spdlog::logger> Logger::logger;
    std::shared_ptr<spdlog::logger> Logger::console;

    // init function creates both a console logger and a file logger
    void Logger::Init(std::string module){
        // make file path based of string passed to init
        std::string dir("/var/log/flexgen/");
        dir.append(module);
        dir.append("/site.log");

        // This is the file logger
        logger = spdlog::basic_logger_mt("site_controller", dir);
        logger->set_level(spdlog::level::trace); // log everything
        logger->flush_on(spdlog::level::trace); // flush after every log
        logger->set_pattern("{\"time\": \"%H:%M:%S\", \"date\": \"%D\", \"level\": \"%l\", %v"); // This pattern is NEEDED site_controller is pushing json.

        // This logs to console
        console = spdlog::stdout_color_mt("console");
        console->set_level(spdlog::level::trace); // log everything
        console->flush_on(spdlog::level::trace); // flush after every log
        console->set_pattern("\n[%H:%M:%S] [SITE_CONTROLLER] |%l| %v\n"); // Mimic the go_flexgen style pretty print to logger. 
    }

    /**
     * @brief Adding file func line to log in json simulated format. 
     * 
     * @param file will be file macro 
     * @param func will be func macro
     * @param line will be line macro
     * @return std::string <- json acceptable build of the fields above. 
     */
    std::string preString( const char * file, const char* func, const int line){ 
        //msg
        char msgBuf[500]; // messages must be in this size. Is this too small???

        //trim file to just the file not the whole path
        std::string trim(file);
        size_t last = trim.find_last_of('/');
        if(last != std::string::npos){
            trim = trim.substr(last+1);
        }

        // wacky newlines and tabs needed to mimic pretty print of go_flexgen/logger
        snprintf (msgBuf, 500, "\n\t\"File\": \"%s\", \"Func\": \"%s\", \"Line\": %d, \n\t\"MSG\": \"", trim.c_str(), func, line);

        return std::string(msgBuf);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * log_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void log_error ( std::string pre, std::string post ){
        Logger::get_logger()->log(spdlog::level::err, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * log_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void log_info ( std::string pre, std::string post ){
        Logger::get_logger()->log(spdlog::level::info, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * log_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void log_warning ( std::string pre, std::string post ){
        Logger::get_logger()->log(spdlog::level::warn, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * log_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void log_debug ( std::string pre, std::string post ){
        Logger::get_logger()->log(spdlog::level::debug, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * log_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void log_test ( std::string pre, std::string post ){
        Logger::get_logger()->log(spdlog::level::warn, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * console_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void console_error ( std::string pre, std::string post ){
        Logger::get_console()->log(spdlog::level::err, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * console_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void console_info ( std::string pre, std::string post ){
        Logger::get_console()->log(spdlog::level::info, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * console_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void console_warning ( std::string pre, std::string post ){
        Logger::get_console()->log(spdlog::level::warn, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * console_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void console_debug ( std::string pre, std::string post ){
        Logger::get_console()->log(spdlog::level::debug, pre+post);
    }

    /**
     * @brief Joins two strings and logs them. 
     * Intended that these strings will be function calls to 
     * console_xxx(::StaticLogs::preString(__FILE__, __FUNCTION__, __LINE__), ::StaticLogs::msgString(__VA_ARGS__)); 
     * Intended to be only used in logger macros
     * 
     * @param pre file func line 
     * @param post user input text to logging macro
     */
    void console_test ( std::string pre, std::string post ){
        Logger::get_console()->log(spdlog::level::warn, pre+post);
    }
}