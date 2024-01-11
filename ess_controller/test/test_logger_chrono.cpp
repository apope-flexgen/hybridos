// #define CHRONO_UTILS_SINGLE_INCLUDE

#include "asset.h"
#include "scheduler.h"
#include "spdlog/sinks/stdout_color_sinks.h" // where the stderr color sink comes from
#include "spdlog/fmt/fmt.h" // for fmt::print(stderr, fmt, args...);

// this is how to use the new single include macro for chrono_utils.
// NOTE: do NOT put this everywhere only in test files

// #include "ESSLogger.hpp"
// #include "chrono_utils.hpp"

typedef std::vector<schedItem*>schlist;
schlist schreqs;
cJSON*getSchListcJ(schlist&schreqs);

cJSON*getSchList()
{
    return getSchListcJ(schreqs);
}


// fmt::print can go straight to stderr, so FPS_ERROR_PRINT can go away now.
// todo: get this to work with spdlog::info, error, warn, etc. (the default logger)
// spdlog has a stderr color sink, but I might need to make a singleton for that.
// it even has an _mt and _st version of that, so the singleton wouldn't be too bad
// as we could just swap out the _st with _mt when moving to a multi-threaded environment.
// Therefore we could get all the benefits of spdlog when printing to the console.
#define NEW_FPS_ERROR_PRINT(...) fmt::print(stderr, __VA_ARGS__);

// my implementaiton of the new FPS singleton pattern using spdlogs stderr_color_sink
// possible problem: we can't have more than one guy going to the same sink, not without sharing the same sink_ptr.
class FPS_Printer final
{
    // todo: could also set this to _mt for a multithreaded environment, singleton pattern could then stay.
    // this is a neat trick because we can get spdlog functionality while still providing macros to access the singleton.
    // the _mt will also allow for thread synchronization when printing to the console if needed.
    // will take it slow, but this will, in my mind, inevitably replace FPS_ERROR_PRINT
    spdlog::logger mLogger;

    FPS_Printer()
        : mLogger("FPS_PRINTER", std::make_shared<spdlog::sinks::stderr_color_sink_st>())
    {
        // if we don't like the redundant [FPS_PRINTER] we can just change the pattern here
        mLogger.set_level(spdlog::level::debug);
    }

public:
// NO copying or moving allowed
    FPS_Printer(const FPS_Printer&) = delete;
    FPS_Printer& operator=(const FPS_Printer&) = delete;
    FPS_Printer(FPS_Printer&&) = delete;
    FPS_Printer& operator=(FPS_Printer&&) = delete;

    static FPS_Printer& get()
    {
        static FPS_Printer instance;
        return instance;
    }

    // There is no set pattern, I think the default pattern is correct for printing to console.
    // Unless we absolutely need to change the pattern, I don't plan on providing that functionality.

    template<typename FormatString, typename... Args>
    void info(const FormatString& fmt, Args&&... args)
    {
        mLogger.info(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void debug(const FormatString& fmt, Args&&... args)
    {
        mLogger.debug(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void warn(const FormatString& fmt, Args&&... args)
    {
        mLogger.warn(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void error(const FormatString& fmt, Args&&... args)
    {
        mLogger.error(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void critical(const FormatString& fmt, Args&&... args)
    {
        mLogger.critical(fmt, std::forward<Args>(args)...);
    }
};

// new FPS_ERROR style macros to access the above singleton:
#define FPS_PRINT_INFO(...) FPS_Printer::get().info(__VA_ARGS__);
#define FPS_PRINT_DEBUG(...) FPS_Printer::get().debug(__VA_ARGS__);
#define FPS_PRINT_WARN(...) FPS_Printer::get().warn(__VA_ARGS__);
#define FPS_PRINT_ERROR(...) FPS_Printer::get().error(__VA_ARGS__);
#define FPS_PRINT_CRITICAL(...) FPS_Printer::get().critical(__VA_ARGS__);


// alternatively the "third way" -> just use the regular stdout spdlog::func(fmt, args);
// this is by default _mt (thread safe) so threads trying to stdout to console will be no problem.
// NOTE: it's not stderr it's stdout so we might need to use the stderr singleton above.

// spdlog::set_level(spdlog::level::debug); // will have to include this for the debug macros, doesn't work in global space though.
#define OUT_PRINT_INFO(...) spdlog::info(__VA_ARGS__);
#define OUT_PRINT_WARN(...) spdlog::warn(__VA_ARGS__);
#define OUT_PRINT_ERROR(...) spdlog::error(__VA_ARGS__);
#define OUT_PRINT_CRITICAL(...) spdlog::critical(__VA_ARGS__);
// #define OUT_PRINT_DEBUG(...) spdlog::debug(__VA_ARGS__); // todo: work on this, default log doesn't print out debug

namespace flex
{
    const std::chrono::system_clock::time_point epoch = flex::please::dont::use::this_func::setEpoch();
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

int main()
{
    const auto now = flex::get_time_dbl(); // default is double seconds.
    const flex::dbl_nano now_ns{now};
    fmt::print(stderr, "stderr can be used too: Time since the program started = {}\n", now);
    NEW_FPS_ERROR_PRINT("Time since the program started in ns = {}\n", now_ns);

    // this is how I could make the new FPS_ERROR_PRINT, just wrap this in a singleton
    // actually this has been implemented in a much more succinct manner above.
    auto fps_logger = spdlog::stderr_color_st("FPS_ERROR_PRINT");
    fps_logger->warn("Something bad starting to happen at time = {}", flex::get_time_dbl<flex::dbl_micro>());
    fps_logger->critical("Something bad is now happening at time = {}", flex::get_time_dbl<flex::dbl_micro>());
    // If you want to do the print for time since epoch without the "e-0x" then you will need something like below
    fps_logger->info(R"(
    {{
        "{}": "{}",
        "{}": {:.9}
    }})",
    "json's and raw strings", "are fun",
    "so is the time since January 1st of this year", flex::time_since_epoch());

    // with the new macros the above fps_logger would just look like this in our code:
    // uses the above FPS_Printer singleton to stderr
    FPS_PRINT_CRITICAL("this is the {} pattern that we can use", "new"); // this provide colors and timestamps to our console output now.

    // and now the ESSLogger (resizable ringBuffer logger)
    // default size is 124 messages, shrinkable and growable to whatever you want.
    ESSLogger::get().info("We can {1} {0} using positional argumentation", "things", "log");
    ESSLogger::get().error("And they are labeled accordingly");
    ESSLogger::get().logIt("testing.txt");
    ESSLogger::get().resetBacktrace(10); // you can even reset your backtrace if you'd like.
    ESSLogger::get().debug("some {} message, put your args here like this, {}, {}, {}", "debug", 3.14, 5UL, "last arg");
    LOGGER_LOGIT("reset_back_macro.txt"); // how to use the new logger with the macros.
    // ESSLogger::get().logIt("reset_the_backtrace.txt");

    OUT_PRINT_INFO("stdout spdlog printing looks like {}", "this");
}
