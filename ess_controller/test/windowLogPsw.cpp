#include <deque>
#include <string>
#include <chrono>
#include <memory>
#include <iostream>
//g++ -std=c++11 -o wl -I spdlog/include windowLog.cpp

// #include "spdlog/fmt/bundled/format.h" // erorr when fmt and spdlog are used together, blegh!
// above is unnecessary it is a part of regular spdlog.h
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

constexpr const char* LogFile = "pswtesting.txt";
constexpr double WindowSize = 5.0; // this is in seconds

// #include "varMapUtils.h"

// compile time checking for FileSize and setting the rotating file num:
// 5 MegaBytes is the max file size before logs are rotated to another file
// todo: get compile time log type in here so we're not stuck with just rotating_file_log_st always
// another template default param will work.
//  Time stuff
// Dont have time today to work out the chrono stuff.

double base_time = 0.0;
long int get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
    return ltime_us;
}

double get_time_dbl()
{
    if(base_time == 0.0)
    {
        base_time = get_time_us()/ 1000000.0;
    }

    return  (double)get_time_us() / 1000000.0 - base_time;
}

// todo: have this work for async loggers as well? Might not be necessary
template<std::size_t MaxFileSize = 1024 * 1024 * 5, std::size_t RotFileNum = 5>
class timedLogger
{
    std::chrono::time_point<std::chrono::high_resolution_clock> mCreationTime = std::chrono::high_resolution_clock::now();
    std::shared_ptr<spdlog::logger> mLogger;

public:
    timedLogger(const char* name, const std::vector<spdlog::sink_ptr>& sinks) // loggers should be named after the asesetVar's name
        : mLogger(std::make_shared<spdlog::logger>(name, sinks.cbegin(), sinks.cend()))
    {
        mLogger->enable_backtrace(1); // stores only one message to go to file at a time.
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> getCreateTime() 
        const { return mCreationTime; }

    template<typename FormatString, typename... Args>
    void info(const FormatString& fmt, Args&&... args)
    {
        mLogger->info(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    template<typename FormatString, typename... Args>
    void debug(const FormatString& fmt, Args&&... args)
    {
        mLogger->debug(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    template<typename FormatString, typename... Args>
    void trace(const FormatString& fmt, Args&&... args)
    {
        mLogger->trace(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    template<typename FormatString, typename... Args>
    void warn(const FormatString& fmt, Args&&... args)
    {
        mLogger->warn(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    template<typename FormatString, typename... Args>
    void error(const FormatString& fmt, Args&&... args)
    {
        mLogger->error(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    template<typename FormatString, typename... Args>
    void critical(const FormatString& fmt, Args&&... args)
    {
        mLogger->critical(fmt, std::forward<Args>(args)...); // might need this to be different depending on log level.
    }

    // outputs log to file that it is assigned to
    void flush()
    {
        mLogger->flush();
    }
};

// this is your "timed window" attached to an assetVar (goes underneath extras), I'll get around to making the
// ostream opeartor<< for the assetVars eventually.
// this uses a deque so not as efficient as it could be, but it works well enough.
// might need to benchmark this eventually, wouldn't be suprised in the performance loss from it.
// "pure" spdlog would, of course, be the fastest.
// the multisink idea is cool, but slows it down quite a bit due to all the heap allocations
// considering making a much more simplified version without the multisink, or just the regular file_sink.

// current MaxFileSize set to 100K  and currently using 5 rotating files
template<std::size_t MaxFileSize = 128 * 1024, std::size_t RotFileNum = 5>
class WindowLog
{
    const char* mLoggerName;
    const std::vector<spdlog::sink_ptr> mSinks; // for storing the sink locations when creating new logs
    // could make above only a reference, but this would cause a segFault if you give it a temporary.

public:
    std::deque<timedLogger<MaxFileSize, RotFileNum>> mLogWindow;
    // creates logs that go to this filename with this logger name:
    // possible todo: make it to where loggers can change name and file location during runtime:
    WindowLog(const char* loggerName, const std::vector<spdlog::sink_ptr>& sinks =
        {std::make_shared<spdlog::sinks::basic_file_sink_st>(LogFile, MaxFileSize, RotFileNum)})
        : mLoggerName(loggerName), mSinks(sinks) {}

    ~WindowLog() // might not need to flush on destruction. If so then comment out.
    {
        this->flush(); // gauranteed logging even when the program ends, just in case.
    }

    void shiftWindow()
    {
        // if (!mLogWindow.empty()) // this is logically unecessary, so it is commented out for now.
        if (0){
            auto* logRef = &(mLogWindow.front());
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dif = now - logRef->getCreateTime();
            while (dif.count() >= WindowSize) // if x (5 by default) seconds have elapsed start popping off from the queue
            {
                logRef->flush(); // log is old, log it to its sinks
                mLogWindow.pop_front(); // get rid of old log, or "shift" the window
                if (mLogWindow.empty()) // nothing to do, return
                {
                    return;
                }
                // go on to the next one
                logRef = &(mLogWindow.front());
                now = std::chrono::high_resolution_clock::now();
                dif = now - logRef->getCreateTime();
            }
        }
    }

    template<typename FormatString, typename... Args>
    void info(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().info(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    template<typename FormatString, typename... Args>
    void debug(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().debug(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    template<typename FormatString, typename... Args>
    void trace(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().trace(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    template<typename FormatString, typename... Args>
    void warn(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().warn(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    template<typename FormatString, typename... Args>
    void error(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().error(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    template<typename FormatString, typename... Args>
    void critical(const FormatString& fmt, Args&&... args)
    {
        mLogWindow.emplace_back(mLoggerName, mSinks);
        mLogWindow.back().critical(fmt, std::forward<Args>(args)...);
        shiftWindow();
    }

    // flush all the loggers.
    void flush()
    {
        while (!mLogWindow.empty())
        {
            mLogWindow.front().flush();
            mLogWindow.pop_front();
        }
    }
};

int main()
{
    // timedLogger<> foo("rotating log", "testing.txt");
    // // foo.error("Hello {}", "foo");
    // // foo.flush();

    // auto* ref = &foo;
    // ref->critical("Hello {}", "foo");
    // ref->flush();

    // timedLogger<> bar("rotating log", "testing.txt");
    // bar.warn("Hello {}", "bar");
    // // bar.flush();
    // std::chrono::duration<double> diff;
    // std::chrono::duration<double> tTot;
    int count = 0;
    //WindowLog<> test("rotating log");
    WindowLog<> test("backtrace log");
    test.enable_backtrace(128);
    double tNow = get_time_dbl();
    double tThen = tNow;
    double diff = 0.0;
    double tTot = 0.0;
    bool fired = false;
    std::cout <<" system all set up \n";

    while (true)
    {
        tNow = get_time_dbl();

        test.info("hello {} count {} elapsed {} diff(uS) {} ", "world", count, tNow, diff*1000000.0);
        tThen = get_time_dbl();
        diff = tThen - tNow;
        tTot += diff;
        count++;
        //if((tNow > 5.0) && ! fired)
        if (count == 256)
        {
            std::cout <<" dumping backtrace \n";
            fired = true;
            test.dump_backtrace(128);
            system ("mv ./testing.txt ./mytesting.txt");

        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // auto logger = spdlog::rotating_logger_st()
    // while (true)
    // {

    // }
}
