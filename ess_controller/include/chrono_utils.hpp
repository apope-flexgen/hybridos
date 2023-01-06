#ifndef CHRONO_UTILS_HPP
#define CHRONO_UTILS_HPP

#include <chrono>
#include <type_traits>

#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/chrono.h"

// copy and paste this macro into your test files so you don't have to define things over and over.
// for release, do NOT include this macro and compile using chrono_utils.cpp (or .o) with the extern variables
// defined once right before your main -> please define epoch before base_time inside the flex namespace
// #define CHRONO_UTILS_SINGLE_INCLUDE

// loss-less conversion between durations as double representation
namespace flex
{
    // NOTE: might need to change these to long doubles to prevent overflow, they seem good for now
    using dbl_nano = std::chrono::duration<double, std::nano>;
    using dbl_micro = std::chrono::duration<double, std::micro>;
    using dbl_milli = std::chrono::duration<double, std::milli>;
    using dbl_sec = std::chrono::duration<double>;

    namespace literals
    {
        namespace chrono_literals
        {
            // literals:
            constexpr dbl_nano operator"" _dns(const long double ns) noexcept
            {
                return dbl_nano{ns};
            }

            constexpr dbl_micro operator"" _dus(const long double us) noexcept
            {
                return dbl_micro{us};
            }

            constexpr dbl_milli operator"" _dms(const long double ms) noexcept
            {
                return dbl_milli{ms};
            }

            constexpr dbl_sec operator"" _ds(const long double s) noexcept
            {
                return dbl_sec{s};
            }
        }
    }

    namespace chrono_literals = literals::chrono_literals;

    namespace please {
    namespace dont {
    namespace use {
    namespace this_func {
        // Returns a timepoint at January 1st of the current year.
        // Used only to set the global epoch for the system.
        // do NOT use this yourself, just use epoch variable
        #ifdef CHRONO_UTILS_SINGLE_INCLUDE 
        const std::chrono::system_clock::time_point setEpoch()
        {
            std::tm tm = fmt::localtime(std::chrono::system_clock::now()); // uses fmt localtime
            tm.tm_sec = 0; // reset seconds
            tm.tm_min = 0; // reset minutes
            tm.tm_hour = 1; // reset hours -> is one hour off when converting, thus needs to be 1
            tm.tm_mday = 1; // reset day -> is one day off when converting, thus needs to be 1
            tm.tm_mon = 0; // reset month -> January
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
        #else
        const std::chrono::system_clock::time_point setEpoch();
        #endif
    }
    }
    }
    }

    #ifdef CHRONO_UTILS_SINGLE_INCLUDE
    const auto epoch = please::dont::use::this_func::setEpoch();
    #else
    extern const std::chrono::system_clock::time_point epoch;
    #endif

    template<typename TimeType = dbl_sec>
    const TimeType time_since_epoch() noexcept
    {
        static_assert(
            std::is_same<TimeType, dbl_nano>::value ||
            std::is_same<TimeType, dbl_micro>::value ||
            std::is_same<TimeType, dbl_milli>::value ||
            std::is_same<TimeType, dbl_sec>::value
            , "Must be of double type flex::dbl_(type).");
        return std::chrono::system_clock::now() - epoch;
    }

    #ifdef CHRONO_UTILS_SINGLE_INCLUDE
    const auto base_time = std::chrono::steady_clock::now();
    #else
    extern const std::chrono::steady_clock::time_point base_time;
    #endif

    // new template version, default is in seconds. Template version makes it fast and safe
    template<typename TimeType = dbl_sec>
    const TimeType get_time_dbl() noexcept
    {
        static_assert(
            std::is_same<TimeType, dbl_nano>::value ||
            std::is_same<TimeType, dbl_micro>::value ||
            std::is_same<TimeType, dbl_milli>::value ||
            std::is_same<TimeType, dbl_sec>::value
            , "Must be of double type flex::dbl_(type).");
        return std::chrono::steady_clock::now() - base_time;
    }
}

namespace std
{
    namespace chrono
    {
        using days = duration<int64_t, ratio_multiply<ratio<24>, hours::period>>;
        using weeks = duration<int64_t, ratio_multiply<ratio<7>, days::period>>;
        using months = duration<int64_t, ratio<2629746>>;
        using years = duration<int64_t, ratio<31556952>>;
    }

    namespace literals
    {
        namespace chrono_literals
        {
            constexpr std::chrono::nanoseconds operator"" _ns(const unsigned long long ns) noexcept
            {
                return std::chrono::nanoseconds{ns};
            }

            constexpr std::chrono::microseconds operator"" _us(const unsigned long long us) noexcept
            {
                return std::chrono::microseconds{us};
            }

            constexpr std::chrono::milliseconds operator"" _ms(const unsigned long long ms) noexcept
            {
                return std::chrono::milliseconds{ms};
            }

            constexpr std::chrono::seconds operator"" _s(const unsigned long long s) noexcept
            {
                return std::chrono::seconds{s};
            }

            constexpr std::chrono::minutes operator"" _m(const unsigned long long m) noexcept
            {
                return std::chrono::minutes{m};
            }

            constexpr std::chrono::hours operator"" _h(const unsigned long long h) noexcept
            {
                return std::chrono::hours{h};
            }

            constexpr std::chrono::days operator"" _d(const unsigned long long d) noexcept
            {
                return std::chrono::days{d};
            }
        }
    }

    namespace chrono_literals = literals::chrono_literals;
}

#endif