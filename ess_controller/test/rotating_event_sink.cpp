#include "ESSLogger.hpp"

#include "chrono_utils.hpp"

using namespace std::chrono_literals;

int main()
{
    // new singleton pattern usage:
    ESSLogger::get().setBacktrace(2); // this is # of messages it stores.
    ESSLogger::get().setPattern("%D -> %v", spdlog::pattern_time_type::local); // make sure %v is there to keep your own message
    ESSLogger::get().info("hello {}", "world");
    ESSLogger::get().info("Really really really really really really really really big");
    ESSLogger::get().info("some more messages");
    ESSLogger::get().critical("blank");
    ESSLogger::get().setBacktrace(1);
    ESSLogger::get().setBacktrace(5);
    ESSLogger::get().warn("a warning");
    ESSLogger::get().critical("critical");
    ESSLogger::get().logIt("old testing.txt");
    ESSLogger::get().setBacktrace(1);
    ESSLogger::get().debug("a debug message");
    ESSLogger::get().error("this is an error"); // after this, call logIt("filename.ext");
    ESSLogger::get().setBacktrace(3);
    // ESSLogger::get().setBacktrace(0); // this deleted all old messages
    // ESSLogger::get().setBacktrace(10); // this makes new space
    ESSLogger::get().resetBacktrace(10);
    auto now = std::chrono::system_clock::now();
    ESSLogger::get().debug("Is spdlog slow? I think not, observe -> {:%F} {:%H:%M:%S}", now, now.time_since_epoch() + 20_h);
    ESSLogger::get().logIt("testing.txt");
}
