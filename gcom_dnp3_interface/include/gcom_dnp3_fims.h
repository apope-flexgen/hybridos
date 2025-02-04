#pragma once

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <iostream>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>
#include <typeinfo>
#include <string_view>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fims/fims.h>
#include <fims/libfims.h>
#include <fims/defer.hpp>

extern "C" {
#include "tmwscl/utils/tmwsim.h"
}

#include "simdjson.h"
#include "shared_utils.hpp"
#include "spdlog/fmt/fmt.h"
#include "Jval_buif.hpp"

using namespace std::literals::string_view_literals;

#ifndef DNP3_MASTER
#define DNP3_MASTER 0
#endif

#ifndef DNP3_OUTSTATION
#define DNP3_OUTSTATION 1
#endif

struct GcomSystem;

enum class FimsMethod : u8
{
    Set,
    Get,
    Pub,
    Unknown
};

enum class FimsFormat : u8
{
    Naked,
    Clothed,
    Full
};

enum FimsEventSeverity : uint8_t
{
    Debug = 0,
    Info = 1,
    Status = 2,
    Alarm = 3,
    Fault = 4
};

struct UriRequest
{
    static constexpr auto Reset_Timings_Request_Suffix = "_reset_timings";  // set
    static constexpr auto Reset_Errors_Request_Suffix = "_reset_errors";    // set
    static constexpr auto Reload_Request_Suffix = "_reload";                // set
    static constexpr auto Debug_Suffix = "_debug";                          // set
    static constexpr auto Force_Suffix = "_force";                          // set
    static constexpr auto Unforce_Suffix = "_unforce";                      // set
    static constexpr auto Pulse_Suffix = "_pulse";                          // set
    static constexpr auto Enable_Suffix = "_enable";                        // set
    static constexpr auto Disable_Suffix = "_disable";                      // set

    static constexpr auto Points_Suffix = "_points";            // get
    static constexpr auto Full_Suffix = "_full";                // get
    static constexpr auto Stats_Suffix = "_stats";              // get
    static constexpr auto Raw_Request_Suffix = "_raw";          // get
    static constexpr auto Timings_Request_Suffix = "_timings";  // get
    static constexpr auto Errors_Request_Suffix = "_errors";    // get
    static constexpr auto Config_Suffix = "_config";            // get

    UriRequest() { clear_uri(); };

    void set_uri(std::string_view& uri_view, char* local_uri, int who, FimsMethod method)
    {
        clear_uri();
        contains_local_uri = false;
        if (local_uri != nullptr && uri_view.substr(0, strlen(local_uri)) == std::string_view(local_uri))
        {
            uri_view = uri_view.substr(strlen(local_uri));
            contains_local_uri = true;
        }
        splitUri(uri_view);
        int num = num_uri_frags;

        for (int idx = 0; idx < num; ++idx)
        {
            if (!uri_frags[idx].empty() && uri_frags[idx].front() == '_')
            {
                num_uri_frags--;

                if (method == FimsMethod::Set)
                {
                    if (uri_frags[idx] == Reset_Timings_Request_Suffix)
                    {
                        is_reset_timings_request = true;
                    }
                    else if (uri_frags[idx] == Reset_Errors_Request_Suffix)
                    {
                        is_reset_errors_request = true;
                    }
                    else if (uri_frags[idx] == Reload_Request_Suffix)
                    {
                        is_reload_request = true;
                    }
                    else if (uri_frags[idx] == Debug_Suffix)
                    {
                        is_debug_request = true;
                    }
                    else if (uri_frags[idx] == Force_Suffix && contains_local_uri)
                    {
                        is_force_request = true;
                    }
                    else if (uri_frags[idx] == Unforce_Suffix && contains_local_uri)
                    {
                        is_unforce_request = true;
                    }
                    else if (uri_frags[idx] == Pulse_Suffix)
                    {
                        is_pulse_request = true;
                        const auto uri_len = uri_view.size();
                        const auto request_len = std::string_view{ "/_pulse" }.size();
                        uri_view = uri_view.substr(0, uri_len - request_len);
                    }
                    else if (uri_frags[idx] == Enable_Suffix)
                    {
                        is_enable_request = true;
                    }
                    else if (uri_frags[idx] == Disable_Suffix)
                    {
                        is_disable_request = true;
                    }
                }
                else if (method == FimsMethod::Get)
                {
                    if (uri_frags[idx] == Config_Suffix)
                        is_config_request = true;
                    else if (uri_frags[idx] == Raw_Request_Suffix)
                        is_raw_request = true;
                    else if (uri_frags[idx] == Timings_Request_Suffix)
                        is_timings_request = true;
                    else if (uri_frags[idx] == Errors_Request_Suffix)
                        is_errors_request = true;
                    else if (uri_frags[idx] == Points_Suffix)
                    {
                        is_points_request = true;
                    }
                    else if (uri_frags[idx] == Full_Suffix &&
                             (who == DNP3_MASTER || (contains_local_uri && who == DNP3_OUTSTATION)))
                    {
                        is_full_request = true;
                    }
                    else if (uri_frags[idx] == Stats_Suffix)
                        is_stats_request = true;
                }
            }
        }
        is_request = is_raw_request || is_timings_request || is_errors_request || is_reset_timings_request ||
                     is_reset_errors_request || is_reload_request || is_config_request || is_debug_request ||
                     is_force_request || is_unforce_request || is_points_request || is_stats_request ||
                     is_enable_request ||
                     is_disable_request;  // omit is_full_request because that's handled a little differently
                                          // also omit is_pulse_request because that's also handled differently
    };

    void clear_uri()
    {
        contains_local_uri = false;
        is_raw_request = false;
        is_timings_request = false;
        is_errors_request = false;
        is_reset_timings_request = false;
        is_reset_errors_request = false;
        is_reload_request = false;
        is_config_request = false;
        is_debug_request = false;
        is_force_request = false;
        is_unforce_request = false;
        is_points_request = false;
        is_full_request = false;
        is_stats_request = false;
        is_pulse_request = false;
        is_enable_request = false;
        is_disable_request = false;
        uri_frags.clear();
    }

    // Function to split the URI path and store parts into a vector
    void splitUri(const std::string_view& uri_view)
    {
        size_t start = 0;
        size_t end = uri_view.find('/');

        while (end != std::string_view::npos)
        {
            uri_frags.emplace_back(uri_view.substr(start, end - start));
            start = end + 1;
            end = uri_view.find('/', start);
        }
        if (start < end) {
            uri_frags.emplace_back(uri_view.substr(start, end));
        }
        num_uri_frags = (int)uri_frags.size();
    }

    bool contains_local_uri;
    bool is_raw_request;
    bool is_timings_request;
    bool is_errors_request;
    bool is_reset_timings_request;
    bool is_reset_errors_request;
    bool is_reload_request;
    bool is_config_request;
    bool is_debug_request;
    bool is_force_request;
    bool is_unforce_request;
    bool is_points_request;
    bool is_full_request;
    bool is_stats_request;
    bool is_pulse_request;
    bool is_enable_request;
    bool is_disable_request;
    bool is_request = false;
    int num_uri_frags;

    std::vector<std::string> uri_frags;
};

/**
 * @brief Send a fims pub to a given uri with a given message body.
 *
 * @param fims_gateway a connected fims socket
 * @param uri string view of the uri to send the message to
 * @param body string view of the message body
 *
 * @pre fims_gateway is connected to fims
 */
bool send_pub(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;

/**
 * @brief Send a fims set to a given uri with a given message body.
 *
 * @param fims_gateway a connected fims socket
 * @param uri string view of the uri to send the message to
 * @param body string view of the message body
 *
 * @pre fims_gateway is connected to fims
 */
bool send_set(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;

/**
 * @brief Send a fims post to a given uri with a given message body.
 *
 * @param fims_gateway a connected fims socket
 * @param uri string view of the uri to send the message to
 * @param body string view of the message body
 *
 * @pre fims_gateway is connected to fims
 */
bool send_post(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;

/**
 * @brief Emits an event to /events
 *
 * @param fims_gateway a connected fims socket
 * @param source the process name (source) of the event
 * @param message the message we want to emit with the event
 * @param severity the event severity level (debug, info, status, alarm, or fault)
 *
 * @pre fims_gateway is connected to fims
 */
void emit_event(fims* fims_gateway, const char* source, const char* message, int severity);

/**
 * @brief Initialize the receiver buffer for fims, create a subscription string for the system ID (sys.id),
 * and generate the process name based on whether we are dealing with a client or server.
 *
 * @param sys A partially initialized GcomSystem, in which "system" information has been populated
 * but not necessarily any data points
 *
 * @pre sys.protocol_dependencies->who, sys.fims_dependencies->data_buf_len, and sys.config_file_name have
 * been set appropriately
 */
bool init_fims(GcomSystem& sys);

/**
 * @brief Add a single uri to sys.fims_dependencies->subs
 *
 * @param sys A partially initialized GcomSystem, in which "system" information has been populated
 * but not necessarily any data points
 * @param name The subscription to add. Must include a '/' as the first character.
 */
bool add_fims_sub(GcomSystem& sys, std::string name);

/**
 * @brief Print and log the contents of sys.fims_dependencies->subs.
 *
 * @param sys A partially initialized GcomSystem, in which sys.fims_dependencies->subs has been populated
 */
bool show_fims_subs(GcomSystem& sys);

/**
 * @brief Connect to fims using sys.fims_dependencies->name and subscribe to all uris in
 * sys.fims_dependencies->subs.
 *
 * @param sys a GcomSystem with a valid sys.fims_dependencies->name (no spaces) and a valid vector of uris to
 * subscribe to
 */
bool fims_connect(GcomSystem& sys);

/**
 * @brief Parse the value in a key-value pair in a JSON object.
 *
 * Of the format <value> or "value": <value>
 *
 * @param val_clothed In a key-value pair, val_clothed represents the value as an object. This may or may not be
 * valid based on the particular JSON message. (Either this will be valid or curr_val will be valid.) Example:
 * {"value": 5}
 * @param curr_val In a key-value pair, curr_val represents the value as a raw value. This may or may not be valid
 * based on the particular JSON message. (Either this will be valid or val_clothed will be valid.) Example: 5
 * @param to_set a Jval_buif that will be se to the value in the json object
 *
 * @pre val_clothed and curr_val contain the results from parsing a simdjson doc object down to a key-value pair
 */
bool extractValueMulti(GcomSystem& sys, simdjson::simdjson_result<simdjson::fallback::ondemand::object>& val_clothed,
                       simdjson::fallback::ondemand::value& curr_val, Jval_buif& to_set, std::string_view key);

/**
 * @brief Parse the value in a single-item JSON message
 *
 * Of the format <value> or {"value": <value>}
 *
 * @param sys GcomSystem with sys.fims_dependencies->doc and pre-parsed sys.fims_dependencies->uri_view
 * @param val_clothed In a single-value message, val_clothed represents the value as an object. This may or may not
 * be valid based on the particular JSON message. (Either this will be valid or curr_val will be valid.) Example:
 * {"value": 5}
 * @param curr_val In a key-value pair, curr_val represents the value as a raw value. This may or may not be valid
 * based on the particular JSON message. (Either this will be valid or val_clothed will be valid.) Example: 5
 * @param to_set a Jval_buif that will be se to the value in the json object
 *
 * @pre val_clothed and curr_val contain the results from parsing a simdjson doc object down to a key-value pair
 */
bool extractValueSingle(GcomSystem& sys, simdjson::simdjson_result<simdjson::fallback::ondemand::object>& val_clothed,
                        simdjson::fallback::ondemand::value& curr_val, Jval_buif& to_set);

/**
 * @brief Convert a Jval_buif to a double, regardless of the subtype.
 *
 * @param to_set the Jval_buif to convert to a double
 */
double jval_to_double(Jval_buif& to_set);

/**
 * @brief Parse header data for an incoming fims message, where data is stored in data_buf.
 *
 * Extract method, uri, reply_to, process_name, username, and message body for an incoming fims message.
 * Look for any request headers that will be processed later in processCmds. Catch any basic errors that
 * result from looking at the request header (such as a "get" with an empty reply-to or an invalid "set"
 * request to an outstation).
 *
 * @param sys a partially initialized GcomSystem (with at least base_uri, local_uri, and "who" initialized)
 * @param meta_data a fims Meta_Data_Info object from an incoming fims message
 * @param data_buf the full data buffer received over fims
 * @param data_buf_len the full length of the received data
 */
bool parseHeader(GcomSystem& sys, Meta_Data_Info& meta_data, char* data_buf, uint32_t data_buf_len);
bool processCmds(GcomSystem& sys, Meta_Data_Info& meta_data);
bool gcom_recv_raw_message(fims& fims_gateway, Meta_Data_Info& meta_data, void* data_buf,
                           uint32_t data_buf_len) noexcept;
bool listener_thread(GcomSystem& sys) noexcept;

// 1 = multi, 2 = single, 3 = multi output status, 4 = single output status
int getUriType(GcomSystem& sys, std::string_view uri);
void replyToFullGet(GcomSystem& sys, fmt::memory_buffer& send_buf);

/**
 * @brief Format a naked TMWSIM_POINT value to a pre-initialized memory buffer.
 *
 * Format an analog value divided by its scale using a %g flag. Format a binary value
 * based on its crob_int and crob_string values. If it's a crob_int, output the value as 0 or 1.
 * If it's a crob_string, output the value using the specified crob_true and crob_false strings.
 * If it's neither crob_int or crob_string, then output the binary value as true or false. If the
 * scale is negative for a binary value, invert the value (true becomes false and vice versa).
 *
 * @param send_buf fmt::memory_buffer to store the output string
 * @param dbPoint TMWSIM_POINT * to a pre-initialized dbPoint
 */
void formatPointValue(fmt::memory_buffer& send_buf, TMWSIM_POINT* dbPoint);

/**
 * @brief Generate the message body for the reply to a fims-get on one of the uris
 * of the GcomSystem, depending on if it's a multi-point or single-point uri.
 *
 * @param sys a fully-initilized GcomSystem with an active TMW session
 * @param send_buf fmt::memory_buffer to store the message body output string
 */
void replyToGet(GcomSystem& sys, fmt::memory_buffer& send_buf);