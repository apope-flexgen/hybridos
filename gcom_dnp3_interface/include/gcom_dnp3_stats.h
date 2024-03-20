#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <mutex>

#include <fims/defer.hpp>

#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/compile.h"
#include "spdlog/details/fmt_helper.h"

extern "C"
{
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwsesn.h"
}

struct GcomSystem;

#ifndef DNP3_MASTER
#define DNP3_MASTER 0
#endif
#ifndef DNP3_OUTSTATION
#define DNP3_OUTSTATION 1
#endif

enum class ChannelErrorType : uint8_t
{
    PhysicalLayer,
    LinkLayer,
    TransportLayer
};

typedef struct
{
    ChannelErrorType type;
    TMWCHNL_ERROR_CODE error_code;
    uint64_t count;
    std::string error_message;
} ERROR_ENTRY;

struct ChannelStats
{
    mutable std::mutex channel_stats_mutex;
    int who;
    GcomSystem *sys;
    uint64_t open_channels = 0;
    uint64_t time_to_connect = 0;
    char port[10];
    std::vector<std::string> connectedIPAddresses;
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t frames_sent = 0;
    uint64_t frames_received = 0;
    uint64_t fragments_sent = 0;
    uint64_t fragments_received = 0;

    uint64_t total_errors = 0;
    uint64_t physical_layer_errors = 0;
    uint64_t link_layer_errors = 0;
    uint64_t transport_layer_errors = 0;

    ERROR_ENTRY error_table[24] = {
        {ChannelErrorType::PhysicalLayer, TMWCHNL_ERROR_PHYS_TRANSMIT, 0, "Error returned from target transmit routine"},
        {ChannelErrorType::PhysicalLayer, TMWCHNL_ERROR_PHYS_CHAR_TIMEOUT, 0, "Intercharacter timeout occurred"},
        {ChannelErrorType::PhysicalLayer, TMWCHNL_ERROR_PHYS_REMOTE_CLOSE, 0, "remote side of channel closed connection"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_FRAME_LENGTH, 0, "Incoming frame too short or exceeded buffer size"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_ADDRESS_UNKNOWN, 0, "Received frame was for an unknown link address"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_ILLEGAL_FUNCTION, 0, "illegal link function code in received frame"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_INVALID_CHECKSUM, 0, "Invalid checksum or CRC"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_NOT_RESET, 0, "Link has not been reset, frame rejected"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_FCB, 0, "Received invalid frame count bit"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_INVALID_START_CHAR, 0, "Did not receive correct starting sync char"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_FRAME_TIMEOUT, 0, "Entire frame was not received in specified time"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_CNFM_TIMEOUT, 0, "Link Confirm was not received in specified time"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_STATUS_TIMEOUT, 0, "Link status response not received in specified time"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_WRONG_SESN, 0, "Response was not from expected session"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_WRONG_REPLY, 0, "Received unexpected reply, frame rejected"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_INVALID_2ND_CHAR, 0, "Did not receive correct second sync char"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_INVALID_END_CHAR, 0, "Did not receive correct ending sync character"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_MISMATCHING_LENGTH, 0, "variable length bytes in FT1.2 frame did not match"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_INV_DIR, 0, "Received invalid dir bit in control octet"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_NO_CNFM_RECEIVED, 0, "Confirm of 104 U-format APDU not received"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_NO_ACK_RECEIVED, 0, "Acknowledge of 104 I-format APDU not received"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_SEQUENCE_UNKNOWN, 0, "Unknown confirming sequence number in received APDU"},
        {ChannelErrorType::LinkLayer, TMWCHNL_ERROR_LINK_OUT_OF_SEQUENCE, 0, "received APDU not in sequence with previous APDU"},
        {ChannelErrorType::TransportLayer, TMWCHNL_ERROR_TPRT_SEQUENCE_ERROR, 0, "Sequence number error"},
    };

    std::map<std::string, uint64_t *> channel_stats_map;

    ChannelStats()
    {
        channel_stats_map["/open_channels"] = &open_channels;
        channel_stats_map["/time_to_connect"] = &time_to_connect;
        channel_stats_map["/bytes_sent"] = &bytes_sent;
        channel_stats_map["/bytes_received"] = &bytes_received;
        channel_stats_map["/frames_sent"] = &frames_sent;
        channel_stats_map["/frames_received"] = &frames_received;
        channel_stats_map["/fragments_sent"] = &fragments_sent;
        channel_stats_map["/fragments_received"] = &fragments_received;
        channel_stats_map["/total_errors"] = &total_errors;
        channel_stats_map["/physical_layer_errors"] = &physical_layer_errors;
        channel_stats_map["/link_layer_errors"] = &link_layer_errors;
        channel_stats_map["/transport_layer_errors"] = &transport_layer_errors;
    };
};

struct SessionStats
{
    mutable std::mutex session_stats_mutex;
    int who;
    GcomSystem *sys;
    uint64_t session_errors = 0;
    uint64_t sessions_online = 0;
    uint64_t asdu_sent = 0; // application service data units
    uint64_t asdu_received = 0;

    uint64_t event_queue_overflows = 0;
    std::map<TMWTYPES_UCHAR, uint64_t> group_num_event_overflows;

    uint64_t events_sent = 0;
    std::map<TMWTYPES_UCHAR, uint64_t> group_num_events_sent;
    // might want to do individual points as well...

    uint64_t events_confirmed = 0;
    std::map<TMWTYPES_UCHAR, uint64_t> group_num_events_confirmed;
    // might want to do individual points as well...

    // how to do delay timer?
    // typedef struct {
    // /* Class mask for this delay timer being started */
    // TMWDEFS_CLASS_MASK eventClass;

    // /* Value of delay timer in milliseconds */
    // TMWTYPES_MILLISECONDS delay;
    // } TMWSESN_STAT_DNPUNSOLTIMER;

    uint64_t unsol_timer_start = 0;

    uint64_t link_status_request_frames_received = 0;
    uint64_t link_status_frames_received = 0;

    uint64_t request_timeouts = 0;
    uint64_t request_fails = 0;

    uint64_t secure_auth_sent = 0;
    uint64_t secure_auth_received = 0;
    uint64_t secure_auth_response_timeout = 0;
    uint64_t secure_auth_key_change = 0;

    std::map<std::string, uint64_t *> session_stats_map;

    SessionStats()
    {
        session_stats_map["/session_errors"] = &session_errors;
        session_stats_map["/sessions_online"] = &sessions_online;
        session_stats_map["/asdu_sent"] = &asdu_sent;
        session_stats_map["/asdu_received"] = &asdu_received;
        session_stats_map["/event_queue_overflows"] = &event_queue_overflows;
        session_stats_map["/events_sent"] = &events_sent;
        session_stats_map["/events_confirmed"] = &events_confirmed;
        session_stats_map["/link_status_request_frames_received"] = &link_status_request_frames_received;
        session_stats_map["/link_status_frames_received"] = &link_status_frames_received;
        session_stats_map["/request_timeouts"] = &request_timeouts;
        session_stats_map["/request_fails"] = &request_fails;
        session_stats_map["/secure_auth_sent"] = &secure_auth_sent;
        session_stats_map["/secure_auth_received"] = &secure_auth_received;
        session_stats_map["/secure_auth_response_timeout"] = &secure_auth_response_timeout;
        session_stats_map["/secure_auth_key_change"] = &secure_auth_key_change;
    };
};

struct Timing
{
    mutable std::mutex mutex;
    uint64_t count = 0;
    uint64_t total = 0;
    uint64_t min = std::numeric_limits<uint64_t>::max();
    uint64_t max = 0;
    mutable double average = 0;
};

struct Timings
{
    mutable std::mutex connection_time_mutex;
    int who;
    uint64_t time_to_connect = 0;
    Timing integrity_responses;
    Timing class1_responses;
    Timing class2_responses;
    Timing class3_responses;
    Timing direct_operates;
    Timing analog_direct_operates;
    Timing binary_direct_operates;
    Timing store_data;
    Timing fims_pub;
    Timing unsolicited_responses;
    std::map<std::string, Timing *> timings_map;

    Timings()
    {
        timings_map["/integrity"] = &integrity_responses;
        timings_map["/class1"] = &class1_responses;
        timings_map["/class2"] = &class2_responses;
        timings_map["/class3"] = &class3_responses;
        timings_map["/direct_operates"] = &direct_operates;
        timings_map["/analog_direct_operates"] = &analog_direct_operates;
        timings_map["/binary_direct_operates"] = &binary_direct_operates;
        timings_map["/store_data"] = &store_data;
        timings_map["/fims_pub"] = &fims_pub;
        timings_map["/unsolicited"] = &unsolicited_responses;
    };
};

void initStatsMonitor(GcomSystem &sys);
void dnp3stats_resetChannelStats();
void dnp3stats_resetSessionStats();
void dnp3stats_resetTiming();
void dnp3stats_resetTimings();
void dnp3stats_chnlEventCallback(void *pCallbackParam, TMWCHNL_STAT_EVENT eventType, void *pEventData);
void dnp3stats_sesnStatCallback(void *pCallbackParam, TMWSESN_STAT_EVENT eventType, void *pEventData);
void dnp3stats_updateTiming(Timing &timing, uint64_t new_time);
bool pub_stats_thread(GcomSystem &sys) noexcept;

template <>
struct fmt::formatter<std::map<TMWTYPES_UCHAR, uint64_t>::value_type>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const std::map<TMWTYPES_UCHAR, uint64_t>::value_type &entry, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), R"("Group {}": {})",
                              entry.first,
                              entry.second);
    }
};

template <>
struct fmt::formatter<ERROR_ENTRY>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const ERROR_ENTRY &error, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), R"("{}": {})",
                              error.error_message,
                              error.count);
    }
};

template <>
struct fmt::formatter<ChannelStats>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const ChannelStats &stats, FormatContext &ctx) const
    {
        stats.channel_stats_mutex.lock();
        defer
        {
            stats.channel_stats_mutex.unlock();
        };
        fmt::format_to(ctx.out(), R"({{)");
        fmt::format_to(ctx.out(), R"("open_channels": {}, )", stats.open_channels);
        if (strlen(stats.port) > 0)
        {
            fmt::format_to(ctx.out(), R"("port": {}, )", stats.port);
        }
        fmt::format_to(ctx.out(), R"("connected_ip": {}, )", stats.connectedIPAddresses);
        fmt::format_to(ctx.out(), R"("time_to_connect_milli": {}, )", stats.time_to_connect);
        fmt::format_to(ctx.out(), R"("bytes_sent": {}, )", stats.bytes_sent);
        fmt::format_to(ctx.out(), R"("bytes_received": {}, )", stats.bytes_received);
        fmt::format_to(ctx.out(), R"("frames_sent": {}, )", stats.frames_sent);
        fmt::format_to(ctx.out(), R"("frames_received": {}, )", stats.frames_received);
        fmt::format_to(ctx.out(), R"("fragments_sent": {}, )", stats.fragments_sent);
        fmt::format_to(ctx.out(), R"("fragments_received": {}, )", stats.fragments_received);
        fmt::format_to(ctx.out(), R"("errors": {{)");
        fmt::format_to(ctx.out(), R"("total_errors": {}, )", stats.total_errors);
        fmt::format_to(ctx.out(), R"("physical_layer_errors": {{"total_errors":{}, )", stats.physical_layer_errors);
        fmt::format_to(ctx.out(), R"("error_breakdown": {{)");
        for (int i = 0; i < 2; i++)
        {
            fmt::format_to(ctx.out(), R"({},)", stats.error_table[i]);
        }
        fmt::format_to(ctx.out(), R"({}}}}},)", stats.error_table[2]);
        fmt::format_to(ctx.out(), R"("link_layer_errors": {{"total_errors":{}, )", stats.link_layer_errors);
        fmt::format_to(ctx.out(), R"("error_breakdown": {{)");
        for (int i = 3; i < 22; i++)
        {
            fmt::format_to(ctx.out(), R"({},)", stats.error_table[i]);
        }
        fmt::format_to(ctx.out(), R"({}}}}},)", stats.error_table[22]);
        fmt::format_to(ctx.out(), R"("transport_layer_errors": {{"total_errors":{}, )", stats.transport_layer_errors);
        fmt::format_to(ctx.out(), R"("error_breakdown": {{)");
        fmt::format_to(ctx.out(), R"({}}}}})", stats.error_table[23]);
        return fmt::format_to(ctx.out(), R"(}}}})");
    }
};

template <>
struct fmt::formatter<SessionStats>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const SessionStats &stats, FormatContext &ctx) const
    {
        stats.session_stats_mutex.lock();
        defer
        {
            stats.session_stats_mutex.unlock();
        };
        if (stats.who == DNP3_MASTER)
        {
            fmt::format_to(ctx.out(), R"({{)");
            fmt::format_to(ctx.out(), R"("sessions_online": {}, )", stats.sessions_online);
            fmt::format_to(ctx.out(), R"("session_errors": {}, )", stats.session_errors);
            fmt::format_to(ctx.out(), R"("application_service_data_units_sent": {}, )", stats.asdu_sent);
            fmt::format_to(ctx.out(), R"("application_service_data_units_received": {}, )", stats.asdu_received);
            fmt::format_to(ctx.out(), R"("link_status_request_frames_received": {}, )", stats.link_status_request_frames_received);
            fmt::format_to(ctx.out(), R"("link_status_frames_received": {}, )", stats.link_status_frames_received);
            fmt::format_to(ctx.out(), R"("request_timeouts": {}, )", stats.request_timeouts);
            fmt::format_to(ctx.out(), R"("request_fails": {}, )", stats.request_fails);
            fmt::format_to(ctx.out(), R"("secure_auth_sent": {}, )", stats.secure_auth_sent);
            fmt::format_to(ctx.out(), R"("secure_auth_received": {}, )", stats.secure_auth_received);
            fmt::format_to(ctx.out(), R"("secure_auth_response_timeout": {}, )", stats.secure_auth_response_timeout);
            fmt::format_to(ctx.out(), R"("secure_auth_key_change": {})", stats.secure_auth_key_change);
            return fmt::format_to(ctx.out(), R"(}})");
        }
        else
        {
            // basically the same thing but with the event-specific items
            fmt::format_to(ctx.out(), R"({{)");
            fmt::format_to(ctx.out(), R"("sessions_online": {}, )", stats.sessions_online);
            fmt::format_to(ctx.out(), R"("session_errors": {}, )", stats.session_errors);
            fmt::format_to(ctx.out(), R"("application_service_data_units_sent": {}, )", stats.asdu_sent);
            fmt::format_to(ctx.out(), R"("application_service_data_units_received": {}, )", stats.asdu_received);
            fmt::format_to(ctx.out(), R"("event_queue_overflows": {}, )", stats.event_queue_overflows);
            fmt::format_to(ctx.out(), R"("overflowed_event_groups": {{{}}}, )", fmt::join(stats.group_num_event_overflows, ", "));
            fmt::format_to(ctx.out(), R"("events_sent": {}, )", stats.events_sent);
            fmt::format_to(ctx.out(), R"("group_num_events_sent": {{{}}}, )", fmt::join(stats.group_num_events_sent, ", "));
            fmt::format_to(ctx.out(), R"("events_confirmed": {}, )", stats.events_confirmed);
            fmt::format_to(ctx.out(), R"("group_num_events_confirmed": {{{}}}, )", fmt::join(stats.group_num_events_confirmed, ", "));
            fmt::format_to(ctx.out(), R"("link_status_request_frames_received": {}, )", stats.link_status_request_frames_received);
            fmt::format_to(ctx.out(), R"("link_status_frames_received": {}, )", stats.link_status_frames_received);
            fmt::format_to(ctx.out(), R"("request_timeouts": {}, )", stats.request_timeouts);
            fmt::format_to(ctx.out(), R"("request_fails": {}, )", stats.request_fails);
            fmt::format_to(ctx.out(), R"("secure_auth_sent": {}, )", stats.secure_auth_sent);
            fmt::format_to(ctx.out(), R"("secure_auth_received": {}, )", stats.secure_auth_received);
            fmt::format_to(ctx.out(), R"("secure_auth_response_timeout": {}, )", stats.secure_auth_response_timeout);
            fmt::format_to(ctx.out(), R"("secure_auth_key_change": {})", stats.secure_auth_key_change);
            return fmt::format_to(ctx.out(), R"(}})");
        }
    }
};

template <>
struct fmt::formatter<Timings>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Timings &stats, FormatContext &ctx) const
    {
        auto update_average = [&](const Timing &timing)
        {
            timing.mutex.lock();
            if (timing.count > 0)
            {
                timing.average = timing.total * 1.0 / timing.count;
            }
            timing.mutex.unlock();
        };
        update_average(stats.integrity_responses);
        update_average(stats.class1_responses);
        update_average(stats.class2_responses);
        update_average(stats.class3_responses);
        update_average(stats.direct_operates);
        update_average(stats.binary_direct_operates);
        update_average(stats.analog_direct_operates);
        update_average(stats.store_data);
        update_average(stats.fims_pub);
        update_average(stats.unsolicited_responses);
        stats.connection_time_mutex.lock();
        stats.integrity_responses.mutex.lock();
        stats.class1_responses.mutex.lock();
        stats.class2_responses.mutex.lock();
        stats.class3_responses.mutex.lock();
        stats.direct_operates.mutex.lock();
        stats.binary_direct_operates.mutex.lock();
        stats.analog_direct_operates.mutex.lock();
        stats.store_data.mutex.lock();
        stats.fims_pub.mutex.lock();
        stats.unsolicited_responses.mutex.lock();
        defer
        {
            stats.connection_time_mutex.unlock();
            stats.integrity_responses.mutex.unlock();
            stats.class1_responses.mutex.unlock();
            stats.class2_responses.mutex.unlock();
            stats.class3_responses.mutex.unlock();
            stats.direct_operates.mutex.unlock();
            stats.binary_direct_operates.mutex.unlock();
            stats.analog_direct_operates.mutex.unlock();
            stats.store_data.mutex.unlock();
            stats.fims_pub.mutex.unlock();
            stats.unsolicited_responses.mutex.unlock();
        };

        if (stats.who == DNP3_MASTER)
        {
            fmt::format_to(ctx.out(), R"({{)");
            fmt::format_to(ctx.out(), R"("time_to_connect_milli": {}, )", stats.time_to_connect);
            fmt::format_to(ctx.out(), R"("num_integrity_polls": {}, )", stats.integrity_responses.count);
            fmt::format_to(ctx.out(), R"("average_integrity_response_time_milli": {:g}, )", stats.integrity_responses.average);
            fmt::format_to(ctx.out(), R"("min_integrity_response_time_milli": {}, )", ((stats.integrity_responses.count == 0) ? 0 : stats.integrity_responses.min));
            fmt::format_to(ctx.out(), R"("max_integrity_response_time_milli": {}, )", stats.integrity_responses.max);
            fmt::format_to(ctx.out(), R"("num_class_1_scans": {}, )", stats.class1_responses.count);
            fmt::format_to(ctx.out(), R"("average_class_1_response_time_milli": {:g}, )", stats.class1_responses.average);
            fmt::format_to(ctx.out(), R"("min_class_1_response_time_milli": {}, )", ((stats.class1_responses.count == 0) ? 0 : stats.class1_responses.min));
            fmt::format_to(ctx.out(), R"("max_class_1_response_time_milli": {}, )", stats.class1_responses.max);
            fmt::format_to(ctx.out(), R"("num_class_2_scans": {}, )", stats.class2_responses.count);
            fmt::format_to(ctx.out(), R"("average_class_2_response_time_milli": {:g}, )", stats.class2_responses.average);
            fmt::format_to(ctx.out(), R"("min_class_2_response_time_milli": {}, )", ((stats.class2_responses.count == 0) ? 0 : stats.class2_responses.min));
            fmt::format_to(ctx.out(), R"("max_class_2_response_time_milli": {}, )", stats.class2_responses.max);
            fmt::format_to(ctx.out(), R"("num_class_3_scans": {}, )", stats.class3_responses.count);
            fmt::format_to(ctx.out(), R"("average_class_3_response_time_milli": {:g}, )", stats.class3_responses.average);
            fmt::format_to(ctx.out(), R"("min_class_3_response_time_milli": {}, )", ((stats.class3_responses.count == 0) ? 0 : stats.class3_responses.min));
            fmt::format_to(ctx.out(), R"("max_class_3_response_time_milli": {}, )", stats.class3_responses.max);
            fmt::format_to(ctx.out(), R"("num_direct_operates": {}, )", stats.direct_operates.count);
            fmt::format_to(ctx.out(), R"("average_direct_operate_response_time_milli": {:g}, )", stats.direct_operates.average);
            fmt::format_to(ctx.out(), R"("min_direct_operate_response_time_milli": {}, )", ((stats.direct_operates.count == 0) ? 0 : stats.direct_operates.min));
            fmt::format_to(ctx.out(), R"("max_direct_operate_response_time_milli": {}, )", stats.direct_operates.max);
            fmt::format_to(ctx.out(), R"("num_binary_direct_operates": {}, )", stats.binary_direct_operates.count);
            fmt::format_to(ctx.out(), R"("average_binary_direct_operate_response_time_milli": {:g}, )", stats.binary_direct_operates.average);
            fmt::format_to(ctx.out(), R"("min_binary_direct_operate_response_time_milli": {}, )", ((stats.binary_direct_operates.count == 0) ? 0 : stats.direct_operates.min));
            fmt::format_to(ctx.out(), R"("max_binary_direct_operate_response_time_milli": {}, )", stats.binary_direct_operates.max);
            fmt::format_to(ctx.out(), R"("num_analog_direct_operates": {}, )", stats.analog_direct_operates.count);
            fmt::format_to(ctx.out(), R"("average_analog_direct_operate_response_time_milli": {:g}, )", stats.analog_direct_operates.average);
            fmt::format_to(ctx.out(), R"("min_analog_direct_operate_response_time_milli": {}, )", ((stats.analog_direct_operates.count == 0) ? 0 : stats.direct_operates.min));
            fmt::format_to(ctx.out(), R"("max_analog_direct_operate_response_time_milli": {}, )", stats.analog_direct_operates.max);
            fmt::format_to(ctx.out(), R"("num_db_stores": {}, )", stats.store_data.count);
            fmt::format_to(ctx.out(), R"("total_time_to_store_data_sec": {:g}, )", std::chrono::duration<double>(std::chrono::duration<uint64_t, std::milli>(stats.store_data.total)).count());
            fmt::format_to(ctx.out(), R"("average_time_to_store_data_micro": {:g}, )", stats.store_data.average);
            fmt::format_to(ctx.out(), R"("min_time_to_store_data_micro": {}, )", ((stats.store_data.count == 0) ? 0 : stats.store_data.min));
            fmt::format_to(ctx.out(), R"("max_time_to_store_data_micro": {}, )", stats.store_data.max);
            fmt::format_to(ctx.out(), R"("num_fims_pubs": {}, )", stats.fims_pub.count);
            fmt::format_to(ctx.out(), R"("total_time_to_prepare_fims_pub_sec": {:g}, )", std::chrono::duration<double>(std::chrono::duration<uint64_t, std::milli>(stats.fims_pub.total)).count());
            fmt::format_to(ctx.out(), R"("average_time_to_prepare_fims_pub_micro": {:g}, )", stats.fims_pub.average);
            fmt::format_to(ctx.out(), R"("min_time_to_prepare_fims_pub_micro": {}, )", ((stats.fims_pub.count == 0) ? 0 : stats.fims_pub.min));
            fmt::format_to(ctx.out(), R"("max_time_to_prepare_fims_pub_micro": {})", stats.fims_pub.max);
            return fmt::format_to(ctx.out(), R"(}})");
        }
        else
        {
            fmt::format_to(ctx.out(), R"({{)");
            fmt::format_to(ctx.out(), R"("time_to_connect_milli": {}, )", stats.time_to_connect);
            fmt::format_to(ctx.out(), R"("num_unsolicited_confirms": {}, )", stats.unsolicited_responses.count);
            fmt::format_to(ctx.out(), R"("average_unsolicited_confirm_time_milli": {:g}, )", stats.unsolicited_responses.average);
            fmt::format_to(ctx.out(), R"("min_unsolicited_confirm_time_milli": {}, )", ((stats.unsolicited_responses.count == 0) ? 0 : stats.unsolicited_responses.min));
            fmt::format_to(ctx.out(), R"("max_unsolicited_confirm_time_milli": {})", stats.unsolicited_responses.max);
            return fmt::format_to(ctx.out(), R"(}})");
        }
    }
};

template <>
struct fmt::formatter<Timing>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Timing &timing, FormatContext &ctx) const
    {
        timing.mutex.lock();
        defer
        {
            timing.mutex.unlock();
        };
        fmt::format_to(ctx.out(), R"({{)");
        fmt::format_to(ctx.out(), R"("count": {}, )", timing.count);
        fmt::format_to(ctx.out(), R"("total": {}, )", timing.total);
        fmt::format_to(ctx.out(), R"("average": {:g}, )", timing.average);
        fmt::format_to(ctx.out(), R"("min": {}, )", timing.min);
        fmt::format_to(ctx.out(), R"("max": {})", timing.max);
        return fmt::format_to(ctx.out(), R"(}})");
    }
};