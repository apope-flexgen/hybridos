#include <chrono>
#include <thread>
#include <iostream>
#include <any>
#include <sys/uio.h>     // for receive timouet calls
#include <sys/socket.h>  // for receive timouet calls

#include <stdio.h>   //printf
#include <string.h>  //memset
#include <stdlib.h>  //for exit(0);
#include <errno.h>   //For errno - the error number
#include <netdb.h>   //hostent
#include <arpa/inet.h>
#include <variant>

#include "spdlog/details/fmt_helper.h"  // for microseconds formattting
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/chrono.h"

// #include "simdjson_noexcept.hpp"

#include "modbus/modbus.h"

// #include "fims/defer.hpp"

// #include "Jval_buif.hpp"
#include "tscns.h"

#include "fims/libfims.h"

#include "logger/logger.h"
#include "gcom_config.h"
#include "gcom_timer.h"
#include "gcom_iothread.h"

// all of these are from gcom_iothreads.cpp
extern ThreadControl threadControl;
extern std::mutex io_output_mutex;
extern ioChannel<int> io_localthreadChan;
extern ioChannel<int> io_threadChan;
extern ioChannel<std::shared_ptr<IO_Work>> io_pollChan;
extern ioChannel<std::shared_ptr<IO_Work>> io_poolChan;

#include "gcom_fims.h"
#include "gcom_perf.h"
#include "gcom_modbus_decode.h"
#include "gcom_stats.h"

#include "version.h"

using namespace std::chrono_literals;
using namespace std::string_view_literals;

bool start_fims(std::vector<std::string>& subs, struct cfg& myCfg);
bool stop_fims(struct cfg& myCfg);
bool test_uri_body(struct cfg& myCfg, const char* uri, const char* method, const char* pname, const char* uname,
                   const char* repto, const char* body);
bool test_uri(struct cfg& myCfg, const char* uri);

bool gcom_test_bit_str(void);
void test_parse_message(const char* uri, const char* method, const char* body);
void test_merge_message(const char* uri, const char* method, const char* body);
bool gcom_points_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg& myCfg, const char* decode);
bool gcom_point_type_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg& myCfg, const char* ptype,
                          const char* decode);
bool gcom_msg_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg& myCfg);
bool gcom_config_test_uri(std::map<std::string, std::any> jsonMapOfConfig, struct cfg& myCfg, const char* uri,
                          const char* id);
bool uri_is_single(std::shared_ptr<cfg::io_point_struct>& io_point, struct cfg& myCfg, struct Uri_req& uri, bool debug);

void clearChan(bool debug);
bool decode_io_point_struct(std::vector<std::shared_ptr<IO_Work>>& work_vec,
                            std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg& myCfg,
                            Uri_req& uri, const char* mode, bool debug);
std::string extractCompFromURI(const std::string& uri);
std::string extractCompIdFromURI(const std::string& uri, const std::string& component);
int test_io_threads(struct cfg& myCfg);
void test_io_point_single(const char* ip, int port, double connection_timeout, const char* oper, int device_id,
                          const char* regtype, int offset, int num_regs, int value, int num_threads, struct cfg& myCfg,
                          bool debug);
void test_io_point_multi(const char* ip, int port, int connection_timeout, const char* oper, int device_id,
                         const char* regtype, int offset, int value, int num_threads, struct cfg& myCfg, bool debug);

#define NEW_FPS_ERROR_PRINT(fmt_str, ...) fmt::print(stderr, FMT_COMPILE(fmt_str), ##__VA_ARGS__)
#define NEW_FPS_ERROR_PRINT_NO_COMPILE(fmt_str, ...) fmt::print(stderr, fmt_str, ##__VA_ARGS__)
#define NEW_FPS_ERROR_PRINT_NO_ARGS(fmt_str) fmt::print(stderr, FMT_COMPILE(fmt_str))
#define FORMAT_TO_BUF(fmt_buf, fmt_str, ...)                                                                           \
    fmt::format_to(std::back_inserter(fmt_buf), FMT_COMPILE(fmt_str), ##__VA_ARGS__)

// constants:
// TODO(WALKER): "broken pipe?" -> larger integer? (will have to investigate)
// static constexpr auto Modbus_Errno_Disconnect   = 104; // "Connection reset by peer" -> disconnect
// static constexpr auto Modbus_Errno_Cant_Connect = 115; // "Operation now in progress" -> no connection
// static constexpr auto Modbus_Server_Busy = 10126; // "Resource temporarily unavailable" -> server busy

// helper macros:
// #define FORMAT_TO_BUF(fmt_buf, fmt_str, ...)            fmt::format_to(std::back_inserter(fmt_buf),
// FMT_COMPILE(fmt_str), ##__VA_ARGS__) #define FORMAT_TO_BUF_NO_COMPILE(fmt_buf, fmt_str, ...)
// fmt::format_to(std::back_inserter(fmt_buf), fmt_str, ##__VA_ARGS__)

// Client event source constant:
static constexpr auto Client_Event_Source = "Modbus Client"sv;

// // Helper function straight from C++20 so we can use it here in C++17:
// // for checking for suffix's over fims (like a raw get request)
// static constexpr
bool str_ends_with(std::string_view str, std::string_view suffix);
// {
// 	const auto str_len = str.size();
// 	const auto suffix_len = suffix.size();
// 	return str_len >= suffix_len
//                 && std::string_view::traits_type::compare(str.end() - suffix_len, suffix.data(), suffix_len) == 0;
// }

// here is our static config
// everything is in here , or it should be
struct cfg myCfg;

// stick the raw config in here
std::map<std::string, std::any> gcom_map;

void test_one_set_uris(struct cfg& myCfg)
{
    test_uri_body(myCfg, "/components/comp_sel_2440/test_id", "set", nullptr, nullptr, nullptr, "{\"value\":10021}");
}

void test_set_uris(struct cfg& myCfg)
{
    test_uri_body(myCfg, "/components/comp_sel_2440/test_id", "set", nullptr, nullptr, nullptr, "{\"value\":10021}");
    test_uri_body(myCfg, "/components/comp_sel_2440/hold_1_1", "set", nullptr, nullptr, nullptr, "{\"value\":-400}");
    // test_uri_body(myCfg, "/components/comp_sel_2440/shold_1_1", "set", nullptr, nullptr, nullptr,"{\"value\":-400}");
    // test_uri_body(myCfg, "/components/comp_sel_2440/hold_1_1", "set", nullptr, nullptr, nullptr,"{\"value\":400}");
    // test_uri_body(myCfg, "/components/comp_sel_2440/shold_1_1", "set", nullptr, nullptr, nullptr,"{\"value\":-400}");
    test_uri_body(myCfg, "/components/comp_sel_2440/hold_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":400}");
    test_uri_body(myCfg, "/components/comp_sel_2440/shold_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":-400}");

    test_uri_body(myCfg, "/components/comp_sel_2440/hold_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":500.34}");
    test_uri_body(myCfg, "/components/comp_sel_2440/shold_1_4", "set", nullptr, nullptr, nullptr,
                  "{\"value\":-500.67}");

    test_uri_body(myCfg, "/components/comp_sel_2440/fhold_1_2", "set", nullptr, nullptr, nullptr, "{\"value\":500.34}");
    test_uri_body(myCfg, "/components/comp_sel_2440/fhold_1_4", "set", nullptr, nullptr, nullptr,
                  "{\"value\":-600.67}");
    test_uri_body(myCfg, "/components/comp_sel_2440/fshold_1_4", "set", nullptr, nullptr, nullptr,
                  "{\"value\":-600.67}");

    //  shift / scale and check all bit stuff
    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_1", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_2", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/shift_1_1", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/shift_1_2", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/shift_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":200}");
    // test_uri_body(myCfg, "/components/comp_sel_2440/shift_1_4", "poll", "{\"value\":200}");
    test_uri_body(myCfg, "/components/comp_sel_2440/test_id", "set", nullptr, nullptr, nullptr, "{\"value\":10022}");

    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_1/_force", "set", nullptr, nullptr, nullptr,
                  "{\"value\":300}");

    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_1", "set", nullptr, nullptr, nullptr, "{\"value\":100}");
    test_uri_body(myCfg, "/components/comp_sel_2440/scale_1_1/_unforce", "set", nullptr, nullptr, nullptr,
                  "{\"value\":150}");

    // TODO bit_mask
    // TODO invert_mask
    test_uri_body(myCfg, "/components/comp_sel_2440/inv_1_16", "set", nullptr, nullptr, nullptr, "{\"value\":0}");
    test_uri_body(myCfg, "/components/comp_sel_2440/inv_1_16", "set", nullptr, nullptr, nullptr, "{\"value\":255}");
    test_uri_body(myCfg, "/components/comp_sel_2440/inv_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":0}");
    test_uri_body(myCfg, "/components/comp_sel_2440/inv_1_4", "set", nullptr, nullptr, nullptr, "{\"value\":128}");

    test_uri_body(myCfg, "/components/comp_sel_2440", "set", nullptr, nullptr, nullptr,
                  "{\"multi_1\":1,\"multi_2\":2,\"multi_3\":3,\"multi_4\":4}");

    test_uri_body(myCfg, "/components/comp_sel_2440/multi_1", "get", nullptr, nullptr, nullptr, "{\"value\":0}");
}

std::vector<std::string> split_string(const std::string& str, char delimiter);

// moved to gcom_modbus_any.cpp
void gcom_load_overrides(struct cfg& myCfg, int next_arg, const int argc, const char* argv[]);

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <any>
#include <algorithm>
#include <cctype>

// // Function to detect the data type of the value
std::any detectTypeAndCast(const std::string& value);

std::map<std::string, std::any> parseQueryString(const std::string& query);

// int zmain() {
//     std::string query = "name=John+Doe&age=23&active=true&pi=3.14159";

//     // Parse the query string into a map with std::any as the value type
//     auto data = parseQueryString(query);

//     // Print the stored values and their types
//     for (const auto& [key, val] : data) {
//         std::cout << key << " : ";
//         if (val.type() == typeid(int)) {
//             std::cout << "int(" << std::any_cast<int>(val) << ")";
//         } else if (val.type() == typeid(float)) {
//             std::cout << "float(" << std::any_cast<float>(val) << ")";
//         } else if (val.type() == typeid(bool)) {
//             std::cout << "bool(" << std::any_cast<bool>(val) << ")";
//         } else if (val.type() == typeid(std::string)) {
//             std::cout << "string(" << std::any_cast<std::string>(val) << ")";
//         }
//         std::cout << std::endl;
//     }

//     return 0;
// }

bool filter_io_point(int argc, const std::shared_ptr<struct cfg::io_point_struct> point, const std::any& crit);
std::any runParseQuery(std::string& query);

// int ymain() {
//     std::string query = "name=John+Doe&age=23&city=New+York";
//     runParseQuery(query);
//     return 0;
// }

int main(const int argc, const char* argv[]) noexcept
{
    // access version info
    Version version;
    version.init();

    printf("build: %s\n", version.get_build());
    printf("commit: %s\n", version.get_commit());
    printf("tag: %s\n", version.get_tag());

    std::string cmd;
    if (argc > 1)
    {
        cmd = std::string(argv[1]);
    }

    if (cmd == "test")
    {
        std::cout << "test_io <ip> <port> <set|pub|get> <device_id> <Holding|Coil> <offset> <value> :runs raw modbus"
                  << std::endl;
        std::cout
            << "test_cfg <config_file>                                              <up next> : basic config file load"
            << std::endl;
        std::cout
            << "??test_cfg_uri <config_file> <uri:id>                                           : provides the IO_work list"
            << std::endl;
        std::cout
            << "test_cfg_msg <config_file>                                                    : parse the config file send set message "
            << std::endl;
        std::cout
            << "test_cfg_pub <config_file>                                                    : parse the config file extract pubs"
            << std::endl;
        std::cout
            << "test_parse <uri> <body>                                                       : test parsing of a single object"
            << std::endl;
        std::cout
            << "test_merge <uri> <body>                                                       : test merging of two maps"
            << std::endl;
        std::cout
            << "test_timer                                                                    : basic timer test using std::list"
            << std::endl;
        std::cout
            << "test_timer_set                                                                : basic timer test using std::set "
            << std::endl;
        std::cout << "test_perf                                                                     : basic perf test"
                  << std::endl;
        std::cout << "test_encode_base                                                              : basic encode rest"
                  << std::endl;
        std::cout
            << "test_bit_str                                                                  : test low level bit_str"
            << std::endl;
        std::cout
            << "test_points  <go>                                                             : test all points in a config"
            << std::endl;
        std::cout
            << "test_points  <configs/client/gcom_test_client.json> <decode>                  : test all points in a config"
            << std::endl;
        std::cout
            << "*test_point_type  <Discrete_Inputs> <decode>                                  : test all points of a given type "
            << std::endl;
        std::cout
            << "*test_fims  fims pacing test                                                  : set up fims connection then take it dowm"
            << std::endl;
        std::cout
            << "*test_fims_uri  simple fims test                                              : set up fims connection set a few then take it dowm"
            << std::endl;
        std::cout
            << "*test_fims_uri_get  simple fims test                                          : set up fims connection get a few then take it dowm"
            << std::endl;
        std::cout
            << "*test_HB  simple HB test                                                      : create a hb struct  and run it"
            << std::endl;
        std::cout
            << "*test_get_local  simple local get test                                        : get all the local values"
            << std::endl;

        std::cout << "test_logger                                                                   : test logger"
                  << std::endl;
        std::cout
            << "test_iothreads  <go>                                                          : test new_iothreads"
            << std::endl;
        // std::cout << "test_compact_io_works  <go>                                                   : test compact
        // io_works" << std::endl;
        std::cout
            << "test_iopoint  <go>                                                            : test iopoint single point checker"
            << std::endl;
        std::cout << "    build/release/gcom_modbus_test test_iopoint 172.17.0.3 502 poll 1 Discrete_Input 395 1 23 4"
                  << std::endl;

        std::cout
            << "test_iopoint_multi  <go>                                                      : test new_iopoint multi optoons"
            << std::endl;
        std::cout << "    build/release/gcom_modbus_test test_iopoint 172.17.0.3 502 poll 1 Discrete_Input 395 1 23 4"
                  << std::endl;
        std::cout
            << "test_io_cfg <config_file> **** <uri> <id>                                     : parse the config file, run pub requests send a sample uri set "
            << std::endl;

        // std::cout << "test_bad_regs  simple fims bad regs test                                       : test for bad
        // regs"  << std::endl;
        std::cout << "test_decode  basic test decode to any                                          : test decode"
                  << std::endl;

        std::cout
            << "test_cfg_pub  <config_file>                                                    : test pub ( no threads)"
            << std::endl;

        std::cout
            << "test_cfg_decode <config_file> <device_id> <Holding|Coil> <offset> <value>     : puts the decode value into the config"
            << std::endl;
        std::cout
            << "test_cfg_encode <config_file> <device_id> <Holding|Coil> <offset>             : gets the decode value from the config"
            << std::endl;
        std::cout
            << "test_cfg_fims_decode <config_file> <fims_body>                                : puts the fims value into the config"
            << std::endl;
        std::cout
            << "test_cfg_fims_encode <config_file> <device_id> <Holding|Coil> <offset>        : gets the fims value from the config"
            << std::endl;
        std::cout << std::endl;
        std::cout << "* for test options          " << std::endl;
        std::cout << "                     : ip:172.17.0.4 " << std::endl;
        std::cout << "                     : port:502 " << std::endl;
        std::cout << "                     : auto_disable:true " << std::endl;
        std::cout << "                     : allow_multi_sets:true " << std::endl;
        std::cout << "                     : force_multi_sets:true " << std::endl;

        return 0;
    }

    if (cmd == "test_perf")
    {
        test_perf();
        return 0;
    }

    // if(cmd == "test_bad_regs") {
    //     test_find_bad_regs();
    //     return 0;
    // }

    if (cmd == "test_decode")
    {
        test_decode_raw();
        return 0;
    }

    if (cmd == "test_fims")
    {
        bool debug = true;

        if (debug)
            std::cout << "test_fims <file>  running :" << std::endl;
        // std::map<std::string, std::any> gcom_map;
        int next_arg = 2;

        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
        {
            filename = argv[2];
            next_arg = 3;
        }

        // in gcom_config_any
        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);
        // gcom_load_overrides(myCfg, next_arg, argc, argv);

        // in gcom_modbus_pub.cpp
        gcom_setup_pubs(gcom_map, myCfg, 5.0, debug);

        std::cout << "  Subs found :" << std::endl;

        for (auto& mysub : myCfg.subs)
        {
            std::cout << "sub: " << mysub << std::endl;
        }

        test_fims_connect(myCfg, debug);

        gcom_load_overrides(myCfg, next_arg, argc, argv);
        return 0;
    }

    if (cmd == "test_fims_uri")
    {
        bool debug = true;

        if (debug)
            std::cout << "test_fims_uri <file>  running :" << std::endl;
        // return 0;
        //  std::map<std::string, std::any> gcom_map;

        const char* filename = "configs/client/gcom_test_client.json";
        int next_arg = 2;
        if (argc > 2)
        {
            filename = argv[2];
            next_arg = 3;
        }

        // in gcom_config_any
        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);

        gcom_load_overrides(myCfg, next_arg, argc, argv);

        // in gcom_modbus_pub.cpp
        gcom_setup_pubs(gcom_map, myCfg, 5.0, debug);

        std::cout << "  Subs found :" << std::endl;

        for (auto& mysub : myCfg.subs)
        {
            std::cout << "sub: " << mysub << std::endl;
        }

        // const auto sub_string = fmt::format("/components/{}", "comp_sel_2440");
        // myCfg.fims_input_buf = nullptr;

        // auto io_fims = make_fims(myCfg);

        std::string name("myname");
        // const auto sub_string = fmt::format("/modbus_client/{}", name);
        // std::vector<std::string> subs;
        // subs.emplace_back(sub_string);

        // io_fims.fims_input_buf = nullptr;
        // std::cout << " set io_fims data buf " << std::endl;
        // set_io_fims_data_buf(io_fims, 100000);

        // start fims threads
        // start_process(myCfg);
        bool start_fims(std::vector<std::string> & subs, struct cfg & myCfg);
        bool stop_fims(struct cfg & myCfg);

        start_fims(myCfg.subs, myCfg);
        // const auto conn_id_str = fmt::format("modbus_client_uri@{}", name);
        const auto conn_test_str = fmt::format("modbus_client_test@{}", name);
        if (!myCfg.fims_test.Connect(conn_test_str.data()))
        {
            NEW_FPS_ERROR_PRINT("For client testuri \"{}\": could not connect to fims_server\n", name);
            return false;
        }

        StartThreads(myCfg, debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1234", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "4321", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "{\"value\":1235}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":1236}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":{\"value\":1237}}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"test_id\":{\"value\":1238},\"inv_1_16\":12,\"inv_1_4\":15 }", debug); test_fims_send_uri(myCfg, "set",
        // "/components/comp_sel_2440", "{\"test_id\":{\"value\":1239},\"inv_1_4\":15 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":{\"value\":1240},\"coil_4\":true
        // }", debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"test_id\":{\"value\":1242},\"coil_4\":0,\"coil_5\":1 }", debug); test_fims_send_uri(myCfg, "set",
        // "/components/comp_sel_2440", "{\"test_id\":{\"value\":1243},\"coil_4\":false,\"coil_5\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":flase }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":false }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":1 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":1 }", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":1 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1300", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":false }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":0 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1301", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1", "2000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1/_force", "1000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1", "3000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1/_unforce", "500", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":1243},\"multi_3\":false,\"multi_4\":true }", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2243},\"multi_3\":{\"value\":3100},\"multi_4\":{\"value\":4100} }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/_disable", "{\"multi_3\":{\"value\":3333} }",
        // debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2244},\"multi_3\":{\"value\":3102},\"multi_4\":{\"value\":4102} }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/_enable", "{\"multi_3\":{\"value\":3444} }",
        // debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2245},\"multi_3\":{\"value\":3103},\"multi_4\":{\"value\":4103} }", debug);

        test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1234", debug);

        std::cout << " Now sleeping for 1 seconds" << std::endl;
        std::this_thread::sleep_for(1000ms);

        StopThreads(myCfg, debug);
        stop_fims(myCfg);
        return 0;
    }
    if (cmd == "test_fims_uri_get")
    {
        bool debug = true;

        if (debug)
            std::cout << "test_fims_uri_get  running :" << std::endl;
        // std::map<std::string, std::any> gcom_map;

        const char* filename = "configs/client/gcom_test_client.json";
        int next_arg = 2;
        if (argc > 2)
        {
            filename = argv[2];
            next_arg = 3;
        }

        // in gcom_config_any
        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);
        gcom_load_overrides(myCfg, next_arg, argc, argv);

        // in gcom_modbus_pub.cpp
        gcom_setup_pubs(gcom_map, myCfg, 5.0, debug);

        std::cout << "  Subs found :" << std::endl;
        for (auto& mysub : myCfg.subs)
        {
            std::cout << "sub: " << mysub << std::endl;
        }

        std::string name("myname");
        // start fims threads
        // start_process(myCfg);

        start_fims(myCfg.subs, myCfg);
        // const auto conn_id_str = fmt::format("modbus_client_uri@{}", name);
        const auto conn_test_str = fmt::format("modbus_client_test@{}", name);
        if (!myCfg.fims_test.Connect(conn_test_str.data()))
        {
            NEW_FPS_ERROR_PRINT("For client testuri \"{}\": could not connect to fims_server\n", name);
            return false;
        }

        StartThreads(myCfg, debug);

        test_fims_send_uri(myCfg, "get", "/components/comp_sel_2440/test_id", nullptr, debug);

        std::cout << " Now sleeping for 0.1 secs" << std::endl;
        std::this_thread::sleep_for(100ms);

        test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/error_1/_disable", nullptr, debug);
        test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/input_7_2/_disable", nullptr, debug);
        test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/input_1_1/_disable", nullptr, debug);
        test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/break_test_1/_disable", nullptr, debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/break_test_2/_disable", nullptr, debug);

        std::cout << " Now sleeping for 0.1 secs" << std::endl;
        std::this_thread::sleep_for(100ms);

        test_fims_send_uri(myCfg, "get", "/components/comp_sel_2440", nullptr, debug);
        std::cout << " Now sleeping for 0.1 secs" << std::endl;
        std::this_thread::sleep_for(100ms);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "4321", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "{\"value\":1235}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":1236}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":{\"value\":1237}}", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"test_id\":{\"value\":1238},\"inv_1_16\":12,\"inv_1_4\":15 }", debug); test_fims_send_uri(myCfg, "set",
        // "/components/comp_sel_2440", "{\"test_id\":{\"value\":1239},\"inv_1_4\":15 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"test_id\":{\"value\":1240},\"coil_4\":true
        // }", debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"test_id\":{\"value\":1242},\"coil_4\":0,\"coil_5\":1 }", debug); test_fims_send_uri(myCfg, "set",
        // "/components/comp_sel_2440", "{\"test_id\":{\"value\":1243},\"coil_4\":false,\"coil_5\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":flase }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":false }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":1 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":1 }", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":true }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":1 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1300", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_4\":false }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440", "{\"coil_5\":0 }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/test_id", "1301", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1", "2000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1/_force", "1000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1", "3000", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/multi_1/_unforce", "500", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":1243},\"multi_3\":false,\"multi_4\":true }", debug);

        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2243},\"multi_3\":{\"value\":3100},\"multi_4\":{\"value\":4100} }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/_disable", "{\"multi_3\":{\"value\":3333} }",
        // debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2244},\"multi_3\":{\"value\":3102},\"multi_4\":{\"value\":4102} }", debug);
        // test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440/_enable", "{\"multi_3\":{\"value\":3444} }",
        // debug); test_fims_send_uri(myCfg, "set", "/components/comp_sel_2440",
        // "{\"multi_2\":{\"value\":2245},\"multi_3\":{\"value\":3103},\"multi_4\":{\"value\":4103} }", debug);

        std::cout << " Now sleeping for 1 seconds" << std::endl;
        std::this_thread::sleep_for(1000ms);

        StopThreads(myCfg, debug);
        stop_fims(myCfg);
        return 0;
    }

    if (cmd == "test_logger")
    {
        test_logger("gcom_test", argc, argv);
        return 0;
    }
    if (cmd == "test_bit_str")
    {
        gcom_test_bit_str();
        return 0;
    }

    if (cmd == "test_timer")
    {
        test_timer();
        return 0;
    }
    if (cmd == "test_timer_set")
    {
        test_timer();
        return 0;
    }

    // if(cmd == "test_encode_base")
    // {
    //     gcom_test_encode_base();
    //     return 0;
    // }
    if (cmd == "test_parse")
    {
        const char* uri = nullptr;
        const char* body = nullptr;

        if (argc > 3)
        {
            uri = argv[2];
            body = argv[3];
        }
        test_parse_message(uri, "set", body);
        return 0;
    }
    if (cmd == "test_merge")
    {
        const char* uri = nullptr;
        const char* body = nullptr;

        if (argc > 3)
        {
            uri = argv[2];
            body = argv[3];
        }
        test_merge_message(uri, "set", body);
        return 0;
    }

    // working on this ... test_uri_body
    bool gcom_findCompVar(std::shared_ptr<cfg::io_point_struct> & io_point, const struct cfg& myCfg,
                          const cfg::component_struct* comp, std::string kvar);
    void get_io_point_full(std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream & ss, struct cfg & myCfg);

    // build/release/gcom_modbus_test test_get_local configs/client/gcom_test_client.json 'offset=300&id=foo+me'
    if (cmd == "test_get_local")
    {
        bool debug = true;
        std::string name("myname");
        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];
        std::any qany;
        if (argc > 3)
        {
            std::string qustr(argv[3]);
            std::cout << " qustr [" << qustr << "]\n";
            qany = runParseQuery(qustr);
        }
        // return 0;
        // gcom_parse_file(gcom_map, filename, true);

        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);
        // first test the uri side of things
        std::cout << "client_name " << myCfg.client_name << "\n";
        gcom_setup_pubs(gcom_map, myCfg, 2.0, debug);
        gcom_setup_stats_pubs(myCfg, 2);
        myCfg.debug_decode = false;

        // now find the heartbeats in the configs

        StartThreads(myCfg, debug);
        std::string subs_str = "Subs found:";

        for (auto& mysub : myCfg.subs)
        {
            subs_str += "\n\t\t" + mysub;
        }

        const auto conn_test_str = fmt::format("modbus_client_test@{}", name);
        if (!myCfg.fims_test.Connect(conn_test_str.data()))
        {
            NEW_FPS_ERROR_PRINT("For client testuri \"{}\": could not connect to fims_server\n", name);
            return false;
        }

        // FPS_INFO_LOG(subs_str);
        start_process(myCfg);
        start_fims(myCfg.subs, myCfg);
        runTimer();

        // this does a remote set
        // bool test_uri_body(struct cfg &myCfg, const char *uri, const char *method, const char *pname, const char
        // *uname, const char *repto, const char *body)
        //  offset 1500
        //  this tests the remote path single set
        test_uri_body(myCfg, "/components/comp_sel_2440/test_id", "set", "gcom_test", "tester_name", nullptr,
                      "{\"value\":10021}");
        // 1503
        // this tests the remote path multi set
        test_uri_body(myCfg, "/components/comp_sel_2440", "set", "gcom_test", "tester_name", nullptr,
                      "{\"multi_1\":2021}");
        // 1503 1504
        test_uri_body(myCfg, "/components/comp_sel_2440", "set", "gcom_test", "tester_name", nullptr,
                      "{\"multi_1\":2021,\"multi_2\":2022}");
        // "set"
        // set myCfg.use_fims to true to actually run an external fims set
        //        test_set_uris(myCfg);

        // now test via the external fims channel
        // 1505 1506
        // auto uri = "/components/comp_sel_2440";
        // auto method = "set";
        // auto body = "{\"multi_3\":2023,\"multi_4\":2024}";

        auto uri = "/components/comp_sel_2440/multi_3";
        auto method = "set";
        auto body = "2023";

        if (!myCfg.fims_test.Send(fims::str_view{ method, strlen(method) }, fims::str_view{ uri, strlen(uri) },
                                  fims::str_view{ nullptr, 0 },
                                  fims::str_view{ "test_uname", sizeof("test_uname") - 1 },
                                  body ? fims::str_view{ body, strlen(body) } : fims::str_view{ nullptr, 0 }))
        {
            std::cout << __func__ << " unable to send to fims_test" << std::endl;
        }
        else
        {
            std::cout << __func__ << " able to send to fims_test" << std::endl;
        }

        // body = "{2024}"; // did not work
        body = "{\"value\":2025}";  // did not work
        myCfg.fims_test.Send(fims::str_view{ method, strlen(method) }, fims::str_view{ uri, strlen(uri) },
                             fims::str_view{ nullptr, 0 }, fims::str_view{ "test_uname", sizeof("test_uname") - 1 },
                             body ? fims::str_view{ body, strlen(body) } : fims::str_view{ nullptr, 0 });

        uri = "/components/comp_sel_2440/multi_4/_local";
        body = "{\"value\":4025}";

        myCfg.fims_test.Send(fims::str_view{ method, strlen(method) }, fims::str_view{ uri, strlen(uri) },
                             fims::str_view{ nullptr, 0 }, fims::str_view{ "test_uname", sizeof("test_uname") - 1 },
                             body ? fims::str_view{ body, strlen(body) } : fims::str_view{ nullptr, 0 });

        //

        uri = "/components/comp_sel_2440/_local";
        body = "{\"multi_5\":5025,\"multi_6\":5125,\"multi_7\":5225,\"multi_8\":5325}";

        myCfg.fims_test.Send(fims::str_view{ method, strlen(method) }, fims::str_view{ uri, strlen(uri) },
                             fims::str_view{ nullptr, 0 }, fims::str_view{ "test_uname", sizeof("test_uname") - 1 },
                             body ? fims::str_view{ body, strlen(body) } : fims::str_view{ nullptr, 0 });

        uri = "/components/comp_sel_2440";
        body = "{\"multi_9\":6025,\"multi_10\":6125,\"multi_11\":6225,\"multi_12\":6325}";

        myCfg.fims_test.Send(fims::str_view{ method, strlen(method) }, fims::str_view{ uri, strlen(uri) },
                             fims::str_view{ nullptr, 0 }, fims::str_view{ "test_uname", sizeof("test_uname") - 1 },
                             body ? fims::str_view{ body, strlen(body) } : fims::str_view{ nullptr, 0 });

        std::cout << " Now sleeping for 3 seconds" << std::endl;
        std::this_thread::sleep_for(3000ms);

        std::stringstream ss;
        ss << "{\"points\":[";
        bool is_first = true;
        for (auto component : myCfg.components)
        {
            for (auto reg_group : component->register_groups)
            {
                for (auto io_point : reg_group->io_point_map)
                {
                    bool ok = true;
                    if (argc > 3)
                    {
                        ok = filter_io_point(argc, io_point, qany);
                    }
                    if (ok)
                    {
                        if (!is_first)
                        {
                            ss << ", ";
                        }
                        else
                        {
                            is_first = false;
                        }
                        std::stringstream ss_temp;
                        get_io_point_full(io_point, ss_temp, myCfg);
                        ss << "\n" << ss_temp.str();
                    }
                }
            }
        }
        ss << "]}";
        std::cout << ss.str() << std::endl;
        std::cout << " Now sleeping for 3 seconds" << std::endl;
        std::this_thread::sleep_for(3000ms);
        // FPS_INFO_LOG("Shutting down");
        stop_fims(myCfg);
        stopTimer();
        StopThreads(myCfg, false);
        stop_process(myCfg);
        std::this_thread::sleep_for(1000ms);
        std::cout << "heartbeats again ..." << std::endl;
        // now find the heartbeats in the configs
        for (auto& myhb : myCfg.heartbeat_points)
        {
            std::cout << myhb.first << std::endl;
        }
        if (argc > 3)
        {
            std::string qustr(argv[3]);
            std::cout << " qustr [" << qustr << "]\n";
            runParseQuery(qustr);
        }
        return 0;
    }

    if (cmd == "test_HB")
    {
        bool debug = true;
        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];
        // gcom_parse_file(gcom_map, filename, true);

        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);
        // first test the uri side of things
        std::cout << "client_name " << myCfg.client_name << "\n";
        gcom_setup_pubs(gcom_map, myCfg, 2.0, debug);
        gcom_setup_stats_pubs(myCfg, 2);
        myCfg.debug_decode = true;

        // now find the heartbeats in the configs

        StartThreads(myCfg, debug);
        std::string subs_str = "Subs found:";

        for (auto& mysub : myCfg.subs)
        {
            subs_str += "\n\t\t" + mysub;
        }

        std::cout << "heartbeats..." << std::endl;
        // now find the heartbeats in the configs
        for (auto& myhb : myCfg.heartbeat_points)
        {
            std::cout << myhb.first << std::endl;
            std::cout << " read uri  [" << myhb.second->component_heartbeat_read_uri << "]" << std::endl;
            std::cout << " write uri  [" << myhb.second->component_heartbeat_write_uri << "]" << std::endl;
            std::cout << " frequency  [" << myhb.second->frequency << "]" << std::endl;
            std::cout << " device id  [" << myhb.second->device_id << "]" << std::endl;

            std::shared_ptr<cfg::io_point_struct> read_point = nullptr;
            if (myhb.second->component_heartbeat_read_uri != "")
            {
                if (gcom_findCompVar(read_point, myCfg, myhb.second->component,
                                     myhb.second->component_heartbeat_read_uri))
                    std::cout << " found read_uri" << std::endl;
            }
            std::shared_ptr<cfg::io_point_struct> write_point = nullptr;
            if (myhb.second->component_heartbeat_write_uri != "")
            {
                if (gcom_findCompVar(write_point, myCfg, myhb.second->component,
                                     myhb.second->component_heartbeat_write_uri))
                    std::cout << " found write_uri" << std::endl;
            }
            double freq = (myhb.second->frequency / 1000.0);
            double offset = (myhb.second->offset_time / 1000.0);
            // gcom_heartbeat.cpp

            myhb.second->setupHb(myCfg, myhb.first, myhb.second->device_id, write_point, read_point, freq, offset);
        }

        // FPS_INFO_LOG(subs_str);
        start_process(myCfg);
        start_fims(myCfg.subs, myCfg);
        runTimer();

        // set myCfg.use_fims to true to actually run an external fims set
        //        test_set_uris(myCfg);

        //
        std::cout << " Now sleeping for 5 seconds" << std::endl;
        std::this_thread::sleep_for(5000ms);

        // FPS_INFO_LOG("Shutting down");
        stop_fims(myCfg);
        stopTimer();
        StopThreads(myCfg, false);
        stop_process(myCfg);
        std::this_thread::sleep_for(1000ms);
        std::cout << "heartbeats again ..." << std::endl;
        // now find the heartbeats in the configs
        for (auto& myhb : myCfg.heartbeat_points)
        {
            std::cout << myhb.first << std::endl;
        }

        return 0;
    }

    if (cmd == "test_cfg")
    {
        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];
        // gcom_parse_file(gcom_map, filename, true);

        gcom_load_cfg_file(gcom_map, filename, myCfg, true);
        // first test the uri side of things
        std::cout << "client_name " << myCfg.client_name << "\n";

        test_uri(myCfg, "/components/comp_sel_2440/door_latch");
        test_uri(myCfg, "/components/comp_sel_2440");
        test_uri(myCfg, "/xcomponents/comp_sel_2440");
        test_uri(myCfg, "/components/comp_sel_2440/xdoor_latch");

        // from test_iothreads
        StartThreads(myCfg, true);
        // set myCfg.use_fims to true to actually run an external fims set
        test_set_uris(myCfg);

        //
        std::cout << " Now sleeping for 2 seconds" << std::endl;
        std::this_thread::sleep_for(2000ms);
        StopThreads(myCfg, true);

        // close context
        // TODO may need locks here
        // CloseModbusCtx(num_threads, ip , port);

        // io_threadChan.send(0);
        //  for (int i = 0; i < num_threads; ++i) {
        //      io_threadChan.send(0);
        //  }

        // threadControl.stopThreads();
        // {
        //     std::lock_guard<std::mutex> lock2(io_output_mutex);
        //     FPS_INFO_LOG("all threads stopping ");
        // }

        //    threadControl.responseThread.join();
        // Signal the io_thread to stop after some time
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        // {
        //     std::lock_guard<std::mutex> lock2(io_output_mutex);
        //     double tNow = get_time_double();
        //     std::cout << " final sleep "<< tNow << std::endl;
        // }

        // {
        //     std::lock_guard<std::mutex> lock2(io_output_mutex);
        //     FPS_INFO_LOG("IO Threads : stopping.");
        //     FPS_LOG_IT("shutdown");
        // }

        return 0;
    }

    if (cmd == "test_points")
    {
        const char* filename = "configs/client/gcom_test_client.json";
        const char* decode = nullptr;
        if (argc > 2)
            filename = argv[2];
        if (argc > 3)
            decode = argv[3];
        {
            gcom_load_cfg_file(gcom_map, filename, myCfg, true);
            gcom_map.clear();
            resetCfg(myCfg);

            gcom_load_cfg_file(gcom_map, filename, myCfg, true);

            gcom_points_test(gcom_map, myCfg, decode);
        }
        return 0;
    }
    if (cmd == "test_point_type")
    {
        const char* filename = "configs/client/gcom_test_client.json";
        const char* ptype = "Direct_Inputs";
        const char* decode = nullptr;
        if (argc > 2)
            ptype = argv[2];
        if (argc > 3)
            decode = argv[3];
        {
            gcom_load_cfg_file(gcom_map, filename, myCfg, true);

            gcom_point_type_test(gcom_map, myCfg, ptype, decode);
        }
        return 0;
    }

    if (cmd == "test_iothreads")
    {
        test_io_threads(myCfg);
        std::cout << " done with io threads " << std::endl;

        return 0;
    }

    // if(cmd == "test_compact_io_works")
    // {
    //     test_compact_io_works();
    //     return 0;
    // }

    if (cmd == "test_cfg_msg")
    {
        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];
        // std::map<std::string, std::any> gcom_map;
        // const char* filename = argv[2];
        // auto rval =
        gcom_parse_file(gcom_map, filename, false, true);
        // std::cout << " File : "<< filename << " parsed :" << rval << std::endl;
        gcom_msg_test(gcom_map, myCfg);
        return 0;
    }

    if (cmd == "test_cfg_uri")
    {
        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];
        try
        {
            // std::map<std::string, std::any> gcom_map;
            // const char* filename = argv[2];
            const char* uri = nullptr;
            const char* id = nullptr;
            if (argc > 3)
                uri = argv[3];
            if (argc > 4)
                id = argv[4];
            // auto rval =
            gcom_parse_file(gcom_map, filename, false, false);
            std::cout << " File : " << filename << " parsed :" << std::endl;
            gcom_config_test_uri(gcom_map, myCfg, uri, id);
        }
        catch (...)
        {
            std::cout << " check args for test_cfg_uri  <cfg_file> <component> <id>" << std::endl;
        }
        return 0;
    }
    if (cmd == "test_io_cfg")
    {
        Logging::Init("modbus_client_test", argc, argv);
        bool debug = true;
        myCfg.connection.debug = true;
        if (debug)
            std::cout << "test_io_cfg <file>  running :" << std::endl;
        // std::map<std::string, std::any> gcom_map;

        const char* filename = "configs/client/gcom_test_client.json";
        if (argc > 2)
            filename = argv[2];

        // const char* uri = argv[3];
        // const char* id = argv[4];
        // if(debug)
        // std::cout << "test_io_cfg <file> : "<< filename << " uri : "<< uri << " Id: "<< id  << std::endl;

        // auto rval =

        // in gcom_config_any
        gcom_load_cfg_file(gcom_map, filename, myCfg, debug);

        // in gcom_modbus_pub.cpp
        gcom_setup_pubs(gcom_map, myCfg, 5.0, debug);
        fims_connect(myCfg, debug);

        StartThreads(myCfg, debug);

        std::cout << "  Subs found :" << std::endl;

        for (auto& mysub : myCfg.subs)
        {
            std::cout << "sub: " << mysub << std::endl;
        }

        std::cout << "  Timers found :" << std::endl;
        showTimerObjects();

        std::cout << "  Run Timer for 10 seconds :" << std::endl;
        runTimer();

        std::this_thread::sleep_for(std::chrono::seconds(10));

        std::cout << "  stop Timer :" << std::endl;
        stopTimer();
        // std::cout <<"  Timer stopped :" <<std::endl;
        // showTimerObjects();
        // std::cout <<"  end of objects :" <<std::endl;

        // //resp thread in test_io_threads does all the backend work
        // startRespThread(myCfg);
        // std::cout <<"  started resp thread :" <<std::endl;

        // sendpollChantoResp(true);
        // std::cout <<"  sent poll to resp :" <<std::endl;
        // //clearrespChan(true);
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        // std::cout <<"  stopping resp :" <<std::endl;
        // stopRespThread(myCfg);
        // std::cout <<"  stoped resp :" <<std::endl;
        // CloseModbusForThread(io_thread, debug);

        StopThreads(myCfg, debug);
        myCfg.performance_timers.modbus_read_timer.show();
        myCfg.performance_timers.modbus_write_timer.show();
        myCfg.performance_timers.pub_timer.show();
        return 0;
    }

    if (argc > 3)
    {
        if (cmd == "test_io")
        {
            // const char* oper="set";
            // int device_id = 1;
            // const char* register_type = "Holding";
            // int offset = 1;
            // int value = 21;
            // bool debug = false;
            // if (argc > 4)
            //     oper = argv[4];
            // if (argc > 5)
            //     device_id = atoi(argv[5]);
            // if (argc > 6)
            //     register_type = argv[6];
            // if (argc > 7)
            //     offset = atoi(argv[7]);
            // if (argc > 8)
            //     value = atoi(argv[8]);
            // if (argc > 9)
            // {
            //     debug = (atoi(argv[9])  == 1);
            //     //std::cout<< " found debug :"<< argv[9] <<"  debug "<<debug<<std::endl;
            // }

            // if(debug)
            std::cout
                << "test_io deprecated <ip> <port> <set|pub|get> <device_id> <Holding|Coil> <offset> <value> <debug>"
                << std::endl;
            // test_iothread(argv[2], atoi(argv[3]), oper, device_id, register_type, offset, value, debug);

            // now we have to peel off the data  from the response queue
            return 0;
        }

        // this the latest
        if (cmd == "test_iopoint_multi")
        {
            const char* oper = "set";
            int device_id = 1;
            const char* register_type = "Holding";
            int offset = 1;
            int value = 21;
            int num_threads = 4;
            bool debug = false;
            if (argc > 4)
                oper = argv[4];
            if (argc > 5)
                device_id = atoi(argv[5]);
            if (argc > 6)
                register_type = argv[6];
            if (argc > 7)
                offset = atoi(argv[7]);
            if (argc > 8)
                value = atoi(argv[8]);
            if (argc > 9)
            {
                num_threads = atoi(argv[9]);
                // std::cout<< " found debug :"<< argv[9] <<"  debug "<<debug<<std::endl;
            }
            if (argc > 10)
            {
                debug = (atoi(argv[10]) == 1);
                // std::cout<< " found debug :"<< argv[9] <<"  debug "<<debug<<std::endl;
            }

            const char* filename = "configs/client/gcom_test_client.json";

            if (debug)
                std::cout << "test_io <ip> <port> <set|pub|get> <device_id> <Holding|Coil> <offset> <value> <debug>"
                          << std::endl;
            // in gcom_config_any
            gcom_load_cfg_file(gcom_map, filename, myCfg, false);

            test_io_point_multi(argv[2], atoi(argv[3]), 2, oper, device_id, register_type, offset, value, num_threads,
                                myCfg, debug);

            // now we have to peel off the data  from the response queue
            return 0;
        }

        // this the latest single point test
        if (cmd == "test_iopoint")
        {
            const char* oper = "set";
            int device_id = 1;
            const char* register_type = "Holding";
            int offset = 1;
            int value = 21;
            int num_regs = 1;
            int num_threads = 4;
            bool debug = false;
            if (argc > 4)
                oper = argv[4];
            if (argc > 5)
                device_id = atoi(argv[5]);
            if (argc > 6)
                register_type = argv[6];
            if (argc > 7)
                offset = atoi(argv[7]);
            if (argc > 8)
                num_regs = atoi(argv[8]);
            if (argc > 9)
                value = atoi(argv[9]);
            if (argc > 10)
            {
                num_threads = atoi(argv[10]);
                // std::cout<< " found debug :"<< argv[9] <<"  debug "<<debug<<std::endl;
            }
            if (argc > 11)
            {
                debug = (atoi(argv[11]) == 1);
                // std::cout<< " found debug :"<< argv[9] <<"  debug "<<debug<<std::endl;
            }

            const char* filename = "configs/client/gcom_test_client.json";

            if (debug)
                std::cout
                    << "test_io <ip> <port> <set|pub|get> <device_id> <Holding|Coil> <offset> <num_regs> <value> <num_threads> <debug>"
                    << std::endl;
            // in gcom_config_any
            gcom_load_cfg_file(gcom_map, filename, myCfg, false);

            test_io_point_single(argv[2], atoi(argv[3]), 2.0, oper, device_id, register_type, offset, num_regs, value,
                                 num_threads, myCfg, debug);

            // now we have to peel off the data  from the response queue
            return 0;
        }

        std::cout << "Please select a test" << std::endl;
    }
    return 0;
}

bool queue_work(cfg::Register_Types register_type, int device_id, int offset, bool off_by_one, int num_regs,
                uint16_t* u16bufs, uint8_t* u8bufs, WorkTypes wtype)
{
    std::shared_ptr<IO_Work> io_work;

    if (!io_poolChan.receive(io_work, 0))
    {  // Assuming receive will return false if no item is available.
        io_work = std::make_shared<IO_Work>();
    }

    // Modify io_work data if necessary here
    io_work->io_points.clear();
    io_work->local = false;

    io_work->tStart = get_time_double();
    io_work->device_id = device_id;
    io_work->register_type = register_type;
    io_work->offset = offset;
    io_work->num_registers = num_regs;
    io_work->off_by_one = off_by_one;

    io_work->set_bufs(num_regs, u16bufs, u8bufs);
    io_work->wtype = wtype;
    io_work->test_mode = false;
    io_work->pub_struct = nullptr;

    io_pollChan.send(std::move(io_work));
    io_threadChan.send(1);

    return true;  // You might want to return a status indicating success or failure.
}

int test_io_threads(struct cfg& myCfg)
{
    int num_threads = 4;
    StartThreads(num_threads, nullptr, 0, 2.0, 0.5, myCfg);

    // Simulate the sending of work
    for (int i = 0; i < 10; ++i)
    {
        queue_work(cfg::Register_Types::Holding, i, 0, false, 5, nullptr, nullptr, WorkTypes::Noop);
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Sleep for a bit before sending the next
    }
    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " jobs queued  at " << tNow << std::endl;
    }

    // Signal the io_thread to stop after some time
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " done sleeping " << tNow << std::endl;
    }

    // io_threadChan.send(0);

    threadControl.stopThreads();
    FPS_INFO_LOG("All threads stopping ");

    threadControl.responseThread.join();
    // Signal the io_thread to stop after some time
    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " final sleep " << tNow << std::endl;
    }

    {
        FPS_INFO_LOG("IO Threads stopping.");
        FPS_LOG_IT("shutdown");
    }
    return 0;
}

void test_io_point_single(const char* ip, int port, double connection_timeout, const char* oper, int device_id,
                          const char* regtype, int offset, int num_regs, int value, int num_threads, struct cfg& myCfg,
                          bool debug)
{
    // auto thread_debug = debug;
    // int num_threads = 4;
    int num_points = 1;

    // TODO fix these
    std::string register_type_str(regtype);
    std::string roper(oper);
    // int num_regs = 1;
    auto register_type = strToRegType(register_type_str);
    auto cfgreg_type = myCfg.typeFromStr(register_type_str);
    std::cout << " cfg Register Type" << myCfg.typeToStr(cfgreg_type) << std::endl;
    auto work_type = strToWorkType(oper);

    uint16_t u16bufs[130];  // MAX_MODBUS_NUM_REGS
    uint8_t u8bufs[130];
    for (int i = 0; i < num_regs; ++i)
    {
        u16bufs[i] = value;
        u8bufs[i] = value;
    }

    StartThreads(num_threads, ip, port, connection_timeout, 0.5, myCfg);

    std::this_thread::sleep_for(100ms);

    for (int j = 0; j < 10; ++j)
    {
        for (int i = 0; i < num_points; ++i)
        {
            double tNow = get_time_double();
            u16bufs[0] = i;
            // std::shared_ptr<IO_Work> make_work(cfg::Register_Types register_type, int device_id, int offset, int
            // num_regs, uint16_t* u16bufs, uint8_t* u8bufs, WorkTypes wtype ) {

            auto io_work = make_work(nullptr, register_type, device_id, offset, false, num_regs, u16bufs, u8bufs,
                                     work_type);
            io_work->test_mode = true;
            io_work->work_group = num_points;
            io_work->work_name = std::string("test_io_point_single");
            io_work->tNow = tNow;

            if (j == 4)
                io_work->test_it = true;
            else
                io_work->test_it = false;

            pollWork(io_work);
            std::cout << " Test sleeping for 1 second" << std::endl;
            std::this_thread::sleep_for(1000ms);
            // std::cout << " Test io_work errno_code  "<< (int)io_work->errno_code<< std::endl;
            if (io_work->errno_code == BAD_DATA_ADDRESS)
            {
                std::cout << " Test io_work errno_code  " << (int)io_work->errno_code << " Skipping Test" << std::endl;
                j = 10;
            }
        }
    }
    std::cout << " Now sleeping for 2 seconds" << std::endl;
    std::this_thread::sleep_for(2000ms);

    // close context
    // TODO may need locks here
    // CloseModbusCtx(num_threads, ip , port);

    io_localthreadChan.send(0);

    for (int i = 0; i < num_threads; ++i)
    {
        io_threadChan.send(0);
    }

    threadControl.stopThreads();
    {
        FPS_INFO_LOG("all threads stopping ");
    }

    threadControl.responseThread.join();
    // Signal the io_thread to stop after some time
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // {
    //     std::lock_guard<std::mutex> lock2(io_output_mutex);
    //     double tNow = get_time_double();
    //     std::cout << " final sleep "<< tNow << std::endl;
    // }

    FPS_INFO_LOG("IO Threads stopped.");
    FPS_LOG_IT("shutdown");
}

void test_io_point_multi(const char* ip, int port, int connection_timeout, const char* oper, int device_id,
                         const char* regtype, int offset, int value, int num_threads, struct cfg& myCfg, bool debug)
{
    // auto thread_debug = debug;
    // int num_threads = 4;
    int num_points = 10;

    // TODO fix these
    std::string register_type_str(regtype);
    std::string roper(oper);
    int num_regs = 1;
    auto register_type = strToRegType(register_type_str);

    auto work_type = strToWorkType(oper);

    uint16_t u16bufs[130];  // MAX_MODBUS_NUM_REGS
    uint8_t u8bufs[130];
    for (int i = 0; i < num_regs; ++i)
    {
        u16bufs[i] = value;
        u8bufs[i] = value;
    }

    StartThreads(num_threads, ip, port, connection_timeout, 0.5, myCfg);
    // SetUPModbusCtx(num_threads, ip , port, connection_timeout);

    std::this_thread::sleep_for(100ms);
    for (int i = 0; i < num_points; ++i)
    {
        u16bufs[0] = i;
        queue_work(register_type, device_id, offset, false, num_regs, u16bufs, u8bufs, work_type);
    }

    std::this_thread::sleep_for(10000ms);

    io_localthreadChan.send(0);
    // this wont work
    for (int i = 0; i < num_threads; ++i)
    {
        io_threadChan.send(0);
    }

    threadControl.stopThreads();
    {
        FPS_INFO_LOG("all threads stopping ");
    }

    threadControl.responseThread.join();

    FPS_INFO_LOG("IO Threads stopped.");
    FPS_LOG_IT("shutdown");
}
