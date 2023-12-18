#include "gcom_dnp3_stats.h"

#include <thread>

extern "C"
{
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/tmwtarg/LinIoTarg/lintcp.h"
}

#include "shared_utils.hpp"
#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_fims.h"
#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_utils.h"

ChannelStats channel_stats;
SessionStats session_stats;
Timings timings;

extern GcomSystem *clientSysp;
extern GcomSystem *serverSysp;

void initStatsMonitor(GcomSystem &sys)
{
    auto &dnp3_sys = sys.protocol_dependencies->dnp3;
    channel_stats.who = sys.protocol_dependencies->who;
    session_stats.who = sys.protocol_dependencies->who;
    timings.who = sys.protocol_dependencies->who;
    dnp3_sys.channel_stats = &channel_stats;
    dnp3_sys.session_stats = &session_stats;
    dnp3_sys.timings = &timings;
    channel_stats.sys = &sys;
    session_stats.sys = &sys;
}

// Channel Event Callback
// pCallbackParam is a user-defined callback parameter
void dnp3stats_chnlEventCallback(void *pCallbackParam, TMWCHNL_STAT_EVENT eventType, void *pEventData)
{
    channel_stats.channel_stats_mutex.lock();
    defer
    {
        channel_stats.channel_stats_mutex.unlock();
    };
    switch (eventType)
    {
    case TMWCHNL_STAT_ERROR:
    {
        channel_stats.total_errors++;
        TMWCHNL_ERROR_CODE_ENUM *errorType = (TMWCHNL_ERROR_CODE_ENUM *)(pEventData);
        channel_stats.error_table[*errorType].count++;
        switch (channel_stats.error_table[*errorType].type)
        {
        case ChannelErrorType::PhysicalLayer:
            if (!spam_limit(channel_stats.sys, channel_stats.sys->comms_errors))
            {
                FPS_ERROR_LOG("physical layer error: %s", channel_stats.error_table[*errorType].error_message);
                FPS_LOG_IT("physical_layer_error");
            }
            break;
        case ChannelErrorType::LinkLayer:
            if (!spam_limit(channel_stats.sys, channel_stats.sys->comms_errors))
            {
                FPS_ERROR_LOG("link layer error: %s", channel_stats.error_table[*errorType].error_message);
                FPS_LOG_IT("link_layer_error");
            }
            break;
        case ChannelErrorType::TransportLayer:
            if (!spam_limit(channel_stats.sys, channel_stats.sys->comms_errors))
            {
                FPS_ERROR_LOG("transport layer error: %s", channel_stats.error_table[*errorType].error_message);
                FPS_LOG_IT("transport_layer_error");
            }
            break;
        }
        break;
    }
    case TMWCHNL_STAT_OPEN:
    {
        /* pEventData is TCP_IO_CHANNEL */
        channel_stats.time_to_connect = ((TCP_IO_CHANNEL *)pEventData)->time_connect;
        sprintf(channel_stats.port, "%d", ((TCP_IO_CHANNEL *)pEventData)->chnlConfig.ipPort);
        channel_stats.connectedIPAddresses.push_back(((TCP_IO_CHANNEL *)pEventData)->lastIPAddress);
        timings.connection_time_mutex.lock();
        timings.time_to_connect = ((TCP_IO_CHANNEL *)pEventData)->time_connect;
        timings.connection_time_mutex.unlock();
        channel_stats.open_channels++;
        if (channel_stats.who == DNP3_MASTER)
        {
            if (!spam_limit(channel_stats.sys, channel_stats.sys->comms_errors))
            {
                FPS_INFO_LOG("Connected to server. Time to connect %d ms.", channel_stats.time_to_connect);
            }
            std::string message = fmt::format("Connected to Server");
            emit_event(&clientSysp->fims_dependencies->fims_gateway, clientSysp->fims_dependencies->name.c_str(), message.c_str(), 3);
        }
        else
        {
            if (!spam_limit(channel_stats.sys, channel_stats.sys->comms_errors))
            {
                FPS_INFO_LOG("Connected to client. Time to connect %d ms.", channel_stats.time_to_connect);
            }
            std::string message = fmt::format("Connected to Client");
            emit_event(&serverSysp->fims_dependencies->fims_gateway, serverSysp->fims_dependencies->name.c_str(), message.c_str(), 3);
        }
        break;
    }
    case TMWCHNL_STAT_CLOSED:
    {
        /* pEventData is TMWDEFS_NULL */
        channel_stats.open_channels--;
        if (channel_stats.who == DNP3_MASTER)
        {
            FPS_ERROR_LOG("Disconnected from server.");
            FPS_LOG_IT("disconnect");
            std::string message = fmt::format("Disconnected from Server");
            emit_event(&clientSysp->fims_dependencies->fims_gateway, clientSysp->fims_dependencies->name.c_str(), message.c_str(), 3);
        }
        else
        {
            FPS_ERROR_LOG("Disconnected from client.");
            FPS_LOG_IT("disconnect");
            std::string message = fmt::format("Disconnected from Client");
            emit_event(&serverSysp->fims_dependencies->fims_gateway, serverSysp->fims_dependencies->name.c_str(), message.c_str(), 3);
        }
        break;
    }

    /* pEventData points to a TMWTYPES_USHORT containing the quantity of bytes */
    case TMWCHNL_STAT_BYTES_SENT:
    {
        TMWTYPES_USHORT num_bytes;
        memcpy(&num_bytes, pEventData, sizeof(TMWTYPES_USHORT));
        channel_stats.bytes_sent += num_bytes;
        break;
    }
    case TMWCHNL_STAT_BYTES_RECEIVED:
    {
        TMWTYPES_USHORT num_bytes;
        memcpy(&num_bytes, pEventData, sizeof(TMWTYPES_USHORT));
        channel_stats.bytes_received += num_bytes;
        break;
    }

    /* pEventData is TMWDEFS_NULL for these four */
    case TMWCHNL_STAT_FRAME_SENT:
    {
        channel_stats.frames_sent++;
        break;
    }
    case TMWCHNL_STAT_FRAME_RECEIVED:
    {
        channel_stats.frames_received++;
        break;
    }
    case TMWCHNL_STAT_FRAGMENT_SENT:
    {
        channel_stats.fragments_sent++;
        break;
    }
    case TMWCHNL_STAT_FRAGMENT_RECEIVED:
    {
        channel_stats.fragments_received++;
        break;
    }
    case TMWCHNL_STAT_T1_TIME_ELAPSED: // specific to another protocol
    case TMWCHNL_STAT_ACTIVE:
    case TMWCHNL_STAT_STARTDT_RCVD:
    case TMWCHNL_STAT_STOPDT_RCVD:
    case TMWCHNL_STAT_STARTDT_CON_RCVD:
    case TMWCHNL_STAT_STOPDT_CON_RCVD:
    case TMWCHNL_STAT_UNKNOWN_SESSION:
    default:
        break;
    }
}

// /* In channel initialization code */
// tmwchnl_setStatCallback(pChannel,_chnlEventCallback, myCallbackData);
void dnp3stats_sesnStatCallback(void *pCallbackParam, TMWSESN_STAT_EVENT eventType, void *pEventData)
{
    session_stats.session_stats_mutex.lock();
    defer
    {
        session_stats.session_stats_mutex.unlock();
    };
    switch (eventType)
    {
    case TMWSESN_STAT_ERROR:
    {
        session_stats.session_errors++;
        break;
    }
    case TMWSESN_STAT_ONLINE:
    {
        session_stats.sessions_online++;
        break;
    }
    case TMWSESN_STAT_OFFLINE:
    {
        session_stats.sessions_online--;
        dnp3stats_resetSessionStats();
        break;
    }
    case TMWSESN_STAT_ASDU_SENT:
    {
        session_stats.asdu_sent++;
        break;
    }
    case TMWSESN_STAT_ASDU_RECEIVED:
    {
        session_stats.asdu_received++;
        break;
    }

    /* Currently only used by SDNP
     * Indicates an event queue has overflowed and an event has been discarded
     * pEventData is a pointer to a TMWTYPES_UCHAR indicating what object group
     * lost the event.
     */
    case TMWSESN_STAT_EVENT_OVERFLOW:
    {
        session_stats.event_queue_overflows++;
        TMWTYPES_USHORT group;
        memcpy(&group, pEventData, sizeof(TMWTYPES_USHORT));
        session_stats.group_num_event_overflows[group]++;

        if (!spam_limit(session_stats.sys, session_stats.sys->comms_errors))
        {
            if (session_stats.sessions_online)
            {
                FPS_ERROR_LOG("Event buffer overflow: Group %d ", group);
                FPS_LOG_IT("event_buffer_overflow");
            }
        }
        break;
    }

    /* Currently only used by SDNP
     * Indicates an event is being sent.
     * pEventData is a pointer to a TMWSESN_STAT_DNPEVENT indicating object group and point
     * number of event that is being sent.
     */
    case TMWSESN_STAT_DNPEVENT_SENT:
    {
        session_stats.events_sent++;
        TMWTYPES_USHORT group;
        memcpy(&group, pEventData, sizeof(TMWTYPES_USHORT));
        session_stats.group_num_events_sent[group]++;
        break;
    }

        /* Currently only used by SDNP
         * Indicates an event has been confirmed by master
         * pEventData is a pointer to a TMWSESN_STAT_DNPEVENT indicating object group and point
         * number of event that was acked.
         */
    case TMWSESN_STAT_DNPEVENT_CONFIRM:
    {
        session_stats.events_confirmed++;
        TMWTYPES_USHORT group;
        memcpy(&group, pEventData, sizeof(TMWTYPES_USHORT));
        session_stats.group_num_events_confirmed[group]++;
        break;
    }

    /* Currently only used by SDNP
     * Indicates an unsolicited delay timer is being started
     * pEventData is a pointer to a TMWSESN_STAT_DNPUNSOLTIMER indicating event class and timer value
     * of delay timer being started.
     */
    case TMWSESN_STAT_DNPUNSOL_TIMER_START:
    {
        // idk
        break;
    }
    case TMWSESN_STAT_DNPUNSOL_SENT:
    {
        session_stats.unsol_timer_start = tmwtarg_getMSTime();
        break;
    }
    case TMWSESN_STAT_DNPUNSOL_CONFIRM:
    {
        dnp3stats_updateTiming(timings.unsolicited_responses, (tmwtarg_getMSTime() - session_stats.unsol_timer_start));
        break;
    }
    /* Used only by DNP
     * Indicates a link status request frame was received.
     */
    case TMWSESN_STAT_DNPLINKSTATUSREQ_RECEIVED:
    {
        session_stats.link_status_request_frames_received++;
        break;
    }

    /* Used only by DNP
     * Indicates a link status frame was received.
     */
    case TMWSESN_STAT_DNPLINKSTATUS_RECEIVED:
    {
        session_stats.link_status_frames_received++;
        break;
    }

    /*
     * Indicates a request sent by a master has timed out before completing.
     */
    case TMWSESN_STAT_REQUEST_TIMEOUT:
    {
        session_stats.request_timeouts++;
        if (!spam_limit(session_stats.sys, session_stats.sys->comms_errors))
        {
            if (session_stats.sessions_online)
            {
                FPS_ERROR_LOG("DNP3 request timeout.");
                FPS_LOG_IT("request_timeout");
            }
        }
        break;
    }

    /*
     * Indicates a request sent by a master received a failure response back from the outstation/slave.
     */
    case TMWSESN_STAT_REQUEST_FAILED:
    {
        session_stats.request_fails++;
        if (!spam_limit(session_stats.sys, session_stats.sys->comms_errors))
        {
            if (session_stats.sessions_online)
            {
                FPS_ERROR_LOG("DNP3 request failed.");
                FPS_LOG_IT("request_failed");
            }
        }
        break;
    }

    /* Currently only used by DNP3 Secure Authentication */
    /* Secure Authentication Message sent */
    case TMWSESN_STAT_AUTH_SENT:
    {
        session_stats.secure_auth_sent++;
        break;
    }
    /* Secure Authentication Message received */
    case TMWSESN_STAT_AUTH_RCVD:
    {
        session_stats.secure_auth_received++;
        break;
    }
    /* Secure Authentication Response Timeout */
    case TMWSESN_STAT_AUTH_RESPTIMEOUT:
    {
        session_stats.secure_auth_response_timeout++;
        if (!spam_limit(session_stats.sys, session_stats.sys->comms_errors))
        {
            FPS_ERROR_LOG("Secure authentication response timeout.");
            FPS_LOG_IT("secure_auth");
        }
        break;
    }
    /* Secure Authentication Key Change timer or count, time to send or receive new session keys */
    case TMWSESN_STAT_AUTH_KEYCHANGE:
    {
        session_stats.secure_auth_key_change++;
        break;
    }
    default:
        break;
    }
}

void dnp3stats_resetChannelStats()
{
    channel_stats.channel_stats_mutex.lock();
    defer
    {
        channel_stats.channel_stats_mutex.unlock();
    };
    channel_stats.open_channels = 0;
    channel_stats.time_to_connect = 0;
    channel_stats.bytes_sent = 0;
    channel_stats.bytes_received = 0;
    channel_stats.frames_sent = 0;
    channel_stats.frames_received = 0;
    channel_stats.fragments_sent = 0;
    channel_stats.fragments_received = 0;

    channel_stats.total_errors = 0;
    channel_stats.physical_layer_errors = 0;
    channel_stats.link_layer_errors = 0;
    channel_stats.transport_layer_errors = 0;

    for (uint64_t i = 0; i < sizeof(channel_stats.error_table) / sizeof(ERROR_ENTRY); i++)
    {
        channel_stats.error_table[i].count = 0;
    }
}
/// @brief
void dnp3stats_resetSessionStats()
{
    session_stats.session_errors = 0;
    session_stats.sessions_online = 0;
    session_stats.asdu_sent = 0;
    session_stats.asdu_received = 0;

    session_stats.event_queue_overflows = 0;
    session_stats.group_num_event_overflows.clear();

    session_stats.events_sent = 0;
    session_stats.group_num_events_sent.clear();

    session_stats.events_confirmed = 0;
    session_stats.group_num_events_confirmed.clear();

    session_stats.link_status_request_frames_received = 0;
    session_stats.link_status_frames_received = 0;

    session_stats.request_timeouts = 0;
    session_stats.request_fails = 0;

    session_stats.secure_auth_sent = 0;
    session_stats.secure_auth_received = 0;
    session_stats.secure_auth_response_timeout = 0;
    session_stats.secure_auth_key_change = 0;
    session_stats.unsol_timer_start = 0;
}
/// @brief
/// @param timing
void dnp3stats_resetTiming(Timing &timing)
{
    timing.mutex.lock();
    timing.count = 0;
    timing.total = 0;
    timing.min = std::numeric_limits<uint64_t>::max();
    timing.max = 0;
    timing.average = 0;
    timing.mutex.unlock();
}
/// @brief
void dnp3stats_resetTimings()
{
    timings.connection_time_mutex.lock();
    timings.time_to_connect = 0;
    timings.connection_time_mutex.unlock();

    dnp3stats_resetTiming(timings.integrity_responses);
    dnp3stats_resetTiming(timings.class1_responses);
    dnp3stats_resetTiming(timings.class2_responses);
    dnp3stats_resetTiming(timings.class3_responses);
    dnp3stats_resetTiming(timings.direct_operates);
    dnp3stats_resetTiming(timings.binary_direct_operates);
    dnp3stats_resetTiming(timings.analog_direct_operates);
    dnp3stats_resetTiming(timings.store_data);
    dnp3stats_resetTiming(timings.fims_pub);
    dnp3stats_resetTiming(timings.unsolicited_responses);
}
/// @brief
/// @param timing
/// @param new_time
void dnp3stats_updateTiming(Timing &timing, uint64_t new_time)
{
    timing.mutex.lock();

    timing.total += new_time;
    timing.count++;
    if (new_time < timing.min)
    {
        timing.min = new_time;
    }
    if (new_time > timing.max)
    {
        timing.max = new_time;
    }

    timing.mutex.unlock();
}
/// @brief
/// @param sys
/// @return
bool pub_stats_thread(GcomSystem &sys) noexcept
{
    fmt::memory_buffer send_buf;
    auto &dnp3_sys = sys.protocol_dependencies->dnp3;

    auto update_average = [&](Timing &timing)
    {
        timing.mutex.lock();
        if (timing.count > 0)
        {
            timing.average = timing.total * 1.0 / timing.count;
        }
        timing.mutex.unlock();
    };

    {
        std::unique_lock<std::mutex> lk{sys.main_mutex};
        sys.main_cond.wait(lk, [&]()
                           { return sys.start_signal; });
    }

    // cleanup/shutdown -> tell main that something is wrong (so it can shut everything down)
    // if we return from this function then something went wrong (hence the "cleanup" defer)
    defer
    {
        sys.start_signal = false;
        sys.keep_running = false;
    };
    // todo add all the spam limit counts in here
    // main loop:
    while (sys.keep_running)
    {
        send_buf.clear();
        send_buf.push_back('{'); // begin object
        // TODO add git build
        FORMAT_TO_BUF(send_buf, R"("git_build": {}, )", sys.git_version_info.get_build());
        FORMAT_TO_BUF(send_buf, R"("git_commit": {}, )", sys.git_version_info.get_commit());
        FORMAT_TO_BUF(send_buf, R"("git_tag": {}, )", sys.git_version_info.get_tag());

        dnp3_sys.channel_stats->channel_stats_mutex.lock();
        FORMAT_TO_BUF(send_buf, R"("open_channels": {}, )", dnp3_sys.channel_stats->open_channels);
        dnp3_sys.channel_stats->channel_stats_mutex.unlock();
        dnp3_sys.session_stats->session_stats_mutex.lock();
        FORMAT_TO_BUF(send_buf, R"("event_queue_overflows": {}, )", dnp3_sys.session_stats->event_queue_overflows);
        FORMAT_TO_BUF(send_buf, R"("request_timeouts": {}, )", dnp3_sys.session_stats->request_timeouts);
        FORMAT_TO_BUF(send_buf, R"("request_fails": {}, )", dnp3_sys.session_stats->request_fails);
        dnp3_sys.session_stats->session_stats_mutex.unlock();
        dnp3_sys.timings->connection_time_mutex.lock();
        FORMAT_TO_BUF(send_buf, R"("time_to_connect_milli": {}, )", dnp3_sys.timings->time_to_connect);
        dnp3_sys.timings->connection_time_mutex.unlock();
        if (sys.protocol_dependencies->who == DNP3_MASTER)
        {
            update_average(dnp3_sys.timings->integrity_responses);
            update_average(dnp3_sys.timings->direct_operates);
            update_average(dnp3_sys.timings->binary_direct_operates);
            update_average(dnp3_sys.timings->analog_direct_operates);
            dnp3_sys.timings->integrity_responses.mutex.lock();
            FORMAT_TO_BUF(send_buf, R"("average_integrity_response_time_milli": {:g}, )", dnp3_sys.timings->integrity_responses.average);
            FORMAT_TO_BUF(send_buf, R"("min_integrity_response_time_milli": {}, )", ((dnp3_sys.timings->integrity_responses.count == 0) ? 0 : dnp3_sys.timings->integrity_responses.min));
            FORMAT_TO_BUF(send_buf, R"("max_integrity_response_time_milli": {}, )", dnp3_sys.timings->integrity_responses.max);
            dnp3_sys.timings->integrity_responses.mutex.unlock();
            dnp3_sys.timings->direct_operates.mutex.lock();
            FORMAT_TO_BUF(send_buf, R"("average_direct_operate_response_time_milli": {:g}, )", dnp3_sys.timings->direct_operates.average);
            FORMAT_TO_BUF(send_buf, R"("min_direct_operate_response_time_milli": {}, )", ((dnp3_sys.timings->direct_operates.count == 0) ? 0 : dnp3_sys.timings->direct_operates.min));
            FORMAT_TO_BUF(send_buf, R"("max_direct_operate_response_time_milli": {} )", dnp3_sys.timings->direct_operates.max);
            dnp3_sys.timings->direct_operates.mutex.unlock();
            dnp3_sys.timings->binary_direct_operates.mutex.lock();
            FORMAT_TO_BUF(send_buf, R"("average_binary_direct_operate_response_time_milli": {:g}, )", dnp3_sys.timings->binary_direct_operates.average);
            FORMAT_TO_BUF(send_buf, R"("min_binary_direct_operate_response_time_milli": {}, )", ((dnp3_sys.timings->binary_direct_operates.count == 0) ? 0 : dnp3_sys.timings->binary_direct_operates.min));
            FORMAT_TO_BUF(send_buf, R"("max_binary_direct_operate_response_time_milli": {} )", dnp3_sys.timings->binary_direct_operates.max);
            dnp3_sys.timings->binary_direct_operates.mutex.unlock();
            dnp3_sys.timings->analog_direct_operates.mutex.lock();
            FORMAT_TO_BUF(send_buf, R"("average_analog_direct_operate_response_time_milli": {:g}, )", dnp3_sys.timings->analog_direct_operates.average);
            FORMAT_TO_BUF(send_buf, R"("min_analog_direct_operate_response_time_milli": {}, )", ((dnp3_sys.timings->analog_direct_operates.count == 0) ? 0 : dnp3_sys.timings->analog_direct_operates.min));
            FORMAT_TO_BUF(send_buf, R"("max_analog_direct_operate_response_time_milli": {} )", dnp3_sys.timings->analog_direct_operates.max);
            dnp3_sys.timings->analog_direct_operates.mutex.unlock();
        }
        else
        {
            update_average(dnp3_sys.timings->unsolicited_responses);
            dnp3_sys.timings->unsolicited_responses.mutex.lock();
            FORMAT_TO_BUF(send_buf, R"("average_unsolicited_response_confirm_time_milli": {:g}, )", dnp3_sys.timings->unsolicited_responses.average);
            FORMAT_TO_BUF(send_buf, R"("min_unsolicited_response_confirm_time_milli": {}, )", ((dnp3_sys.timings->unsolicited_responses.count == 0) ? 0 : dnp3_sys.timings->unsolicited_responses.min));
            FORMAT_TO_BUF(send_buf, R"("max_unsolicited_response_confirm_time_milli": {})", dnp3_sys.timings->unsolicited_responses.max);
            dnp3_sys.timings->unsolicited_responses.mutex.unlock();
        }

        sys.error_mutex.lock();
        FORMAT_TO_BUF(send_buf, R"("fims_message_parse_errors": {}, )", sys.parse_errors);
        FORMAT_TO_BUF(send_buf, R"("point_errors": {}, )", sys.point_errors);
        FORMAT_TO_BUF(send_buf, R"("fims_errors": {},)", sys.fims_errors);
        FORMAT_TO_BUF(send_buf, R"("comms_errors": {},)", sys.comms_errors);
        FORMAT_TO_BUF(send_buf, R"("heartbeat_errors": {},)", sys.heartbeat_errors);
        FORMAT_TO_BUF(send_buf, R"("watchdog_errors": {})", sys.watchdog_errors);
        sys.error_mutex.unlock();

        send_buf.push_back('}');
        if (!send_pub(sys.fims_dependencies->fims_gateway, dnp3_sys.stats_pub_uri, std::string_view{send_buf.data(), send_buf.size()}))
        {
            if (!spam_limit(&sys, sys.fims_errors))
            {
                FPS_ERROR_LOG("%s cannot publish onto fims, exiting", sys.fims_dependencies->name.c_str());
                FPS_LOG_IT("fims_send_error");
            }
            return false;
        }
        // TODO put this in config ( that's done I know ) and allow us to turn it off
        usleep(sys.protocol_dependencies->dnp3.stats_pub_frequency * 1000);
    }
    return true;
}