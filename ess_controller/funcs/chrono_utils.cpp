#include "chrono_utils.hpp"

namespace flex
{
    namespace please {
    namespace dont {
    namespace use {
    namespace this_func {
        // Returns a timepoint at January 1st of the current year.
        // Used only to set the global epoch for the system.
        // do NOT use this yourself, just use epoch variable
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
    }
    }
    }
    }
}

// int main()
// {
    
// }