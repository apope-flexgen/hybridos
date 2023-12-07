// gcom_utils.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_22_2023

#include <iostream>
#include <any>
#include <map>
#include <vector>
#include <simdjson.h>  // used in 
//#include <spdlog/spdlog.h>
//#include <spdlog/fmt/ostr.h>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <tuple>
#include <random>
#include <regex>
//#include <cxxabi.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "gcom_config.h"
#include "logger/logger.h"
#include "gcom_config_tmpl.h"
#include "gcom_iothread.h"
#include "gcom_utils.h"

//////////////////////////////////////
////////// UTILITY METHODS ////////////
//////////////////////////////////////// Helper function straight from C++20 so we can use it here in C++17:


std::string addQuote(const std::string &si)
{
    return "\"" + si + "\"";
}

std::string workTypeToStr(WorkTypes &work_type)
{
    if (work_type == WorkTypes::Set)
        return "Set";
    else if (work_type == WorkTypes::Get)
        return "Get";
    else if (work_type == WorkTypes::Poll)
        return "Poll";
    return "Unknown";
}

cfg::Register_Types strToRegType(std::string &register_type_str)
{
    // std::cout << " Register Type ["<< register_type <<"]"<<std::endl;
    auto register_type = cfg::Register_Types::Holding;
    if (register_type_str == "Coil")
        register_type = cfg::Register_Types::Coil;
    else if (register_type_str == "Input")
        register_type = cfg::Register_Types::Input;
    else if (register_type_str == "Input Registers")
        register_type = cfg::Register_Types::Input;
    else if (register_type_str == "Discrete_Input")
        register_type = cfg::Register_Types::Discrete_Input;
    else if (register_type_str == "Discrete Inputs")
        register_type = cfg::Register_Types::Discrete_Input;
    else if (register_type_str == "Discrete")
        register_type = cfg::Register_Types::Discrete_Input;
    return register_type;
}

std::string regTypeToStr(cfg::Register_Types &register_type)
{
    if (register_type == cfg::Register_Types::Holding)
        return "Holding";
    else if (register_type == cfg::Register_Types::Coil)
        return "Coil";
    else if (register_type == cfg::Register_Types::Input)
        return "Input";
    else if (register_type == cfg::Register_Types::Discrete_Input)
        return "Discrete Input";
    return "Unknown";
}

WorkTypes strToWorkType(std::string roper, bool debug = false)
{
    auto work_type = WorkTypes::Noop; // no op
    if (roper == "set")
    {
        work_type = WorkTypes::Set;
        if (debug)
            std::cout << " pushing set_work  " << std::endl;
    }
    else if (roper == "get")
    {
        work_type = WorkTypes::Get;
        if (debug)
            std::cout << "  running with Get  " << roper << std::endl;
    }
    else if (roper == "poll")
    {
        work_type = WorkTypes::Get;
        if (debug)
            std::cout << "  running with Get  " << roper << std::endl;
    }
    else
    {
        std::cout << " operation " << roper << " not yet supported just use set or poll" << std::endl;
    }
    return work_type;
}

// for checking for suffix's over fims (like a raw get request)
//static constexpr 
void randomDelay(int minMicroseconds, int maxMicroseconds)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(minMicroseconds, maxMicroseconds);

    int sleepTime = distrib(gen);
    std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
}

bool str_ends_with(std::string_view str, std::string_view suffix) noexcept
{
	const auto str_len = str.size();
	const auto suffix_len = suffix.size();
	return str_len >= suffix_len 
                && std::string_view::traits_type::compare(str.end() - suffix_len, suffix.data(), suffix_len) == 0;
}

void gcom_show_overrides()
{
    std::cout << "help                   " << "       show override help "                                    << std::endl;
    std::cout << "auto_disable:false     " << "       turn off the automatic invalid point disable feature "  << std::endl;
    std::cout << "allow_multi_sets:false " << "       allow set operations to be grouped together  "          << std::endl;
    std::cout << "force_multi_sets:false " << "       force set operations to be grouped together  "          << std::endl;
    std::cout << "ip:172.17.0.3          " << "       override the config ip_address  "                       << std::endl;
    std::cout << "port:503               " << "       override the config port number  "                      << std::endl;
    std::cout << "format:naked|clothed   " << "       override the config format  "                           << std::endl;
    std::cout << "debug:true             " << "       turn on debug options  "                                << std::endl;
    std::cout << "debug_decode:true      " << "       turn on decode debug options  "                         << std::endl;
    std::cout << "debug_connection:true  " << "       turn on connection debug options  "                     << std::endl;
    std::cout << "debug_fims:true        " << "       turn on fims debug options  "                           << std::endl;
    std::cout << "debug_pub:false        " << "       turn on pub  debug options  "                           << std::endl;
    std::cout << "debug_hb:false         " << "       turn on heartbeat debug options  "                      << std::endl;
    std::cout << "pub_coil:false         " << "       turn pub output for coils  "                            << std::endl;
    std::cout << "pub_holding:false      " << "       turn pub output for holding  "                          << std::endl;
    std::cout << "pub_input:true         " << "       turn pub output for input  "                            << std::endl;
    std::cout << "pub_sync:false         " << "       turn off pub sync  "                                    << std::endl;
    std::cout << "pub_discrete:true      " << "       turn pub output for disctete inputs  "                  << std::endl;
    std::cout << "pub_stats:false        " << "       turn off stats pub  "                                   << std::endl;
    std::cout << "max_num_connections:1  " << "       set up number of threads  "                             << std::endl;
    std::cout << "use_new_watchdog:false " << "       use new watchdog   "                                    << std::endl;
}

std::string removeSlashesAndExtension(std::string filename)
{
    std::size_t last_slash = filename.find_last_of("/\\");
    filename = filename.substr(last_slash + 1);
    std::size_t extension = filename.find(".json");
    filename = filename.substr(0, extension); // not entirely sure why I had to do this in two steps, but trying to do it in one step didn't work
    return filename;
}

bool gcom_load_overrides(struct cfg &myCfg, int next_arg, const int argc, const char* argv[])
{
    //auto_disable:false;
    //allow_multi_sets:false;
    //force_multi_sets:false;
    //ip:172.17.0.3
    //port:503
    //debug:true
    
    while (next_arg < argc)
    {
        std::string next_str(argv[next_arg]);
        auto next_vec = split_string(next_str, ':');
        //FPS_INFO_LOG("Received signal: [%s]", strsignal(sig));
        std::stringstream ss;
        ss << " option [" << argv[next_arg]  << "]";

        std::string ok("Ok");
        if(next_vec.size() == 1)
        {
            if (next_vec[0] == "help")
            {
                gcom_show_overrides();
            }
        }
        if(next_vec.size() > 1)
        {
            bool bval = false;
            if (next_vec[1] == "true")
            {
                bval = true;
            }

            if (next_vec[0] == "auto_disable")
            {
                myCfg.auto_disable = bval;
            }
            else if (next_vec[0] == "allow_multi_sets")
            {
                myCfg.allow_multi_sets = bval;
            }
            else if (next_vec[0] == "force_multi_sets")
            {
                myCfg.force_multi_sets = bval;
            }
            else if (next_vec[0] == "debug_decode")
            {
                myCfg.debug_decode = bval;
            }
            else if (next_vec[0] == "debug_hb")
            {
                myCfg.debug_hb = bval;
            }
            else if (next_vec[0] == "debug_connection")
            {
                myCfg.connection.debug = bval;
            }
            else if (next_vec[0] == "debug_pub")
            {
                myCfg.debug_pub = bval;
            }
            else if (next_vec[0] == "ip")
            {
                myCfg.connection.ip_address = std::string(next_vec[1]);
            }
            else if (next_vec[0] == "port")
            {
                myCfg.connection.port = atoi(next_vec[1].c_str());
            }
            else if (next_vec[0] == "max_num_connections")
            {
                int num = atoi(next_vec[1].c_str());
                if (num < 0 || num > 16)
                    num =1;
                myCfg.connection.max_num_connections = num;
            }
            else if (next_vec[0] == "pub_coil")
            {
                myCfg.pub_coil = bval;
            }
            else if (next_vec[0] == "pub_holding")
            {
                myCfg.pub_holding = bval;
            }
            else if (next_vec[0] == "pub_input")
            {
                myCfg.pub_input = bval;
            }
            else if (next_vec[0] == "pub_discrete")
            {
                myCfg.pub_discrete = bval;
            }
            else if (next_vec[0] == "pub_sync")
            {
                myCfg.pub_sync = bval;
            }
            else if (next_vec[0] == "pub_stats")
            {
                myCfg.pub_stats = bval;
            }
            else if (next_vec[0] == "use_new_watchdog")
            {
                myCfg.use_new_wdog = bval;
            }
            else if (next_vec[0] == "format")
            {
                myCfg.connection.format = std::string(next_vec[1]);
            }
            else
            {
                ok = "unknown option";
            }
            ss << " ..." << ok;
            FPS_INFO_LOG("Processing Override : %s", ss.str().c_str());

        }
        next_arg++;
    }
    return false;
}

// Function to detect the data type of the value
std::any detectTypeAndCast(const std::string& value) {
    // Check for boolean
    if (value == "true") {
        return std::any(true);
    } else if (value == "false") {
        return std::any(false);
    }

    // Check for integer
    if (std::all_of(value.begin(), value.end(), ::isdigit)) {
        return std::any(std::stoi(value));
    }

    // Check for floating point number
    try {
        size_t sz;
        float f = std::stof(value, &sz);
        if (sz == value.size()) {
            return std::any(f);
        }
    } catch (...) {
        // Not a float, fall through to string
    }

    // Default to string
    return std::any(value);
}

std::map<std::string, std::any> parseQueryString(const std::string& query) 
{
    std::map<std::string, std::any> data;
    std::istringstream queryStream(query);
    std::string pair;

    while (std::getline(queryStream, pair, '&')) {
        auto equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);

            // Replace '+' with ' ' (space) if needed
            std::replace(value.begin(), value.end(), '+', ' ');

            // Detect the type of the value and cast it
            data[key] = detectTypeAndCast(value);
        }
    }

    return data;
}

bool filter_io_point(int argc, const std::shared_ptr<struct cfg::io_point_struct> point,
                                       const std::any& crit) 
{

    bool match = true;
    if (argc < 3 ) return true;

    //std::map<std::string, std::any> 
    auto qany = std::any_cast<std::map<std::string, std::any>>(crit);

    for (const auto& [key, value] : qany) {
        // Check if the key is 'id' and if the type is an integer
        if (key == "id" && value.type() == typeid(std::string)) {
            match = match && (std::any_cast<std::string>(value) == point->id);
        }
        // Check if the key is 'offset' and if the type is a float
        else if (key == "offset" && value.type() == typeid(int)) {
            match = match && (std::any_cast<int>(value) == point->offset);
        }
        else if (key == "enabled" && value.type() == typeid(bool)) {
            match = match && (std::any_cast<bool>(value) == point->is_enabled);
        }
        // ... handle other fields similarly

        // If any match fails, break out of the loop
        if (!match) {
            return false;
        }
    }
    return true;

}

std::any runParseQuery(std::string &query)
{

    // Parse the query string into std::any
    auto data = parseQueryString(query);

     // Print the stored values and their types
    for (const auto& [key, val] : data) {
        std::cout << key << " : ";
        if (val.type() == typeid(int)) {
            std::cout << "int(" << std::any_cast<int>(val) << ")";
        } else if (val.type() == typeid(float)) {
            std::cout << "float(" << std::any_cast<float>(val) << ")";
        } else if (val.type() == typeid(bool)) {
            std::cout << "bool(" << std::any_cast<bool>(val) << ")";
        } else if (val.type() == typeid(std::string)) {
            std::cout << "string(" << std::any_cast<std::string>(val) << ")";
        }
        std::cout << std::endl;
    }

    return data;
}

std::vector<std::string> split_string(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void replace_space_with_underscores(std::string& str) {
    std::replace(str.begin(), str.end(), ' ', '_');
}

std::string replace_space_with_underscores(std::string_view name) 
{
    std::string result(name.begin(), name.end());
    std::replace(result.begin(), result.end(), ' ', '_');
    return result;
}

//std::string check_str_for_error(const std::string_view str, const std::string_view Forbidden_Chars = R"({}\/ "%)", 
//                            const std::size_t Max_Str_Size = std::numeric_limits<u8>::max())
std::string check_str_for_error(const std::string_view str, const std::string_view Forbidden_Chars, 
                            const std::size_t Max_Str_Size)
{
    if (str.empty())
    {
        return "string is empty";
    }
    if (str.find_first_of(Forbidden_Chars) != std::string_view::npos)
    {
        return fmt::format("string (currently: \"{}\") contains one of the forbidden characters: '{}'", str, fmt::join(Forbidden_Chars, "', '"));
    }
    if (str.size() > Max_Str_Size)
    {
        return fmt::format("string (currently: \"{}\", size: {}) has exceeded the maximum character limit of {}", str, str.size(), Max_Str_Size);
    }
    return "";
}

int hostname_to_ip(std::string_view hostname, char *ip, int iplen)
{
    struct hostent *he;
    struct in_addr **addr_list;
    if (he = gethostbyname(hostname.data()); he == NULL)
    {
        // get the host info
        return 1;
    }
    addr_list = (struct in_addr **)he->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++)
    {
        // Return the first one;
        strncpy(ip, inet_ntoa(*addr_list[i]), iplen);
        return 0;
    }
    return 1;
}

bool service_to_port(std::string_view service, int &port)
{
    struct servent *serv;
    /* getservbyname() - opens the etc.services file and returns the */
    /* values for the requested service and protocol.                */
    serv = getservbyname(service.data(), "tcp");
    if (serv == NULL)
    {
        FPS_INFO_LOG("port cannot be derived from service [%s] for protocol [tcp] (it doesn't exist), going back to port provided in config (or default)", service);
        return false;
    }
    port = ntohs(serv->s_port);
    return true;
}

#include <chrono>
#include <sstream>
#include <iomanip>

// Assuming time_fraction is a template function that extracts the desired part of the time
template<typename DurationType>
DurationType time_fraction(std::chrono::system_clock::time_point timestamp) {
    return std::chrono::duration_cast<DurationType>(timestamp.time_since_epoch()) % std::chrono::seconds(1);
}

void addTimeStamp(std::stringstream &ss)
{
    const auto timestamp = std::chrono::system_clock::now();
    // Convert time_point to time_t for use with std::localtime
    std::time_t timestamp_t = std::chrono::system_clock::to_time_t(timestamp);
    const auto timestamp_micro = time_fraction<std::chrono::microseconds>(timestamp);

    // Format the timestamp into the stringstream
    ss << std::put_time(std::localtime(&timestamp_t), R"("Timestamp":"%m-%d-%Y %T)");
    ss << '.' << std::setfill('0') << std::setw(6) << timestamp_micro.count() << "\"";

    // Now 'ss' contains the formatted timestamp
    return;
}



// std::string gcom_show_subs(struct cfg &myCfg, bool debug = false)
// {
//     std::ostringstream oss;
//     oss << "\"subs\": [";
//     bool first = true;
//     for (const auto &sub : subs)
//     {
//         if (!first)
//             oss << ",\n         \"" << sub << "\"";
//         else
//             oss << "\n         \"" << sub << "\"";
//         first = false;
//     }
//     oss << "\n        ]\n";
//     if (debug)
//         std::cout << oss.str() << "\n";
//     return oss.str();
// }

// std::string gcom_show_pubs(struct cfg &myCfg, bool debug = false)
// {
//     std::ostringstream json;
//     json << "{\n";
//     std::vector<std::string> freqStrings;
//     for (const auto &freqPair : pubs)
//     {
//         int freq = freqPair.first;
//         std::ostringstream freqStream;
//         freqStream << "\t\"" << freq << "\": {\n";
//         std::vector<std::string> offsetStrings;
//         for (const auto &offsetPair : freqPair.second)
//         {
//             int offset_time = offsetPair.first;
//             std::ostringstream offsetStream;
//             offsetStream << "\t\t\"" << offset_time << "\": {\n";
//             std::vector<std::string> regStrings;
//             for (auto regPtr : offsetPair.second)
//             {
//                 std::ostringstream regStream;
//                 regStream << "\t\t\t\"" << regPtr->register_type_str << "\": [ [" << regPtr->device_id << "," << regPtr->starting_offset << ", " << regPtr->number_of_registers << "] ]";
//                 regStrings.push_back(regStream.str());
//             }
//             offsetStream << join(",\n", regStrings.begin(), regStrings.end());
//             offsetStream << "\n\t\t}";
//             offsetStrings.push_back(offsetStream.str());
//         }
//         freqStream << join(",\n", offsetStrings.begin(), offsetStrings.end());
//         freqStream << "\n\t}";
//         freqStrings.push_back(freqStream.str());
//     }
//     json << join(",\n", freqStrings.begin(), freqStrings.end());
//     json << "\n}";
//     if (debug)
//         std::cout << "\"pubs\" : \n"
//                   << json.str() << "\n";
//     return json.str();
// }

// /// @brief show the types found in a cfg
// std::string gcom_show_types(struct cfg &myCfg)
// {
//     std::ostringstream oss;
//     oss << "\"Types found\": {\n";
//     bool firstTypeItem = true;
//     for (const auto &type_item : types)
//     {
//         if (!firstTypeItem)
//         {
//             oss << ",\n";
//         }
//         oss << "      \"" << type_item.first << "\": {\n";
//         bool firstTypeReg = true;
//         for (const auto &type_reg : type_item.second)
//         {
//             if (!firstTypeReg)
//             {
//                 oss << ",\n";
//             }
//             oss << "         \"" << type_reg.first << "\": {";          // If the type_map has any properties, add them here
//             oss << "\"map_id\": \"" << type_reg.second.map->id << "\""; // Adding the map_id property
//             oss << "}";
//             firstTypeReg = false;
//         }
//         oss << "\n      }";
//         firstTypeItem = false;
//     }
//     oss << "\n}";
//     std::cout << oss.str() << "\n";
//     return oss.str();
// }
