#pragma once
#include <cstdint>
#include <mutex>

#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/compile.h"
#include "spdlog/details/fmt_helper.h"

#include <fims/defer.hpp>

extern "C"
{
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwtargio.h"
}

struct GcomSystem;
struct DNP3Dependencies;

struct PointTimeoutStruct
{
    DNP3Dependencies *dnp3_sys = nullptr;
    TMWSIM_POINT *dbPoint = nullptr;
};

struct PointStatusInfo
{
    mutable std::mutex point_status_mutex;

    uint64_t num_analog_inputs = 0;
    uint64_t num_binary_inputs = 0;
    uint64_t num_analog_outputs = 0;
    uint64_t num_binary_outputs = 0;
    uint64_t num_counters = 0;

    uint64_t num_analog_inputs_online = 0;
    uint64_t num_binary_inputs_online = 0;
    uint64_t num_analog_outputs_online = 0;
    uint64_t num_binary_outputs_online = 0;
    uint64_t num_counters_online = 0;

    uint64_t num_analog_inputs_restart = 0;
    uint64_t num_binary_inputs_restart = 0;
    uint64_t num_analog_outputs_restart = 0;
    uint64_t num_binary_outputs_restart = 0;
    uint64_t num_counters_restart = 0;

    uint64_t num_analog_inputs_comm_lost = 0;
    uint64_t num_binary_inputs_comm_lost = 0;
    uint64_t num_analog_outputs_comm_lost = 0;
    uint64_t num_binary_outputs_comm_lost = 0;
    uint64_t num_counters_comm_lost = 0;
};

/**
 * @brief Changes the status of analog and binary output points to ONLINE.
 *
 * Upon startup of the DNP3 Server, analog output and binary output points
 * should be changed from RESTART to ONLINE when the server is ready to
 * receive commands for those points. This function should be called just after
 * the dnp3 connection is started so that the analog and binary outputs
 * can receive sets from the client.
 *
 * @param sys GcomSystem for DNP3 Server
 */
void outputPointsGoOnline(GcomSystem &sys);

/**
 * @brief Timer callback for when an analog or binary input point has not been received
 * during the point's timeout period.
 *
 * Upon startup of the DNP3 Server, each analog input and binary input point with a
 * NON-ZERO timeout should be associated with a pointTimeout callback function (using
 * initTimers). If the point is in RESTART mode, this function should just restart
 * the timer. If the point is ONLINE, a call to this function indicates that the point
 * has timed out and is now in COMM_LOST mode. This function will create an applicable
 * dnp3 event and report an error when a point switches to COMM_LOST.
 *
 * @param pPointTimeoutStruct pointer to a PointTimeoutStruct containing 1) a pointer to
 * the dnp3_sys structure within the GcomSystem and 2) a pointer to the specific analog
 * input or binary input dbPoint on the server
 */
void pointTimeout(void *pPointTimeoutStruct);

/**
 * @brief Initialize the timer callbacks for analog and binary input points on the server.
 *
 * Upon startup of the DNP3 Server, each analog input and binary input point with a
 * NON-ZERO timeout should be associated with a pointTimeout callback function using
 * this function.
 *
 * @param sys GcomSystem for DNP3 Server
 */
void initTimers(GcomSystem &sys);

/**
 * @brief Update input point status to ONLINE (from whatever state it was previously in).
 *
 * Each time an analog input or binary input point is received by the DNP3 Server (via
 * fims), update this point's status to ONLINE. Clear the RESTART and COMM_LOST flags
 * if they are currently 1.
 *
 * @param dbPoint pointer to an analog or binary input TMWSIM_POINT stored in the server
 * database
 */
void setInputPointOnline(TMWSIM_POINT *dbPoint);

/**
 * @brief Check if an input point has changed from ONLINE to COMM_LOST or vice versa.
 *
 * Each time an analog input or binary input point is received by the DNP3 client (via
 * DNP3), check if this point has changed state from ONLINE to COMM_LOST or vice versa.
 * Give an appropriate error message if changing to COMM_LOST and give an info message
 * if changing to ONLINE from COMM_LOST.
 *
 * @param dbPoint pointer to an analog or binary input TMWSIM_POINT stored in the client
 * database
 */
void checkPointCommLost(TMWSIM_POINT *dbPoint);

/**
 * @brief Check if an input point has changed to/from OVER_RANGE or CHATTER_FILTER.
 *
 * Each time an analog input or binary input point is received via fims, check if this
 * point has changed state to/from OVER_RANGE or CHATTER_FILTER.
 * Give an appropriate error message if changing to OVER_RANGE or CHATTER_FILTER and give
 * an info message when the flag becomes 0 again.
 *
 * @param dbPoint pointer to an analog or binary input TMWSIM_POINT stored in the server
 * database
 */
void checkInputOverflow(TMWSIM_POINT *dbPoint);

/**
 * @brief Check if an output point has changed to/from OVER_RANGE or CHATTER_FILTER.
 *
 * Each time an analog output or binary output point is received via fims, check if this
 * point has changed state to/from OVER_RANGE or CHATTER_FILTER.
 * Give an appropriate error message if changing to OVER_RANGE or CHATTER_FILTER and give
 * an info message when the flag becomes 0 again.
 *
 * @param dbPoint pointer to an analog or binary output TMWSIM_POINT stored in the server
 * database
 */
void checkOutputOverflow(TMWSIM_POINT *dbPoint);

/**
 * @brief Update PointStatusInfo for all points.
 *
 * Prior to replying to a fims_get request for point status on a DNP3 Client, update
 * the PointStatusInfo struct associated with the system.
 *
 * @param sys GcomSystem for DNP3 Client
 */
void updatePointStatus(GcomSystem &sys);

struct DNP_FLAGS
{
    TMWTYPES_UCHAR flags;
};
struct DNP_FLAG
{
    TMWTYPES_UCHAR flag;
};
template <>
struct fmt::formatter<DNP_FLAG>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const DNP_FLAG &flag, FormatContext &ctx) const
    {
        switch (flag.flag)
        {
        case DNPDEFS_DBAS_FLAG_ON_LINE:
            return fmt::format_to(ctx.out(), "\"ONLINE\"");
        case DNPDEFS_DBAS_FLAG_OFF_LINE:
            return fmt::format_to(ctx.out(), "\"OFFLINE\"");
        case DNPDEFS_DBAS_FLAG_RESTART:
            return fmt::format_to(ctx.out(), "\"RESTART\"");
        case DNPDEFS_DBAS_FLAG_COMM_LOST:
            return fmt::format_to(ctx.out(), "\"COMM_LOST\"");
        case DNPDEFS_DBAS_FLAG_REMOTE_FORCED:
            return fmt::format_to(ctx.out(), "\"REMOTE_FORCED\"");
        case DNPDEFS_DBAS_FLAG_LOCAL_FORCED:
            return fmt::format_to(ctx.out(), "\"LOCAL_FORCED\"");
        case DNPDEFS_DBAS_FLAG_CHATTER:
            return fmt::format_to(ctx.out(), "\"CHATTER, CNTR_ROLLOVER, STAT_ROLLOVER, OVER_RANGE, or OVER_RANGE\"");
        case DNPDEFS_DBAS_FLAG_REFERENCE_CHK:
            return fmt::format_to(ctx.out(), "\"REFERENCE_CHK or DISCONTINUITY\"");
        case DNPDEFS_DBAS_FLAG_BINARY_ON:
            return fmt::format_to(ctx.out(), "\"TRUE\"");
        default:
            return fmt::format_to(ctx.out(), "\"\"");
        }
    }
};

template <>
struct fmt::formatter<DNP_FLAGS>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const DNP_FLAGS &flags, FormatContext &ctx) const
    {
        std::vector<DNP_FLAG> individual_flags;
        for (TMWTYPES_UCHAR i = 0; i < 8; i++)
        {
            if ((flags.flags & ((TMWTYPES_UCHAR)1 << i)) != 0)
            {
                individual_flags.push_back(DNP_FLAG{(TMWTYPES_UCHAR)(1 << i)});
            }
        }
        return fmt::format_to(ctx.out(), R"({})", individual_flags);
    }
};

template <>
struct fmt::formatter<PointStatusInfo>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const PointStatusInfo &point_status_info, FormatContext &ctx) const
    {
        point_status_info.point_status_mutex.lock();
        defer
        {
            point_status_info.point_status_mutex.unlock();
        };

        fmt::format_to(ctx.out(), R"("analog_inputs": {{)");
        fmt::format_to(ctx.out(), R"("total": {}, )", point_status_info.num_analog_inputs);
        fmt::format_to(ctx.out(), R"("online": {}, )", point_status_info.num_analog_inputs_online);
        fmt::format_to(ctx.out(), R"("restart": {}, )", point_status_info.num_analog_inputs_restart);
        fmt::format_to(ctx.out(), R"("comm_lost": {})", point_status_info.num_analog_inputs_comm_lost);
        fmt::format_to(ctx.out(), R"(}}, )");

        fmt::format_to(ctx.out(), R"("binary_inputs": {{)");
        fmt::format_to(ctx.out(), R"("total": {}, )", point_status_info.num_binary_inputs);
        fmt::format_to(ctx.out(), R"("online": {}, )", point_status_info.num_binary_inputs_online);
        fmt::format_to(ctx.out(), R"("restart": {}, )", point_status_info.num_binary_inputs_restart);
        fmt::format_to(ctx.out(), R"("comm_lost": {})", point_status_info.num_binary_inputs_comm_lost);
        fmt::format_to(ctx.out(), R"(}}, )");

        fmt::format_to(ctx.out(), R"("analog_outputs": {{)");
        fmt::format_to(ctx.out(), R"("total": {}, )", point_status_info.num_analog_outputs);
        fmt::format_to(ctx.out(), R"("online": {}, )", point_status_info.num_analog_outputs_online);
        fmt::format_to(ctx.out(), R"("restart": {})", point_status_info.num_analog_outputs_restart);
        // no comm_lost because online = ready for commands
        fmt::format_to(ctx.out(), R"(}}, )");

        fmt::format_to(ctx.out(), R"("binary_outputs": {{)");
        fmt::format_to(ctx.out(), R"("total": {}, )", point_status_info.num_binary_outputs);
        fmt::format_to(ctx.out(), R"("online": {}, )", point_status_info.num_binary_outputs_online);
        fmt::format_to(ctx.out(), R"("restart": {})", point_status_info.num_binary_outputs_restart);
        // no comm_lost because online = ready for commands
        fmt::format_to(ctx.out(), R"(}}, )");

        fmt::format_to(ctx.out(), R"("counters": {{)");
        fmt::format_to(ctx.out(), R"("total": {}, )", point_status_info.num_counters);
        fmt::format_to(ctx.out(), R"("online": {}, )", point_status_info.num_counters_online);
        fmt::format_to(ctx.out(), R"("restart": {}, )", point_status_info.num_counters_restart);
        fmt::format_to(ctx.out(), R"("comm_lost": {})", point_status_info.num_counters_comm_lost);
        return fmt::format_to(ctx.out(), R"(}})");
    }
};
