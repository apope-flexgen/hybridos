#include "ESSLogger.hpp"

// HOW TO BUILD:
// g++ -std=c++11 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
// -DSPDLOG_COMPILED_LIB -I include/ path/to/file/test_logging_timestamp.cpp
// -lspdlog -o test_logging_timestamp && ./test_logging_timestamp

namespace flex
{
const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

int main()
{
    ESSLogger::get().info("hello world");
    ESSLogger::get().logIt("testing123.txt");
    ESSLogger::get().logIt("testing123.txt");
    ESSLogger::get().logIt("testing123.txt", true);
}
