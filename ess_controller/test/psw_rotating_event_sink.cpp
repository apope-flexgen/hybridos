#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>

#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/chrono.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>
#include <deque>
#include <ratio>

template<typename T>
class ringBuf
{
    std::deque<T> mBuf;
    std::size_t mSize;

public:
    ringBuf(std::size_t size)
        : mSize(size) {}

    template<typename... Args>
    void emplace(Args... args)
    {
        mBuf.emplace_back(std::forward<Args>(args)...);
        if (mBuf.size() > mSize)
        {
            mBuf.pop_front();
        }
    }

    void reSize(std::size_t size)
    {
        mSize = size;
        while (mBuf.size() > mSize)
        {
            mBuf.pop_front();
        }
    }

    std::deque<T>& getBuf()
    {
        return mBuf;
    }

    // T& getHead()
    // {
    //     return mBuf.front();
    // }

    // T& getTail()
    // {
    //     return mBuf.back();
    // }
};

// namespace std
// {
//     namespace chrono
//     {
//         using days = duration<unsigned long long, ratio_multiply<std::ratio<24>, hours::period>>;
//     }
// }

// todo: get event type for this calc as well:
struct event_time_filename_calculator
{
    static spdlog::filename_t calc_filename(const spdlog::filename_t& filename)
    {
        spdlog::filename_t basename, ext;
        std::tie(basename, ext) = spdlog::details::file_helper::split_by_extension(filename);
        auto now = spdlog::log_clock::now();
        return fmt::format(
            SPDLOG_FILENAME_T("{}_{:%F}-{:%H:%M:%S}{}"), basename, 
                now, now.time_since_epoch() + std::chrono::hours(20), ext);
    }
};

//
// Rotating event sink based on date (possibly event type as well -> will look into this)
// todo: put "bucket" (actual backtrace) here so any logger can just sink_it to here auto magically.
//
template<typename Mutex, typename FileNameCalc = event_time_filename_calculator>
class rotating_event_sink final : public spdlog::sinks::base_sink<Mutex>
{
    // spdlog::filename_t base_filename_;
    spdlog::details::file_helper file_helper_;
    ringBuf<spdlog::memory_buf_t> buf_;

public:
    rotating_event_sink(std::size_t bufSize)
        : buf_(bufSize)
    {}

    void setBackTraceNum(std::size_t size)
    {
        buf_.reSize(size);
    }

    void log_it_(const spdlog::filename_t& fileName)
    {
        rotate_(fileName);
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        buf_.emplace();
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, buf_.getBuf().back());
    }

    // flushing just consists of dumping the backtrace logs into the 
    void flush_() override
    {
        // rotate_();
        return;
    }

private:
    // Rotate file:
    // log.txt -> log_(event_type)_(date).txt
    void rotate_(const spdlog::filename_t& fileName)
    {
        file_helper_.close();
        auto newFile = FileNameCalc::calc_filename(fileName);
        file_helper_.open(newFile);
        for (const auto& msg : buf_.getBuf())
        {
            file_helper_.write(msg);
        }
    }
};

// aliases:
using rotating_event_sink_mt = rotating_event_sink<std::mutex>;
using rotating_event_sink_st = rotating_event_sink<spdlog::details::null_mutex>;

class EventLogger
{
    std::shared_ptr<rotating_event_sink_st> mSink;
    std::shared_ptr<spdlog::logger> mLogger;

public:
    EventLogger(const char* name, std::size_t backTraceSize)
        : mSink(std::make_shared<rotating_event_sink_st>(backTraceSize))
    {
        spdlog::sink_ptr temp{mSink};
        mLogger = std::make_shared<spdlog::logger>(name, temp);
    }

    void enable_backtrace(std::size_t size)
    {
        mSink->setBackTraceNum(size);
    }

    // call this function yourself to log the stored messages when needed.
    void logIt(const spdlog::filename_t& fileName)
    {
        mSink->log_it_(fileName);
    }

    template<typename FormatString, typename... Args>
    void info(const FormatString& fmt, Args&&... args)
    {
        mLogger->info(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void debug(const FormatString& fmt, Args&&... args)
    {
        mLogger->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void warn(const FormatString& fmt, Args&&... args)
    {
        mLogger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void error(const FormatString& fmt, Args&&... args)
    {
        mLogger->error(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    void critical(const FormatString& fmt, Args&&... args)
    {
        mLogger->critical(fmt, std::forward<Args>(args)...);
    }
};

auto base_time = std::chrono::steady_clock::now();
double get_time_dbl()
{
    std::chrono::duration<double, std::micro> diff{std::chrono::steady_clock::now() - base_time};
    return diff.count();
}

int main()
{
    double tNow;
    EventLogger foo("foo", 4);
    foo.info("hello {}","world");
    foo.warn("warning");
    foo.critical("critical");
    foo.logIt("pswtesting.txt");
    foo.enable_backtrace(5);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    tNow = get_time_dbl();
    foo.error("error {} {:02.3f}", " it's an error", 2.12345);
    foo.error("error {} {:02.3f}", " diff 1 uS", (get_time_dbl() - tNow));
    tNow = get_time_dbl();
    foo.error("error {} {:02.3f}", " diff 2 uS", (get_time_dbl() - tNow));
    foo.logIt("pswtesting.txt");
}
