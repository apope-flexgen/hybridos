#ifndef GCOM_CONFIG_H
#define GCOM_CONFIG_H

// gcom config headers
// p. wilshire
// 11_22_2023

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <any>
#include <map>
#include <vector>
#include <optional>
#include <iostream>


#include <simdjson.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/compile.h>
#include <spdlog/fmt/bundled/format.h>

#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <algorithm> // for std::find
#include <variant>
#include <stdint.h>
#include <float.h>

#include "modbus/modbus.h"
#include "fims/libfims.h"
#include "version.h"


#include "gcom_perf.h"
#include "shared_utils.h"


using namespace std::string_view_literals;

typedef uint64_t u64;
typedef int64_t s64;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t u8;

enum class Fims_Format {
    Naked,
    Clothed,
    Full,
    Undefined
};

struct cfg
{

    enum class Register_Types : uint8_t
    {
        Holding,
        Input,
        Coil,
        Discrete_Input,
        Undefined
    };

    struct register_group_struct;
    struct component_struct;
    struct watchdog_struct;
    struct heartbeat_struct;

    struct connect_struct
    {
        std::string device_name;
        std::string name;
        std::string ip_address;
        std::string protocol;

        int port;
        int max_num_connections;
        int device_id;
        double connection_timeout; // default 2 secs
        double transfer_timeout;   // default 0.5 sec
        int data_buffer_size;   // default 100000
        std::string serial_device;
        int baud_rate;
        int data_bits;
        int stop_bits;
        char parity;
        bool debug;
        bool multi_write_op_code = false;
        bool off_by_one = false;
        bool is_RTU;
        std::string format;
        std::string stats_uri;
        int stats_frequency_ms;

    } connection;

    struct performance_timers_struct {
        Stats modbus_read_timer;
        Stats modbus_write_timer;
        Stats pub_timer;

        performance_timers_struct(){
            modbus_read_timer.set_label("Modbus Read Timer");
            modbus_write_timer.set_label("Modbus Write Timer");
            pub_timer.set_label("End-to-End Pub Timer");
        };
    } performance_timers;

    struct inherited_fields_struct
    {
        bool off_by_one;
        bool byte_swap;
        bool multi_write_op_code;
        int frequency;
        int device_id;
        int debounce;  // in milliSecs
        
    } inherited_fields;

    struct io_point_struct
    {
        ~io_point_struct()
        {
        }
        std::string id;
        std::string name;
        std::string register_type_str;
        std::any value = (uint64_t)0;
        std::any last_value = (uint64_t)0;
        std::any set_value = (uint64_t)0;
        Fims_Format format = Fims_Format::Undefined;

        int offset;
        int size;
        int gap = 0;

        int starting_bit_pos;
        int number_of_bits;
        int bit_mask;

        int shift;
        int scale;
        bool normal_set;

        u64 invert_mask;
        u64 care_mask;   // new
        bool uses_masks; //  new
        bool off_by_one = false;

        bool is_float;
        bool is_signed;
        int word_order=0; // 1234 , 1324, 3412 etc used to decode 4 register systemsfrom odd servers
        bool is_byte_swap;
        int byte_index[4];

        // TODO
        bool is_bit_string_type;
        bool is_individual_bits;
        bool is_bit_field;

        bool packed_register;
        // this stops lots of leaks
        std::weak_ptr<io_point_struct> packer;

        u8 bit_index;
        // io_point.bit_str
        u64 bits_known;
        u64 bits_unknown;
        std::vector<std::string> bit_str;
        std::vector<bool> bit_str_known;
        std::vector<int> bit_str_num;
        std::vector<std::shared_ptr<io_point_struct>> bit_ranges;

        // std::vector<int>bit_num;   // maybe
        // todo read in the array
        bool is_enum = false;
        bool is_random_enum = false;
        bool is_forced = false;
        bool is_enabled = true;
        bool is_disconnected = false;  // this is set when we cant find the point on the server
        double reconnect = 0.0;

        u64 forced_val = 0;
        u64 raw_val = 0;

        u8 reg8[4]; // holds the raw data
        u16 reg16[4];
        Register_Types register_type;
        std::weak_ptr<io_point_struct> next;  // used in packed
        int device_id;

        double debounce_time = 0.0; // time when debounce is turned off
        double debounce = 0.0;      // time to debounce
        double deadband = 0.0;      // time when debounce is turned off
        bool use_debounce;
        bool use_deadband;

        double float_val;      // used in deadband
        double last_float_val; // used in deadband
        u64 last_raw_val;      // used in deadband

        // todo fix these up
        bool use_bool = true;
        bool use_hex = false;
        bool use_raw = false;
        bool is_bit = false;
        
        // filled in when we service sets or pubs
        std::string process_name;
        std::string username;

        bool is_watchdog = false;
        std::weak_ptr<cfg::watchdog_struct> watchdog_point;

        bool multi_write_op_code = false;
        int errno_code;
        int error;  //io->work error code
        bool data_error = false;  //io->work error code
        
        register_group_struct* register_group;
        component_struct* component;
        std::mutex mtx;
        double tUpdate = 0.0;
        double tlastUpdate = 0.0;

        static bool compare(const io_point_struct &a, const io_point_struct &b)
        {
            return a.offset < b.offset;
        };
    };

    struct register_group_struct
    {
        std::string register_type_str;
        Register_Types register_type;
        int starting_offset;
        int number_of_registers;
        int device_id;
        bool enabled = true;
        bool is_byte_swap;// = false;
        bool off_by_one;// = false;

        int word_order=0; // 1234 , 1324, 3412 etc used to decode 4 register systemsfrom odd servers

        bool multi_write_op_code = false;
        Fims_Format format = Fims_Format::Undefined;

        std::vector<std::shared_ptr<io_point_struct>> io_point_map;
        std::map<int, std::shared_ptr<io_point_struct>> io_point_map_lookup; // faster lookup for maps
        std::string id; // this is the second part of the uri. e.g. "comp_sel_2440" in the uri /components/comp_sel_2440 - inherited from the component that this register group belongs to
        std::string component_id; // this is the first part of the uri. e.g. "components" in /components/comp_sel_2440 - inherited from the component that this register group belongs to
        component_struct* component;
    };

    struct component_struct
    {
        std::string component_id; // this is the first part of the uri. e.g. "components" in /components/comp_sel_2440
        std::string id; // this is the second part of the uri. e.g. "comp_sel_2440" in the uri /components/comp_sel_2440

        int frequency;
        int offset_time;
        int device_id;
        int word_order=0; // 1234 , 1324, 3412 etc used to decode 4 register systemsfrom odd servers

        bool is_byte_swap = false;
        bool off_by_one = false;

        bool heartbeat_enabled = false;
        bool watchdog_enabled = false;
        bool use_bool = true;

        cfg::heartbeat_struct* heartbeat = nullptr;
        int  component_heartbeat_max_value = 0;
        std::string component_heartbeat_read_uri;
        std::string component_heartbeat_write_uri;
        int  modbus_heartbeat_timeout_ms;
        int  component_heartbeat_timeout_ms;
        
        cfg::watchdog_struct* watchdog = nullptr;
        std::string watchdog_uri;
        int watchdog_fault_timeout; // normal->alarm->fault->recovery->fault
        int watchdog_alarm_timeout;
        int watchdog_recovery_timeout;
        int watchdog_time_to_recover;
        int watchdog_frequency; // monitoring frequency

        Fims_Format format = Fims_Format::Undefined;

        std::vector<std::shared_ptr<register_group_struct>> register_groups;
        double syncPct     = 0.0; // if polls take too long to retrieve data, what is our threshold (as a percent of pub frequency) for pushing back the next poll?
        bool use_sync = false; // do we push back polls if they're excessively late?

        struct cfg *myCfg;
    };

    // TODO complete this
    struct heartbeat_struct
    {
        std::string id;
        Stats heartbeatStats;

        bool enabled = false;
        bool init = false;  // no action until we start getting action
        bool first_val = true;  // no action until we start getting action

        double tLate;    // when is it late
        double tAvg;
        double tMax;
        double tTotal;

        double last_time=0.0;

        bool state_normal =  true;
        bool state_fault = false;
        bool state_frozen = true;
        std::string state_str = "INIT";

        //bool state_alarm;
        //bool state_recovery;

        bool value_changed;
        u64 num_changes = 0; 
        u64 value = 0; 
        u64 last_val = 0; 
        int count;
        int frequency=1000;
        int timeout;
        int offset_time=0;
        int device_id;
        u64 max_value = 0;
        struct component_struct* component;
        std::string component_heartbeat_read_uri;
        std::string component_heartbeat_write_uri;

        std::shared_ptr<struct io_point_struct>heartbeat_write_point = nullptr;
        std::shared_ptr<struct io_point_struct>heartbeat_read_point = nullptr;
        struct cfg *cfg;
        void setupHb(struct cfg &myCfg, const std::string &name, int device_id
                                        , std::shared_ptr<struct io_point_struct> io_point
                                        , std::shared_ptr<struct io_point_struct> io_ref
                                        , int hbTime , int hbOffset = 0);
        void touchHb(struct cfg *myCfg, const std::string &name, std::shared_ptr<struct io_point_struct> io_point);
        void checkHb(struct cfg *myCfg, const std::string &name, std::shared_ptr<struct io_point_struct> io_point);

        //std::vector<std::shared_ptr<register_group_struct>> disabled_regs;
    };

    // TODO complete this
    struct watchdog_struct
    {
        std::string id;
        Stats watchdogStats;
        bool enabled = false;

        double faultTimeout = DBL_MAX;
        double timeOfFault = 0;
        double maxFaultToRecoveryTime = 0;

        double alarmTimeout = DBL_MAX;
        double timeOfAlarm = 0;
        double maxAlarmToRecoveryTime = 0;

        double recoveryTimeout = DBL_MAX;
        double timeOfRecoveryPeriodStart = 0;
        double timeInRecovery = 0;

        double timeRequiredToRecover = 0;

        bool state_init = true;
        bool state_normal = false;
        bool state_fault = false;
        bool state_alarm = false;
        bool state_recovery = false;
        std::string state_str = "INIT";

        uint64_t value = 0;
        int value_changed = 0;
        int last_value_changed = 0;
        int frequency=1000;
        int offset_time=0;
        int device_id;

        component_struct* component = nullptr;
        std::shared_ptr<struct io_point_struct> io_point = nullptr;
        struct cfg *cfg;

        void setupWatchdogTimer(struct cfg &myCfg, const std::string &name, double watchdog_frequency, double watchdog_offset);
        void touchWatchdog();
        void checkWatchdog();
        void setNormalState();
        void setFaultState();
        void setAlarmState();
        void setRecoveryState();
        void disable();
        void enable();
    };


    struct pub_struct
    {
        ~pub_struct()
        {};
        std::string id;
        Stats pubStats;
        double tLate;    // when is it late
        std::mutex pmtx;
        int frequency;
        int offset_time;
        struct component_struct* component;
        struct cfg *cfg;
        double syncPct     = 0.0; // if polls take too long to retrieve data, what is our threshold (as a percent of pub frequency) for pushing back the next poll?
        bool use_sync = false; // do we push back polls if they're excessively late?
        int pending = 0;
        int pub_threads = 0;
    };


    std::vector<std::shared_ptr<component_struct>> components;
    std::vector<std::string> subs;

    std::map<std::string, std::shared_ptr<struct heartbeat_struct>> heartbeat_points;
    std::map<std::string, std::shared_ptr<struct watchdog_struct>> watchdog_points;
    std::map<std::string, std::shared_ptr<struct pub_struct>> pubs;


    // config_any
    void addPub(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg);
    void addWatchdog(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg);
    void addWatchdog(std::shared_ptr<io_point_struct> io_point, std::string &pstr,std::shared_ptr<component_struct> component,struct cfg *myCfg);
    void addOldWatchdog(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg);
    void addOldWatchdog(std::shared_ptr<io_point_struct> io_point, std::string &pstr,std::shared_ptr<component_struct> component,struct cfg *myCfg);
    void addHeartbeat(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg);
    void addSub(const std::string &base, const std::string &component_name);

    std::vector<std::string> *getSubs();

    Register_Types typeFromStr(std::string &register_type);
    std::string typeToStr(Register_Types register_type);
    std::string typeToServerStr(Register_Types register_type);

    using Component_IO_point_map = std::map<std::string, std::map<std::string, std::map<std::string, std::shared_ptr<io_point_struct>>>>;
    using MapIdMap = std::map<int, std::map<Register_Types, std::map<int, io_point_struct *>>>;
    Component_IO_point_map component_io_point_map;
    MapIdMap idMap;

    /**
     * @brief Given a set of uri fragments, find the io_point map for a given component.
     * 
     * For example, the uri is /components/sel_3530_slow_rtac/some_point, the keys passed
     * to the function should be [components,sel_3530_slow_rtac,some_point]. The return value
     * will be all of the points associated with /components/sel_3530_slow_rtac.
     * 
     * Currently only used in gcom_modbus_test.
     * 
     * @param keys the uri fragments from a fims message, used to look up the io_point map for a component
    */
    std::map<std::string, std::shared_ptr<cfg::io_point_struct>> *findIOPointMapFromUriFragments(std::vector<std::string> keys);

    fims fims_gateway;
    fims fims_test;
    std::string client_name;
    Stats fims_message_stats;
    std::string filename;

    bool use_dbi = false;
    bool reload =  false;

    bool keep_running = true ; // TODO temp for gcom_fims
    bool keep_fims_running = true ; // TODO temp for gcom_fims
    bool auto_disable     =  false;
    bool allow_multi_sets =  false;
    bool force_multi_sets =  false;
    bool test_fims = false;
    int max_register_group_size = 125;
    int max_bit_size = 125;
    int max_io_tries = 10;
    bool debug_decode = false;
    bool debug_hb = false;
    int set_idx = 0;
    int get_idx = 0;
    bool pub_coil      = false;
    bool pub_holding   = true;
    bool pub_input     = true;
    bool pub_discrete  = true;
    bool pub_debug     = false;
    bool pub_sync      =  true;
    bool pub_stats     =  false;
    double syncPct     = 0.0;
    
    Fims_Format format = Fims_Format::Undefined;

    double reconnect_delay = 1.0;

    bool debug_pub = false;
    bool use_new_wdog = true;  // we may deprecate this its set up too late anyway
    bool use_bool = true;
    bool fims_running = false;
    bool fims_connected = false;

    int fims_set_errors = 0;
    int fims_get_errors = 0;
    int fims_pub_errors = 0;

    int num_fims = 0;
    int max_fims = 32;
    int max_fims_wait = 100;

    bool allow_gaps = true; 

    Version git_version_info;
};

struct type_map
{
    struct std::shared_ptr<cfg::io_point_struct> map;
    struct std::shared_ptr<cfg::register_group_struct> register_group;
    struct std::shared_ptr<cfg::component_struct> component;
};

struct Uri_req
{
    static constexpr auto Raw_Request_Suffix = "_raw";
    static constexpr auto Timings_Request_Suffix = "_timings";
    static constexpr auto Reset_Timings_Request_Suffix = "_reset_timings";
    static constexpr auto Reload_Request_Suffix = "_reload";
    static constexpr auto Disable_Suffix = "_disable";
    static constexpr auto Enable_Suffix = "_enable";
    static constexpr auto Force_Suffix = "_force";
    static constexpr auto Unforce_Suffix = "_unforce";
    static constexpr auto Local_Suffix = "_local";
    static constexpr auto Remote_Suffix = "_remote";
    static constexpr auto Full_Suffix = "_full";
    static constexpr auto Stats_Suffix = "_stats";
    static constexpr auto Server_Suffix = "_server";

    Uri_req()
    {
        clear_uri();
    };

    Uri_req(std::string_view &uri_view, const char *uri)
    {
        if (uri)
            uri_view = std::string_view{uri, strlen(uri)};
        set_uri(uri_view);
    }

    void set_uri(std::string_view &uri_view)
    {
        clear_uri();
        splitUri(uri_view);
        int num = num_uris;
        for (int idx = 0; idx < num; ++idx)
        {
            if (uri_vec[idx].front() == '_')
            {
                num_uris--;
                if (uri_vec[idx] == Raw_Request_Suffix)               is_raw_request           = true; 
                else if (uri_vec[idx] == Timings_Request_Suffix)           is_timings_request       = true ;
                else if (uri_vec[idx] == Reset_Timings_Request_Suffix)     is_reset_timings_request = true ;
                else if (uri_vec[idx] == Reload_Request_Suffix)            is_reload_request        =  true;
                else if (uri_vec[idx] == Enable_Suffix)                    is_enable_request        =  true;
                else if (uri_vec[idx] == Disable_Suffix)                   is_disable_request       =  true;
                else if (uri_vec[idx] == Force_Suffix)                     is_force_request         =  true;
                else if (uri_vec[idx] == Unforce_Suffix)                   is_unforce_request       =  true;
                else if (uri_vec[idx] == Local_Suffix)                     is_local_request         =  true;
                else if (uri_vec[idx] == Remote_Suffix)                    is_remote_request        =  true;
                else if (uri_vec[idx] == Full_Suffix)                      is_full_request          =  true;
                else if (uri_vec[idx] == Stats_Suffix)                     is_stats_request         =  true;
                else if (uri_vec[idx] == Server_Suffix)                    is_server_request        =  true;
            }
        }
    };

    void clear_uri()
    {
        is_raw_request = false;
        is_timings_request = false;
        is_reset_timings_request = false;
        is_reload_request = false;
        is_enable_request = false;
        is_disable_request = false;
        is_force_request = false;
        is_unforce_request = false;
        is_reset_request = false;
        is_local_request = false;
        is_remote_request = false;
        is_full_request = false;
        is_stats_request = false;
        is_server_request = false;
        uri_vec.clear();
    }

    bool str_ends_with(const std::string_view &str, const std::string_view &suffix)
    {
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    // Function to split the URI path and store parts into a vector
    void splitUri(const std::string_view &uri_view)
    {
        size_t start = 0;
        size_t end = uri_view.find('/');

        while (end != std::string_view::npos)
        {
            uri_vec.emplace_back(uri_view.substr(start, end - start));
            start = end + 1;
            end = uri_view.find('/', start);
        }
        uri_vec.emplace_back(uri_view.substr(start, end));
        num_uris = (int)uri_vec.size(); 
    }
    
    bool is_raw_request;
    bool is_timings_request;
    bool is_reset_timings_request;
    bool is_reset_request;
    bool is_reload_request;
    bool is_enable_request;
    bool is_disable_request;
    bool is_force_request;
    bool is_unforce_request;
    bool is_local_request;
    bool is_remote_request;
    bool is_full_request;
    bool is_stats_request;
    bool is_server_request;
    bool is_request = false;
    int num_uris;
    
    std::vector<std::string> uri_vec;
};

class PublishGroup
{
private:
public:
    double timeId; // this is the time from get_time_double that we submitted this item
    int numItems;  //  this is the number of items in the vector.
    std::string name;
    int frequency;
    int offset_time;

    // Constructor
    PublishGroup(const std::string &component_id, const std::string &id,
                 int freq, int offset)
        : name(component_id + "_" + id), frequency(freq), offset_time(offset) {}
};

struct IO_Work; // defined in gcom_iothread.h
using Component_IO_point_map = std::map<std::string, std::map<std::string, std::map<std::string, std::shared_ptr<cfg::io_point_struct>>>>;
using MapIdMap = std::map<int, std::map<cfg::Register_Types, std::map<int, cfg::io_point_struct *>>>;

/**
 * @brief convert a set of raw 16-bit registers to its hexadecimal representation as a string
 * 
 * @param raw16 a pointer to an array of uint16_t registers to covert to its hexadecimal representation
 * @param size the size of the raw16 array
*/
std::string hex_to_str(u16 *raw16, int size);

/**
 * @brief convert a set of raw 16-bit registers to its hexadecimal representation as a string
 * 
 * @param raw16 a pointer to an array of size 4 of uint16_t registers to covert to its hexadecimal representation
*/
std::string hex_to_str(u16 *raw16);

/**
 * @brief convert a set of raw 8-bit binary registers to its hexadecimal representation as a string
 * 
 * @param raw8 a pointer to an array of size 4 of uint8_t registers to covert to its hexadecimal representation
*/
std::string hex_to_str(u8 *raw8);

/**
 * @brief convert a uint64_t to its hexadecimal representation as a string
 * 
 * @param raw64 a uint64_t value to covert to its hexadecimal representation
*/
std::string hex_to_str(u64 raw64);

/**
 * @brief convert an (unspecified) integer to its hexadecimal representation as a string
 * 
 * @param raw_int an int value to covert to its hexadecimal representation
*/
std::string hex_to_str(int raw_int);

/**
 * @brief Convert a simdjson::dom::object to a map<string, any>.
 *
 * @param obj simdjson::dom::object JSON object created after parsing using simdjson::dom::parser
 */
std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj);

/**
 * @brief Convert a simdjson::dom::element to an std::any
 *
 * @param elem simdjson::dom::element JSON element created after parsing using simdjson::dom::parser
 */
std::any jsonToAny(simdjson::dom::element elem);

/**
 * @brief Load an entire config file into the config structure (myCfg) for this program instance.
 *
 * Open and load a file from [filename] into jsonMapOfConfig. Then, parse all values into the organized config
 * structure that holds all settings and global information. Loads in subs and pubs from the parsed registers.
 * Calls gcom_parse_file, extract_connection, and extract_components.
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param filename const char * of the file to be opened and parsed
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool gcom_load_cfg_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, struct cfg &myCfg, bool debug);

/**
 * @brief Open a file, parse it using simdjson, then convert it to a map<string, any>
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param filename const char * of the file to be opened and parsed
 * @param use_dbi bool value representing whether or not to get the file from fims rather than the local file system
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool gcom_parse_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, bool use_dbi, bool debug);

/**
 * @brief Parse the connection information from a config map into myCfg.
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param query the field to look for
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool extract_connection(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug);

/**
 * @brief Parse the component information from a config map into myCfg.
 * 
 * Makes calls to extract_register_groups and extract_io_point_map.
 * 
 * This is the big unpacker.
 *     components
 *       ->  register_groups (extract_register_groups)
 *              -> io_point_map (extract_io_point_map)
 *                   ->
 * 
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param query the field to look for
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool extract_components(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug);

/**
 * @brief Parse the register information in a component into myCfg.
 * 
 * @param register_groups an empty register_group_struct, typically belonging to component
 * @param raw_register_groups_json_array an std::any struct representing all register_groups in a component (as a json list)
 * @param component a pre-populated component_struct, typically belonging to myCfg
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool extract_register_groups(std::vector<std::shared_ptr<cfg::register_group_struct>> &register_groups, const std::any &raw_register_groups_json_array, std::shared_ptr<struct cfg::component_struct> component, struct cfg &myCfg, bool debug);

/**
 * @brief Parse the information in an io_point list (belonging to a single register group of a single
 * component) into myCfg.
 * 
 * @param register_group the register group struct that the list belongs to
 * @param packed_io_point the parent io_point that contains packed registers within it. NULL if this is the parent io_point or if the io_point is not packed.
 * @param rawIOPointList an std::any struct representing all io_points in a register_group (as a json list)
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
*/
bool extract_io_point_map(std::shared_ptr<struct cfg::register_group_struct> register_group, std::shared_ptr<cfg::io_point_struct> packed_io_point, std::any &rawIOPointList, struct cfg &myCfg, bool debug);

/**
 * @brief Parse the "bit_strings" field in an io_point
 * 
 * @param io_point the io_point that the list of bit_strings belongs to
 * @param rawStringData an std::any struct representing all bit_strings in an io_point (as a json list)
*/
void extract_bitstrings(std::shared_ptr<cfg::io_point_struct> io_point, std::any &rawStringData);

/**
 * @brief Demangle the type of a bit_string so that the appropriate exception can be thrown.
 * 
 * @param value the std::any value that is trying to be unpacked into a bit_string io_point
 * @return a string representation of the type of the std::any value
*/
std::string anyTypeString(const std::any &value);

/**
 * @brief Check that all non-packed io_points in a register group have a non-zero point size.
 * 
 * If an io_point is not packed and its size == 0, the io_point->size will be set to 1 instead.
 * 
 * @param register_group the register group containing the io_points to check
 * @param io_point_map the io_point map belonging to that register group
 * @param debug bool value representing whether or not to print messages to help debug code
*/
void checkNonZeroIOPointSize(std::shared_ptr<cfg::register_group_struct> register_group, std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map, bool debug);

/**
 * @brief Sort a vector of io_point_structs by their offsets.
 * 
 * @param elements a vector of io_point_structs, typically the io_point_map contained in a register_group
*/
void sortIOPointVectorByOffset(std::vector<std::shared_ptr<cfg::io_point_struct>> &elements);

/**
 * @brief Get the first offset in a vector of io_points.
 * 
 * @param elements a pre-sorted vector of io_point_structs, typically the io_point_map contained in a register_group
 * 
 * @pre elements is sorted by offset
*/
int getFirstOffset(const std::vector<std::shared_ptr<cfg::io_point_struct>> elements);

/**
 * @brief Get the total size of a register group in terms of number of 16-bit registers.
 * 
 * If there is a gap between io_points, provide a message to alert the user. If there is an overlap between
 * io_points, provide an error message.
 * 
 * @param io_point_map a pre-sorted vector of io_point_structs, typically associated with a particular register_group
 * @return the combined size of all io_points in a register_group, excluding any gaps. If registers overlap, return -1.
 * 
 * @pre io_point_map is sorted by offset
*/
int getTotalNumRegisters(std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map, struct cfg &myCfg);

/**
 * @brief Reassign a config instance to a new config structure. Used by reload.
 * 
 * @param cfgInstance the current config instance to overwrite with "blank" data
*/
void resetCfg(struct cfg &cfgInstance);

/**
 * @brief Based on the settings in io_point, convert an std::any value to a 0/1 value to be stored in an 8-bit register.
 * 
 * @param io_point an io_point_struct that contains the settings to base the conversion on
 * @param val the std::any value to convert to 0 or 1
 * @param uri the URI request found in a fims message
 * @param regs8 a pointer to the 4x8-bit register to store the 0/1 value
 * 
 * @return the 8-bit value stored in regs8
*/
uint8_t get_any_to_bool(std::shared_ptr<cfg::io_point_struct>io_point, std::any val, Uri_req& uri, uint8_t*regs8);

/**
 * @brief Based on the settings in io_point, convert an std::any value to a uint64 value to be stored in a 
 * set of 16-bit registers.
 * 
 * @param io_point an io_point_struct that contains the settings to base the conversion on
 * @param val the std::any value to convert to a uint64 value
 * @param uri the URI request found in a fims message
 * @param regs16 a pointer to the 4x16-bit register to store the uint64 value. If nullptr, function will omit storing the value.
 * 
 * @return the 64-bit value (also stored in regs16 if not null)
*/
uint64_t get_any_to_uint64(std::shared_ptr<cfg::io_point_struct>io_point, std::any val, Uri_req& uri, uint16_t*regs16);

/**
 * @brief Based on the settings in io_point, store a uint64 value in a set of 16-bit registers.
 * 
 * @param io_point an io_point_struct that contains the settings to base the conversion on
 * @param uval the uint64 value to store in regs16
 * @param regs16 a pointer to the 4x16-bit register to store the uint64 value.
 * 
 * @return io_point->size
*/
int set_reg16_from_uint64(std::shared_ptr<cfg::io_point_struct> io_point, uint64_t &uval, uint16_t *regs16);

/**
 * @brief Based on the settings in io_point, convert an std::any value to a uint64 value to be stored in a 
 * set of 16-bit registers.
 * 
 * @param io_point an io_point_struct that contains the settings to base the conversion on
 * @param val the std::any value to convert to a uint64 value
 * @param uri the URI request found in a fims message
 * @param regs8 a pointer to the 4x16-bit register to store the uint64 value
 * 
 * @return the 64-bit value stored in regs16
*/
uint64_t set_any_to_uint64(struct cfg &myCfg,std::shared_ptr<cfg::io_point_struct>io_point, std::any val);

/**
 * @brief Add an io_point to the a component_io_point_map, typically the one belonging to myCfg.
 * 
 * This map is used to look up io_points by their URI fragments.
 * 
 * using Component_IO_point_map = std::map<std::string, std::map<std::string, std::map<std::string, std::shared_ptr<cfg::io_point_struct>>>>
 * (Map of component_prefix->component_id->io_point_id->io_point)
 * 
 * @param imap the component_io_point_map to add the point to
 * @param component_uri_prefix the base of the component uri, e.g. "components" in /components/comp_sel_2440/coil_0
 * @param component_id the id of the component of interest, e.g. "comp_sel_2440" in /components/comp_sel_2440/coil_0
 * @param io_point the point to add to imap. The id of this point will be used to look up the pointer to this point.
*/
void addIOPointToComponentIOPointMap(Component_IO_point_map &imap, const char *component_uri_prefix, const char *component_id, std::shared_ptr<cfg::io_point_struct> io_point);

/**
 * @brief Add an io_point to the a MapIdMap, typically the one belonging to myCfg.
 * 
 * This map is used to look up io_points by their URI fragments.
 * 
 * using MapIdMap = std::map<int, std::map<Register_Types, std::map<int, io_point_struct *>>>;
 * (Map of device_id->register_type->io_point_offset->io_point)
 * 
 * @param imap the MapIdMap to add the point to
 * @param io_point the point to add to imap. The offset of this point will be used to look up the pointer to this point.
*/
void addIOPointToIdMap(MapIdMap &imap, std::shared_ptr<cfg::io_point_struct> io_point);

/**
 * @brief check if an io_point exists within a configuration
 * 
 * @param io_point a reference to the io_point struct to populate if the point lookup is successful
 * @param myCfg the config to search for the point within
 * @param uri_keys a vector of uri fragments to identify the point (e.g. /some/random/uri would be [some, random, uri])
 * @param key_size the number of uri fragments in uri_keys
 * @param io_point_key the io_point id to look for, if not specified by the uri. Default is "".
*/
bool ioPointExists(std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, int key_size, std::string io_point_key);

/**
 * @brief Format the details of the server config document to a stringstream.
 * 
 * @param myCfg the client configuration used to populate the server config stringstream
 * @param ss the string stream to format the details of the server config to
*/
void showServerConfig(struct cfg &myCfg, std::stringstream &ss);

/**
 * @brief Format the details of the server config "system" field to a stringstream.
 * 
 * Includes name, protocol, id, ip_address, port, and device id.
 * 
 * @param myCfg the client configuration used to populate the server config "system" stringstream
 * @param ss the string stream to format the details of the server config "system" to
*/
void showSystem(struct cfg &myCfg, std::stringstream &ss);

/**
 * @brief Format a register group, including its device_id and all io_points, to a stringstream.
 * 
 * @param myCfg the client configuration used to populate the server config "registers" stringstream
 * @param dev_id the device_id of the current register group
 * @param reg_group_type the register group type (e.g. holding, coil, input, discrete input)
 * @param io_point_map_lookup std::map<int, cfg::io_point_struct *> containing all points in a register group
 * @param ss the string stream to format the details of the io_point to
*/
void showDeviceRegisterGroup(struct cfg &myCfg, int dev_id, cfg::Register_Types reg_group_type, std::map<int, cfg::io_point_struct *> io_point_map_lookup, std::stringstream &ss);

/**
 * @brief Format the details of an io_point to a stringstream.
 * 
 * Includes id, name, offset, size, shift, scale, signed, and float.
 * 
 * @param io_point the io_point to unwrap
 * @param ss the string stream to format the details of the io_point to
*/
void showIOPoint(cfg::io_point_struct *io_point, std::stringstream &ss);

/**
 * @brief Compares the offsets of two io_points. If a->offset is less than b->offset, returns true.
 * If a->offset is greater than or equal to b->offset, returns false.
 * 
 * Also compares register type and device_id (in that order) prior to point offset.
 * 
 * @param a the first io_point
 * @param b the second io_point
*/
bool compareIOPointOffsets(const std::shared_ptr<cfg::io_point_struct> &a, const std::shared_ptr<cfg::io_point_struct> &b);

/**
 * @brief Given a vector of io_points, create a vector of IO_Work objects that are separated by
 * device, register group, and number of registers (limited by maximum). If there are any gaps
 * in point offset, a new IO_Work object is created.
 * 
 * @param io_work_vec a reference to the vector of IO_Work objects to populate
 * @param io_map_vec a vector of io_points to include in the IO_Work objects
 * @param myCfg the configuration used to identify and separate the points
 * @param work_type the work type for the IO_Work objects (get, poll, set, etc.)
 * @param include_all_points bool value specifying whether or not to include all given points. If true, IO_Work objects will include disabled and forced points. If false, IO_Work objects will exclude these points
 * @param debug bool value representing whether or not to print messages to help debug code
*/
void check_work_items(std::vector<std::shared_ptr<IO_Work>>& io_work_vec, std::vector<std::shared_ptr<cfg::io_point_struct>>& io_map_vec, struct cfg & myCfg, const char* work_type, bool include_all_points, bool debug);

/**
 * @brief Check if a point should be disabled due to debounce and/or update its debounce time.
 * 
 * Debounce is effectively a way of throttling sets when a value is being spammed to the client
 * before it has time to poll the value from the client. (Otherwise, we use all of our resources to
 * send sets rather than polling the server.)
 * 
 * @param enabled a reference to a boolean value that reflects the point's enabled status
 * @param io_point the io_point to look at for debounce
 * @param debug bool value representing whether or not to print messages to help debug code
*/
void check_item_debounce(bool &enabled, std::shared_ptr<cfg::io_point_struct> io_point, bool debug);

bool gcom_parse_data(std::any &anyFimsMessageBody, const char *data, size_t length, bool debug);
bool extractJsonValue(const simdjson::dom::element &el, std::any &value);
std::optional<std::string> getMapString(const std::map<std::string, std::any> &m, const std::string &query);
bool getMapType(const std::map<std::string, std::any> &m, const std::string &query, const std::type_info &targetType);
std::vector<std::string> split_string(const std::string &str, char delimiter);
bool gcom_load_cfg_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, struct cfg &myCfg, bool debug);
bool encode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg &myCfg, Uri_req &uri, std::string& replyto, const char* mode,  bool debug);
void addMapId(MapIdMap &imap, const int device_id, cfg::Register_Types register_type, cfg::io_point_struct *io_point);
std::shared_ptr<cfg::io_point_struct> findItem(const std::map<std::string, std::map<std::string, std::map<std::string, std::shared_ptr<cfg::io_point_struct>>>> &items, const std::string &component, const std::string &id, const std::string &name);
bool parseFimsMessage(struct cfg &myCfg, const Component_IO_point_map &items, std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result, const std::string &method, const std::string &uri, const std::string &body);
std::string anyToString(const std::any &value);

std::vector<std::string> any_split(std::string_view str, char delimiter);
int gcom_setup_pubs(std::map<std::string, std::any> &jsonMapOfConfig, struct cfg &myCfg, double wait_time, bool debug);
std::string mapToString(const std::map<std::string, std::any> &m);

bool add_all_component_points_to_io_vec(std::vector<std::shared_ptr<cfg::io_point_struct>> &io_point_vec, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, bool skip_disabled);
bool gcom_findCompVar(
    std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const cfg::component_struct*comp, std::string kvar);

void printAny(std::any &value, int indent);
void printMap(const std::map<std::string, std::any> &baseMap, int indent);


#ifdef FPS_DEBUG_MODE
using ItemSharedPtr = std::shared_ptr<cfg::io_point_struct>;
using ItemMap = std::map<std::string, std::map<std::string, std::map<std::string, ItemSharedPtr>>>;
bool extractJsonValue(const simdjson::dom::element &el, std::any &value);
std::string mapToString(const std::map<std::string, std::any> &m);
int some_test(std::map<std::string, std::any> &m);
size_t computeSize(const std::map<std::string, std::any> &baseMap, int indent);
int test_buffer_size();
bool test_decode_raw();
std::string anyToString(const std::any &value);
bool test_uri_body(struct cfg &myCfg, const char *uri, const char *method, const char *pname, const char *uname, const char *repto, const char *body);
std::vector<std::string> any_split(std::string_view str, char delimiter);
void gcom_extract_types(struct cfg &myCfg);
struct type_map *gcom_get_type(std::string type, int offset, bool debug);
void gcom_extract_comps(struct cfg &myCfg);
void gcom_extract_subs(struct cfg &myCfg);
bool gcom_config_test_uri(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *uri, const char *id);
void gcom_extract_pubs(struct cfg &myCfg);
int merge_IO_Work_Reg(std::vector<std::shared_ptr<IO_Work>> &work_vector,std::vector<std::shared_ptr<IO_Work>> &discard_vector);
int sort_IO_Work(std::vector<std::shared_ptr<IO_Work>> &work_vector);
using IO_point_ptr = std::shared_ptr<cfg::io_point_struct>;
using RetVar = std::variant<std::monostate, std::map<std::string, std::shared_ptr<cfg::io_point_struct>> *, IO_point_ptr>;
RetVar findMapVar(struct cfg &myCfg, std::vector<std::string> keys);
std::string getElementTypeString(simdjson::dom::element_type etype);
std::string processFieldAsString(const simdjson::dom::element &field_value);
std::any processField(const simdjson::dom::element &field_value);
std::map<std::string, std::any> parseInputMessage(std::map<std::string, std::any> sysMap, const std::string &uri, const std::string &method, const std::string &body);
void test_parse_message(const char *uri, const char *method, const char *body);
std::map<std::string, std::any> parseMessage(const std::string &uri, const std::string method, const std::string &body);
int test_extract();
void test_merge_message(const char *uri, const char *method, const char *body);
void addMapId(MapIdMap &imap, const int device_id, cfg::Register_Types register_type, cfg::io_point_struct *io_point);
bool parseFimsMessage(struct cfg &myCfg, const Component_IO_point_map &items, std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result,const std::string &method, const std::string &uri, const std::string &body);
bool compressResults(const std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &results,std::vector<CompressedItem> &compressedResults);
void mapToRawBuffer(const std::map<std::string, std::any> &baseMap, spdlog::memory_buf_t &buf, int indent);
void mapToBuffer(const std::map<std::string, std::any> &baseMap, spdlog::memory_buf_t &buf, int indent);
std::any parseValue(const std::string &mystr);
bool gcom_msg_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg);
int test_ParseValue();
int test_printMap();
void timer_create(const std::string &name, int frequency, int offset, void (*callback)(void *, void *), void *data);
void register_publish_groups_with_timer(const std::vector<std::shared_ptr<PublishGroup>> &publishGroups);
bool extract_publish_groups(std::vector<std::shared_ptr<PublishGroup>> &publishGroups, const ItemMap& items, const std::map<std::string, std::any>& jsonData);
bool gcom_pub_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg);
std::string extractCompFromURI(const std::string &uri);
std::string extractCompIdFromURI(const std::string &uri, const std::string &component);
std::string extractIdFromURI(const std::string &uri);
std::string getKeyFromURI(const std::string &uri);
void mergeSubMaps(std::map<std::string, std::any> &base, const std::map<std::string, std::any> &toMerge);
void publish_cb(void *timer, void *data);
std::shared_ptr<cfg::io_point_struct> findItem(const Component_IO_point_map &items, const std::string &component, const std::string &id, const std::string &name);
struct type_map *gcom_get_comp(struct cfg &myCfg, std::string component, std::string id, bool debug);
std::string gcom_show_types(struct cfg &myCfg);
bool gcom_config_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg);
void clearChan(bool debug);
Component_IO_point_map extract_structure(Component_IO_point_map &structure, const std::map<std::string, std::any> &jsonData);
void printComponentIOPointMap(const Component_IO_point_map &items);
void printIOPoint(const std::shared_ptr<cfg::io_point_struct> io_point);
void printResultVector(const std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result);
void printPublishGroups(std::vector<std::shared_ptr<PublishGroup>> &publishGroups);
std::string gcom_show_FirstLevel(const std::map<std::string, std::any> &m, std::string key);
std::string gcom_show_subs(struct cfg &myCfg, bool debug);
std::string gcom_show_pubs(struct cfg &myCfg, bool debug);
bool gcom_points_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *decode);
bool gcom_point_type_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *ptype, const char *decode);
bool test_findMapVar(std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, std::string kvar);
bool test_findMapMap(std::map<std::string, std::shared_ptr<cfg::io_point_struct>> &map_result, const struct cfg &myCfg, const std::vector<std::string> &uri_keys);
bool test_uri(struct cfg &myCfg, const char *uri);
bool decode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg &myCfg, Uri_req &uri, const char *mode, bool debug);
bool uri_is_single(std::shared_ptr<cfg::io_point_struct> &io_point, struct cfg &myCfg, struct Uri_req &uri, bool debug);
bool gcom_test_bit_str();
bool encode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg &myCfg, Uri_req &uri, std::string &replyto, const char *mode, bool debug);
#endif

#endif
