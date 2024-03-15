#include <chrono>
#include <thread>
#include <sys/uio.h> // for receive timouet calls
#include <sys/socket.h> // for receive timouet calls

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h>	//hostent
#include<arpa/inet.h>

#include "spdlog/details/fmt_helper.h" // for microseconds formattting
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/chrono.h"

#include "simdjson_noexcept.hpp"

#include "rigtorp/SPSCQueue.h"
#include "modbus/modbus.h"

#include "fims/defer.hpp"
#include "Jval_buif.hpp"
#include "tscns.h"

#include "fims/libfims.h"

#include "config_loaders/client_config_loader.hpp"
#include "client/client_structs.hpp"

using namespace std::chrono_literals;
using namespace std::string_view_literals;

// constants:
// TODO(WALKER): "broken pipe?" -> larger integer? (will have to investigate)
// static constexpr auto Modbus_Errno_Disconnect   = 104; // "Connection reset by peer" -> disconnect
// static constexpr auto Modbus_Errno_Cant_Connect = 115; // "Operation now in progress" -> no connection
static constexpr auto Modbus_Server_Busy = 10126; // "Resource temporarily unavailable" -> server busy

// helper macros:
#define FORMAT_TO_BUF(fmt_buf, fmt_str, ...)            fmt::format_to(std::back_inserter(fmt_buf), FMT_COMPILE(fmt_str), ##__VA_ARGS__)
#define FORMAT_TO_BUF_NO_COMPILE(fmt_buf, fmt_str, ...) fmt::format_to(std::back_inserter(fmt_buf), fmt_str, ##__VA_ARGS__)

// NOTE(WALKER): this is taken from spdlog's library credit goes to them
// return fraction of a second of the given time_point.
// e.g.
// fraction<std::milliseconds>(tp) -> will return the millis part of the second
template<typename ToDuration>
static inline ToDuration time_fraction(std::chrono::system_clock::time_point tp)
{
    using std::chrono::duration_cast;
    using std::chrono::seconds;
    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

// Helper function straight from C++20 so we can use it here in C++17:
// for checking for suffix's over fims (like a raw get request)
static constexpr bool str_ends_with(std::string_view str, std::string_view suffix) noexcept
{
	const auto str_len = str.size();
	const auto suffix_len = suffix.size();
	return str_len >= suffix_len 
                && std::string_view::traits_type::compare(str.end() - suffix_len, suffix.data(), suffix_len) == 0;
}

// fims helper functions:
static bool send_pub(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"pub", sizeof("pub") - 1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}
static bool send_set(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"set", sizeof("set") - 1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}
// NOTE(WALKER): This is only for emit_event really (not used anywhere else)
static bool send_post(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"post", sizeof("post") -1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}

// NOTE(WALKER): use these in combination with send_event for proper severity levels
enum Event_Severity : uint8_t
{
    Debug = 0,
    Info = 1,
    Status = 2,
    Alarm = 3,
    Fault = 4
};
// [DEBUG, INFO, STATUS, ALARM, FAULT] 0..4 (from Kyle on slack thread)

// emits an event to /events
template<std::size_t Size, typename... Fmt_Args>
static bool emit_event(fmt::basic_memory_buffer<char, Size>& send_buf, fims& fims_gateway, std::string_view source, Event_Severity severity, fmt::format_string<Fmt_Args...> fmt_str, Fmt_Args&&... fmt_args) noexcept
{
    send_buf.clear();

    FORMAT_TO_BUF(send_buf, R"({{"source":"{}","message":")", source);
    FORMAT_TO_BUF_NO_COMPILE(send_buf, fmt_str, std::forward<Fmt_Args>(fmt_args) ...);
    FORMAT_TO_BUF(send_buf, R"(","severity":{}}})", severity);

    return send_post(fims_gateway, "/events"sv, std::string_view{send_buf.data(), send_buf.size()});
}

// Helper function for acquiring a locked connection (tries every single connection in the pool)
// will stop at the original connection if it can't acquire another slot
static Mod_Conn& acquire_locked_conn(Ctx_Pool& ctx_pool, const std::size_t client_thread_idx)
{
    for (u8 i = 0; i < ctx_pool.num_conns; ++i)
    {
        auto& the_conn = ctx_pool.pool[(client_thread_idx + i) % ctx_pool.num_conns];
        if (the_conn.lock.try_lock()) return the_conn; // we have acquired a lock (could be beyond the original assignment), use that one
    }
    auto& the_conn = ctx_pool.pool[client_thread_idx % ctx_pool.num_conns];
    the_conn.lock.lock(); // we have exhausted all options, return the original one you were assigned
    return the_conn;
}

// Client event source constant:
static constexpr auto Client_Event_Source = "Modbus Client"sv;

// Global workspace (all functions will use this for client):
Main_Workspace main_workspace;

enum class Arg_Types : u8
{
    Error,
    Help,
    File,
    Uri, // for a "get" over fims
    Expand
};

static std::pair<Arg_Types, std::string> parse_command_line_arguments(const int argc, const char* argv[]) noexcept
{
    static constexpr std::size_t Max_Arg_Size = std::numeric_limits<u8>::max(); // at most 255 characters for argument paths
    static constexpr auto Modbus_Client_Help_String = 
R"(
Modbus Client Usage:

-h, --help:   print out this help string
-f, --file:   provide a json file to run Modbus Client with directly (default flag)
-u, --uri:    provide a json file uri over fims (without extension) to run Modbus Client with using a fims get command
-e, --expand: provide a json file for Modbus Client to parse through and produce another file expanded (without ranges and templating)

NOTE: no flag is the same as -f/--file (simply provide the json file to parse)

examples:

help:
/path/to/modbus_client -h
or
/path/to/modbus_client --help

file:
/path/to/modbus_client /path/to/some_json.json
or
/path/to/modbus_client -f /path/to/some_json.json
or
/path/to/modbus_client --file /path/to/some_json.json

uri:
/path/to/modbus_client -u /fims/uri/to/get
or
/path/to/modbus_client --uri /fims/uri/to/get

expand:
/path/to/modbus_client -e /path/to/json_file_to_expand.json
or
/path/to/modbus_client --expand /path/to/json_file_to_expand.json
)"sv;

    std::pair<Arg_Types, std::string> args; // error by default

    if (argc <= 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        args.first = Arg_Types::Help;
        NEW_FPS_ERROR_PRINT_NO_ARGS(Modbus_Client_Help_String);
    }
    else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0)
    {
        if (argc >= 2 && argv[2] && strlen(argv[2]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::File;
            args.second = argv[2];
        }
        else
        {
            NEW_FPS_ERROR_PRINT("error in --file: json file not provided, or json file path was more than {} characters\n", Max_Arg_Size);
        }
    }
    else if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--uri") == 0)
    {
        if (argc >= 2 && argv[2] && strlen(argv[2]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::Uri;
            args.second = argv[2];
        }
        else
        {
            NEW_FPS_ERROR_PRINT("error in --uri: fims json path not provided, or fims json path was more than {} characters\n", Max_Arg_Size);
        }
    }
    else if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--expand") == 0)
    {
        if (argc >= 2 && argv[2] && strlen(argv[2]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::Expand;
            args.second = argv[2];
        }
        else
        {
            NEW_FPS_ERROR_PRINT("error in --expand: json file path not provided, or json file path was more than {} characters\n", Max_Arg_Size);
        }
    }
    else // normal file (this might change to dbi in the future by default who knows):
    {
        if (strlen(argv[1]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::File;
            args.second = argv[1];
        }
        else
        {
            NEW_FPS_ERROR_PRINT("error in --file: json file path was more than {} characters\n", Max_Arg_Size);
        }
    }

    // prune the extensions of the input file then append .json
    // this way input extension doesn't matter (it can be optional or messed up by accident)
    if (args.first == Arg_Types::File || args.first == Arg_Types::Expand)
    {
        const auto first_extension_index = args.second.find_first_of('.');
        if (first_extension_index != args.second.npos) args.second.resize(first_extension_index);
        args.second.append(".json");
    }

    return args;
}

static bool load_config(const std::pair<Arg_Types, std::string>& args) noexcept
{
    if (args.first == Arg_Types::Error || args.first == Arg_Types::Help) return false; // shouldn't reach this point

    config_loader::Client_Configs_Array configs_array;
    config_loader::Error_Location err_loc;

    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;

    simdjson::simdjson_result<simdjson::padded_string> json;
    simdjson::ondemand::object config_data_obj;
    simdjson::ondemand::array config_data_array; // this is unused (but might be in the future, who knows)

    if (args.first == Arg_Types::File || args.first == Arg_Types::Expand)
    {
        json = simdjson::padded_string::load(args.second);
    }
    else if (args.first == Arg_Types::Uri)
    {
        if (args.second.front() != '/')
        {
            NEW_FPS_ERROR_PRINT("For client with init uri \"{}\": the uri does not begin with `/`\n", args.second);
            return false;
        }
        fims fims_gateway;
        const auto conn_id_str = fmt::format("modbus_client_uri_init@{}", args.second);
        if (!fims_gateway.Connect(conn_id_str.data()))
        {
            NEW_FPS_ERROR_PRINT("For client with init uri \"{}\": could not connect to fims_server\n", args.second);
            return false;
        }
        const auto sub_string = fmt::format("/modbus_client_uri_init{}", args.second);
        if (!fims_gateway.Subscribe(std::vector<std::string>{sub_string}))
        {
            NEW_FPS_ERROR_PRINT("For client with init uri \"{}\": failed to subscribe for uri init\n", args.second);
            return false;
        }
        if (!fims_gateway.Send(fims::str_view{"get", sizeof("get") - 1}, fims::str_view{args.second.data(), args.second.size()}, fims::str_view{sub_string.data(), sub_string.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}))
        {
            NEW_FPS_ERROR_PRINT("For client with inti uri \"{}\": failed to send a fims get message\n", args.second);
            return false;
        }
        auto config_msg = fims_gateway.Receive_Timeout(5000000); // give them 5 seconds to respond before erroring
        defer { fims::free_message(config_msg); };
        if (!config_msg)
        {
            NEW_FPS_ERROR_PRINT("For client with init uri \"{}\": failed to receive a message in 5 seconds\n", args.second);
            return false;
        }
        if (!config_msg->body)
        {
            NEW_FPS_ERROR_PRINT("For client with init uri \"{}\": message was received, but body doesn't exist\n", args.second);
            return false;
        }
        json = simdjson::padded_string{std::string{config_msg->body}};
    }

    if (const auto err = parser.iterate(json).get(doc); err)
    {
        NEW_FPS_ERROR_PRINT("error parsing config json \"{}\", err = {}\n", args.second, simdjson::error_message(err));
        return false;
    }

    if (const auto err = doc.get(config_data_obj); err)
    {
        NEW_FPS_ERROR_PRINT("error parsing config json \"{}\" as an object, err = {}\n", args.second, simdjson::error_message(err));
        return false;
    }

    if(!configs_array.load(config_data_obj, err_loc))
    {
        NEW_FPS_ERROR_PRINT("{}\n", err_loc);
        return false;
    }

    if (args.first == Arg_Types::Expand)
    {
        fmt::print("{}\n", configs_array);
        return true;
    }

    if (!initialize_main_workspace_from_config(configs_array, main_workspace)) return false;

    // finish setup for each client (fims, modbus, heartbeat_timeout check, etc.):
    bool has_single_connection = false;
    fmt::memory_buffer str_buf;
    for (u8 client_idx = 0; client_idx < main_workspace.num_clients; ++client_idx)
    {
        str_buf.clear();
        auto& client_workspace = *main_workspace.client_workspaces[client_idx];
        const auto client_name = main_workspace.string_storage.get_str(client_workspace.conn_workspace.conn_info.name);
        FORMAT_TO_BUF(str_buf, "modbus_client@{}\0", client_name);
        if (!client_workspace.fims_gateway.Connect(str_buf.data()))
        {
            NEW_FPS_ERROR_PRINT("For client \"{}\": could not connect to fims_server\n", client_name);
            return false;
        }
        std::vector<std::string> sub_array; // sub array for this client
        for (const auto& comp : configs_array.configs[client_idx].components)
        {
            str_buf.clear();
            FORMAT_TO_BUF(str_buf, "{}{}", Main_Uri_Frag, comp.id);
            sub_array.emplace_back(std::string{str_buf.data(), str_buf.size()});
        }
        if (!client_workspace.fims_gateway.Subscribe(sub_array))
        {
            NEW_FPS_ERROR_PRINT("For client \"{}\": could not subscribe to component's uris\n", client_name);
            return false;
        }

        // connect modbus contexts (no retries, don't bother, systemctl will restart it):
        const auto has_connection = client_workspace.startup_modbus_conns(main_workspace.string_storage);
        has_single_connection = has_single_connection || has_connection;
        if (has_connection)
        {
            NEW_FPS_ERROR_PRINT("client \"{}\": Connected to server with {} connection(s)\n", client_name, client_workspace.conn_workspace.ctx_pool.num_conns);
            // emit_event about that client connecting:
            if (!emit_event(str_buf, client_workspace.fims_gateway, Client_Event_Source, Event_Severity::Info, R"(client \"{}\": Modbus connection established with {} total connections)", client_name, client_workspace.conn_workspace.ctx_pool.num_conns))
            {
                NEW_FPS_ERROR_PRINT("client \"{}\": Cannot send an event out on fims. Exiting\n", client_name);
                return false;
            }
        }
    }

    // Must have at least one client that could connect successfully before continuing the program:
    if (!has_single_connection)
    {
        NEW_FPS_ERROR_PRINT_NO_ARGS("Could not establish a single connection for any client, exiting\n");
        return false;
    }

    // after connection checks:
    for (u8 client_idx = 0; client_idx < main_workspace.num_clients; ++client_idx)
    {
        auto& client_workspace = *main_workspace.client_workspaces[client_idx];
        const auto client_name = main_workspace.string_storage.get_str(client_workspace.conn_workspace.conn_info.name);
        for (const auto& comp : configs_array.configs[client_idx].components)
        {
            // check each component_heartbeat_timeout_ms to make sure it's at least 2x frequency, no exit on error (just an event and print out warning):
            if (comp.heartbeat_enabled && comp.heartbeat_timeout < comp.frequency * 2)
            {
                NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": \"modbus_heartbeat_timeout_ms\" (currently: {}) is less than 2x frequency. This could cause false timeout issues, consider changing it.\n", client_name, comp.id, comp.heartbeat_timeout);
                if (!emit_event(str_buf, client_workspace.fims_gateway, Client_Event_Source, Event_Severity::Alarm, R"(client \"{}\", component \"{}\": \"modbus_heartbeat_timeout_ms\" (currently: {}) is less than 2x frequency. This could cause false timeout issues, consider changing it.)", client_name, comp.id, comp.heartbeat_timeout))
                {
                    NEW_FPS_ERROR_PRINT("client \"{}\": Cannot send an event out on fims. Exiting\n", client_name);
                    return false;
                }
            }
        }
    }

    return true;
}

static bool listener_thread(const u8 client_idx) noexcept
{
    // cleanup/shutdown -> tell main that something is wrong (so it can shut everything down)
    // if we return from this function then something went wrong (hence the "cleanup" defer)
    defer {
        main_workspace.start_signal = false;
        main_workspace.main_cond.notify_one();
    };

    // constants:
    static constexpr auto Raw_Request_Suffix           = "/_raw"sv;
    static constexpr auto Timings_Request_Suffix       = "/_timings"sv;
    static constexpr auto Reset_Timings_Request_Suffix = "/_reset_timings"sv;
    static constexpr auto Reload_Request_Suffix        = "/_reload"sv;
    // NOTE(WALKER): an "x_request" is a "get" request that ends the get "uri" in one of the above suffixes
    // for example:
    // get on uri : "/components/bms_info"      -> regular
    // raw get uri: "/components/bms_info/_raw" -> raw_request
    // NOTE(WALKER): a "reload" request is a "set" request that ends with /_reload (this reloads the WHOLE client, NOT just that particular component)

    // method types supported by modbus_client:
    enum class Method_Types : u8
    {
        Set,
        Get
    };

    // listener variables:
    auto& my_workspace     = *main_workspace.client_workspaces[client_idx];
    const auto client_name = main_workspace.string_storage.get_str(my_workspace.conn_workspace.conn_info.name);
    const auto connection_timeout = my_workspace.conn_workspace.conn_info.connection_timeout;

    auto& fims_gateway     = my_workspace.fims_gateway;
    auto& receiver_bufs    = my_workspace.receiver_bufs;
    auto& meta_data        = receiver_bufs.meta_data;
    auto& parser           = my_workspace.parser;
    auto& doc              = my_workspace.doc;

    // wait until main signals everyone to start:
    {
        std::unique_lock<std::mutex> lk{main_workspace.main_mutex};
        main_workspace.main_cond.wait(lk, [&]() {
            return main_workspace.start_signal.load();
        });
    }

    // setup the timeout to be 2 seconds (so we can stop listener thread without it spinning infinitely on errors):
    struct timeval tv;

    tv.tv_sec  = 2;
    tv.tv_usec = 0;
    if (setsockopt(fims_gateway.get_socket(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) == -1)
    {
        NEW_FPS_ERROR_PRINT("listener for \"{}\": could not set socket timeout to 2 seconds. Exiting\n", client_name);
        return false;
    }

    if(connection_timeout != 0.5)
        NEW_FPS_ERROR_PRINT("listener for \"{}\": set modbus connection timeout to {} seconds\n", client_name, connection_timeout);
    // main loop:
    while (my_workspace.keep_running)
    {
        if (!recv_raw_message(fims_gateway.get_socket(), receiver_bufs))
        {
            const auto curr_errno = errno;
            if (curr_errno == EAGAIN || curr_errno == EWOULDBLOCK) continue; // we just timed out
            // This is if we have a legitimate error (no timeout):
            NEW_FPS_ERROR_PRINT("Listener for \"{}\": could not receive message over fims. Exiting\n", client_name);
            return false;
        }

        // meta data views:
        const auto method_view       = std::string_view{receiver_bufs.get_method_data(), meta_data.method_len};
        auto uri_view                = std::string_view{receiver_bufs.get_uri_data(), meta_data.uri_len};
        const auto replyto_view      = std::string_view{receiver_bufs.get_replyto_data(), meta_data.replyto_len};
        const auto process_name_view = std::string_view{receiver_bufs.get_process_name_data(), meta_data.process_name_len};
        // const auto user_name_view    = std::string_view{receiver_bufs.get_user_name_data(), meta_data.user_name_len};

        Method_Types method;

        // method check:
        if (method_view == "set")
        {
            method = Method_Types::Set;
        }
        else if (method_view == "get")
        {
            method = Method_Types::Get;
        }
        else // method not supported by modbus_client
        {
            NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": method \"{}\" is not supported by modbus_client. Message dropped\n", client_name, process_name_view, method_view);
            if (!replyto_view.empty())
            {
                static constexpr auto err_str = "Modbus Client -> method not supported"sv;
                if (!send_set(fims_gateway, replyto_view, err_str))
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": could not send replyto fims message. Exiting\n", client_name, process_name_view);
                    return false;
                }
            }
            continue;
        }

        // /_x request checks:
        const bool is_raw_request           = str_ends_with(uri_view, Raw_Request_Suffix);
        const bool is_timings_request       = str_ends_with(uri_view, Timings_Request_Suffix);
        const bool is_reset_timings_request = str_ends_with(uri_view, Reset_Timings_Request_Suffix);
        const bool is_reload_request        = str_ends_with(uri_view, Reload_Request_Suffix);
        if (is_raw_request)           uri_view.remove_suffix(Raw_Request_Suffix.size());
        if (is_timings_request)       uri_view.remove_suffix(Timings_Request_Suffix.size());
        if (is_reset_timings_request) uri_view.remove_suffix(Reset_Timings_Request_Suffix.size());
        if (is_reload_request)        uri_view.remove_suffix(Reload_Request_Suffix.size());

        // uri check:
        uri_view.remove_prefix(Main_Uri_Frag_Length); // removes /components/ from the uri for processing (hashtable uri lookup)
        const auto uri_it = my_workspace.uri_map.find(uri_map_hash(uri_view));
        if (uri_it == my_workspace.uri_map.end())
        {
            NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": uri \"{}\" could not be found. Message dropped\n", client_name, process_name_view, uri_view);
            if (!replyto_view.empty())
            {
                static constexpr auto err_str = "\"Modbus Client -> uri doesn't exist\""sv;
                if (!send_set(fims_gateway, replyto_view, err_str))
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": could not send replyto fims message. Exiting\n", client_name, process_name_view, uri_view);
                    return false;
                }
            }
            continue;
        }
        const auto& base_uri_info = uri_it->second;
        auto& msg_staging_area = my_workspace.msg_staging_areas[base_uri_info.component_idx];

        if (is_reload_request)
        {
            // POTENTIAL TODO(WALKER): do these need to respond to "replyto"? (for now no, seems kinda redundant)
            if (method != Method_Types::Set)
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": reload request method must be \"set\"\n", client_name, process_name_view);
                continue;
            }
            else
            {
                NEW_FPS_ERROR_PRINT_NO_ARGS("reload request received, reloading modbus_client\n");
                main_workspace.reload = true;
                return true;
            }
        }

        // do logic based on uri type and multi/single uri type
        if (method == Method_Types::Set)
        {
            msg_staging_area.set_send_mask.reset(); // clear all sets

            // defer replyto so we don't have to have bloated if/else logic everywhere:
            bool set_had_errors = false;
            std::string_view success_str;
            defer {
                static constexpr auto err_str = "\"Modbus Client -> set had errors\""sv;
                if (!replyto_view.empty())
                {
                    bool fims_ok = true;
                    if (!set_had_errors && !send_set(fims_gateway, replyto_view, success_str))
                    {
                        fims_ok = false;
                    }
                    else if (set_had_errors && !send_set(fims_gateway, replyto_view, err_str))
                    {
                        fims_ok = false;
                    }
                    if (!fims_ok)
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": error sending replyto to uri \"{}\" on fims.\n", client_name, process_name_view, uri_view);
                        // no return, but next receive() call will error out at the top
                    }
                }
            };

            // no raw/timings requests for set:
            if (is_raw_request || is_timings_request || is_reset_timings_request)
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": sets are not accepted on raw/timings request uris.\n", client_name, process_name_view);
                set_had_errors = true;
                continue;
            }

            // got a timeoutmore data than expected (right now this is 10,000):
            if (meta_data.data_len > receiver_bufs.get_max_expected_data_len())
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": got a timeouta \"set\" request but got a timeout{} bytes even though the maximum expected for modbus_client is {}. Please reduce message sizes or increase the maximum expected for modbus_client. Message dropped\n", client_name, process_name_view, meta_data.data_len, receiver_bufs.get_max_expected_data_len());
                set_had_errors = true;
                continue;
            }

            // "data" must exist (decrypt buff and free with defer)
            void* data = nullptr;
            defer { if (data && fims_gateway.has_aes_encryption()) free(data); };
            data = decrypt_buf(receiver_bufs);

            if (!data)
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with base uri \"{}\": got a set request with no body. Message dropped\n", client_name, process_name_view, uri_view);
                set_had_errors = true;
                continue;
            }

            // set success_str for replyto defer:
            success_str = std::string_view{reinterpret_cast<const char*>(data), meta_data.data_len};

            // if we don't have encryption set padding bytes inside buffer to '\0' so we can use a view when parsing
            if (!fims_gateway.has_aes_encryption())
            {
                memset(reinterpret_cast<u8*>(data) + meta_data.data_len, '\0', Simdj_Padding);
            }
            // NOTE(WALKER): "parser" holds the iterator of the json string and the doc gives you "handles" back to parse the underlying strings
            if (const auto err = parser.iterate(reinterpret_cast<const char*>(data), meta_data.data_len, meta_data.data_len + Simdj_Padding).get(doc); err)
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with base uri \"{}\": could not parse json string from set request, err = {} Message dropped\n", client_name, process_name_view, uri_view, simdjson::error_message(err));
                set_had_errors = true;
                continue;
            }

            // now set onto channels based on multi or single set uri:
            Jval_buif to_set;
            if (base_uri_info.decode_idx == Decode_All_Idx) // multi-set
            {
                auto& uri_buf = my_workspace.uri_buf;

                simdjson::ondemand::object set_obj;
                if (const auto err = doc.get(set_obj); err)
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with base uri \"{}\": could not get multi-set fims message as a json object, err = {} Message dropped\n", client_name, process_name_view, uri_view, simdjson::error_message(err));
                    set_had_errors = true;
                    continue;
                }

                bool inner_json_it_err = false; // if we have a "parser" error during object iteration we "break" 
                // iterate over object and extract out values (clothed or unclothed):
                for (auto pair : set_obj)
                {
                    const auto key = pair.unescaped_key();
                    if (const auto err = key.error(); err)
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with base uri \"{}\": parsing error on getting a multi-set key, err = {} Message dropped\n", client_name, process_name_view, uri_view, simdjson::error_message(err));
                        inner_json_it_err = true;
                        break;
                    }
                    const auto key_view = key.value_unsafe();
                    auto val = pair.value();
                    if (const auto err = val.error(); err)
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with base uri \"{}\", on multi-set key \"{}\": parse error on getting value, err = {} Message dropped\n", client_name, process_name_view, uri_view, key_view, simdjson::error_message(err));
                        inner_json_it_err = true;
                        break;
                    }

                    uri_buf.clear();
                    FORMAT_TO_BUF(uri_buf, "{}/{}", uri_view, key_view);
                    const auto curr_set_uri_view = std::string_view{uri_buf.data(), uri_buf.size()};

                    // uri check:
                    const auto inner_uri_it = my_workspace.uri_map.find(uri_map_hash(curr_set_uri_view));
                    if (inner_uri_it == my_workspace.uri_map.end())
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": multi-set uri \"{}\" could not be found. Skipping that set\n", client_name, process_name_view, curr_set_uri_view);
                        continue;
                    }
                    const auto& inner_uri_info = inner_uri_it->second;

                    // reg type check:
                    if (inner_uri_info.reg_type == Register_Types::Input || inner_uri_info.reg_type == Register_Types::Discrete_Input)
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with multi-set uri \"{}\": This uri is of a register type that does not accept set requests. Skipping this uri's set\n", client_name, process_name_view, curr_set_uri_view);
                        continue;
                    }

                    // extract out value (clothed or unclothed):
                    auto curr_val = val.value_unsafe();
                    auto val_clothed = curr_val.get_object();
                    if (const auto err = val_clothed.error(); !(err == simdjson::error_code::SUCCESS || err == simdjson::error_code::INCORRECT_TYPE))
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with multi-set uri \"{}\": could not get a clothed value as an object while parsing a multi-set, err = {} Message dropped\n", client_name, process_name_view, curr_set_uri_view, simdjson::error_message(err));
                        inner_json_it_err = true;
                        break;
                    }
                    if (!val_clothed.error())
                    {
                        auto inner_val = val_clothed.find_field("value");
                        if (const auto err = inner_val.error(); err)
                        {
                            NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with multi-set uri \"{}\": could not get the clothed key \"value\" while parsing a multi-set, err = {} skipping this uri's set\n", client_name, process_name_view, curr_set_uri_view, simdjson::error_message(err));
                            continue;
                        }
                        curr_val = std::move(inner_val.value_unsafe());
                    }

                    // extract out value based on type into Jval:
                    if (auto val_uint = curr_val.get_uint64(); !val_uint.error())
                    {
                        to_set = val_uint.value_unsafe();
                    }
                    else if (auto val_int = curr_val.get_int64(); !val_int.error())
                    {
                        to_set = val_int.value_unsafe();
                    }
                    else if (auto val_float = curr_val.get_double(); !val_float.error())
                    {
                        to_set = val_float.value_unsafe();
                    }
                    else if (auto val_bool = curr_val.get_bool(); !val_bool.error())
                    {
                        to_set = static_cast<u64>(val_bool.value_unsafe()); // just set booleans equal to the whole numbers 1/0 for sets
                    }
                    else
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with multi-set uri \"{}\": only floats, uints, ints, and bools are supported for sets, skipping this uri's set\n", client_name, process_name_view, curr_set_uri_view);
                        continue;
                    }

                    // check for true/false and 1/0 for coils and individual_bits:
                    if (inner_uri_info.reg_type == Register_Types::Coil || inner_uri_info.flags.is_individual_bit())
                    {
                        if (!to_set.holds_uint() || !(to_set.get_uint_unsafe() == 1UL || to_set.get_uint_unsafe() == 0UL))
                        {
                            NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with multi-set uri \"{}\": cannot set Coil or individual_bit uri to the value: {}, only true/false and 1/0 are accepted, skipping this uri's set\n", client_name, process_name_view, curr_set_uri_view, to_set);
                            continue;
                        }
                    }

                    msg_staging_area.set_send_mask[inner_uri_info.thread_idx] = true;
                    msg_staging_area.set_msgs[inner_uri_info.thread_idx].set_vals.emplace_back(Set_Info{inner_uri_info.decode_idx, inner_uri_info.bit_idx, inner_uri_info.enum_idx, to_set});
                }
                if (inner_json_it_err)
                {
                    // If we have any sets that we setup then we need to clear them out so they don't linger the next time:
                    if (msg_staging_area.set_send_mask.any())
                    {
                        for (u8 thread_idx = 0; thread_idx < msg_staging_area.num_threads; ++thread_idx)
                        {
                            if (msg_staging_area.set_send_mask[thread_idx])
                            {
                                msg_staging_area.set_msgs[thread_idx].set_vals.clear();
                            }
                        }
                    }
                    set_had_errors = true;
                    continue;
                }

                // This means that all of the sets in this set had an error (but not a parsing error):
                // for example, all of them tried to set on "Input" registers
                if (msg_staging_area.set_send_mask.none())
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": all of the sets for this message had logical errors in one way or another (for example: all the set uris were \"Input\" registers). These are NOT parsing errors. Message dropped\n", client_name, process_name_view);
                    set_had_errors = true;
                    continue;
                }

                for (u8 i = 0; i < msg_staging_area.num_threads; ++i)
                {
                    if (msg_staging_area.set_send_mask[i])
                    {
                        auto& thread_workspace = *msg_staging_area.comp_workspace->decoded_caches[i].thread_workspace;
                        auto& to_set_chan = *thread_workspace.set_channel;
                        if (!to_set_chan.q.try_emplace(std::move(msg_staging_area.set_msgs[i])))
                        {
                            NEW_FPS_ERROR_PRINT_NO_ARGS("could not push a set message onto a channel, things are really backed up. This should NEVER happpen, shutting the whole thing down\n");
                            return false;
                        }
                        // now wakeup the thread (lock mutex just in case to prevent race condition):
                        {
                            std::lock_guard<std::mutex> lk{thread_workspace.worker_mutex};
                        }
                        thread_workspace.worker_cond.notify_one(); // notify the worker thread about work
                    }
                }
            }
            else // single-set:
            {
                // reg type check:
                if (base_uri_info.reg_type == Register_Types::Input || base_uri_info.reg_type == Register_Types::Discrete_Input)
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with single-set uri \"{}\": This uri is of a register type that does not accept set requests. Message dropped\n", client_name, process_name_view, uri_view);
                    set_had_errors = true;
                    continue;
                }

                simdjson::ondemand::value curr_val;
                auto val_clothed = doc.get_object();
                if (const auto err = val_clothed.error(); !(err == simdjson::error_code::SUCCESS || err == simdjson::error_code::INCORRECT_TYPE))
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with single-set uri \"{}\": cannot check if set message is clothed in an object, err = {} Message dropped\n", client_name, process_name_view, uri_view, simdjson::error_message(err));
                    set_had_errors = true;
                    continue;
                }
                if (!val_clothed.error()) // they sent a clothed value:
                {
                    auto inner_val = val_clothed.find_field("value");
                    if (const auto err = inner_val.error(); err)
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with single-set uri \"{}\": could not get \"value\" as a key inside clothed object, err = {} Message dropped\n", client_name, process_name_view, uri_view, simdjson::error_message(err));
                        set_had_errors = true;
                        continue;
                    }
                    curr_val = std::move(inner_val.value_unsafe());
                }

                // NOTE(WALKER): This logic has to be like this instead of like the multi set
                // because simdjson has a special case where if the json string itself is just a scalar
                // then you can't use doc.get_value() (will return an error code). So this is custom tailored 
                // to take that into account. Do NOT try and duplicate the multi set logic for this.
                if (auto val_uint = val_clothed.error() ? doc.get_uint64() : curr_val.get_uint64(); !val_uint.error()) // it is an unsigned integer
                {
                    to_set = val_uint.value_unsafe();
                }
                else if (auto val_int = val_clothed.error() ? doc.get_int64() : curr_val.get_int64(); !val_int.error()) // it is an integer
                {
                    to_set = val_int.value_unsafe();
                }
                else if (auto val_float = val_clothed.error() ? doc.get_double() : curr_val.get_double(); !val_float.error()) // it is a float
                {
                    to_set = val_float.value_unsafe();
                }
                else if (auto val_bool = val_clothed.error() ? doc.get_bool() : curr_val.get_bool(); !val_bool.error()) // it is a bool
                {
                    to_set = static_cast<u64>(val_bool.value_unsafe()); // just set booleans to the whole numbers 1/0
                }
                else // unsupported type:
                {
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with single-set uri \"{}\": only floats, uints, ints, and bools are supported for sets. Message dropped\n", client_name, process_name_view, uri_view);
                    set_had_errors = true;
                    continue;
                }

                // check for true/false and 1/0 for coils and individual_bits:
                if (base_uri_info.reg_type == Register_Types::Coil || base_uri_info.flags.is_individual_bit())
                {
                    if (!to_set.holds_uint() || !(to_set.get_uint_unsafe() == 1UL || to_set.get_uint_unsafe() == 0UL))
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with single-set uri \"{}\": cannot set Coil or individual_bit uri to the value: {}, only true/false and 1/0 are accepted. Message dropped\n", client_name, process_name_view, uri_view, to_set);
                        set_had_errors = true;
                        continue;
                    }
                }

                Set_Chan_Msg to_push;
                to_push.set_vals.emplace_back(Set_Info{base_uri_info.decode_idx, base_uri_info.bit_idx, base_uri_info.enum_idx, to_set});
                auto& thread_workspace = *my_workspace.msg_staging_areas[base_uri_info.component_idx].comp_workspace->decoded_caches[base_uri_info.thread_idx].thread_workspace;
                auto& to_set_chan = *thread_workspace.set_channel;

                if (!to_set_chan.q.try_emplace(std::move(to_push)))
                {
                    NEW_FPS_ERROR_PRINT_NO_ARGS("could not push a set message onto a channel, things are really backed up. This should NEVER happpen, shutting the whole thing down\n");
                    return false;
                }
                // now wakeup the thread (lock mutex just in case to prevent race condition):
                {
                    std::lock_guard<std::mutex> lk{thread_workspace.worker_mutex};
                }
                thread_workspace.worker_cond.notify_one();
            }
        }
        else // "get" (replyto required)
        {
            if (replyto_view.empty())
            {
                NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with uri \"{}\": get request does not have a replyto. Message dropped\n", client_name, process_name_view, uri_view);
                continue;
            }
            if (is_timings_request || is_reset_timings_request) // they can only do timings requests on whole components (NOT single registers)
            {
                if (base_uri_info.decode_idx != Decode_All_Idx)
                {
                    static constexpr auto err_str = "Modbus Client -> timings requests can NOT be given for a single register"sv;
                    NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\", with uri \"{}\": get requests for single register uris cannot accept timing requests. Message dropped\n", client_name, process_name_view, uri_view);
                    if (!send_set(fims_gateway, replyto_view, err_str))
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": error sending replyto to uri \"{}\" on fims.\n", client_name, process_name_view, uri_view);
                        // fims error will cause listener to exit at the top on next receive() call
                    }
                    continue;
                }
                if (is_reset_timings_request) // send them a message back to indicate reset 
                {
                    static constexpr auto success_str = "Modbus Client -> timings reset"sv;
                    if (!send_set(fims_gateway, replyto_view, success_str))
                    {
                        NEW_FPS_ERROR_PRINT("Listener for \"{}\", from sender: \"{}\": error sending replyto to uri \"{}\" on fims.\n", client_name, process_name_view, uri_view);
                        // fims error will cause listener to exit at the top on next receive() call
                    }
                }
            }

            auto& to_push_onto = msg_staging_area.comp_workspace->get_channel;
            auto& get_msg = msg_staging_area.get_msg;
            get_msg.flags.flags = 0; // reset flags from last time
            get_msg.flags.set_raw_request(is_raw_request);
            get_msg.flags.set_timings_request(is_timings_request);
            get_msg.flags.set_reset_timings_request(is_reset_timings_request);
            get_msg.thread_idx = base_uri_info.thread_idx;
            get_msg.decode_idx = base_uri_info.decode_idx;
            get_msg.bit_idx    = base_uri_info.bit_idx;
            get_msg.enum_idx   = base_uri_info.enum_idx;
            get_msg.replyto    = replyto_view;
            if (!to_push_onto.try_emplace(std::move(get_msg)))
            {
                NEW_FPS_ERROR_PRINT("could not push onto \"get\" channel for uri \"{}\" as it was full. From sender: \"{}\". Shutting the whole thing down\n", uri_view, process_name_view);
                return false;
            }
            // wakeup aggregator ("component") thread:
            {
                std::lock_guard<std::mutex> lock{msg_staging_area.comp_workspace->aggregator_mutex};
            }
            msg_staging_area.comp_workspace->aggregator_cond.notify_one();
        }
    }

    return true;
}

static bool aggregator_thread(const u8 client_idx, const u8 comp_idx) noexcept
{
    // cleanup/shutdown -> tell main that something is wrong (so it can shut everything down)
    // if we return from this function then something went wrong (hence the "cleanup" defer)
    defer {
        main_workspace.start_signal = false;
        main_workspace.main_cond.notify_one();
    };

    // workspace variables:
    auto& str_storage = main_workspace.string_storage;
    auto& fims_gateway = main_workspace.client_workspaces[client_idx]->fims_gateway;
    auto& my_workspace = *main_workspace.client_workspaces[client_idx]->msg_staging_areas[comp_idx].comp_workspace;
    auto& send_buf = my_workspace.send_buf;
    const auto client_name = str_storage.get_str(main_workspace.client_workspaces[client_idx]->conn_workspace.conn_info.name);
    const auto comp_uri = str_storage.get_str(my_workspace.comp_uri);
    auto comp_name = comp_uri;
    comp_name.remove_prefix(Main_Uri_Frag_Length);

    // modbus context from pool (uses first worker thread's context) for heartbeat if we have it:
    auto& ctx_pool = main_workspace.client_workspaces[client_idx]->conn_workspace.ctx_pool;
    // auto& my_conn = ctx_pool.pool[my_workspace.decoded_caches[0].thread_workspace->client_thread_idx % ctx_pool.num_conns];

    // main logic variables:
    u8 errno_counter = 0; // if this reaches 5 then we return and main will shut everything down (5 failed polls -> like the old client)
    bool has_pub = false;
    bool has_get_msg = false;
    // heartbeat stuff (TODO(WALKER): probably a better or neater way to do this based off of poll count determined by frequencies/timeouts):
    bool component_connected = true;
    bool heartbeat_changed = false;
    bool heartbeat_sent_disconnect_event = false;
    s64 heartbeat_last_timeout_time;
    // s64 heartbeat_last_check_time;
    // _timings stuff:
    timings_duration_type conn_get_time_sum{};
    timings_duration_type response_time_sum{};
    // error reporting spam stuff:
    std::bitset<std::numeric_limits<decltype(my_workspace.num_threads)>::max()> has_done_pub_error_message; // for keeping track of which threads we've done the poll notify on

    // for making sure that "component_connected" is false upon shutdown (that way it doesn't latch on a disconnect)
    defer {
        if (my_workspace.heartbeat_enabled)
        {
            static constexpr auto comp_disconnect_str = R"({{"component_connected":false}})"sv;
            send_pub(fims_gateway, comp_uri, comp_disconnect_str);
        }
    };

    // wait until main signals everyone to start:
    {
        std::unique_lock<std::mutex> lk{main_workspace.main_mutex};
        main_workspace.main_cond.wait(lk, [&]() {
            return main_workspace.start_signal.load();
        });
    }

    // setup heartbeat variables to now:
    heartbeat_last_timeout_time = main_workspace.mono_clock.rdns();
    // heartbeat_last_check_time = heartbeat_last_timeout_time;

    // main loop:
    while (my_workspace.keep_running)
    {
        // wait for a command to come in from either the worker threads or the listener thread
        {
            std::unique_lock<std::mutex> lk{my_workspace.aggregator_mutex};
            my_workspace.aggregator_cond.wait(lk, [&]() {
                has_pub = my_workspace.pub_counter >= my_workspace.num_threads;
                has_get_msg = !my_workspace.get_channel.empty();
                return !my_workspace.keep_running || has_get_msg || has_pub;
            });
        }
        if (!my_workspace.keep_running) return false;
        // Update values from modbus server first:
        if (has_pub)
        {
            bool has_errno = false;
            u8 num_actual_threads_with_pub_msg = 0;
            // Put stuff into their decoded caches and do changed mask stuff before stringification:
            for (u8 thread_idx = 0; thread_idx < my_workspace.num_threads; ++thread_idx)
            {
                auto& curr_decoded_cache = my_workspace.decoded_caches[thread_idx];
                curr_decoded_cache.changed_mask.reset(); // reset to no change for this cache (for future stuff uncomment the "continue" checks later on during the stringify phase)
                auto& thread_workspace = *curr_decoded_cache.thread_workspace;
                auto top = thread_workspace.pub_channel.front();
                if (!top)
                {
                    if (!has_done_pub_error_message[thread_idx])
                    {
                        has_done_pub_error_message[thread_idx] = true; // so we don't spam that this thread is behind schedule over and over again
                        NEW_FPS_ERROR_PRINT("aggregator for comp \"{}\", on thread #{}: this thread didn't update on schedule. Consider increasing frequency. Skipping it.\n", comp_name, thread_idx);
                    }
                    continue;
                    // return false;
                }
                auto pub_msg = std::move(*top);
                thread_workspace.pub_channel.pop();
                ++num_actual_threads_with_pub_msg; // to decrement the pub_counter by at the end

                // do _timings stuff:
                ++my_workspace.num_timings_recorded;
                // conn_get_time:
                const auto conn_get_time_as_ms = timings_duration_type{pub_msg.conn_get_time};
                if (my_workspace.min_conn_get_time > conn_get_time_as_ms) my_workspace.min_conn_get_time = conn_get_time_as_ms;
                if (my_workspace.max_conn_get_time < conn_get_time_as_ms) my_workspace.max_conn_get_time = conn_get_time_as_ms;
                conn_get_time_sum += conn_get_time_as_ms;
                my_workspace.avg_conn_get_time = conn_get_time_sum / my_workspace.num_timings_recorded;
                // response_time:
                const auto response_time_as_ms = timings_duration_type{pub_msg.response_time};
                if (my_workspace.min_response_time > response_time_as_ms) my_workspace.min_response_time = response_time_as_ms;
                if (my_workspace.max_response_time < response_time_as_ms) my_workspace.max_response_time = response_time_as_ms;
                response_time_sum += response_time_as_ms;
                my_workspace.avg_response_time = response_time_sum / my_workspace.num_timings_recorded;                

                if (pub_msg.errno_code != 0
                    // NOTE(WALKER): These are errno codes to ignore because they don't really count (they are more like warnings than actual errors)
                    && pub_msg.errno_code != Modbus_Server_Busy)
                {
                    if (!has_errno) ++errno_counter;
                    has_errno = true;
                    NEW_FPS_ERROR_PRINT("aggregator thread for comp \"{}\": got an errno while reading the server from thread #{}, err_code = {}, err_str = \"{}\". skipping that decode\n",
                             comp_name, thread_idx, pub_msg.errno_code, modbus_strerror(pub_msg.errno_code));
                    continue;
                }
                if (curr_decoded_cache.reg_type == Register_Types::Holding || curr_decoded_cache.reg_type == Register_Types::Input)
                {
                    for (u8 decode_idx = 0; decode_idx < curr_decoded_cache.num_decode; ++decode_idx)
                    {
                        auto& val = pub_msg.pub_vals[decode_idx];
                        auto& curr_decoded_info = curr_decoded_cache.decoded_vals[decode_idx];

                        if (curr_decoded_info.bit_str_array_idx != Bit_Str_All_Idx) // NOTE(WALKER): do NOT change this to Bit_Str_Array_All_Idx because that is for u16's and NOT u8's (which this array_idx is)
                        {
                            auto& curr_bit_str_info = curr_decoded_cache.bit_strings_arrays[curr_decoded_info.bit_str_array_idx];
                            auto curr_unsigned_val = val.decoded_val.get_uint_unsafe();                            val.decoded_val = curr_unsigned_val;
                            curr_bit_str_info.changed_mask = curr_decoded_info.decoded_val.get_uint_unsafe() ^ curr_unsigned_val;
                        }
                        // compare decoded u64 vals for single bit of change (after care_masking of course):
                        // NOTE(WALKER): "if" this type of comparison (uses tagged union) is invalid or causes undefined behaviour, then maybe do something else (but this works for now, so leave it alone)
                        curr_decoded_cache.changed_mask[decode_idx] = curr_decoded_info.decoded_val.get_uint_unsafe() != val.decoded_val.get_uint_unsafe();
                        curr_decoded_info.raw_data = val.raw_data;
                        curr_decoded_info.decoded_val = val.decoded_val;
                    }
                }
                else // Coil and Discrete Input:
                {
                    for (u8 decode_idx = 0; decode_idx < curr_decoded_cache.num_decode; ++decode_idx)
                    {
                        const auto& val = pub_msg.pub_vals[decode_idx];
                        auto& curr_decoded_bool = curr_decoded_cache.bool_vals[decode_idx];

                        curr_decoded_cache.changed_mask[decode_idx] = static_cast<u64>(curr_decoded_bool) != val.decoded_val.get_uint_unsafe();
                        curr_decoded_bool = val.decoded_val.get_uint_unsafe() == 1UL;
                    }
                }
            }
            // errno_counter check:
            if (errno_counter >= 5)
            {
                // emit_event about polling server errors and client shutdown:
                if (!emit_event(send_buf, fims_gateway, Client_Event_Source, Event_Severity::Fault, R"(client \"{}\", component \"{}\": 5 total poll errors have occurred. Exiting)", client_name, comp_name))
                {
                    NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": Cannot send an event out on fims. Exiting\n", client_name, comp_name);
                }
                NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": 5 total poll errors occurred. Exiting\n", client_name, comp_name);
                return false;
            }
            if (!has_errno) errno_counter = 0; // reset errno counter if everything is ok
            my_workspace.pub_counter -= num_actual_threads_with_pub_msg; // sub pub counter to bring it under control (based on number of threads that actually had a message)

            // Do heartbeat check after putting data into caches:
            if (my_workspace.heartbeat_enabled)
            {
                auto& curr_decoded_cache = my_workspace.decoded_caches[my_workspace.heartbeat_read_thread_idx];

                // heartbeat_changed = heartbeat_changed || curr_decoded_cache.changed_mask[my_workspace.heartbeat_read_decode_idx];
                heartbeat_changed = curr_decoded_cache.changed_mask[my_workspace.heartbeat_read_decode_idx];
                const auto curr_time = main_workspace.mono_clock.rdns();
                if (heartbeat_changed)
                {
                    heartbeat_last_timeout_time = curr_time;
                    if (!component_connected)
                    {
                        component_connected = true;
                        heartbeat_sent_disconnect_event = false;
                        // Emit an event that the component's heartbeat is ok again:
                        if (!emit_event(send_buf, fims_gateway, Client_Event_Source, Event_Severity::Info, R"(client \"{}\", component \"{}\": reconnected after picking up a heartbeat change)", client_name, comp_name))
                        {
                            NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": Cannot send an event out on fims. Exiting\n", client_name, comp_name);
                            return false;
                        }
                        NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": reconnected after picking up a heartbeat change\n", client_name, comp_name);
                    }
                }
                // do heartbeat check if the time for it has passed:
                // auto heartbeat_elapsed = std::chrono::nanoseconds{curr_time - heartbeat_last_check_time};
                // if (heartbeat_elapsed >= my_workspace.heartbeat_frequency)
                if (true)
                {
                    // while (heartbeat_elapsed >= my_workspace.heartbeat_frequency)
                    // {
                    //     heartbeat_last_check_time += my_workspace.heartbeat_frequency.count();
                    //     heartbeat_elapsed -= my_workspace.heartbeat_frequency;
                    // }
                    // as long as the heartbeat register changed once between the last time we checked we are still connnected
                    // otherwise we're not, then reset the intermediate variable
                    const auto timeout_elapsed = std::chrono::nanoseconds{curr_time - heartbeat_last_timeout_time};
                    // do timeout check if that time has passed:
                    if (timeout_elapsed >= my_workspace.heartbeat_timeout)
                    {
                        component_connected = heartbeat_changed ? true : false;
                        if (!component_connected)
                        {
                            heartbeat_last_timeout_time = curr_time;
                            // Emit an event that the component's heartbeat has disconnected if we haven't already:
                            if (!heartbeat_sent_disconnect_event)
                            {
                                heartbeat_sent_disconnect_event = true; // so we don't keep sending this message on successive timeouts
                                if (!emit_event(send_buf, fims_gateway, Client_Event_Source, Event_Severity::Info, R"(client \"{}\", component \"{}\": heartbeat no longer detected)", client_name, comp_name))
                                {
                                    NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": Cannot send an event out on fims. Exiting\n", client_name, comp_name);
                                    return false;
                                }
                                NEW_FPS_ERROR_PRINT("client \"{}\", component \"{}\": heartbeat no longer detected\n", client_name, comp_name);
                            }
                        }
                    }
                    heartbeat_changed = false;
                    // do heartbeat write here if we have it:
                    if (my_workspace.heartbeat_write_thread_idx != Thread_All_Idx)
                    {
                        auto& curr_write_decoded_cache = my_workspace.decoded_caches[my_workspace.heartbeat_write_thread_idx];
                        auto curr_decoded_val = curr_write_decoded_cache.decoded_vals[my_workspace.heartbeat_write_decode_idx].decoded_val;
                        const auto& curr_encoding_info = curr_write_decoded_cache.thread_workspace->decode_array[my_workspace.heartbeat_write_decode_idx];
                        // increment one bit of the union (no matter the type) then encode:
                        ++curr_decoded_val.u;
                        u16 raw_buf[4];
                        encode(raw_buf, curr_encoding_info, curr_decoded_val);
                        has_errno = false;
                        // Now send the incremented value out:
                        {
                            auto& my_conn = acquire_locked_conn(ctx_pool, my_workspace.decoded_caches[0].thread_workspace->client_thread_idx);
                            defer { my_conn.lock.unlock(); };
                            // std::lock_guard<std::mutex> lk{my_conn.lock};
                            modbus_set_slave(my_conn.ctx, curr_write_decoded_cache.thread_workspace->slave_address);
                            if (curr_encoding_info.flags.is_size_one() && !curr_encoding_info.flags.is_multi_write_op_code()) // special case for size 1 registers (apparently it doesn't like the code if the size is 1, lame)
                            {
                                has_errno = modbus_write_register(my_conn.ctx, curr_encoding_info.offset, raw_buf[0]) == -1;
                            }
                            else // write 2 or 4 registers (or single register with multi_write_op_code set to true)
                            {
                                has_errno = modbus_write_registers(my_conn.ctx, curr_encoding_info.offset, curr_encoding_info.flags.get_size(), raw_buf) == -1;
                            }
                        }
                        if (has_errno)
                        {
                            // TODO(WALKER): figure out what the error condition for this is and make sure that we exit if it is one we can't recover from
                            NEW_FPS_ERROR_PRINT("client #{}, comp \"{}\" aggregator thread: error incrementing a heartbeat register on write, err_code = {}, err_str = \"{}\"\n", client_idx, comp_name, errno, modbus_strerror(errno));
                            // continue;
                        } 
                    }
                }
            }

            // This is the stringification phase for publishes:
            bool has_one_change = false;
            send_buf.clear();
            send_buf.push_back('{'); // begin object
            for (u8 thread_idx = 0; thread_idx < my_workspace.num_threads; ++thread_idx)
            {
                auto& curr_decoded_cache = my_workspace.decoded_caches[thread_idx];

                // if (curr_decoded_cache.changed_mask.none()) continue; // skip stringifying caches that have no changes
                has_one_change = true;

                if (curr_decoded_cache.reg_type == Register_Types::Holding || curr_decoded_cache.reg_type == Register_Types::Input)
                {
                    for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                    {
                        // if (!curr_decoded_cache.changed_mask[decoded_idx]) continue; // skip values that have no changes

                        auto& curr_decoded_info = curr_decoded_cache.decoded_vals[decoded_idx];
                        if (!curr_decoded_info.flags.is_bit_string_type()) // normal formatting:
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decoded_info.decoded_val);
                        }
                        else // format using "bit_strings":
                        {
                            const auto curr_unsigned_val = curr_decoded_info.decoded_val.get_uint_unsafe();
                            const auto& curr_bit_str_info = curr_decoded_cache.bit_strings_arrays[curr_decoded_info.bit_str_array_idx];

                            if (curr_decoded_info.flags.is_individual_bits()) // resolve to multiple uris (true/false per bit):
                            {
                                for (u8 bit_str_idx = 0; bit_str_idx < curr_bit_str_info.num_bit_strs; ++bit_str_idx)
                                {
                                    const auto& bit_str = curr_bit_str_info.bit_strs[bit_str_idx];
                                    // if (((curr_bit_str_info.changed_mask >> bit_str.begin_bit) & 1UL) != 1UL) continue; // skip bits that have not changed since last time
                                    FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(bit_str.str), ((curr_unsigned_val >> bit_str.begin_bit) & 1UL) == 1UL);
                                }
                            }
                            else if (curr_decoded_info.flags.is_bit_field()) // resolve to array of strings for each bit/bits that is/are == the expected value (1UL for individual bits):
                            {
                                FORMAT_TO_BUF(send_buf, R"("{}":[)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]));
                                bool has_one_bit_high = false;
                                u8 last_idx = 0;
                                for (u8 bit_str_idx = 0; bit_str_idx < curr_bit_str_info.num_bit_strs; ++bit_str_idx)
                                {
                                    const auto& bit_str = curr_bit_str_info.bit_strs[bit_str_idx];
                                    last_idx = bit_str.end_bit + 1;
                                    if (!bit_str.flags.is_unknown()) // "known" bits that are not ignored:
                                    {
                                        if (((curr_unsigned_val >> bit_str.begin_bit) & 1UL) == 1UL) // only format bits that are high:
                                        {
                                            has_one_bit_high = true;
                                            FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"{}"}},)",
                                                                bit_str.begin_bit, 
                                                                str_storage.get_str(bit_str.str));
                                        }
                                    }
                                    else // "unknown" bits that are "inbetween" bits (and not ignored):
                                    {
                                        for (u8 unknown_idx = bit_str.begin_bit; unknown_idx <= bit_str.end_bit; ++unknown_idx)
                                        {
                                            if (((curr_unsigned_val >> unknown_idx) & 1UL) == 1UL) // only format bits that are high:
                                            {
                                                has_one_bit_high = true;
                                                FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", unknown_idx);
                                            }
                                        }
                                    }
                                }
                                // all the other bits that we haven't masked out during the initial decode step that are high we print out as "Unknown" (all at the end of the array):
                                const u8 num_bits = curr_decoded_info.flags.get_size() * 16;
                                for (; last_idx < num_bits; ++last_idx)
                                {
                                    if (((curr_unsigned_val >> last_idx) & 1UL) == 1UL) // only format bits that are high:
                                    {
                                        has_one_bit_high = true;
                                        FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", last_idx);
                                    }
                                }
                                send_buf.resize(send_buf.size() - (1 * has_one_bit_high)); // this gets rid of the last excessive ',' character
                                send_buf.append("],"sv); // finish the array
                            }
                            else if (curr_decoded_info.flags.is_enum()) // resolve to single string based on current input value (unsigned whole numbers):
                            {
                                bool enum_found = false;
                                for (u8 enum_str_idx = 0; enum_str_idx < curr_bit_str_info.num_bit_strs; ++enum_str_idx)
                                {
                                    const auto& enum_str = curr_bit_str_info.bit_strs[enum_str_idx];
                                    if (curr_unsigned_val == enum_str.enum_val)
                                    {
                                        enum_found = true;
                                        FORMAT_TO_BUF(send_buf, R"("{}":[{{"value":{},"string":"{}"}}],)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_unsigned_val, str_storage.get_str(enum_str.str));
                                        break;
                                    }
                                }
                                if (!enum_found) // config did not account for this value:
                                {
                                    FORMAT_TO_BUF(send_buf, R"("{}":[{{"value":{},"string":"Unknown"}}],)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_unsigned_val);
                                }
                            }
                            // TODO(WALKER): In the future make sure to do "enum_field" and "individual_enums"
                        }
                    }
                }
                else // Coil and Discrete Input:
                {
                    for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                    {
                        // if (!curr_decoded_cache.changed_mask[decoded_idx]) continue; // skip values that have no changes
                        FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decoded_cache.bool_vals[decoded_idx]);
                    }
                }
            }
            // do heartbeat stuff here if we have it:
            if (my_workspace.heartbeat_enabled)
            {
                has_one_change = true;
                const auto curr_decoded_val = my_workspace.decoded_caches[my_workspace.heartbeat_read_thread_idx].decoded_vals[my_workspace.heartbeat_read_decode_idx].decoded_val;
                FORMAT_TO_BUF(send_buf, R"("modbus_heartbeat":{},"component_connected":{},)", curr_decoded_val, component_connected);
            }

            // pub out the strings if we have changes:
            if (has_one_change)
            {
                // "Timestamp" stuff:
                const auto timestamp       = std::chrono::system_clock::now();
                const auto timestamp_micro = time_fraction<std::chrono::microseconds>(timestamp);
                FORMAT_TO_BUF_NO_COMPILE(send_buf, R"("Timestamp":"{:%m-%d-%Y %T}.{}"}})", timestamp, timestamp_micro.count());

                // send_buf.resize(send_buf.size() - 1); // This gets rid of the last excessive ',' character
                // send_buf.push_back('}');
                if (!send_pub(fims_gateway, comp_uri, std::string_view{send_buf.data(), send_buf.size()}))
                {
                    NEW_FPS_ERROR_PRINT("aggregator thread \"{}\" cannot publish onto fims, exiting\n", comp_name);
                    return false;
                }
            }
        }
        // Get message request:
        if (has_get_msg)
        {
            const auto get_msg = std::move(*my_workspace.get_channel.front());
            my_workspace.get_channel.pop();

            send_buf.clear();
            if (get_msg.thread_idx == Thread_All_Idx) // They want everything:
            {
                send_buf.push_back('{');
                if (!get_msg.flags.flags) // normal get (no flags set):
                {
                    for (u8 thread_idx = 0; thread_idx < my_workspace.num_threads; ++thread_idx)
                    {
                        auto& curr_decoded_cache = my_workspace.decoded_caches[thread_idx];
                        if (curr_decoded_cache.reg_type == Register_Types::Holding || curr_decoded_cache.reg_type == Register_Types::Input)
                        {
                            for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                            {
                                auto& curr_decoded_info = curr_decoded_cache.decoded_vals[decoded_idx];
                                if (!curr_decoded_info.flags.is_bit_string_type()) // normal formatting:
                                {
                                    FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decoded_info.decoded_val);
                                }
                                else // format using "bit_strings":
                                {
                                    const auto curr_unsigned_val = curr_decoded_info.decoded_val.get_uint_unsafe();
                                    const auto& curr_bit_str_info = curr_decoded_cache.bit_strings_arrays[curr_decoded_info.bit_str_array_idx];

                                    if (curr_decoded_info.flags.is_individual_bits()) // resolve to multiple uris (true/false per bit):
                                    {
                                        for (u8 bit_str_idx = 0; bit_str_idx < curr_bit_str_info.num_bit_strs; ++bit_str_idx)
                                        {
                                            const auto& bit_str = curr_bit_str_info.bit_strs[bit_str_idx];
                                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(bit_str.str), ((curr_unsigned_val >> bit_str.begin_bit) & 1UL) == 1UL);
                                        }
                                    }
                                    else if (curr_decoded_info.flags.is_bit_field()) // resolve to array of strings for each bit/bits that is/are == the expected value (1UL for individual bits):
                                    {
                                        FORMAT_TO_BUF(send_buf, R"("{}":[)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]));
                                        bool has_one_bit_high = false;
                                        u8 last_idx = 0;
                                        for (u8 bit_str_idx = 0; bit_str_idx < curr_bit_str_info.num_bit_strs; ++bit_str_idx)
                                        {
                                            const auto& bit_str = curr_bit_str_info.bit_strs[bit_str_idx];
                                            last_idx = bit_str.end_bit + 1;
                                            if (!bit_str.flags.is_unknown()) // "known" bits that are not ignored:
                                            {
                                                if (((curr_unsigned_val >> bit_str.begin_bit) & 1UL) == 1UL) // only format bits that are high:
                                                {
                                                    has_one_bit_high = true;
                                                    FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"{}"}},)",
                                                                        bit_str.begin_bit, 
                                                                        str_storage.get_str(bit_str.str));
                                                }
                                            }
                                            else // "unknown" bits that are "inbetween" bits (and not ignored):
                                            {
                                                for (u8 unknown_idx = bit_str.begin_bit; unknown_idx <= bit_str.end_bit; ++unknown_idx)
                                                {
                                                    if (((curr_unsigned_val >> unknown_idx) & 1UL) == 1UL) // only format bits that are high:
                                                    {
                                                        has_one_bit_high = true;
                                                        FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", unknown_idx);
                                                    }
                                                }
                                            }
                                        }
                                        // all the other bits that we haven't masked out during the initial decode step that are high we print out as "Unknown" (all at the end of the array):
                                        const u8 num_bits = curr_decoded_info.flags.get_size() * 16;
                                        for (; last_idx < num_bits; ++last_idx)
                                        {
                                            if (((curr_unsigned_val >> last_idx) & 1UL) == 1UL) // only format bits that are high:
                                            {
                                                has_one_bit_high = true;
                                                FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", last_idx);
                                            }
                                        }
                                        send_buf.resize(send_buf.size() - (1 * has_one_bit_high)); // this gets rid of the last excessive ',' character
                                        send_buf.append("],"sv); // finish the array
                                    }
                                    else if (curr_decoded_info.flags.is_enum()) // resolve to single string based on current input value (unsigned whole numbers):
                                    {
                                        bool enum_found = false;
                                        for (u8 enum_str_idx = 0; enum_str_idx < curr_bit_str_info.num_bit_strs; ++enum_str_idx)
                                        {
                                            const auto& enum_str = curr_bit_str_info.bit_strs[enum_str_idx];
                                            if (curr_unsigned_val == enum_str.enum_val)
                                            {
                                                enum_found = true;
                                                FORMAT_TO_BUF(send_buf, R"("{}":[{{"value":{},"string":"{}"}}],)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_unsigned_val, str_storage.get_str(enum_str.str));
                                                break;
                                            }
                                        }
                                        if (!enum_found) // config did not account for this value:
                                        {
                                            FORMAT_TO_BUF(send_buf, R"("{}":[{{"value":{},"string":"Unknown"}}],)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_unsigned_val);
                                        }
                                    }
                                    // TODO(WALKER): In the future make sure to do "enum_field" and "individual_enums"
                                }
                            }
                        }
                        else // Coil and Discrete Input:
                        {
                            for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                            {
                                FORMAT_TO_BUF(send_buf, R"("{}":{},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decoded_cache.bool_vals[decoded_idx]);
                            }
                        }
                    }
                }
                else if (get_msg.flags.is_raw_request()) // raw get: 
                {
                    for (u8 thread_idx = 0; thread_idx < my_workspace.num_threads; ++thread_idx)
                    {
                        const auto& curr_decoded_cache = my_workspace.decoded_caches[thread_idx];
                        if (curr_decoded_cache.reg_type == Register_Types::Holding || curr_decoded_cache.reg_type == Register_Types::Input)
                        {
                            for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                            {
                                const auto& curr_decode_info = curr_decoded_cache.decoded_vals[decoded_idx];
                                FORMAT_TO_BUF(send_buf, R"("{0}":{{"value":{1},"binary":"{1:0>{2}b}","hex":"{1:0>{3}X}"}},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decode_info.raw_data, curr_decode_info.flags.get_size() * 16, curr_decode_info.flags.get_size() * 4);
                            }
                        }
                        else // Coil and Discrete Input:
                        {
                            for (u8 decoded_idx = 0; decoded_idx < curr_decoded_cache.num_decode; ++decoded_idx)
                            {
                                FORMAT_TO_BUF(send_buf, R"("{0}":{{"value":{1},"binary":"{1:b}","hex":"{1:X}"}},)", str_storage.get_str(curr_decoded_cache.decode_ids[decoded_idx]), curr_decoded_cache.bool_vals[decoded_idx]);
                            }
                        }
                    }
                }
                else // timings requests (could also be a reset):
                {
                    if (get_msg.flags.is_reset_timings_request())
                    {
                        conn_get_time_sum = timings_duration_type{0.0};
                        response_time_sum = timings_duration_type{0.0};
                        my_workspace.min_conn_get_time = timings_duration_type{std::numeric_limits<f64>::max()};
                        my_workspace.max_conn_get_time = timings_duration_type{0.0};
                        my_workspace.avg_conn_get_time = timings_duration_type{0.0};
                        my_workspace.min_response_time = timings_duration_type{std::numeric_limits<f64>::max()};
                        my_workspace.max_response_time = timings_duration_type{0.0};
                        my_workspace.avg_response_time = timings_duration_type{0.0};
                        my_workspace.num_timings_recorded = 0.0;
                        continue; // don't send anything out
                    }
                    FORMAT_TO_BUF(send_buf, R"("min_conn_get_time":"{}","max_conn_get_time":"{}","avg_conn_get_time":"{}","min_response_time":"{}","max_response_time":"{}","avg_response_time":"{}","num_timings_recorded":{},)", 
                        my_workspace.min_conn_get_time, my_workspace.max_conn_get_time, my_workspace.avg_conn_get_time, my_workspace.min_response_time, my_workspace.max_response_time, my_workspace.avg_response_time, my_workspace.num_timings_recorded);
                }
                send_buf.resize(send_buf.size() - 1); // gets rid of the last excessive ',' character
                send_buf.push_back('}');
            }
            else // They want a single variable (no timings requests, no key -> POSSIBLE TODO(WALKER): do they want it "clothed", I assume unclothed for now, maybe have a config option for this?):
            {
                // NOTE(WALKER): the check for single get timings request is done in listener thread.
                auto& curr_decoded_cache = my_workspace.decoded_caches[get_msg.thread_idx];
                if (curr_decoded_cache.reg_type == Register_Types::Holding || curr_decoded_cache.reg_type == Register_Types::Input)
                {
                    auto& curr_decoded_info = curr_decoded_cache.decoded_vals[get_msg.decode_idx];
                    if (!get_msg.flags.flags) // normal get (no flags):
                    {
                        if (!curr_decoded_info.flags.is_bit_string_type()) // normal formatting:
                        {
                            FORMAT_TO_BUF(send_buf, R"({})", curr_decoded_info.decoded_val);
                        }
                        else // format using "bit_strings":
                        {
                            const auto curr_unsigned_val = curr_decoded_info.decoded_val.get_uint_unsafe();
                            const auto& curr_bit_str_info = curr_decoded_cache.bit_strings_arrays[curr_decoded_info.bit_str_array_idx];

                            if (curr_decoded_info.flags.is_individual_bits()) // resolve to multiple uris (true/false per bit):
                            {
                                FORMAT_TO_BUF(send_buf, R"({})", ((curr_unsigned_val >> get_msg.bit_idx) & 1UL) == 1UL);
                            }
                            else if (curr_decoded_info.flags.is_bit_field()) // resolve to array of strings for each bit/bits that is/are == the expected value (1UL for individual bits):
                            {
                                send_buf.push_back('[');
                                bool has_one_bit_high = false;
                                u8 last_idx = 0;
                                for (u8 bit_str_idx = 0; bit_str_idx < curr_bit_str_info.num_bit_strs; ++bit_str_idx)
                                {
                                    const auto& bit_str = curr_bit_str_info.bit_strs[bit_str_idx];
                                    last_idx = bit_str.end_bit + 1;
                                    if (!bit_str.flags.is_unknown()) // "known" bits that are not ignored:
                                    {
                                        if (((curr_unsigned_val >> bit_str.begin_bit) & 1UL) == 1UL) // only format bits that are high:
                                        {
                                            has_one_bit_high = true;
                                            FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"{}"}},)",
                                                                bit_str.begin_bit, 
                                                                str_storage.get_str(bit_str.str));
                                        }
                                    }
                                    else // "unknown" bits that are "inbetween" bits (and not ignored):
                                    {
                                        for (u8 unknown_idx = bit_str.begin_bit; unknown_idx <= bit_str.end_bit; ++unknown_idx)
                                        {
                                            if (((curr_unsigned_val >> unknown_idx) & 1UL) == 1UL) // only format bits that are high:
                                            {
                                                has_one_bit_high = true;
                                                FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", unknown_idx);
                                            }
                                        }
                                    }
                                }
                                // all the other bits that we haven't masked out during the initial decode step that are high we print out as "Unknown" (all at the end of the array):
                                const u8 num_bits = curr_decoded_info.flags.get_size() * 16;
                                for (; last_idx < num_bits; ++last_idx)
                                {
                                    if (((curr_unsigned_val >> last_idx) & 1UL) == 1UL) // only format bits that are high:
                                    {
                                        has_one_bit_high = true;
                                        FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}},)", last_idx);
                                    }
                                }
                                send_buf.resize(send_buf.size() - (1 * has_one_bit_high)); // this gets rid of the last excessive ',' character
                                send_buf.push_back(']'); // finish the array
                            }
                            else if (curr_decoded_info.flags.is_enum()) // resolve to single string based on current input value (unsigned whole numbers):
                            {
                                bool enum_found = false;
                                for (u8 enum_str_idx = 0; enum_str_idx < curr_bit_str_info.num_bit_strs; ++enum_str_idx)
                                {
                                    const auto& enum_str = curr_bit_str_info.bit_strs[enum_str_idx];
                                    if (curr_unsigned_val == enum_str.enum_val)
                                    {
                                        enum_found = true;
                                        FORMAT_TO_BUF(send_buf, R"([{{"value":{},"string":"{}"}}])", curr_unsigned_val, str_storage.get_str(enum_str.str));
                                        break;
                                    }
                                }
                                if (!enum_found) // config did not account for this value:
                                {
                                    FORMAT_TO_BUF(send_buf, R"([{{"value":{},"string":"Unknown"}}])", curr_unsigned_val);
                                }
                            }
                            // TODO(WALKER): In the future make sure to do "enum_field" and "individual_enums"
                        }
                    }
                    else if (get_msg.flags.is_raw_request()) // they want raw data (format it):
                    {
                        FORMAT_TO_BUF(send_buf, R"({{"value":{0},"binary":"{0:0>{1}b}","hex":"{0:0>{2}X}"}})", curr_decoded_info.raw_data, curr_decoded_info.flags.get_size() * 16, curr_decoded_info.flags.get_size() * 4);
                    }
                }
                else // Coil and Discrete Input:
                {
                    const auto curr_bool_val = curr_decoded_cache.bool_vals[get_msg.decode_idx];
                    if (!get_msg.flags.flags) // normal get (no flags):
                    {
                        FORMAT_TO_BUF(send_buf, R"({})", curr_bool_val);
                    }
                    else if (get_msg.flags.is_raw_request()) // they want raw data (format it):
                    {
                        FORMAT_TO_BUF(send_buf, R"({{"value":{0},"binary":"{0:b}","hex":"{0:X}"}})", curr_bool_val);
                    }
                }
            }
            // Send out using replyto here:
            if (!send_set(fims_gateway, get_msg.replyto, std::string_view{send_buf.data(), send_buf.size()}))
            {
                NEW_FPS_ERROR_PRINT("client #{}, aggregator thread \"{}\": could not send to replyto uri \"{}\" over fims, exiting\n", client_idx, comp_name, get_msg.replyto);
                return false;
            }
        }
    }

    return true;
}

template<Register_Types Reg_Type>
static bool worker_thread(const u8 client_idx, const u8 comp_idx, const u8 thread_idx) noexcept
{
    // constants:
    // TODO(WALKER): figure out a good way to set modbus's "timeout":
    // static constexpr auto Modbus_Default_Timeout = 50ms; // the default timeout we want for modbus polls -> if we don't specify this it is 500ms for newly created contexts

    // cleanup/shutdown -> tell main that something is wrong (so it can shut everything down)
    // if we return from this function then something went wrong (hence the "cleanup" defer)
    defer {
        main_workspace.start_signal = false;
        main_workspace.main_cond.notify_one();
    };

    auto& my_conn_workspace = main_workspace.client_workspaces[client_idx]->conn_workspace;
    auto& my_comp_workspace = *main_workspace.client_workspaces[client_idx]->msg_staging_areas[comp_idx].comp_workspace;
    auto& my_workspace = *my_comp_workspace.decoded_caches[thread_idx].thread_workspace;
    // auto& my_conn = my_conn_workspace.ctx_pool.pool[my_workspace.client_thread_idx % my_conn_workspace.ctx_pool.num_conns];

    auto comp_name = main_workspace.string_storage.get_str(my_comp_workspace.comp_uri);
    comp_name.remove_prefix(Main_Uri_Frag_Length);

    // variables for polling the server:
    bool has_done_pub_error_message = false;
    bool time_to_poll_server = false;
    std::chrono::nanoseconds sleep_time{};
    std::chrono::nanoseconds poll_time_left{};
    std::chrono::nanoseconds poll_overshoot_time{};
    s64 poll_last_time;

    // variables for timings requests:
    s64 begin_timing;
    s64 end_timing;
    s64 conn_get_time;
    s64 response_time;

    // helper function for polling the server:
    auto poll_server = [&]() {
        bool has_errno = false;

        { // empty scope for acquiring a lock
            begin_timing = main_workspace.mono_clock.rdns();
            auto& my_conn = acquire_locked_conn(my_conn_workspace.ctx_pool, my_workspace.client_thread_idx);
            end_timing = main_workspace.mono_clock.rdns();
            conn_get_time = end_timing - begin_timing;

            defer { my_conn.lock.unlock(); };

            // setup the connection based on the thread's workspace information:
            modbus_set_slave(my_conn.ctx, my_workspace.slave_address);
            // modbus_set_response_timeout(my_conn.ctx, 0, ?);

            // poll server based on register type:
            begin_timing = main_workspace.mono_clock.rdns();
            if constexpr (Reg_Type == Register_Types::Holding)
            {
                has_errno = modbus_read_registers(my_conn.ctx, my_workspace.start_offset, my_workspace.num_regs, my_workspace.raw_registers) == -1;
            }
            else if constexpr (Reg_Type == Register_Types::Input)
            {
                has_errno = modbus_read_input_registers(my_conn.ctx, my_workspace.start_offset, my_workspace.num_regs, my_workspace.raw_registers) == -1;
            }
            else if constexpr (Reg_Type == Register_Types::Coil)
            {
                has_errno = modbus_read_bits(my_conn.ctx, my_workspace.start_offset, my_workspace.num_regs, my_workspace.bool_registers) == -1;
            }
            else // Discrete Input:
            {
                has_errno = modbus_read_input_bits(my_conn.ctx, my_workspace.start_offset, my_workspace.num_regs, my_workspace.bool_registers) == -1;
            }
            end_timing = main_workspace.mono_clock.rdns();
        }
        response_time = end_timing - begin_timing;

        Pub_Chan_Msg pub_msg;
        pub_msg.conn_get_time = std::chrono::nanoseconds{conn_get_time};
        pub_msg.response_time = std::chrono::nanoseconds{response_time};
        if (has_errno)
        {
            pub_msg.errno_code = errno;
        }
        else // we have no errno, so we can decode our stuff and send it out:
        {
            u16 curr_offset;
            u64 raw_val;
            Jval_buif to_set;
            pub_msg.pub_vals.reserve(my_workspace.num_decode);
            for (u8 i = 0; i < my_workspace.num_decode; ++i)
            {
                if constexpr (Reg_Type == Register_Types::Holding || Reg_Type == Register_Types::Input)
                {
                    const auto& curr_decode = my_workspace.decode_array[i];
                    curr_offset = curr_decode.offset;
                    raw_val = decode(&my_workspace.raw_registers[curr_offset - my_workspace.start_offset], curr_decode, to_set);
                    // if we have bit_strings that we can set for this decode (like "individual_bits/enums") then we need to store that here for setting purposes (also for debounce if we have it):
                    if constexpr (Reg_Type == Register_Types::Holding)
                    {
                        if (curr_decode.bit_str_array_idx != Bit_Str_Array_All_Idx)
                        {
                            my_workspace.bit_str_encoding_array[curr_decode.bit_str_array_idx].prev_unsigned_val = to_set.get_uint_unsafe();
                        }
                    }
                }
                else // Coil and Discrete Input:
                {
                    curr_offset = my_workspace.bool_offset_array[i];
                    raw_val = my_workspace.bool_registers[curr_offset - my_workspace.start_offset];
                    to_set = raw_val; // do we need to cast this to a bool? (probably not, so oh well)
                }
                pub_msg.pub_vals.emplace_back(Pub_Info{raw_val, to_set});
            }
        }
        // send out on your pub_channel here:
        if (!my_workspace.pub_channel.try_emplace(std::move(pub_msg))) return false;

        const auto curr_pub_counter = ++my_comp_workspace.pub_counter;
        if (curr_pub_counter >= my_comp_workspace.num_threads)
        {
            // lock to prevent missed wakeup for aggregator thread:
            {
                std::lock_guard<std::mutex> lk{my_comp_workspace.aggregator_mutex};
            }
            my_comp_workspace.aggregator_cond.notify_one();
        }
        return true;
    };

    // wait until main signals everyone to start:
    {
        std::unique_lock<std::mutex> lk{main_workspace.main_mutex};
        main_workspace.main_cond.wait(lk, [&]() {
            return main_workspace.start_signal.load();
        });
    }

    // set poll_last_time to now:
    poll_last_time = main_workspace.mono_clock.rdns() - my_workspace.frequency.count();

    // main loop:
    while (my_workspace.keep_running)
    {
        if constexpr (Reg_Type == Register_Types::Holding || Reg_Type == Register_Types::Coil)
        {
            // "task" variables:
            bool has_set_msg = false;
            bool has_debounce = false;
            // "task" timer variables:
            std::chrono::nanoseconds debounce_time_left{};
            // This is used by "Holding" and "Coil" register workers to determine how long they need to sleep
            // This is because we might have a small time to sleep before debouncing a value which is shorter
            // then the time we have left before polling the server:
            auto calc_sleep_time = [&]() {
                s64 min_debounce_time = std::numeric_limits<s64>::max();
                if (my_workspace.debounce > 0ns && my_workspace.debounce_to_set_mask.any())
                {
                    for (u8 debounce_idx = 0; debounce_idx < my_workspace.num_decode; ++debounce_idx)
                    {
                        if (my_workspace.debounce_to_set_mask[debounce_idx]) // This check also works for "individual_bits/enums" as the global flag is set if any single bit needs to be debounced
                        {
                            if constexpr (Reg_Type == Register_Types::Holding)
                            {
                                const auto& curr_decode = my_workspace.decode_array[debounce_idx];
                                if (curr_decode.bit_str_array_idx == Bit_Str_Array_All_Idx) // regular debounce check:
                                {
                                    if (my_workspace.debounce_last_set_times[debounce_idx] < min_debounce_time)
                                    {
                                        min_debounce_time = my_workspace.debounce_last_set_times[debounce_idx];
                                    }
                                }
                                else // check "bit_str_encoding_info" ("individual_bit/enums"):
                                {
                                    // TODO(WALKER): make sure that in the future debouncing "individual_enums" also works
                                    auto& curr_bit_str_encoding_info = my_workspace.bit_str_encoding_array[curr_decode.bit_str_array_idx];
                                    const u8 num_bits = curr_decode.flags.get_size() * 16;
                                    for (u8 bit_idx = 0; bit_idx < num_bits; ++bit_idx)
                                    {
                                        if (curr_bit_str_encoding_info.debounce_to_set_mask[bit_idx] && curr_bit_str_encoding_info.debounce_last_set_times[bit_idx] < min_debounce_time)
                                        {
                                            min_debounce_time = curr_bit_str_encoding_info.debounce_last_set_times[bit_idx];
                                        }
                                    }
                                }
                            }
                            else // Coil (regular debounce check):
                            {
                                if (my_workspace.debounce_last_set_times[debounce_idx] < min_debounce_time)
                                {
                                    min_debounce_time = my_workspace.debounce_last_set_times[debounce_idx];
                                }
                            }
                        }
                    }
                }
                // return poll_time_left or debounce_time_left -> whichever is shorter (this is how much we sleep by):
                const auto curr_time = main_workspace.mono_clock.rdns();
                poll_time_left = my_workspace.frequency - std::chrono::nanoseconds{curr_time - poll_last_time} - poll_overshoot_time;
                debounce_time_left = my_workspace.debounce - std::chrono::nanoseconds{curr_time - min_debounce_time};
                return poll_time_left < debounce_time_left ? poll_time_left : debounce_time_left;
            };

            // wait for an action to occur (either we have to poll the server/debounce or we get a set message coming in):
            {
                std::unique_lock<std::mutex> lk{my_workspace.worker_mutex};
                while (!my_workspace.worker_cond.wait_for(lk, sleep_time, [&]() {
                    sleep_time = calc_sleep_time(); // NOTE(WALKER): calc_sleep_time() changes poll_time_left and debounce_time_left during the function call
                    time_to_poll_server = poll_time_left <= 0ns;
                    has_set_msg = !my_workspace.set_channel->q.empty();
                    has_debounce = debounce_time_left <= 0ns;
                    return !my_workspace.keep_running || time_to_poll_server || has_set_msg || has_debounce;
                })) { continue; }
            }
            // Do actions based on condition variable exit (4 total actions upon wakeup):
            if (!my_workspace.keep_running) return false;
            if (time_to_poll_server)
            {
                poll_overshoot_time = (-poll_time_left) % my_workspace.frequency; // set overshoot_time
                poll_last_time = main_workspace.mono_clock.rdns(); // set last poll time to now
                if (!poll_server())
                {
                    if (!has_done_pub_error_message)
                    {
                        has_done_pub_error_message = true;
                        NEW_FPS_ERROR_PRINT("component \"{}\", thread #{}: could not aggregate poll upwards. Consider increasing frequency.\n", comp_name, thread_idx);
                    }
                    // return false;
                }
            }
            if (has_set_msg)
            {
                const auto curr_msg = std::move(*my_workspace.set_channel->q.front());
                my_workspace.set_channel->q.pop();

                bool has_errno = false;
                {
                    auto& my_conn = acquire_locked_conn(my_conn_workspace.ctx_pool, my_workspace.client_thread_idx);
                    defer { my_conn.lock.unlock(); };
                    // std::lock_guard<std::mutex> lk{my_conn.lock};

                    // setup the context based on the thread's workspace information:
                    modbus_set_slave(my_conn.ctx, my_workspace.slave_address);
                    // modbus_set_response_timeout(my_conn.ctx, 0, ?);

                    // POSSIBLE TODO(WALKER): maybe cut down on the mono clock polls
                    // and possibly put the if/else logic for debounce around the loop instead of in it?
                    // should cut down on some things
                    // Also, maybe figure out a way to do sets without sending out one variable at a time?
                    // possibly just encode the values into our raw array then send all the bits out at the
                    // minimum offset after encoding? (don't know if that is a good idea or not compared to sending)
                    // them out one variable at a time (will have to experiment in the future)
                    for (const auto& val : curr_msg.set_vals)
                    {
                        // debounce check:
                        if (my_workspace.debounce > 0ns)
                        {
                            if constexpr (Reg_Type == Register_Types::Holding)
                            {
                                if (val.bit_idx == Bit_All_Idx) // A whole register set (this includes Coil):
                                {
                                    if (std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - my_workspace.debounce_last_set_times[val.decode_idx]} <= my_workspace.debounce)
                                    {
                                        // skip this set (we are in debounce period):
                                        my_workspace.debounce_to_set_mask[val.decode_idx] = true;
                                        my_workspace.debounce_to_set_vals[val.decode_idx] = val.set_val;
                                        continue; // skip this set (it is on debounce cooldown)
                                    }
                                }
                                else // They want to set some bits -> reach into bit_str_encoding_array:
                                {
                                    auto& curr_bit_str_encoding_info = my_workspace.bit_str_encoding_array[my_workspace.decode_array[val.decode_idx].bit_str_array_idx];
                                    if (std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - curr_bit_str_encoding_info.debounce_last_set_times[val.bit_idx]} <= my_workspace.debounce)
                                    {
                                        // Set the global debounce to true for all of these bits (so we know that at least one bit has debounce or not for primary logic):
                                        my_workspace.debounce_to_set_mask[val.decode_idx] = true;
                                        // build on the previous bits that were debounced here (build on the current mask of bits we want to set):
                                        // TODO(WALKER): make sure that in the future "individual_enums" gets masked out properly using its "care_mask"
                                        // this means it probably needs to go into curr_bit_str_encoding_info
                                        // using "enum_idx" in the future
                                        auto curr_bits = my_workspace.debounce_to_set_vals[val.decode_idx].get_uint_unsafe();
                                        curr_bits = (curr_bits & ~(1UL << val.bit_idx)) | (val.set_val.get_uint_unsafe() << val.bit_idx);
                                        my_workspace.debounce_to_set_vals[val.decode_idx] = curr_bits;
                                        continue;
                                    }
                                }
                            }
                            else // Coil:
                            {
                                if (std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - my_workspace.debounce_last_set_times[val.decode_idx]} <= my_workspace.debounce)
                                {
                                    // skip this set (we are in debounce period):
                                    my_workspace.debounce_to_set_mask[val.decode_idx] = true;
                                    my_workspace.debounce_to_set_bools[val.decode_idx] = static_cast<u8>(val.set_val.get_uint_unsafe());
                                    continue;
                                }
                            }
                        }

                        u16 write_start_offset;

                        if constexpr (Reg_Type == Register_Types::Holding)
                        {
                            u16 raw_to_set[4];
                            const auto& curr_decode = my_workspace.decode_array[val.decode_idx];
                            write_start_offset = curr_decode.offset;
                            encode(
                                raw_to_set, 
                                curr_decode, 
                                val.set_val, 
                                val.bit_idx, 
                                curr_decode.flags.is_individual_enums() ? 0UL : 1UL, // TODO(WALKER): in the future, this will go into bit_str_encoding_array and use the "care_masks" there (for now it is unused)
                                curr_decode.bit_str_array_idx != Bit_Str_Array_All_Idx ? &my_workspace.bit_str_encoding_array[curr_decode.bit_str_array_idx].prev_unsigned_val : nullptr);
                            if (curr_decode.flags.is_size_one() && !curr_decode.flags.is_multi_write_op_code()) // special case for size 1 registers (apparently it doesn't like the multi-set if the size is 1, lame)
                            {
                                has_errno = modbus_write_register(my_conn.ctx, curr_decode.offset, raw_to_set[0]) == -1;
                            }
                            else // write 2 or 4 registers (or single register with multi_write_op_code set to true)
                            {
                                has_errno = modbus_write_registers(my_conn.ctx, curr_decode.offset, curr_decode.flags.get_size(), raw_to_set) == -1;
                            }
                        }
                        else // coil:
                        {
                            write_start_offset = my_workspace.bool_offset_array[val.decode_idx];
                            has_errno = modbus_write_bit(my_conn.ctx, write_start_offset, static_cast<int>(val.set_val.get_uint_unsafe())) == -1;
                        }
                        if (has_errno)
                        {
                            // TODO(WALKER): figure out what to do here based on what errno is
                            NEW_FPS_ERROR_PRINT("component \"{}\", thread #{}: got an error while writing to register(s) at offset {}, err = \"{}\"\n", comp_name, thread_idx, write_start_offset, modbus_strerror(errno));
                            continue;
                        }
                        // reset debounce variable if we have debounce (no longer in debounce period):
                        if (my_workspace.debounce > 0ns)
                        {
                            if constexpr (Reg_Type == Register_Types::Holding)
                            {
                                if (val.bit_idx == Bit_All_Idx) // whole register
                                {
                                    my_workspace.debounce_to_set_mask[val.decode_idx] = false;
                                    my_workspace.debounce_last_set_times[val.decode_idx] = main_workspace.mono_clock.rdns();
                                }
                                else // setting "individual_bits/enums" debounce stuff:
                                {
                                    auto& curr_bit_str_encoding_info = my_workspace.bit_str_encoding_array[my_workspace.decode_array[val.decode_idx].bit_str_array_idx];
                                    curr_bit_str_encoding_info.debounce_to_set_mask[val.bit_idx] = false;
                                    curr_bit_str_encoding_info.debounce_last_set_times[val.bit_idx] = main_workspace.mono_clock.rdns();
                                    if (curr_bit_str_encoding_info.debounce_to_set_mask.none()) my_workspace.debounce_to_set_mask[val.decode_idx] = false; // Notify that there are no debouncing bits for this whole register (for sleeping reasons)
                                }
                            }
                            else // Coil:
                            {
                                my_workspace.debounce_to_set_mask[val.decode_idx] = false;
                                my_workspace.debounce_last_set_times[val.decode_idx] = main_workspace.mono_clock.rdns();
                            }
                        }
                    }
                }
            }
            // NOTE(WALKER): do we "recalc" debounce times after polling and sets? (right here -> some time has passed between when we did a poll and sets)
            if (has_debounce && my_workspace.debounce_to_set_mask.any()) // The .any() check is to ensure that after doing "sets" (if we have any) that we still have to values to debounce (even if we detected a debounce timeout upon wakeup):
            {
                auto& my_conn = acquire_locked_conn(my_conn_workspace.ctx_pool, my_workspace.client_thread_idx);
                defer { my_conn.lock.unlock(); };
                // std::lock_guard<std::mutex> lk{my_conn.lock};
                // setup the context based on the thread's workspace information:
                modbus_set_slave(my_conn.ctx, my_workspace.slave_address);
                // modbus_set_response_timeout(my_conn.ctx, 0, ?);

                bool has_errno = false;
                for (u8 debounce_idx = 0; debounce_idx < my_workspace.num_decode; ++debounce_idx)
                {
                    if (my_workspace.debounce_to_set_mask[debounce_idx])
                    {
                        // TODO(WALKER): figure out how to "debounce" for "individual_bits" (and in the future, "individual_enums")
                        if constexpr (Reg_Type == Register_Types::Holding)
                        {
                            u16 raw_to_set[4];
                            const auto& curr_decode = my_workspace.decode_array[debounce_idx];
                            if (curr_decode.bit_str_array_idx == Bit_Str_Array_All_Idx)
                            {
                                if (std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - my_workspace.debounce_last_set_times[debounce_idx]} > my_workspace.debounce)
                                {
                                    encode(raw_to_set, curr_decode, my_workspace.debounce_to_set_vals[debounce_idx]);
                                    if (curr_decode.flags.is_size_one() && !curr_decode.flags.is_multi_write_op_code()) // special case for size 1 registers (apparently it doesn't like the multi-set if the size is 1, lame)
                                    {
                                        has_errno = modbus_write_register(my_conn.ctx, curr_decode.offset, raw_to_set[0]) == -1;
                                    }
                                    else // write 2 or 4 registers (or single register with multi_write_op_code set to true)
                                    {
                                        has_errno = modbus_write_registers(my_conn.ctx, curr_decode.offset, curr_decode.flags.get_size(), raw_to_set) == -1;
                                    }
                                    if (!has_errno)
                                    {
                                        // This value is no longer debounced:
                                        my_workspace.debounce_to_set_mask[debounce_idx] = false;
                                        my_workspace.debounce_last_set_times[debounce_idx] = main_workspace.mono_clock.rdns();
                                    }
                                }
                            }
                            else // Debouncing "individual_bits/enums":
                            {
                                auto& curr_bit_str_encoding_info = my_workspace.bit_str_encoding_array[curr_decode.bit_str_array_idx];
                                const u8 num_bits = curr_decode.flags.get_size() * 16;
                                for (u8 bit_idx = 0; bit_idx < num_bits; ++bit_idx)
                                {
                                    // POSSIBLE TODO(WALKER): maybe sending individual_bits back to back might not be the best idea if we have a debounce period
                                    // maybe take into account the global debounce time as well?
                                    // maybe only debounce one bit per period?
                                    // will have to think about this logic, but this seems good to me for now
                                    if (curr_bit_str_encoding_info.debounce_to_set_mask[bit_idx] && std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - curr_bit_str_encoding_info.debounce_last_set_times[debounce_idx]} > my_workspace.debounce)
                                    {
                                        auto& curr_debounce_val = my_workspace.debounce_to_set_vals[debounce_idx];
                                        // TODO(WALKER): make sure to account for "individual_enums" in the future (probably just use the enum's care mask instead of the bit itself for the set -> should work)
                                        // for now only do "individual_bits" logic
                                        encode(
                                            raw_to_set, 
                                            curr_decode, 
                                            curr_debounce_val, 
                                            bit_idx, 
                                            curr_decode.flags.is_individual_enums() ? 0UL : 1UL, // TODO(WALKER): in the future, this will go into bit_str_encoding_array and use the "care_masks" there (for now it is unused)
                                            &curr_bit_str_encoding_info.prev_unsigned_val);
                                        if (curr_decode.flags.is_size_one() && !curr_decode.flags.is_multi_write_op_code()) // special case for size 1 registers (apparently it doesn't like the multi-set if the size is 1, lame)
                                        {
                                            has_errno = modbus_write_register(my_conn.ctx, curr_decode.offset, raw_to_set[0]) == -1;
                                        }
                                        else // write 2 or 4 registers (or single register with multi_write_op_code set to true)
                                        {
                                            has_errno = modbus_write_registers(my_conn.ctx, curr_decode.offset, curr_decode.flags.get_size(), raw_to_set) == -1;
                                        }
                                        if (!has_errno)
                                        {
                                            // This value is no longer debounced (TODO(WALKER): in the future make sure we only mask out the bits we set if we use "individual_enums" using its care mask):
                                            curr_debounce_val = curr_debounce_val.get_uint_unsafe() & ~(1UL << bit_idx); // Set that bit to false so we don't debounce it again next time by accident (clear it in the debounce bits mask)
                                            curr_bit_str_encoding_info.debounce_to_set_mask[debounce_idx] = false;
                                            curr_bit_str_encoding_info.debounce_last_set_times[debounce_idx] = main_workspace.mono_clock.rdns();
                                            if (curr_bit_str_encoding_info.debounce_to_set_mask.none()) my_workspace.debounce_to_set_mask[debounce_idx] = false; // No more values to debounce
                                        }
                                        else // If we have an error setting one bit then all of them can't be set and break out of the set loop:
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else // coil:
                        {
                            if (std::chrono::nanoseconds{main_workspace.mono_clock.rdns() - my_workspace.debounce_last_set_times[debounce_idx]} > my_workspace.debounce)
                            {
                                has_errno = modbus_write_bit(my_conn.ctx, my_workspace.bool_offset_array[debounce_idx], my_workspace.debounce_to_set_bools[debounce_idx]) == -1;
                                if (!has_errno)
                                {
                                    // This value is no longer debounced:
                                    my_workspace.debounce_to_set_mask[debounce_idx] = false;
                                    my_workspace.debounce_last_set_times[debounce_idx] = main_workspace.mono_clock.rdns();
                                }
                            }
                        }
                        // POSSIBLE TODO(WALKER): maybe just "break" the loop due to a single failed set
                        // acquiring the mutex for all our debounced values could be quite costly
                        // might have to do research into how long sets take before returning (timeout?)
                        if (has_errno)
                        {
                            NEW_FPS_ERROR_PRINT("component \"{}\", thread #{}: Could not debounce a value (or \"individual_bit\") properly, due to modbus error: {}\n", comp_name, thread_idx, modbus_strerror(errno));
                            continue;
                        }
                    }
                }
            }
        }
        else // Input and Discrete Input (we only poll the server based on poll_time_left, can still exit based on conditioned exit):
        {
            // This is used by "Input" and "Discrete Input" register workers to determine how long they need to sleep
            auto calc_sleep_time = [&]() {
                const auto curr_time = main_workspace.mono_clock.rdns();
                poll_time_left = my_workspace.frequency - std::chrono::nanoseconds{curr_time - poll_last_time} - poll_overshoot_time;
                return poll_time_left;
            };

            // wait for an action to occur (either we have to poll the server or we need to exit due to component timeout, etc.):
            {
                std::unique_lock<std::mutex> lk{my_workspace.worker_mutex};
                while (!my_workspace.worker_cond.wait_for(lk, sleep_time, [&]() {
                    sleep_time = calc_sleep_time(); // NOTE(WALKER): calc_sleep_time() changes poll_time_left and debounce_time_left during the function call
                    time_to_poll_server = poll_time_left <= 0ns;
                    return !my_workspace.keep_running || time_to_poll_server;
                })) { continue; }
            }
            // Do actions based on condition variable exit (2 actions to take):
            if (time_to_poll_server)
            {
                poll_overshoot_time = (-poll_time_left) % my_workspace.frequency; // set overshoot_time
                poll_last_time = main_workspace.mono_clock.rdns(); // set last poll time to now
                if (!poll_server())
                {
                    if (!has_done_pub_error_message)
                    {
                        has_done_pub_error_message = true;
                        NEW_FPS_ERROR_PRINT("component \"{}\", thread #{}: could not aggregate poll upwards. Consider increasing frequency.\n", comp_name, thread_idx);
                    }
                    // return false;
                }
            }
        }
    }
    return true;
}

int main(const int argc, const char* argv[]) noexcept
{
    bool has_done_mono_clock_calibrate = false;
    main_workspace.mono_clock.init();
    const auto args = parse_command_line_arguments(argc, argv);
    if (args.first == Arg_Types::Error) return EXIT_FAILURE;
    if (args.first == Arg_Types::Help)  return EXIT_SUCCESS;

    do
    {
        if (main_workspace.reload)
        {
            // cleanup stuff here if we reload (all in main_workspace):
            main_workspace.cleanup();
            free(main_workspace.arena.data);
            main_workspace.arena.data = nullptr;
            main_workspace.arena.allocated = 0;
            main_workspace.arena.current_idx = 0;
            main_workspace.client_workspaces = nullptr;
            main_workspace.string_storage.data = nullptr;
        }
        main_workspace.reload = false; // reset reload here if we had one

        if (!load_config(args)) return EXIT_FAILURE;
        if (args.first == Arg_Types::Expand) return EXIT_SUCCESS;

        for (u8 client_idx = 0; client_idx < main_workspace.num_clients; ++client_idx)
        {
            // launch client listener thread:
            auto& client_workspace = *main_workspace.client_workspaces[client_idx];
            if (!client_workspace.conn_workspace.connected) continue; // skip clients that aren't connected (TODO(WALKER): will do restart logic later)

            client_workspace.keep_running = true;
            client_workspace.listener_future = std::async(std::launch::async, listener_thread, client_idx);

            for (u8 comp_idx = 0; comp_idx < client_workspace.num_comps; ++comp_idx)
            {
                // launch component aggregator thread:
                auto& comp_workspace = *client_workspace.msg_staging_areas[comp_idx].comp_workspace;
                comp_workspace.keep_running = true;
                comp_workspace.comp_future = std::async(std::launch::async, aggregator_thread, client_idx, comp_idx);

                // launch worker threads:
                for (u8 thread_idx = 0; thread_idx < comp_workspace.num_threads; ++thread_idx)
                {
                    auto& thread_workspace = *comp_workspace.decoded_caches[thread_idx].thread_workspace;
                    thread_workspace.keep_running = true;
                    // worker thread starts up based on register type:
                    if (thread_workspace.reg_type == Register_Types::Holding)
                    {
                        thread_workspace.thread_future = std::async(std::launch::async, worker_thread<Register_Types::Holding>, client_idx, comp_idx, thread_idx);
                    }
                    else if (thread_workspace.reg_type == Register_Types::Input)
                    {
                        thread_workspace.thread_future = std::async(std::launch::async, worker_thread<Register_Types::Input>, client_idx, comp_idx, thread_idx);
                    }
                    else if (thread_workspace.reg_type == Register_Types::Coil)
                    {
                        thread_workspace.thread_future = std::async(std::launch::async, worker_thread<Register_Types::Coil>, client_idx, comp_idx, thread_idx);
                    }
                    else // Discrete Input:
                    {
                        thread_workspace.thread_future = std::async(std::launch::async, worker_thread<Register_Types::Discrete_Input>, client_idx, comp_idx, thread_idx);
                    }
                }
            }
        }

        if (!has_done_mono_clock_calibrate)
        {
            has_done_mono_clock_calibrate = true;
            // wait a little bit before sending the "start" signal for all the threads (wait 2 seconds and synchronize mono_clock, then tell threads to start):
            NEW_FPS_ERROR_PRINT_NO_ARGS("Waiting 2 seconds to initialize the Mono clock before beginning the main program\n");
            std::this_thread::sleep_for(2s);
            NEW_FPS_ERROR_PRINT("Mono clock finished synchronizing after a 2 second wait, this hardware's frequency = {} GHz. Beginning main program\n", main_workspace.mono_clock.calibrate());
        }

        main_workspace.start_signal = true;
        main_workspace.main_cond.notify_all(); // tell all the threads to start

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock{main_workspace.main_mutex};
                main_workspace.main_cond.wait(lock, [&](){ return !main_workspace.start_signal.load(); });
            }
            main_workspace.client_workspaces[0]->stop_client();
            break;
        }
    } while (main_workspace.reload);
}
