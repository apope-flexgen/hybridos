#pragma once
#include <fims/libfims.h>
#include <future>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <vector>

extern "C" {
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwtargio.h"
}
#include "gcom_dnp3_fims.h"
#include "gcom_dnp3_flags.h"
#include "gcom_dnp3_watchdog.h"
#include "rigtorp/MPMCQueue.h"
#include "shared_utils.hpp"
#include "simdjson.h"
#include <cjson/cJSON.h>
// #include "tscns.h"
#include "version.h"

#ifndef DNP3_MASTER
#define DNP3_MASTER 0
#endif

#ifndef DNP3_OUTSTATION
#define DNP3_OUTSTATION 1
#endif

struct ChannelStats;     // defined in gcom_dnp3_stats.h
struct SessionStats;     // defined in gcom_dnp3_stats.h
struct Timings;          // defined in gcom_dnp3_stats.h
struct PointStatusInfo;  // defined in gcom_dnp3_flags.h
struct Heartbeat;        // defined in gcom_dnp3_heartbeat.h
struct Watchdog;         // defined in gcom_dnp3_watchdog.h
struct GcomSystem;       // defined below

enum class FimsMethod : uint8_t;
enum class FimsFormat : uint8_t;
enum FimsEventSeverity : uint8_t;
struct FimsReceiver
{
    Meta_Data_Info meta_data;
    uint8_t* data_buf;
    uint32_t data_buf_len;

    ~FimsReceiver()
    {
        if (data_buf != nullptr)
        {
            free(data_buf);
        }
    }
};

struct PointGroup
{
    std::string group_name;
    std::map<std::string, TMWSIM_POINT*> points;
};

struct char_dcmp
{
    bool operator()(std::string a, std::string b) const { return a < b; }
};

struct SetWork
{
    fmt::memory_buffer send_uri;
    fmt::memory_buffer send_buf;
    TMWSIM_POINT* dbPoint;
    double value;
    TMWTYPES_UCHAR count;
    TMWTYPES_ULONG onTime;
    TMWTYPES_ULONG offTime;
};

struct PubPoint
{
    TMWSIM_POINT* dbPoint;
    double value;
    TMWTYPES_UCHAR flags;
    TMWDTIME changeTime;
};

typedef struct FlexPoint_t
{
public:
    GcomSystem* sys;
    bool crob_string;       // whether or not the CROB value is represented as a string
                            // in fims messages
    bool crob_int;          // whether or not the CROB value is represented as an int in
                            // fims messages
    bool crob_bool;         // whether or not the CROB value is represented as true/false
                            // (output active/inactive) in fims messages
    bool batch_sets;        // when we get lots of commands, do we "batch" them so we
                            // only send out the last one every batch interval?
    bool interval_sets;     // when we get a single command, keep sending the set out
                            // at an interval, regardless of if we keep receiving that
                            // set
    bool batch_pubs;        // when we get an unsolicited event, do we publish it
                            // independently? -- irrelevant if unsolUpdate is false
    bool interval_pubs;     // pub data out at a regular interval that is outside the
                            // regular integrity poll/class scan rate -- always
                            // relevant for client but irrelevant for server if
                            // unsolUpdate is false
    int batch_set_rate;     // how often to pub out events if batch_pubs is true
    int interval_set_rate;  // how often to send out sets if interval_sets is true
    int batch_pub_rate;     // how often to send out sets if batch_pubs is true
    int interval_pub_rate;  // how often to send out sets if interval_pubs is true
    bool direct_sets;
    bool direct_pubs;
    bool event_pub;
    bool show_output_status = false;  // do we want to publish the output status in
                                      // addition to the operate value? (i.e. 40
                                      // AND 41? or just 41?)
    bool sent_operate = false;
    bool is_output_point = false;                      // is it an input or an output? (e.g. Analog
                                                       // Input vs. Analog Output)
    bool is_forced = false;                            // is the point currently in forced mode?
    bool sent_operate_before_or_during_force = false;  // did we receive a REAL "set" on the
                                                       // client while the point was being forced?
    bool pulse_crob = false;  // Do we send pulse_on/pulse_off commands or latch_on/latch_off commands (CROBs only)
    bool is_enabled = true;   // Basically, if a point is disabled, we pretend it doesn't exist in our system.
                              // DNP3 requests will still contain data for this point, but we will not update point
                              // values, nor send anything out over fims for this point. Output points will not send out
                              // operate commands over DNP3 if disabled on the client side, nore will they send out fims
                              // sets if disabled on the server side.

    Register_Types type;
    std::string name;                            // how this point is identified
    const char* site_uri;                        // when running in server mode this will be the uri we
                                                 // listen to pubs on -- This may need to be per site
                                                 // (not per point)
    const char* uri;                             // fims uri
    char* output_status_uri;                     // the fims uri that we publish output status to
    char* last_update_username = new char[256];  // the fims username that sent the
                                                 // last set (if client) or pub (if
                                                 // server) for this point
    char* last_update_process = new char[256];   // the fims username that sent the
                                                 // last set (if client) or pub (if
                                                 // server) for this point

    double scale;                    // scale based on how we're representing the value on this side
                                     // of the connection (e.g. x1000, x10, /1000, /10)
    double offset;                   // shift based off of how we're representing the value on this
                                     // side of the connection
    double timeout;                  // used to detect COMM_LOSS
    double standby_value = 0;        // (server: ) value is currently in LOCAL_FORCED mode, and we
                                     // are saving this value for when we clear that flag
                                     // (client inputs: ) value is currently being forced, and this is the
                                     // value that we want to publish instead of the actual value
                                     // (client outputs: ) value is currently being forced, and this is the
                                     // value that we last set to the server prior to forcing
    double operate_value = 0;        // the last value sent as an operate command from
                                     // the client (Group 12 or Group 41); this is what we will
                                     // publish by default
    double resend_tolerance = 0;     // do we send another operate command if the
                                     // output status differs by specific tolerance?
                                     // (only after value has adjusted)
    double resend_rate_ms = 0;       // how often do we send another operate command if
                                     // the output status differs by specific tolerance?
                                     // Measured in milliseconds.
    double last_operate_time = 0;    // measured in seconds
    int pulse_on_ms = 0;             // for CROBS; on the client side: determines how long the "on" time is.
                                     // On the server side: send out a set of "true", then send out a set of
                                     // "false" XXX milliseconds later -- see dnp3 manual pg 507 for more details
    int pulse_off_ms = 0;            // for CROBS; on the client side: determines how long the "off" time is.
                                     // On the server side: send out a set of "false", then send out a set of
                                     // "true" XXX milliseconds later -- see dnp3 manual pg 507 for more details
    int pulse_count = 0;             // for CROBS; how many pulses -- see dnp3 manual pg 507 for more details
    int num_switches_completed = 0;  // for CROBS; used to implement pulses on server side

    std::chrono::time_point<std::chrono::system_clock> last_pub;  // time of last update
    TMWTYPES_UCHAR lastFlags = DNPDEFS_DBAS_FLAG_RESTART;

    FimsFormat format;  // how do we publish values? Naked, clothed, full

    TMWTIMER pub_timer;  // used for batching outputs or sending them out on an
                         // interval (if a point comes in many times, limit output
                         // rate to event_rate)
    TMWTIMER set_timer;  // used for batching outputs or sending them out on an
                         // interval (if a point comes in many times, limit output
                         // rate to event_rate)
    SetWork set_work;    // when sending out sets, this will hold what we need to
                         // send out values

    TMWTIMER timeout_timer;  // used to detect COMM_LOSS

    std::string crob_true = std::string("LATCH_ON");    // WHEN the crob is true, what is the string
                                                        // representation (e.g. "POWER_ON", "LATCH_ON",
                                                        // etc.)
    std::string crob_false = std::string("LATCH_OFF");  // WHEN the crob is false, what is the string
                                                        // representation (e.g. "POWER_OFF",
                                                        // "LATCH_OFF", etc.)

    bool is_bitfield = false;
    bool is_enum = false;
    bool is_individual_bits = false;
    std::vector<std::pair<std::string, uint32_t>> dbBits;  // if a point is
                                                           // represented as a bit
                                                           // string, these are the
                                                           // corresponding bits

    int value_set;  // how many times has this value been set?

    FlexPoint_t(GcomSystem* pSys, const char* iname, const char* iuri)
    {
        memset(last_update_username, '\0', 256);
        memset(last_update_process, '\0', 256);
        site_uri = nullptr;
        output_status_uri = nullptr;
        show_output_status = false;
        sys = pSys;
        scale = 0.0;
        timeout = 0.0;
        crob_string = false;
        crob_int = false;
        value_set = 0;
        lastFlags = DNPDEFS_DBAS_FLAG_RESTART;

        batch_sets = false;
        interval_sets = false;
        batch_pubs = false;
        interval_pubs = false;

        direct_sets = true;
        direct_pubs = true;

        batch_set_rate = 0;
        interval_set_rate = 0;
        batch_pub_rate = 0;
        interval_pub_rate = 0;
        lastFlags = DNPDEFS_DBAS_FLAG_RESTART;
        format = FimsFormat::Naked;

        if (iname)
        {
            name = std::string(iname);
        }
        else
        {
            name = "";
        }

        if (iuri)
        {
            uri = strdup(iuri);
        }
        else
        {
            uri = "";
        }
    };
    ~FlexPoint_t()
    {
        if (site_uri != nullptr)
        {
            free((void*)site_uri);
        }
        if (output_status_uri != nullptr)
        {
            free((void*)output_status_uri);
        }
        if (uri != nullptr)
        {
            free((void*)uri);
        }
        if (last_update_process != nullptr)
        {
            delete[] last_update_process;
        }
        if (last_update_username != nullptr)
        {
            delete[] last_update_username;
        }
    }
} FlexPoint;

typedef struct varList_t
{
    varList_t(const char* iuri) { uri = iuri; };
    ~varList_t(){};

    void addDb(TMWSIM_POINT* dbPoint) { dbmap[((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str()] = dbPoint; }
    int size() { return (int)dbmap.size(); }

    std::string uri;
    bool multi;
    bool bit;
    std::map<std::string, TMWSIM_POINT*> dbmap;
} varList;

struct PubWork
{
    std::string pub_uri;
    std::map<std::string, PubPoint*> pub_vals;

    ~PubWork()
    {
        for (auto pair : pub_vals)
        {
            if (pair.second)
            {
                delete pair.second;
                pair.second = nullptr;
            }
        }
    }
};
struct FimsDependencies
{
    std::string name = std::string("");
    fims fims_gateway;
    bool (*parseBody)(GcomSystem& sys, Meta_Data_Info& meta_data);

    FimsReceiver receiver_bufs;
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;

    FimsMethod method;

    std::mutex uris_with_data_mutex;
    std::map<std::string, std::vector<PubWork*>> uris_with_data;
    rigtorp::MPMCQueue<PubWork*> pub_q{ 256 };

    std::vector<std::string> subs;

    std::string_view method_view;
    std::string_view uri_view;
    std::string_view replyto_view;
    std::string_view process_name_view;
    std::string_view username_view;
    UriRequest uri_requests;
    char* data_buf;
    int data_buf_len = 20000;

    FimsFormat format;  // may be overridden by dbPoint->format
    fmt::memory_buffer send_buf;
    double max_pub_delay;

    FimsDependencies()
    {
        parseBody = nullptr;
        receiver_bufs.data_buf = nullptr;
        data_buf = nullptr;
        format = FimsFormat::Naked;
    };

    ~FimsDependencies()
    {
        for (auto pair : uris_with_data)
        {
            for (auto pub_work_item : pair.second)
            {
                if (pub_work_item)
                    delete pub_work_item;
            }
        }
    };
};

struct ModbusDependencies
{
};

struct DNP3Dependencies
{
    bool (*openTMWChannel)(GcomSystem& sys);
    bool (*openTMWSession)(GcomSystem& sys);

    TMWTARGIO_CONFIG IOConfig;
    MDNPSESN_CONFIG clientSesnConfig;
    SDNPSESN_CONFIG serverSesnConfig;
    DNPCHNL_CONFIG channelConfig;
    DNPTPRT_CONFIG tprtConfig;  // transport layer
    DNPLINK_CONFIG linkConfig;  // link layer
    TMWPHYS_CONFIG physConfig;  // physical layer
    TMWTARG_CONFIG targConfig;  // target layer

    MDNPBRM_REQ_DESC pBinaryCommandRequestDesc;
    MDNPBRM_REQ_DESC pAnalogCommandRequestDesc;
    MDNPBRM_REQ_DESC pIntegrityRequestDesc;
    MDNPBRM_REQ_DESC pClass1RequestDesc;
    MDNPBRM_REQ_DESC pClass2RequestDesc;
    MDNPBRM_REQ_DESC pClass3RequestDesc;
    MDNPBRM_ANALOG_INFO* analogOutputVar1Values;
    MDNPBRM_ANALOG_INFO* analogOutputVar2Values;
    MDNPBRM_ANALOG_INFO* analogOutputVar3Values;
    MDNPBRM_ANALOG_INFO* analogOutputVar4Values;
    int count_var1_requests = 0;
    int count_var2_requests = 0;
    int count_var3_requests = 0;
    int count_var4_requests = 0;
    MDNPBRM_CROB_INFO* CROBInfo;

    TMWAPPL* pApplContext;
    TMWCHNL* pChannel;
    TMWSESN* pSession;
    uint8_t* pUserHandle;  // usually just a pointer to an int for now

    void* dbHandle;  // can be MDNPSIM_DATABASE* or SDNPSIM_DATABASE*
    ChannelStats* channel_stats;
    SessionStats* session_stats;
    Timings* timings;
    int stats_pub_frequency = 0;
    char* stats_pub_uri;
    char* output_status_uri;
    PointStatusInfo* point_status_info;
    std::map<std::string, PointGroup> point_group_map;

    bool pub_outputs = false;
    bool show_output_status = false;
    bool pub_all = false;
    bool sign = true;

    double resend_tolerance = 0;
    double resend_rate_ms = 0;
    int freq1 = 0;
    int freq2 = 0;
    int freq3 = 0;
    bool unsolUpdate = false;
    bool event_pub;         // IF unsolUpdate is true, do we publish out events for ALL
                            // points? (By default, this value is "true". Can be
                            // overridden by individual points.)
    int batch_set_rate;     // how often to pub out events if batch_pubs is true
    int interval_set_rate;  // how often to send out sets if interval_sets is true
    int batch_pub_rate;     // how often to send out sets if batch_pubs is true
    int interval_pub_rate;  // how often to send out sets if interval_pubs is true

    int event_buffer = 100;

    DNP3Dependencies()
    {
        // We may need to remove these null pointers. I don't
        // know if they break things or fix things...
        openTMWChannel = nullptr;
        openTMWSession = nullptr;
        pApplContext = nullptr;
        pChannel = nullptr;
        pSession = nullptr;
        pUserHandle = nullptr;
        dbHandle = nullptr;
        stats_pub_uri = nullptr;
        output_status_uri = nullptr;
        analogOutputVar1Values = nullptr;
        analogOutputVar2Values = nullptr;
        analogOutputVar3Values = nullptr;
        analogOutputVar4Values = nullptr;
        CROBInfo = nullptr;
        point_status_info = nullptr;
        channel_stats = nullptr;
        session_stats = nullptr;
        timings = nullptr;

        pub_all = false;
        batch_set_rate = 0;
        interval_set_rate = 0;
        batch_pub_rate = 0;
        interval_pub_rate = 0;
        event_pub = true;
        show_output_status = false;
    };

    ~DNP3Dependencies()
    {
        if (stats_pub_uri != nullptr)
        {
            free(stats_pub_uri);
        }
        if (output_status_uri != nullptr)
        {
            free(output_status_uri);
        }
    };
};

struct ProtocolDependencies
{
    ModbusDependencies modbus;
    DNP3Dependencies dnp3;
    bool connected = false;  // To tell main whether or not this workspace is up or not
    bool use_tls = false;

    Conn_Type conn_type = Conn_Type::TCP;  // TCP, RTU
    Mode mode = Mode::Client;              // client, server
    Protocol protocol;                     // modbus or dnp3

    int frequency = 0;
    int port = 0;
    double deadband = 0;

    int baud = 0;
    int dataBits = 0;
    double stopBits = 0;
    char* parity;

    char* flowType;
    int asyncOpenDelay = 0;
    char* deviceName;
    TMWTARG232_DATA_BITS data_bits = TMWTARG232_DATA_BITS_8;  // for direct TMW use
    TMWTARG232_STOP_BITS stop_bits = TMWTARG232_STOP_BITS_1;  // for direct TMW use
    TMWTARG232_PARITY parity_type = TMWTARG232_PARITY_NONE;   // for direct TMW use
    TMWTARG232_PORT_MODE port_mode = TMWTARG232_MODE_NONE;    // for direct TMW use

    int master_address = 1;
    int station_address = 10;
    char* ip_address;

    int who = 0;

    double timeout = 0;
    double respTime = 0;
    int maxElapsed = 0;

    ProtocolDependencies(Protocol param_protocol)
    {
        deviceName = nullptr;
        parity = nullptr;
        flowType = nullptr;
        ip_address = nullptr;
        protocol = param_protocol;
        if (protocol == Protocol::DNP3)
        {
            DNP3Dependencies();
        }
        else
        {
            ModbusDependencies();
        }
    };

    ProtocolDependencies()
    {
        deviceName = nullptr;
        parity = nullptr;
        flowType = nullptr;
        ip_address = nullptr;
    };

    ~ProtocolDependencies()
    {
        if (parity != nullptr)
        {
            free(parity);
        }
        if (flowType != nullptr)
        {
            free(flowType);
        }
        if (deviceName != nullptr)
        {
            free(deviceName);
        }
        if (ip_address != nullptr)
        {
            free(ip_address);
        }
    };
};

struct GcomSystem
{
    Heartbeat* heartbeat;
    Watchdog* watchdog;
    bool start_signal;  // to synchronize every single thread upon first startup
    bool keep_running;
    // TSCNS mono_clock;
    std::future<bool> listener_future;
    std::future<bool> stats_pub_future;
    std::future<bool> watchdog_future;
    std::future<bool> heartbeat_future;
    std::mutex main_mutex;
    std::condition_variable main_cond;
    cJSON* config;

    FimsDependencies* fims_dependencies;
    ProtocolDependencies* protocol_dependencies;
    std::string config_file_name;
    char* id;
    char* defUri;
    char* base_uri;
    char* local_uri;

    int debug;

    std::map<std::string, varList*> dburiMap;
    std::map<std::string, varList*> outputStatusUriMap;
    std::map<std::string, varList*> individualBitsMap;
    std::shared_mutex db_mutex;  // for R/W locking of TMWSIM_POINTs
    std::mutex error_mutex;

    double dbi_save_frequency_seconds;
    TMWTIMER dbi_save_timer;
    int parse_errors = 0;
    int point_errors = 0;
    int fims_errors = 0;
    int comms_errors = 0;
    int heartbeat_errors = 0;
    int watchdog_errors = 0;

    Version git_version_info;

    GcomSystem()
    {
        dbi_save_frequency_seconds = 0;
        debug = 0;
        parse_errors = 0;
        point_errors = 0;
        fims_errors = 0;
        comms_errors = 0;
        heartbeat_errors = 0;
        watchdog_errors = 0;

        start_signal = false;
        keep_running = false;

        fims_dependencies = new FimsDependencies();
        protocol_dependencies = new ProtocolDependencies();
        id = nullptr;
        defUri = nullptr;
        base_uri = nullptr;
        local_uri = nullptr;
        config = nullptr;
        watchdog = nullptr;
        heartbeat = nullptr;
        fims_dependencies = nullptr;
        protocol_dependencies = nullptr;
    };

    GcomSystem(Protocol protocol)
    {
        fims_dependencies = new FimsDependencies();
        protocol_dependencies = new ProtocolDependencies(protocol);
        id = nullptr;
        defUri = nullptr;
        base_uri = nullptr;
        local_uri = nullptr;
        config = nullptr;
        watchdog = nullptr;
        heartbeat = nullptr;
        debug = 0;
        start_signal = false;
        keep_running = false;
    };

    ~GcomSystem()
    {
        if (fims_dependencies != nullptr)
        {
            delete fims_dependencies;
        }
        if (protocol_dependencies != nullptr)
        {
            delete protocol_dependencies;
        }
        if (id != nullptr)
        {
            free(id);
        }
        if (defUri != nullptr)
        {
            free(defUri);
        }
        if (base_uri != nullptr)
        {
            free(base_uri);
        }
        if (local_uri != nullptr)
        {
            free(local_uri);
        }
        if (config != nullptr)
        {
            cJSON_Delete(config);
        }
        for (auto& pair : dburiMap)
        {
            if (pair.second != nullptr)
            {
                delete pair.second;  // Deallocate memory for varList* value
            }
        }
        dburiMap.clear();  // Clear the map
        for (auto& pair : outputStatusUriMap)
        {
            if (pair.second != nullptr)
            {
                delete pair.second;  // Deallocate memory for varList* value
            }
        }
        outputStatusUriMap.clear();  // Clear the map

        for (auto& pair : individualBitsMap)
        {
            if (pair.second != nullptr)
            {
                delete pair.second;  // Deallocate memory for varList* value
            }
        }
        individualBitsMap.clear();  // Clear the map
    };
};

template <>
struct fmt::formatter<TMWSIM_POINT>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const TMWSIM_POINT& dbPoint, FormatContext& ctx) const
    {
        FlexPoint* flexPoint = (FlexPoint*)dbPoint.flexPointHandle;
        fmt::format_to(ctx.out(), R"({{)");
        fmt::format_to(ctx.out(), R"("name": "{}", )", flexPoint->name);
        fmt::format_to(ctx.out(), R"("uri": "{}", )", flexPoint->uri);
        fmt::format_to(ctx.out(), R"("register_type": "{}", )", flexPoint->type);
        fmt::format_to(ctx.out(), R"("point_num": {}, )", dbPoint.pointNumber);
        fmt::format_to(ctx.out(), R"("static_variation": {}, )", dbPoint.defaultStaticVariation);
        fmt::format_to(ctx.out(), R"("event_variation": {}, )", dbPoint.defaultEventVariation);
        if (dbPoint.type == TMWSIM_TYPE_ANALOG)
        {
            fmt::format_to(ctx.out(), R"("raw_value": {}, )", dbPoint.data.analog.value);
            if (flexPoint->scale == 0)
            {
                fmt::format_to(ctx.out(), R"("scaled_value": {}, )", dbPoint.data.analog.value);
            }
            else
            {
                fmt::format_to(ctx.out(), R"("scaled_value": {}, )", dbPoint.data.analog.value / flexPoint->scale);
            }
            fmt::format_to(ctx.out(), R"("deadband": {}, )", dbPoint.data.analog.deadband);
        }
        else
        {
            fmt::format_to(ctx.out(), R"("raw_value": {}, )", dbPoint.data.binary.value ? "true" : "false");
            if (flexPoint->scale < 0)
            {
                fmt::format_to(ctx.out(), R"("scaled_value": {}, )", (!dbPoint.data.binary.value) ? "true" : "false");
            }
            else
            {
                fmt::format_to(ctx.out(), R"("scaled_value": {}, )", dbPoint.data.binary.value ? "true" : "false");
            }
        }
        fmt::format_to(ctx.out(), R"("is_forced": {},)", flexPoint->is_forced);
        fmt::format_to(ctx.out(), R"("flags": {},)", DNP_FLAGS{ dbPoint.flags });
        if (flexPoint->timeout > 0)
        {
            fmt::format_to(ctx.out(), R"("timeout": {}, )", flexPoint->timeout);
        }
        if (flexPoint->batch_pubs)
        {
            fmt::format_to(ctx.out(), R"("batch_pub_rate": {}, )", flexPoint->batch_pub_rate);
        }
        if (flexPoint->interval_pubs)
        {
            fmt::format_to(ctx.out(), R"("interval_pub_rate": {}, )", flexPoint->interval_pub_rate);
        }
        if (flexPoint->batch_sets)
        {
            fmt::format_to(ctx.out(), R"("batch_set_rate": {}, )", flexPoint->batch_set_rate);
        }
        if (flexPoint->interval_sets)
        {
            fmt::format_to(ctx.out(), R"("interval_set_rate": {}, )", flexPoint->interval_set_rate);
        }
        const TMWDTIME* tmpPtr = &(dbPoint.timeStamp);
        fmt::format_to(ctx.out(),
                       R"("value_last_updated":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}", )", tmpPtr->year,
                       tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000,
                       tmpPtr->mSecsAndSecs % 1000);
        fmt::format_to(ctx.out(), R"("value_updated_by_process":"{}", )", flexPoint->last_update_process);
        fmt::format_to(ctx.out(), R"("value_updated_by_username":"{}")", flexPoint->last_update_username);
        return fmt::format_to(ctx.out(), R"(}})");
    }
};

template <>
struct fmt::formatter<PointGroup>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const PointGroup& point_group, FormatContext& ctx) const
    {
        fmt::format_to(ctx.out(), R"({{)");
        auto it = point_group.points.begin();
        for (it = point_group.points.begin(); it != std::prev(point_group.points.end()); it++)
        {
            const TMWSIM_POINT* dbPoint = it->second;
            fmt::format_to(ctx.out(), R"("{}": )", it->first);
            fmt::format_to(ctx.out(), R"({{)");
            fmt::format_to(ctx.out(), R"("uri": "{}", )", ((FlexPoint*)dbPoint->flexPointHandle)->uri);
            fmt::format_to(ctx.out(), R"("flags": {})", DNP_FLAGS{ dbPoint->flags });
            fmt::format_to(ctx.out(), R"(}}, )");
        }
        const TMWSIM_POINT* dbPoint = it->second;
        fmt::format_to(ctx.out(), R"("{}": )", it->first);
        fmt::format_to(ctx.out(), R"({{)");
        fmt::format_to(ctx.out(), R"("uri": "{}", )", ((FlexPoint*)dbPoint->flexPointHandle)->uri);
        fmt::format_to(ctx.out(), R"("flags": {})", DNP_FLAGS{ dbPoint->flags });
        fmt::format_to(ctx.out(), R"(}})");
        return fmt::format_to(ctx.out(), R"(}})");
    }
};