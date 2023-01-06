#ifndef ESSLOGGER_HPP
#define ESSLOGGER_HPP

#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "EventLogger.hpp"
#include "chrono_utils.hpp"


class elapsed_time_formatter : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest) override
    {
        fmt::format_to(std::back_inserter(dest), "{:<{}}", flex::get_time_dbl(), padinfo_.width_);
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<elapsed_time_formatter>();
    }
};

// an eventLogger Singleton for the system, the default EventLogger
class ESSLogger final
{
    EventLogger mLogger{"ESS Logger", 64}; // stores 64 messags by default

    // sets the default format for our logger and have get_time_dbl() implicitly there
    // now we can remove the extra get_time_dbl calls for our log calls
    ESSLogger()
    {
        auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%Y-%m-%d %T.%e] [%-8*] [%n] [%l] %v");
        mLogger.setFormatter(std::move(formatter));
    }

public:
    // NO copying or moving allowed
    ESSLogger(const ESSLogger&) = delete;
    ESSLogger& operator=(const ESSLogger&) = delete;
    ESSLogger(ESSLogger&&) = delete;
    ESSLogger& operator=(ESSLogger&&) = delete;

    static ESSLogger& get()
    {
        static ESSLogger instance;
        return instance;
    }

    // NOTE: it is your responsibility to set the size > 0.
    void setBacktrace(std::size_t size)
    {
        mLogger.setBacktrace(size);
    }

    // NOTE: it is your responsibility to set the size > 0.
    // flushes the previous logs and starts with a fresh buffer of size
    void resetBacktrace(std::size_t size)
    {
        mLogger.resetBacktrace(size);
    }

    // todo: check efficiency? Don't know if all the moves are necessary, but should be fine.
    // might want template parameters for this though, we'll see.
    // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    void setPattern(std::string pattern, spdlog::pattern_time_type time_type = spdlog::pattern_time_type::local)
    {
        mLogger.setPattern(std::move(pattern), time_type);
    }

    // call this function yourself to log the stored messages when needed.
    // maybe todo: just call critical with a message and then call logIt from here.
    // that way we just say logIt(fileAndDir, fmt, args...); Less manual calls that way
    void logIt(const spdlog::filename_t& fileName, bool add_timestamp = false)
    {
        mLogger.logIt(fileName, add_timestamp);
    }

    // NOTE: there is no "trace" because we are always tracing, thus it is unecessary

    template<typename... Args>
    void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        mLogger.info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        mLogger.debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        mLogger.warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        mLogger.error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        mLogger.critical(fmt, std::forward<Args>(args)...);
    }
};

// todo: provide convenience macros here similar to the way FPS_printer does
// example #define LOGGER_INFO(...) ESSLogger::get().info(__VA_ARGS__);
// etc.
// a lot less typing in the long run.
// also: might possibly need to implement a .cpp for this if we run into multiple definition errors?
// will have to test this and make sure ess_controller runs fine and dandy.

// new macros, working on it.
// todo: get resize in here, but am reluctant to do so
#define LOGGER_INFO(...) ESSLogger::get().info(__VA_ARGS__);
#define LOGGER_DEBUG(...) ESSLogger::get().debug(__VA_ARGS__);
#define LOGGER_WARN(...) ESSLogger::get().warn(__VA_ARGS__);
#define LOGGER_ERROR(...) ESSLogger::get().error(__VA_ARGS__);
#define LOGGER_CRITICAL(...) ESSLogger::get().critical(__VA_ARGS__);
#define LOGGER_LOGIT(fileName) ESSLogger::get().logIt(fileName);


#endif