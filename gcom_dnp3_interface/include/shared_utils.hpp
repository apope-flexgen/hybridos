#pragma once

#include <cstdint>
#include "spdlog/fmt/compile.h"
#include "spdlog/details/fmt_helper.h"
#include "spdlog/fmt/chrono.h"

// helper macros:
#define FORMAT_TO_BUF(fmt_buf, fmt_str, ...) fmt::format_to(std::back_inserter(fmt_buf), FMT_COMPILE(fmt_str), ##__VA_ARGS__)
#define FORMAT_TO_BUF_NO_COMPILE(fmt_buf, fmt_str, ...) fmt::format_to(std::back_inserter(fmt_buf), fmt_str, ##__VA_ARGS__)

// helper macros for error messages:
#define NEW_FPS_ERROR_PRINT(fmt_str, ...) fmt::print(stderr, FMT_COMPILE(fmt_str), ##__VA_ARGS__)
#define NEW_FPS_ERROR_PRINT_NO_COMPILE(fmt_str, ...) fmt::print(stderr, fmt_str, ##__VA_ARGS__)
#define NEW_FPS_ERROR_PRINT_NO_ARGS(fmt_str) fmt::print(stderr, FMT_COMPILE(fmt_str))

// enums:

enum class Protocol : uint8_t
{
    Modbus,
    DNP3
};

enum class Mode : uint8_t
{
    Client,
    Server
};

enum class Register_Types : uint8_t
{
    AnOPInt16,      // dnp3: Analog Output Int 16
    AnOPInt32,      // dnp3: Analog Output Int 32
    AnOPF32,        // dnp3: Analog Output Float 32
    CROB,            // dnp3: Binary Command (Group 12 Variation 1)
    Analog,         // dnp3: Analog Input, any variation
    Binary,         // dnp3: Binary Input, any variation
    AnalogOS,       // dnp3: Analog Output Status, any variation
    BinaryOS,       // dnp3: Binary Output Status, any variation
    Counter         // dnp3: Counters, any variation
};

enum class Conn_Type : uint8_t
{
    TCP,
    RTU
};

enum class Arg_Types : uint8_t
{
    Error,
    Help,
    File,
    Uri, // for a "get" over fims
    Expand,
    Server
};

// aliases:
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;


// NOTE(WALKER): this is taken from spdlog's library credit goes to them
// return fraction of a second of the given time_point.
// e.g.
// fraction<std::milliseconds>(tp) -> will return the millis part of the second
template <typename ToDuration>
static inline ToDuration time_fraction(std::chrono::system_clock::time_point tp)
{
    using std::chrono::duration_cast;
    using std::chrono::seconds;
    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

template <>
struct fmt::formatter<Register_Types>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Register_Types &type, FormatContext &ctx) const
    {
        switch (type)
{
    // case Register_Types::Holding:
    //     return fmt::format_to(ctx.out(),"analog output (Holding Register)");
    // case Register_Types::Input:
    //     return fmt::format_to(ctx.out(),"analog input (Input Register)");
    // case Register_Types::Coil:
    //     return fmt::format_to(ctx.out(),"binary output (Coil)");
    // case Register_Types::Discrete_Input:
    //     return fmt::format_to(ctx.out(),"binary input (Discrete Input)");
    case Register_Types::AnOPInt16:
        return fmt::format_to(ctx.out(),"analog output (Group 40)");
    case Register_Types::AnOPInt32:
        return fmt::format_to(ctx.out(),"analog output (Group 40)");
    case Register_Types::AnOPF32:
        return fmt::format_to(ctx.out(),"analog output (Group 40)");
    case Register_Types::Analog:
        return fmt::format_to(ctx.out(),"analog input (Group 30)");
    case Register_Types::AnalogOS:
        return fmt::format_to(ctx.out(),"analog output (Group 40)");
    case Register_Types::Binary:
        return fmt::format_to(ctx.out(),"binary input (Group 1)");
    case Register_Types::BinaryOS:
        return fmt::format_to(ctx.out(),"binary output with commands disabled (Group 10)");
    case Register_Types::CROB:
        return fmt::format_to(ctx.out(),"binary output with commands enabled (CROB; Group 12)");
    default:
        return fmt::format_to(ctx.out(),"");
}
    }
};