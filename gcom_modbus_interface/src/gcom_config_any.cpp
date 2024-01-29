// gcom_config_any.cpp
// p. wilshire
// 11_08_2023
// self review 11_23_2023
// another self review 11_29_2023

#include <iostream>
#include <any>
#include <map>
#include <vector>
#include <simdjson.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <tuple>
#include <regex>
#include <cxxabi.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "gcom_config.h"
#include "logger/logger.h"
#include "gcom_config_tmpl.h"
#include "gcom_iothread.h"
#include "gcom_modbus_decode.h"
#include "gcom_utils.h"


using namespace std::string_view_literals;

///////////////////////////////////////
////////// UTILITY METHODS ////////////
///////////////////////////////////////

void printConstAny(const std::any &value, int indent);

/**
 * @brief convert a set of raw 16-bit registers to its hexadecimal representation as a string
 * 
 * @param raw16 a pointer to an array of uint16_t registers to covert to its hexadecimal representation
 * @param size the size of the raw16 array
*/
std::string hex_to_str(u16 *raw16, int size)
{
    std::stringstream stream;
    stream << "0x";
    for (int i = 0; i < size; i++)
    {
        stream << std::hex << raw16[i];
    }
    return stream.str();
}

/**
 * @brief convert a set of raw 16-bit registers to its hexadecimal representation as a string
 * 
 * @param raw16 a pointer to an array of size 4 of uint16_t registers to covert to its hexadecimal representation
*/
std::string hex_to_str(u16 *raw16)
{
    std::stringstream stream;
    stream << "0x";
    for (int i = 0; i < 4; i++)
    {
        stream << std::hex << raw16[i];
    }
    return stream.str();
}

/**
 * @brief convert a set of raw 8-bit binary registers to its hexadecimal representation as a string
 * 
 * @param raw8 a pointer to an array of size 4 of uint8_t registers to covert to its hexadecimal representation
*/
std::string hex_to_str(u8 *raw8)
{
    std::stringstream stream;
    stream << "0x" << std::hex << static_cast<u16>(raw8[0]);
    return stream.str();
}

/**
 * @brief convert a uint64_t to its hexadecimal representation as a string
 * 
 * @param raw64 a uint64_t value to covert to its hexadecimal representation
*/
std::string hex_to_str(u64 raw64)
{
    std::stringstream stream;
    stream << "0x" << std::hex << raw64;
    return stream.str();
}

/**
 * @brief convert an (unspecified) integer to its hexadecimal representation as a string
 * 
 * @param raw_int an int value to covert to its hexadecimal representation
*/
std::string hex_to_str(int raw_int)
{
    std::stringstream stream;
    stream << "0x" << std::hex << raw_int;
    return stream.str();
}

/**
 * @brief Convert a simdjson::dom::object to a map<string, any>.
 *
 * @param obj simdjson::dom::object JSON object created after parsing using simdjson::dom::parser
 */
std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj)
{
    std::map<std::string, std::any> map;
    for (auto [key, value] : obj)
    {
        map[std::string(key)] = jsonToAny(value);
    }
    return map;
}

/**
 * @brief Convert a simdjson::dom::element to an std::any
 *
 * @param elem simdjson::dom::element JSON element created after parsing using simdjson::dom::parser
 */
std::any jsonToAny(simdjson::dom::element elem)
{
    switch (elem.type())
    {
    case simdjson::dom::element_type::INT64:
    {
        int64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= INT32_MAX && val >= INT32_MIN)
            {
                return static_cast<int32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::BOOL:
    {
        bool val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::UINT64:
    {
        uint64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= UINT32_MAX)
            {
                return static_cast<uint32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::DOUBLE:
    {
        double val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::STRING:
    {
        std::string_view sv;
        if (elem.get(sv) == simdjson::SUCCESS)
        {
            return std::string(sv);
        }
        break;
    }
    case simdjson::dom::element_type::ARRAY:
    {
        simdjson::dom::array arr = elem;
        std::vector<std::any> vec;
        for (simdjson::dom::element child : arr)
        {
            vec.push_back(jsonToAny(child));
        }
        return vec;
    }
    case simdjson::dom::element_type::OBJECT:
        return jsonToMap(elem);
    default:
        return std::any(); // Empty std::any
    }
    // If we reach here, something went wrong.
    FPS_ERROR_LOG("Error processing JSON element.");
    return std::any(); // Return empty std::any
}

///////////////////////////////////////
//////// CONFIG PARSING METHODS////////
///////////////////////////////////////

/**
 * @brief Load an entire config file into the config structure (myCfg) for this program instance.
 *
 * Open and load a file from [filename] into jsonMapOfConfig. Then, parse all values into the organized config
 * structure that holds all settings and global information. Loads in subs and pubs from the parsed registers.
 * Calls gcom_parse_file, extract_connection, and extract_components.
 * 
 * TODO 1/ assumes that the component name is "components" correct
 * TODO 2/ get client_name done, from connection.name
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param filename const char * of the file to be opened and parsed
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool gcom_load_cfg_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, struct cfg &myCfg, bool debug)
{
    bool ok = true;
    ok = gcom_parse_file(jsonMapOfConfig, filename, myCfg.use_dbi, false);
    if (!ok)
    {
        FPS_ERROR_LOG("Unable to parse config file [%s]. Quitting.", filename);
        return false;
    }
    // extract connection information into myCfg
    ok = extract_connection(jsonMapOfConfig, "connection", myCfg, false);
    if (!ok)
    {
        FPS_ERROR_LOG("Unable to extract connection information from [%s]. Quitting.", filename);
        return false;
    }
    // pull out the components into myCfg
    ok = extract_components(jsonMapOfConfig, "components", myCfg, false);
    if (!ok)
    {
        FPS_ERROR_LOG("Unable to extract components from [%s]. Quitting.", filename);
        return false;
    }
#ifdef FPS_DEBUG_MODE
    if (debug)
    {
        printf(" >>>>>> components found \n");
        for (auto &component : myCfg.component_io_point_map)
        {
            printf(" >>>>>>>>>>>>> <%s> component >> <%s> \n", __func__, component.first.c_str());
        }
    }
#endif
    FPS_INFO_LOG("Connection IP: [%s]. Port: [%d]", myCfg.connection.ip_address, myCfg.connection.port);
    // now load up the components we have found
    for (auto &component : myCfg.components)
    {
#ifdef FPS_DEBUG_MODE
       if (debug)
        {
            std::cout
                << __func__ 
                << "  component id: " << component->id
                << " device_id: " << component->device_id
                << " freq " << component->frequency
                << " offset_time :" << component->offset_time
                << std::endl;
        }
#endif

        // add a subscription to this component
        myCfg.addSub("components", component->id);

        // this is the key to setting up a pub.
        // the io_work framework is here
        // add a pub system for this component
        myCfg.addPub("components", component->id, component, &myCfg);

        // we'll do HB and Wd at the component level for now
        // GCOM allows a more liberal definition of these elements
        if(1 || myCfg.use_new_wdog)
        {
            myCfg.addWatchdog("components", component->id, component, &myCfg);
            myCfg.addHeartbeat("components", component->id, component, &myCfg);
        }
        else
        {
            myCfg.addOldWatchdog("components", component->id, component, &myCfg);
        }

        // now check register_group sizes and correct if needed
        for (auto &register_group : component->register_groups)
        {
            checkNonZeroIOPointSize(register_group, register_group->io_point_map, false);

            // TODO add next pointers
            sortIOPointVectorByOffset(register_group->io_point_map);

            int first_offset = getFirstOffset(register_group->io_point_map);
            if (first_offset < 0)
            {
                FPS_ERROR_LOG("Register group [%s] for component [%s] is empty.\n", register_group->register_type_str.c_str(), component->id.c_str());
                return false;
            }

            int total_size = getTotalNumRegisters(register_group->io_point_map);
            if (total_size < 0)
            {
                return false;
            }

            // maybe report config file error here
            register_group->starting_offset = first_offset;
            register_group->number_of_registers = total_size;
#ifdef FPS_DEBUG_MODE
            if (debug)
            {
                std::cout << "     register type: " << register_group->register_type_str
                          << " offset: " << register_group->starting_offset
                          << " num_regs: " << register_group->number_of_registers
                          << " first_offset: " << first_offset
                          << " total_size: " << total_size
                          << std::endl;
                for (auto &mymap : register_group->io_point_map)
                {
                    std::cout << "         map : " << mymap->id
                              << " offset : " << mymap->offset
                              << " size : " << mymap->size
                              << std::endl;
                }
            }
#endif
        }
    }

    return true;
}

/**
 * @brief Open a file, parse it using simdjson, then convert it to a map<string, any>
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param filename const char * of the file to be opened and parsed
 * @param use_dbi bool value representing whether or not to get the file from fims rather than the local file system
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool gcom_parse_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, bool use_dbi, bool debug)
{
    simdjson::dom::parser parser;
    std::string fname(filename);
    simdjson::padded_string padded_content;
    if(use_dbi){
        if (fname.front() != '/')
        {
            FPS_ERROR_LOG("For client with init uri \"%s\": the uri does not begin with `/`", filename);
            return false;
        }
        fims fims_gateway;
        const auto conn_id_str = "modbus_client_uri_init@" + fname;
        if (!fims_gateway.Connect(conn_id_str.data()))
        {
            FPS_ERROR_LOG("For client with init uri \"%s\": could not connect to fims_server", filename);
            return false;
        }
        const auto sub_string = "/modbus_client_uri_init"+fname;
        if (!fims_gateway.Subscribe(std::vector<std::string>{sub_string}))
        {
            FPS_ERROR_LOG("For client with init uri \"%s\": failed to subscribe for uri init", filename);
            return false;
        }
        if (!fims_gateway.Send(fims::str_view{"get", sizeof("get") - 1}, fims::str_view{fname.data(), fname.size()}, fims::str_view{sub_string.data(), sub_string.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}))
        {
            FPS_ERROR_LOG("For client with inti uri \"%s\": failed to send a fims get message", filename);
            return false;
        }
        auto config_msg = fims_gateway.Receive_Timeout(5000000); // give them 5 seconds to respond before erroring
        if (!config_msg)
        {
            FPS_ERROR_LOG("For client with init uri \"%s\": failed to receive a message in 5 seconds", filename);
            return false;
        }
        if (!config_msg->body)
        {
            FPS_ERROR_LOG("For client with init uri \"%s\": message was received, but body doesn't exist", filename);
            return false;
        }
        padded_content = simdjson::padded_string{std::string{config_msg->body}};
        fims::free_message(config_msg);
        FPS_INFO_LOG("File loaded from DBI");
    } else{
    
    // Open file
    std::ifstream file; //(fname, std::ios::in | std::ios::binary);
    file = std::ifstream(fname, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        fname += ".json";
        file = std::ifstream(fname, std::ios::in | std::ios::binary);
    }
    if (!file.is_open())
    {
        FPS_ERROR_LOG("Failed to open config file [%s].", filename);
        return false;
    }
    // Read content into padded_string
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    padded_content = simdjson::padded_string(content);
    }
    if (debug)
    {
        std::cout << " \"parse_file\": {" << std::endl;
        std::cout << "                 \"File name\": \"" << filename << "\"," << std::endl;
        std::cout << "                 \"File length\": " << padded_content.size() << std::endl;
        std::cout << "} " << std::endl;
    }
    // Parse
    auto result = parser.parse(padded_content);
    if (result.error())
    {
        FPS_ERROR_LOG("Error parsing json config: %s", simdjson::error_message(result.error()));
        return false;
    }
    // Convert JSON to map
    jsonMapOfConfig = jsonToMap(result.value());
    return true;
}

/**
 * @brief Parse the connection information from a config map into myCfg.
 *
 * @param jsonMapOfConfig std::map<std::string, std::any> that represents a parsed config file
 * @param query the field to look for
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool extract_connection(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug = false)
{
    bool ok = true;
    std::string error_message;
    // connection name

    ok &= getItemFromMap(jsonMapOfConfig, "connection.name", myCfg.connection.name, std::string("modbus_client"), true, true, false);
    

    replace_space_with_underscores(myCfg.connection.name);

    error_message = check_str_for_error(myCfg.connection.name);
    if (error_message.length() > 0)
    {
        FPS_INFO_LOG("Connection field \"name\" %s. Using default of \"modbus_client\".", error_message);
        error_message = "";
    }
    myCfg.client_name = myCfg.connection.name;


    // port
    ok &= getItemFromMap(jsonMapOfConfig, "connection.port", myCfg.connection.port, static_cast<int>(502), true, true, false);
    if (ok && (myCfg.connection.port < 0 || myCfg.connection.port > std::numeric_limits<u16>::max()))
    {
        FPS_INFO_LOG("Port must be between 0 and %d. Configured value is [%d]. Using default of 502.", std::numeric_limits<u16>::max(), myCfg.connection.port);
        myCfg.connection.port = 502;
    }
    // ip_address
    ok &= getItemFromMap(jsonMapOfConfig, "connection.ip_address", myCfg.connection.ip_address, std::string("172.3.0.2"), true, true, false);
    if (ok && !myCfg.connection.ip_address.empty())
    {
        char new_ip[HOST_NAME_MAX + 1];
        new_ip[HOST_NAME_MAX] = '\0';
        auto ret = hostname_to_ip(myCfg.connection.ip_address, new_ip, HOST_NAME_MAX);
        if (ret == 0)
        {
            myCfg.connection.ip_address = new_ip;
        }
        else
        {
            FPS_ERROR_LOG("ip_address \"%s\" isn't valid or can't be found from the local service file", myCfg.connection.ip_address);
            ok = false;
        }
    }
    ok &= getItemFromMap(jsonMapOfConfig, "connection.protocol", myCfg.connection.protocol, std::string("Modbus TCP"), true, true, false);
    ok &= getItemFromMap(jsonMapOfConfig, "connection.name", myCfg.connection.device_name, myCfg.connection.name, true, true, false);

    // debug
    ok &= getItemFromMap(jsonMapOfConfig, "connection.debug", myCfg.connection.debug, false, true, true, false);
    // connection_timeout
    ok &= getItemFromMap(jsonMapOfConfig, "connection.connection_timeout", myCfg.connection.connection_timeout, 2.0, true, true, false);
    if (ok && (myCfg.connection.connection_timeout < 2 || myCfg.connection.connection_timeout > 10))
    {
        FPS_INFO_LOG("Connection timeout must be between 2 and 10 seconds. Configured value is [%f] seconds. Using default of 2s.", myCfg.connection.connection_timeout);
        myCfg.connection.connection_timeout = 2.0;
    }
    ok &= getItemFromMap(jsonMapOfConfig, "connection.transfer_timeout", myCfg.connection.transfer_timeout, 0.2, true, true, false);
    if (ok && (myCfg.connection.transfer_timeout < 0.05 || myCfg.connection.transfer_timeout > 1.00))
    {
        FPS_INFO_LOG("Transfer timeout must be between 0.05 and 1.0 seconds. Configured value is [%f] seconds. Using default of 0.5s.", myCfg.connection.transfer_timeout);
        myCfg.connection.transfer_timeout = 0.5;
    }
    // // transfer_timeout
    // ok &= getItemFromMap(jsonMapOfConfig, "connection.transfer_timeout", myCfg.connection.transfer_timeout, 500, true, true, false);
    // if (ok && (myCfg.connection.transfer_timeout < 10 || myCfg.connection.transfer_timeout > 2000))
    // {
    //     FPS_INFO_LOG("Transfer timeout must be between 10 and 2000 milli seconds. Configured value is [%d] milliseconds. Using default of 500.", myCfg.connection.transfer_timeout);
    //     myCfg.connection.transfer_timeout = 500;
    // }
    // double dval = 0.5;
    // ok &= getItemFromMap(jsonMapOfConfig, "connection.sync_percent", myCfg.connection.syncPct, dval, true, true, false);
    // if (ok && (myCfg.connection.syncPct > 0.0) && (myCfg.connection.syncPct < 0.1 || myCfg.connection.syncPct > 0.9))
    // {
    //     FPS_INFO_LOG("Sync Percent must be between 0.1 and 0.9. Configured value is [%f] milliseconds. Using default of 0.5.", myCfg.connection.syncPct);
    //     myCfg.connection.syncPct = 0.5;
    // }

    // max_num_connections
    ok &= getItemFromMap(jsonMapOfConfig, "connection.max_num_connections", myCfg.connection.max_num_connections, 1, true, true, false);
    if (ok && (myCfg.connection.max_num_connections <= 0 || myCfg.connection.max_num_connections > 25))
    {
        FPS_INFO_LOG("max_num_connections must be greater than 0. Configured value is [%d]. Using default of 1.", myCfg.connection.max_num_connections);
        myCfg.connection.max_num_connections = 1;
    }
    // data_buffer_size
    ok &= getItemFromMap(jsonMapOfConfig, "connection.data_buffer_size", myCfg.connection.data_buffer_size, 100000, true, true, false);
    if (ok && (myCfg.connection.data_buffer_size <= 10000 || myCfg.connection.data_buffer_size > 200000))
    {
        FPS_INFO_LOG("data_buffer_size must be between 10000 and 200000. Configured value is [%d]. Using default of 100000.", myCfg.connection.data_buffer_size);
        myCfg.connection.data_buffer_size = 100000;
    }

    // auto_disable
    ok &= getItemFromMap(jsonMapOfConfig, "connection.auto_disable", myCfg.auto_disable, true, true, true, false);

    // allow_multi_sets
    ok &= getItemFromMap(jsonMapOfConfig, "connection.allow_multi_sets", myCfg.allow_multi_sets, false, true, true, false);

    // force_multi_sets
    ok &= getItemFromMap(jsonMapOfConfig, "connection.force_multi_sets", myCfg.force_multi_sets, false, true, true, false);

    // max_register_group_size
    ok &= getItemFromMap(jsonMapOfConfig, "connection.max_register_group_size", myCfg.max_register_group_size, 125, true, true, false);
    if (ok && myCfg.max_register_group_size <= 0)
    {
        FPS_INFO_LOG("max register size must be greater than 0. Configured value is [%d]. Using default of 125.", myCfg.max_register_group_size);
        myCfg.max_register_group_size = 125;
    }

    // max_bit_size
    ok &= getItemFromMap(jsonMapOfConfig, "connection.max_bit_size", myCfg.max_bit_size, 125, true, true, false);
    if (ok && myCfg.max_bit_size <= 0)
    {
        FPS_INFO_LOG("max bit size must be greater than 0. Configured value is [%d]. Using default of 125.", myCfg.max_bit_size);
        myCfg.max_bit_size = 125;
    }
    // service
    std::string service;
    ok &= getItemFromMap(jsonMapOfConfig, "connection.service", service, std::string(""), false, false, false);
    if (ok && !service.empty())
    {
        ok = service_to_port(service, myCfg.connection.port);
    }
    // serial_device
    bool is_RTU = false;
    ok &= getItemFromMap(jsonMapOfConfig, "connection.serial_device", myCfg.connection.device_name, std::string(""), false, false, false);
    if (ok && !myCfg.connection.device_name.empty())
    {
        error_message = check_str_for_error(myCfg.connection.name);
        if (error_message.length() > 0)
        {
            FPS_ERROR_LOG("Connection field \"serial_device\" %s.", error_message);
            error_message = "";
            ok = false;
        }
        else
        {
            is_RTU = true;
        }
    }
    myCfg.connection.is_RTU = is_RTU;
    // baud_rate
    ok &= getItemFromMap(jsonMapOfConfig, "connection.baud_rate", myCfg.connection.baud_rate, 115200, true, is_RTU, false);
    if (ok && myCfg.connection.baud_rate < 0)
    {
        FPS_INFO_LOG("Baud rate cannot be less than 0. Current value is [%d]. Setting to 115200.", myCfg.connection.baud_rate);
        myCfg.connection.baud_rate = 115200;
    }
    // parity
    std::string parity("none");
    ok &= getItemFromMap(jsonMapOfConfig, "connection.parity", parity, parity, true, is_RTU, false);
    if (ok && !parity.empty())
    {
        if (parity == "none")
        {
            myCfg.connection.parity = 'N';
        }
        else if (parity == "even")
        {
            myCfg.connection.parity = 'E';
        }
        else if (parity == "odd")
        {
            myCfg.connection.parity = 'O';
        }
        else
        {
            FPS_ERROR_LOG("parity (currently: \"%s\") must be one of \"none\", \"even\", or \"odd\". Setting to default of \"none\".", parity);
            myCfg.connection.parity = 'N';
        }
    }
    // data_bits
    ok &= getItemFromMap(jsonMapOfConfig, "connection.data_bits", myCfg.connection.data_bits, 8, true, is_RTU, false);
    if (ok && (myCfg.connection.data_bits < 5 || myCfg.connection.data_bits > 8))
    {
        FPS_INFO_LOG("data_bits (currently: %d) must be between 5 and 8. Using default of 8.", myCfg.connection.data_bits);
        myCfg.connection.data_bits = 8;
    }
    // stop_bits
    ok &= getItemFromMap(jsonMapOfConfig, "connection.stop_bits", myCfg.connection.stop_bits, 1, true, is_RTU, false);
    if (ok && (!(myCfg.connection.stop_bits == 1 || myCfg.connection.stop_bits == 2)))
    {
        FPS_INFO_LOG("stop_bits (currently: %d) must be either 1 or 2. Using default of 1.", myCfg.connection.stop_bits);
        myCfg.connection.stop_bits = 1;
    }

    ok &= getItemFromMap(jsonMapOfConfig, "connection.stats_pub_uri", myCfg.connection.stats_uri, std::string("/stats/modbus_client"), true, true, false);
    if (myCfg.connection.stats_uri.length() > 0)
    {
        error_message = check_str_for_error(myCfg.connection.stats_uri, R"({}\ "%)");
        if (error_message.length() > 0)
        {
            FPS_INFO_LOG("Connection field \"stats_pub_uri\" %s. Omitting stats pubs.", error_message);
            myCfg.connection.stats_uri = "";
            error_message = "";
        }
    }

    ok &= getItemFromMap(jsonMapOfConfig, "connection.stats_pub_frequency", myCfg.connection.stats_frequency_ms, 1000, true, true, false);
    if (ok && myCfg.connection.stats_uri.length() > 0 && myCfg.connection.stats_frequency_ms <= 0)
    {
        FPS_INFO_LOG("Connection field \"stats_pub_frequency\" must be greater than 0. Setting to default of 1000 ms.");
        myCfg.connection.stats_frequency_ms = 1000;
    }

    // inherited stuff:
    // device_id
    ok &= getItemFromMap(jsonMapOfConfig, "connection.device_id", myCfg.connection.device_id, 1, true, true, false);
    // off_by_one
    ok &= getItemFromMap(jsonMapOfConfig, "connection.off_by_one", myCfg.inherited_fields.off_by_one, false, true, true, false);
    if (myCfg.inherited_fields.off_by_one)
    {
        FPS_INFO_LOG("Connection field \"off_by_one \" set ");
    }
    // byte_swap
    ok &= getItemFromMap(jsonMapOfConfig, "connection.byte_swap", myCfg.inherited_fields.byte_swap, false, true, true, false);

    // multi_write_op_code
    ok &= getItemFromMap(jsonMapOfConfig, "connection.multi_write_op_code", myCfg.inherited_fields.multi_write_op_code, false, true, true, false);

    // frequency
    ok &= getItemFromMap(jsonMapOfConfig, "connection.frequency", myCfg.inherited_fields.frequency, 100, true, true, false);
    // debounce
    ok &= getItemFromMap(jsonMapOfConfig, "connection.debounce", myCfg.inherited_fields.debounce, 0, true, true, false);

    // format
    std::string format_str;
    ok &= getItemFromMap(jsonMapOfConfig, "connection.format", format_str, std::string(""), true, true, false);

    if (format_str.compare("naked") == 0)
    {
        myCfg.format = Fims_Format::Naked;
    }
    else if (format_str.compare("clothed") == 0)
    {
        myCfg.format = Fims_Format::Clothed;
    }
    else if (format_str.compare("full") == 0)
    {
        myCfg.format = Fims_Format::Full;
    }
    else
    {
        myCfg.format = Fims_Format::Naked;
    }

    if (debug)
    {
        std::cout <<" >>>>>>>>>>>>> <"<<__func__ <<">  device_id "<<myCfg.connection.device_id<< std::endl;
    }
    return ok;
}

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
bool extract_components(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug = false)
{
    bool ok = true;
    std::optional<std::vector<std::any>> compArray = getMapValue<std::vector<std::any>>(jsonMapOfConfig, query);
    if (compArray.has_value())
    {
        if (debug)
            std::cout << " Found components " << std::endl;
    }
    else
    {
        FPS_ERROR_LOG("Could not find \"components\" in config file.");
        return false;
    }
    std::vector<std::any> rawCompArray = compArray.value();
    for (const std::any &rawComp : rawCompArray)
    {
        if (debug)
            std::cout << " Processing component" << std::endl;
        if (rawComp.type() == typeid(std::map<std::string, std::any>))
        {
            std::map<std::string, std::any> jsonComponentMap = std::any_cast<std::map<std::string, std::any>>(rawComp);
            std::shared_ptr<cfg::component_struct> component = std::make_shared<cfg::component_struct>();
            std::string componentId;
            getItemFromMap(jsonComponentMap, "component_id", componentId, std::string("components"), true, true, false);
            component->component_id = componentId;
            component->myCfg = &myCfg;
            ok &= getItemFromMap(jsonComponentMap, "id", component->id, std::string("noId"), true, true, false);
            ok &= getItemFromMap(jsonComponentMap, "device_id", component->device_id, myCfg.connection.device_id, true, true, false);
            ok &= getItemFromMap(jsonComponentMap, "frequency", component->frequency, myCfg.inherited_fields.frequency, true, true, false);
            ok &= getItemFromMap(jsonComponentMap, "offset_time", component->offset_time, 0, true, true, false);


            ok &= getItemFromMap(jsonComponentMap, "byte_swap", component->is_byte_swap,  myCfg.inherited_fields.byte_swap, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "off_by_one", component->off_by_one,   myCfg.inherited_fields.off_by_one, true, true, debug);
            if (component->off_by_one)
            {
                FPS_INFO_LOG("component field \"off_by_one \" set ");
            }

            ok &= getItemFromMap(jsonComponentMap, "pub_sync",                       myCfg.pub_sync, true, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "heartbeat_enabled",              component->heartbeat_enabled, false, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_enabled",               component->watchdog_enabled, false, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_read_uri",   component->component_heartbeat_read_uri, std::string(""), true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_write_uri",  component->component_heartbeat_write_uri, std::string(""), true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_max_value",  component->component_heartbeat_max_value, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "modbus_heartbeat_timeout_ms",    component->modbus_heartbeat_timeout_ms, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_timeout_ms", component->component_heartbeat_timeout_ms, 0, true, true, debug);

            ok &= getItemFromMap(jsonComponentMap, "watchdog_uri", component->watchdog_uri, std::string(""), true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_alarm_timeout_ms", component->watchdog_alarm_timeout, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_fault_timeout_ms", component->watchdog_fault_timeout, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_recovery_timeout_ms", component->watchdog_recovery_timeout, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_recovery_time_ms", component->watchdog_time_to_recover, 0, true, true, debug);
            ok &= getItemFromMap(jsonComponentMap, "watchdog_frequency_ms", component->watchdog_frequency, 1000, true, true, debug);

            // I think word_swap is actually the same as byte_swap?
            int word_order = 0;
            ok &= getItemFromMap(jsonComponentMap, "word_swap", component->is_byte_swap, false, true, true, debug);

            ok &= getItemFromMap(jsonComponentMap, "word_order", component->word_order, word_order, true, true, debug);

            std::string format_str;
            ok &= getItemFromMap(jsonComponentMap, "format", format_str, std::string(""), true, true, false);

            if (format_str.compare("naked") == 0)
            {
                component->format = Fims_Format::Naked;
            }
            else if (format_str.compare("clothed") == 0)
            {
                component->format = Fims_Format::Clothed;
            }
            else if (format_str.compare("full") == 0)
            {
                component->format = Fims_Format::Full;
            }
            else
            {
                component->format = myCfg.format;
            }
            int ival = (int)(myCfg.syncPct * 100.0);
            ok &= getItemFromMap(jsonComponentMap, "sync_percent", ival, ival, true, true, false);
            if(0)FPS_INFO_LOG("component [%s] freq [%d] Sync Percent ival [%d]."
                    , component->id.c_str(), component->frequency, ival);
            //if (ok)
            {
                if ( (ival > 0) && (ival < 10 || ival > 90))
                {
                    FPS_INFO_LOG("Sync Percent must be between 10 and 90. Configured value is [%d] percent. Using default of 50.", ival);
                    component->syncPct = 0.5;
                }
                else
                {
                    component->syncPct = (double)(ival/100.0);
                }
            }
            if(0)FPS_INFO_LOG("component [%s] freq [%d] Sync Percent ival [%d] value is [%f]."
                    , component->id.c_str(), component->frequency, ival, component->syncPct);
            if (component->syncPct > 0.0)
                component->use_sync = true;
            else
                component->use_sync = false;
            if(0)FPS_INFO_LOG("component [%s] freq [%d] Sync Percent ival [%d] value is [%f] use sync %s."
                    , component->id.c_str(), component->frequency, ival
                    , component->syncPct
                    , (component->use_sync? "true":"false")
                    );


            std::map<std::string, std::any>::iterator reg_group_it = jsonComponentMap.find("registers");
            if (reg_group_it != jsonComponentMap.end())
            {
                if (debug)
                    std::cout << " Processing  registers" << std::endl;
                ok &= extract_register_groups(component->register_groups, reg_group_it->second, component, myCfg, debug);
            }
            else
            {
                ok = false;
            }
            if (ok)
                myCfg.components.push_back(component);
        }
    }
    if (debug)
    {
        std::cout <<" >>>>>>>>>>>>> <" << __func__ << " > done " << std::endl;;
        for (auto &myComp : myCfg.component_io_point_map)
        {
            std::cout <<" >>>>>>>>>>>>> <" << __func__ << "> component <"<< myComp.first << ">" << std::endl;
        }
    }
    return ok;
}

/**
 * @brief Parse the register information in a component into myCfg.
 * 
 * @param register_groups an empty register_group_struct, typically belonging to component
 * @param raw_register_groups_json_array an std::any struct representing all register_groups in a component (as a json list)
 * @param component a pre-populated component_struct, typically belonging to myCfg
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
 */
bool extract_register_groups(std::vector<std::shared_ptr<cfg::register_group_struct>> &register_groups, const std::any &raw_register_groups_json_array
              , std::shared_ptr<struct cfg::component_struct> component, struct cfg &myCfg, bool debug = false)
{
    bool ok = true;
    // initialize component
    if (raw_register_groups_json_array.type() == typeid(std::vector<std::any>))
    {
        std::vector<std::any> raw_register_groups = std::any_cast<std::vector<std::any>>(raw_register_groups_json_array);
        for (const std::any &raw_register_group : raw_register_groups)
        {
            if (raw_register_group.type() == typeid(std::map<std::string, std::any>))
            {
                std::map<std::string, std::any> jsonRegisterGroupMap = std::any_cast<std::map<std::string, std::any>>(raw_register_group);
                std::shared_ptr<cfg::register_group_struct> register_group = std::make_shared<cfg::register_group_struct>();
                register_group->multi_write_op_code = false; // component->multi_write_op_code;
                register_group->component = component.get();
                register_group->id = component->id;
                register_group->component_id = component->component_id;
                register_group->off_by_one = component->off_by_one;
                if (register_group->off_by_one)
                {
                    FPS_INFO_LOG("register_group \"off_by_one \" set ");
                }


                if (debug)
                    printf(" >>>>>>>>>>>>> <%s> component %p  initial register_group %p \n", __func__, (void *)component.get(), (void *)register_group.get());
                ok &= getItemFromMap(jsonRegisterGroupMap, "type", register_group->register_type_str, std::string("type"), true, true, debug);
                // now we get modbus specific
                register_group->register_type = myCfg.typeFromStr(register_group->register_type_str);
                ok &= getItemFromMap(jsonRegisterGroupMap, "starting_offset", register_group->starting_offset, 0, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "number_of_registers", register_group->number_of_registers, 0, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "device_id", register_group->device_id, component->device_id, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "byte_swap", register_group->is_byte_swap, component->is_byte_swap, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "word_swap", register_group->is_byte_swap, component->is_byte_swap, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "word_order", register_group->word_order, component->word_order, true, true, debug);
                ok &= getItemFromMap(jsonRegisterGroupMap, "multi_write_op_code", register_group->multi_write_op_code, false, true, true, debug);
                std::string format_str;
                ok &= getItemFromMap(jsonRegisterGroupMap, "format", format_str, std::string(""), true, true, false);

                if (format_str.compare("naked") == 0)
                {
                    register_group->format = Fims_Format::Naked;
                }
                else if (format_str.compare("clothed") == 0)
                {
                    register_group->format = Fims_Format::Clothed;
                }
                else if (format_str.compare("full") == 0)
                {
                    register_group->format = Fims_Format::Full;
                }
                else
                {
                    register_group->format = component->format;
                }

                // Extract register details (like type, starting_offset, etc.) here...
                // Call extract_io_point_map for each register to extract map details...
                if (ok)
                {
                    // register_group.io_point_map = extract_io_point_map(&register_group, jsonRegisterGroupMap["map"], myCfg);
                    //  Add register details + extracted io_point_map to the register_groups vector...
                    register_groups.push_back(register_group);
                    // register_groups.emplace_back(register_group);
                    std::shared_ptr<cfg::register_group_struct> &register_group = register_groups.back();
                    // register_group->component = component;
                    extract_io_point_map(register_group, nullptr, jsonRegisterGroupMap["map"], myCfg, debug);
                    if (debug)
                        printf(" >>>>>>>>>>>>> <%s> component %p  register_group %p  device_id %d \n", __func__, (void *)component.get(), (void *)&register_group, register_group->device_id);
                }
            }
        }
    }
    return ok;
}

/**
 * @brief Parse the information in an io_point (belonging to a single register group of a single
 * component) into myCfg.
 * 
 * @param json_io_point an std::any struct representing one io_point from  a register_group
 * @param register_group the register group struct that the point belongs to
 * @param packed_io_point the parent io_point that contains packed registers within it. NULL if this is the parent io_point or if the io_point is not packed.
 * @param new_point  bool value representing whether or not this is a new point if set to false this function can be used to update an existing point.
 * @param myCfg config structure to load file data into
 * @param debug bool value representing whether or not to print messages to help debug code
*/
bool extract_io_point( std::map<std::string, std::any>&json_io_point, std::shared_ptr<struct cfg::register_group_struct> &register_group, 
                    std::shared_ptr<cfg::io_point_struct>&io_point, std::shared_ptr<cfg::io_point_struct>&packed_io_point, bool new_point, struct cfg &myCfg, bool debug)
{
    
    std::shared_ptr<cfg::io_point_struct> parent_io_point = io_point;
    if (packed_io_point)
        parent_io_point = packed_io_point;
    getItemFromMap(json_io_point, "id", io_point->id, std::string("Some_id"), new_point, new_point, debug);
    getItemFromMap(json_io_point, "name", io_point->name, std::string("Some_name"), new_point, new_point, debug);
    getItemFromMap(json_io_point, "offset", io_point->offset, parent_io_point->offset, new_point, new_point, debug);
    getItemFromMap(json_io_point, "size", io_point->size, parent_io_point->size, new_point, new_point, debug);
    getItemFromMap(json_io_point, "multi_write_op_code", io_point->multi_write_op_code, parent_io_point->multi_write_op_code, new_point, new_point, debug);
    //getItemFromMap(json_io_point, "off_by_one",          io_point->off_by_one, register_group->off_by_one, new_point, new_point, debug);
    io_point->off_by_one = register_group->off_by_one;


    // set up default
    io_point->number_of_bits = io_point->size * 16;
    getItemFromMap(json_io_point, "shift", io_point->shift, parent_io_point->shift, new_point, new_point, debug);
    getItemFromMap(json_io_point, "starting_bit_pos", io_point->starting_bit_pos, parent_io_point->starting_bit_pos, new_point, new_point, debug);
    getItemFromMap(json_io_point, "number_of_bits", io_point->number_of_bits, parent_io_point->number_of_bits, new_point, new_point, debug);
    io_point->bit_mask = (io_point->number_of_bits * io_point->number_of_bits) - 1;
    getItemFromMap(json_io_point, "scale", io_point->scale, parent_io_point->scale, new_point, new_point, debug);
    getItemFromMap(json_io_point, "normal_set", io_point->normal_set, parent_io_point->normal_set, new_point, new_point, debug);
    getItemFromMap(json_io_point, "signed", io_point->is_signed, parent_io_point->is_signed, new_point, new_point, debug);
    std::string format_str;
    getItemFromMap(json_io_point, "format", format_str, std::string(""), new_point, new_point, false);

    if (format_str.compare("naked") == 0)
    {
        io_point->format = Fims_Format::Naked;
    }
    else if (format_str.compare("clothed") == 0)
    {
        io_point->format = Fims_Format::Clothed;
    }
    else if (format_str.compare("full") == 0)
    {
        io_point->format = Fims_Format::Full;
    }
    else
    {
        if (packed_io_point)
        {
            io_point->format = parent_io_point->format;
        }
        else
        {
            io_point->format = register_group->format;
        }
    }
    io_point->invert_mask = 0;
    io_point->care_mask = std::numeric_limits<u64>::max();
    std::string invert_mask_str("0x0");
    std::string default_invert_mask_str("0x0");
    io_point->invert_mask = 0;
    io_point->invert_mask = parent_io_point->invert_mask;
    getItemFromMap(json_io_point, "invert_mask", invert_mask_str, default_invert_mask_str, new_point, new_point, debug);
    if (invert_mask_str != "0x0")
    {
        uint64_t invmask = 0;
        const char *istr = invert_mask_str.c_str();
        if (invert_mask_str.length() > 1 && (istr[0] == '0') && (istr[1] == 'x'))
        {
            istr += 2;
            invmask = strtoull(istr, NULL, 16);
        }
        else if ((invert_mask_str[0] == '0') && (invert_mask_str[1] == 'b'))
        {
            istr += 2;
            invmask = strtoull(istr, NULL, 2);
        }
        io_point->invert_mask = invmask;
    }
    // Extract io_point details (like id, offset, name, etc.) here...
    // Add to the io_point_map vector...
    io_point->is_float = false;
    io_point->is_byte_swap = false;
    io_point->is_byte_swap = false;
    getItemFromMap(json_io_point, "float", io_point->is_float, parent_io_point->is_float, new_point, new_point, debug);
    getItemFromMap(json_io_point, "word_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, new_point, new_point, debug);
    getItemFromMap(json_io_point, "word_order", io_point->word_order, parent_io_point->word_order, new_point, new_point, debug);
    getItemFromMap(json_io_point, "byte_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, new_point, new_point, debug);

    // for the benefit of size 4 regs
    // TODO remove word_swap
    if (io_point->is_byte_swap)
    {
        io_point->byte_index[0] = 3;
        io_point->byte_index[1] = 2;
        io_point->byte_index[2] = 1;
        io_point->byte_index[3] = 0;
    }
    else
    {
        io_point->byte_index[0] = 0;
        io_point->byte_index[1] = 1;
        io_point->byte_index[2] = 2;
        io_point->byte_index[3] = 3;
    }
    // word_order is 1234 or 4321 or 1324 etc
    if (io_point->word_order > 0)
    {
        io_point->byte_index[0] = (io_point->word_order / 1000) - 1;
        io_point->byte_index[1] = ((io_point->word_order % 1000) / 100) - 1;
        io_point->byte_index[2] = ((io_point->word_order % 100) / 10) - 1;
        io_point->byte_index[3] = (io_point->word_order % 10) - 1;
    }

    io_point->is_enum = false;
    io_point->is_random_enum = false;
    io_point->is_individual_bits = false;
    io_point->is_bit_field = false;
    getItemFromMap(json_io_point, "enum", io_point->is_enum, parent_io_point->is_enum, new_point, new_point, debug);
    getItemFromMap(json_io_point, "random_enum", io_point->is_random_enum, parent_io_point->is_random_enum, new_point, new_point, debug);
    getItemFromMap(json_io_point, "individual_bits", io_point->is_individual_bits, parent_io_point->is_individual_bits, new_point, new_point, debug);
    getItemFromMap(json_io_point, "bit_field", io_point->is_bit_field, io_point->is_bit_field, new_point, new_point, debug);
    double dval = 0.0;
    io_point->debounce = dval;
    io_point->deadband = dval;
    io_point->use_bool = true;
    getItemFromMap(json_io_point, "debounce", io_point->debounce, parent_io_point->debounce, new_point, new_point, debug);
    getItemFromMap(json_io_point, "deadband", io_point->deadband, parent_io_point->deadband, new_point, new_point, debug);
    if (io_point->debounce > 0.0)
        io_point->use_debounce = true;
    if (io_point->deadband > 0.0)
        io_point->use_deadband = true;
    getItemFromMap(json_io_point, "use_bool", io_point->use_bool, parent_io_point->use_bool, new_point, new_point, debug);
    if ((io_point->is_enum) || (io_point->is_random_enum) || (io_point->is_individual_bits) || (io_point->is_bit_field)
        /* maybe add more here */)
    {
        extract_bitstrings(io_point, json_io_point["bit_strings"]);
    }
    io_point->forced_val = 0;
    io_point->raw_val = 0;
    io_point->device_id = register_group->device_id;
    io_point->packer = packed_io_point;
    if (!packed_io_point)
    {
        register_group->io_point_map.push_back(io_point);
        if (debug)
            printf(" mapping register id [%s]\n", io_point->id.c_str());
        auto &io_point_map = register_group->io_point_map.back();
        // auto regshr = mymap->register_group.lock();
        register_group->io_point_map_lookup[io_point_map->offset] = io_point_map;
    }
    else
    {
        if (debug)
            printf(" packing register id [%s]\n", io_point->id.c_str());
        packed_io_point->bit_ranges.push_back(io_point);
    }
    return true;
}

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
bool extract_io_point_map(std::shared_ptr<struct cfg::register_group_struct> register_group, std::shared_ptr<cfg::io_point_struct> packed_io_point, std::any &rawIOPointList, struct cfg &myCfg, bool debug = false)
{
    // did we get a vector of io_points
    if (rawIOPointList.type() == typeid(std::vector<std::any>))
    {
        std::vector<std::any> rawIOPoints = std::any_cast<std::vector<std::any>>(rawIOPointList);
        for (const std::any &rawIOPoint : rawIOPoints)
        {
            if (rawIOPoint.type() == typeid(std::map<std::string, std::any>))
            {
                std::map<std::string, std::any> json_io_point = std::any_cast<std::map<std::string, std::any>>(rawIOPoint);
                // struct cfg::io_point_struct map;
                std::shared_ptr<cfg::io_point_struct> io_point = std::make_shared<cfg::io_point_struct>();
                io_point->register_type = register_group->register_type;
                io_point->register_group = register_group.get();
                io_point->component = register_group->component;
                io_point->multi_write_op_code = register_group->multi_write_op_code;
                io_point->size = 1;
                double scale = 0.0;
                io_point->scale = scale;
                io_point->register_type_str = register_group->register_type_str;
                extract_io_point(json_io_point, register_group, io_point, packed_io_point, true, myCfg, debug);
                io_point->off_by_one = register_group->off_by_one;


                // bool extract_io_point(std::any&json_io_point, std::shared_ptr<cfg::io_point_struct>&io_point, std::shared_ptr<cfg::io_point_struct>&packed_io_point, struct cfg &myCfg)
                // {
                //     std::shared_ptr<cfg::io_point_struct> parent_io_point = io_point;
                //     if (packed_io_point)
                //         parent_io_point = packed_io_point;
                //     getItemFromMap(json_io_point, "id", io_point->id, std::string("Some_id"), true, true, debug);
                //     getItemFromMap(json_io_point, "name", io_point->name, std::string("Some_name"), true, true, debug);
                //     getItemFromMap(json_io_point, "offset", io_point->offset, parent_io_point->offset, true, true, debug);
                //     getItemFromMap(json_io_point, "size", io_point->size, parent_io_point->size, true, true, debug);
                //     getItemFromMap(json_io_point, "multi_write_op_code", io_point->multi_write_op_code, parent_io_point->multi_write_op_code, true, true, debug);
                //     // getItemFromMap(json_io_point, "off_by_one",          io_point->off_by_one, myCfg.inherited_fields.off_by_one, true, true, debug);

                //     // set up default
                //     io_point->number_of_bits = io_point->size * 16;
                //     getItemFromMap(json_io_point, "shift", io_point->shift, parent_io_point->shift, true, true, debug);
                //     getItemFromMap(json_io_point, "starting_bit_pos", io_point->starting_bit_pos, parent_io_point->starting_bit_pos, true, true, debug);
                //     getItemFromMap(json_io_point, "number_of_bits", io_point->number_of_bits, parent_io_point->number_of_bits, true, true, debug);
                //     io_point->bit_mask = (io_point->number_of_bits * io_point->number_of_bits) - 1;
                //     getItemFromMap(json_io_point, "scale", io_point->scale, parent_io_point->scale, true, true, debug);
                //     getItemFromMap(json_io_point, "normal_set", io_point->normal_set, parent_io_point->normal_set, true, true, debug);
                //     getItemFromMap(json_io_point, "signed", io_point->is_signed, parent_io_point->is_signed, true, true, debug);
                //     std::string format_str;
                //     getItemFromMap(json_io_point, "format", format_str, std::string(""), true, true, false);

                //     if (format_str.compare("naked") == 0)
                //     {
                //         io_point->format = Fims_Format::Naked;
                //     }
                //     else if (format_str.compare("clothed") == 0)
                //     {
                //         io_point->format = Fims_Format::Clothed;
                //     }
                //     else if (format_str.compare("full") == 0)
                //     {
                //         io_point->format = Fims_Format::Full;
                //     }
                //     else
                //     {
                //         if (packed_io_point)
                //         {
                //             io_point->format = parent_io_point->format;
                //         }
                //         else
                //         {
                //             io_point->format = register_group->format;
                //         }
                //     }
                //     io_point->invert_mask = 0;
                //     io_point->care_mask = std::numeric_limits<u64>::max();
                //     std::string invert_mask_str("0x0");
                //     std::string default_invert_mask_str("0x0");
                //     io_point->invert_mask = 0;
                //     io_point->invert_mask = parent_io_point->invert_mask;
                //     getItemFromMap(json_io_point, "invert_mask", invert_mask_str, default_invert_mask_str, true, true, debug);
                //     if (invert_mask_str != "0x0")
                //     {
                //         uint64_t invmask = 0;
                //         const char *istr = invert_mask_str.c_str();
                //         if (invert_mask_str.length() > 1 && (istr[0] == '0') && (istr[1] == 'x'))
                //         {
                //             istr += 2;
                //             invmask = strtoull(istr, NULL, 16);
                //         }
                //         else if ((invert_mask_str[0] == '0') && (invert_mask_str[1] == 'b'))
                //         {
                //             istr += 2;
                //             invmask = strtoull(istr, NULL, 2);
                //         }
                //         io_point->invert_mask = invmask;
                //     }
                //     // Extract io_point details (like id, offset, name, etc.) here...
                //     // Add to the io_point_map vector...
                //     io_point->is_float = false;
                //     io_point->is_byte_swap = false;
                //     io_point->is_byte_swap = false;
                //     getItemFromMap(json_io_point, "float", io_point->is_float, parent_io_point->is_float, true, true, debug);
                //     getItemFromMap(json_io_point, "word_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, true, true, debug);
                //     getItemFromMap(json_io_point, "word_order", io_point->word_order, parent_io_point->word_order, true, true, debug);
                //     getItemFromMap(json_io_point, "byte_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, true, true, debug);

                //     // for the benefit of size 4 regs
                //     // TODO remove word_swap
                //     if (io_point->is_byte_swap)
                //     {
                //         io_point->byte_index[0] = 3;
                //         io_point->byte_index[1] = 2;
                //         io_point->byte_index[2] = 1;
                //         io_point->byte_index[3] = 0;
                //     }
                //     else
                //     {
                //         io_point->byte_index[0] = 0;
                //         io_point->byte_index[1] = 1;
                //         io_point->byte_index[2] = 2;
                //         io_point->byte_index[3] = 3;
                //     }
                //     // word_order is 1234 or 4321 or 1324 etc
                //     if (io_point->word_order > 0)
                //     {
                //         io_point->byte_index[0] = (io_point->word_order / 1000) - 1;
                //         io_point->byte_index[1] = ((io_point->word_order % 1000) / 100) - 1;
                //         io_point->byte_index[2] = ((io_point->word_order % 100) / 10) - 1;
                //         io_point->byte_index[3] = (io_point->word_order % 10) - 1;
                //     }

                //     io_point->is_enum = false;
                //     io_point->is_random_enum = false;
                //     io_point->is_individual_bits = false;
                //     io_point->is_bit_field = false;
                //     getItemFromMap(json_io_point, "enum", io_point->is_enum, parent_io_point->is_enum, true, true, debug);
                //     getItemFromMap(json_io_point, "random_enum", io_point->is_random_enum, parent_io_point->is_random_enum, true, true, debug);
                //     getItemFromMap(json_io_point, "individual_bits", io_point->is_individual_bits, parent_io_point->is_individual_bits, true, true, debug);
                //     getItemFromMap(json_io_point, "bit_field", io_point->is_bit_field, io_point->is_bit_field, true, true, debug);
                //     double dval = 0.0;
                //     io_point->debounce = dval;
                //     io_point->deadband = dval;
                //     io_point->use_bool = true;
                //     getItemFromMap(json_io_point, "debounce", io_point->debounce, parent_io_point->debounce, true, true, debug);
                //     getItemFromMap(json_io_point, "deadband", io_point->deadband, parent_io_point->deadband, true, true, debug);
                //     if (io_point->debounce > 0.0)
                //         io_point->use_debounce = true;
                //     if (io_point->deadband > 0.0)
                //         io_point->use_deadband = true;
                //     getItemFromMap(json_io_point, "use_bool", io_point->use_bool, parent_io_point->use_bool, true, true, debug);
                //     if ((io_point->is_enum) || (io_point->is_random_enum) || (io_point->is_individual_bits) || (io_point->is_bit_field)
                //         /* maybe add more here */)
                //     {
                //         extract_bitstrings(io_point, json_io_point["bit_strings"]);
                //     }
                //     io_point->forced_val = 0;
                //     io_point->raw_val = 0;
                //     io_point->device_id = register_group->device_id;
                //     io_point->packer = packed_io_point;
                //     if (!packed_io_point)
                //     {
                //         register_group->io_point_map.push_back(io_point);
                //         if (debug)
                //             printf(" mapping register id [%s]\n", io_point->id.c_str());
                //         auto &io_point_map = register_group->io_point_map.back();
                //         // auto regshr = mymap->register_group.lock();
                //         register_group->io_point_map_lookup[io_point_map->offset] = io_point_map;
                //     }
                //     else
                //     {
                //         if (debug)
                //             printf(" packing register id [%s]\n", io_point->id.c_str());
                //         packed_io_point->bit_ranges.push_back(io_point);
                //     }
                // }
                // we only want the root registers in the main dict
                // if(!packed_io_point)
                // this will also add the packed_io_point items to the io_point so that they can be found with sets or gets
                addIOPointToComponentIOPointMap(myCfg.component_io_point_map, register_group->component_id.c_str(), register_group->id.c_str(), io_point);
                addIOPointToIdMap(myCfg.idMap, io_point);
                // addMapId(myCfg.idMap, device_id, io_point.register_type, mymap);
                getItemFromMap(json_io_point, "packed_register", io_point->packed_register, false, true, true, debug);
                if (io_point->packed_register && !packed_io_point)
                {
                    extract_io_point_map(register_group, io_point, json_io_point["bit_ranges"], myCfg, debug);
                }
                if (debug)
                    printf(" >>>>>>>>>>>>> <%s> component [%s] register_group [%s] id [%s] packed_io_point [%d] struct size %d \n", __func__,
                           register_group->component_id.c_str(), register_group->id.c_str(), io_point->id.c_str(), (int)io_point->packed_register, (int)sizeof(cfg::io_point_struct));
                // pull out
            }
        }
    }
    return true;
}

/**
 * @brief Parse the "bit_strings" field in an io_point
 * 
 * @param io_point the io_point that the list of bit_strings belongs to
 * @param rawStringData an std::any struct representing all bit_strings in an io_point (as a json list)
*/
void extract_bitstrings(std::shared_ptr<cfg::io_point_struct> io_point, std::any &rawStringData)
{
    // Ensure io_point is not nullptr
    if (!io_point)
    {
        throw std::runtime_error("io_point pointer is null!");
    }
    io_point->bits_known = 0;
    io_point->bits_unknown = 0;
    if (rawStringData.type() == typeid(std::vector<std::any>))
    {
        std::vector<std::any> rawDataList = std::any_cast<std::vector<std::any>>(rawStringData);
        auto bit_num = 0;
        for (const std::any &rawData : rawDataList)
        {
            // Handle simple string
            if (rawData.type() == typeid(std::string))
            {
                std::string stringData = std::any_cast<std::string>(rawData);
                io_point->bit_str.push_back(stringData);
                io_point->bit_str_num.push_back(bit_num);
            }
            // Handle object with "value" and "string"
            else if (rawData.type() == typeid(std::map<std::string, std::any>))
            {
                std::map<std::string, std::any> rawMap = std::any_cast<std::map<std::string, std::any>>(rawData);
                if (rawMap.find("string") != rawMap.end() && rawMap["string"].type() == typeid(std::string))
                {
                    io_point->bit_str.push_back(std::any_cast<std::string>(rawMap["string"]));
                }
                if (rawMap.find("value") != rawMap.end() && rawMap["value"].type() == typeid(int))
                {
                    bit_num = std::any_cast<int>(rawMap["value"]);
                    io_point->bit_str_num.push_back(bit_num);
                }
                else
                {
                    io_point->bit_str_num.push_back(bit_num);
                }
            }
            else if (rawData.type() == typeid(void))
            {
                io_point->bit_str.push_back("");
                io_point->bit_str_num.push_back(bit_num);
                io_point->bits_unknown |= 1 << bit_num;
            }
            else
            {
                std::string err = io_point->id;
                err += ": unknown bitstring type ";
                err += anyTypeString(rawData);
                throw std::runtime_error(err);
            }
            io_point->bits_known |= 1 << bit_num;
            bit_num++;
        }
    }
    return;
}

/**
 * @brief Demangle the type of a bit_string so that the appropriate exception can be thrown.
 * 
 * @param value the std::any value that is trying to be unpacked into a bit_string io_point
 * @return a string representation of the type of the std::any value
*/
std::string anyTypeString(const std::any &value)
{
    int status;
    char *demangled = abi::__cxa_demangle(value.type().name(), 0, 0, &status);
    if (status == 0)
    {
        std::string unknown_type = fmt::format("{}", demangled);
        free(demangled);
        return unknown_type;
    }
    else
    {
        std::string unknown_type = fmt::format("{}", value.type().name());
        return unknown_type;
    }
}

/**
 * @brief Check that all non-packed io_points in a register group have a non-zero point size.
 * 
 * If an io_point is not packed and its size == 0, the io_point->size will be set to 1 instead.
 * 
 * @param register_group the register group containing the io_points to check
 * @param io_point_map the io_point map belonging to that register group
 * @param debug bool value representing whether or not to print messages to help debug code
*/
void checkNonZeroIOPointSize(std::shared_ptr<cfg::register_group_struct> register_group, std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map, bool debug = false)
{
    for (size_t i = 0; i < io_point_map.size(); ++i)
    {
        if (!io_point_map[i]->packed_register)
        {
            if (debug)
                printf(">>>> %s setting [%d] io_point_map_lookup %d to map %p id %s\n", __func__, (int)i, io_point_map[i]->offset, (void *)io_point_map[i].get(), io_point_map[i]->id.c_str());

            if (io_point_map[i]->size == 0)
                io_point_map[i]->size = 1;
        }
    }
}

/**
 * @brief Sort a vector of io_point_structs by their offsets.
 * 
 * @param elements a vector of io_point_structs, typically the io_point_map contained in a register_group
*/
void sortIOPointVectorByOffset(std::vector<std::shared_ptr<cfg::io_point_struct>> &elements)
{
    if (elements.size() > 1)
    {
        // TODO check then both compares are the same
        //std::sort(elements.begin(), elements.end(), compareIOPointOffsets);

        std::sort(elements.begin(), elements.end(),
              [](const std::shared_ptr<cfg::io_point_struct> a, const std::shared_ptr<cfg::io_point_struct> b) -> bool
              {
                  return cfg::io_point_struct::compare(*a, *b); // Dereference the shared_ptr here
              });
    }
}

/**
 * @brief Get the first offset in a vector of io_points.
 * 
 * @param elements a pre-sorted vector of io_point_structs, typically the io_point_map contained in a register_group
 * 
 * @pre elements is sorted by offset
*/
int getFirstOffset(const std::vector<std::shared_ptr<cfg::io_point_struct>> elements)
{
    if (elements.empty())
    {
        return -1;
    }
    return elements[0]->offset;
}

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
int getTotalNumRegisters(std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map)
{
    if (io_point_map.empty())
    {
        FPS_ERROR_LOG("Error: io_point_map vector is empty.");
        return 0; // or handle error appropriately
    }
    int total_size = io_point_map[0]->size;
    for (size_t i = 1; i < io_point_map.size(); ++i)
    {
        auto unpacked = io_point_map[i];
        //FPS_INFO_LOG("Checking Register [%s]: offset [%d] size [%d]", unpacked->id.c_str(), unpacked->offset,unpacked->size);
        unpacked->next.reset();
        if (!unpacked->packed_register)
        {
            total_size += unpacked->size;
            if (unpacked->offset > io_point_map[i - 1]->offset + io_point_map[i - 1]->size)
            {
                FPS_INFO_LOG("Register size gap at Offset [%d]: [%s]", io_point_map[i - 1]->offset, io_point_map[i - 1]->id.c_str());
            }
            else if (unpacked->offset < io_point_map[i - 1]->offset + io_point_map[i - 1]->size)
            {
                FPS_ERROR_LOG("Overlapping registers at Offset [%d]: [%s] (size %d) and Offset [%d]: [%s] (size %d)"
                        , io_point_map[i - 1]->offset, io_point_map[i - 1]->id.c_str(), io_point_map[i - 1]->size
                        , unpacked->offset, unpacked->id.c_str(), unpacked->size);
                // TODO stop on invalid config...
                //return -1;
            }
        }
        io_point_map[i - 1]->next = unpacked; // setting the 'next' pointer to the next shared_ptr
    }
    return total_size;
}

/**
 * @brief Reassign a config instance to a new config structure. Used by reload.
 * 
 * @param cfgInstance the current config instance to overwrite with "blank" data
*/
void resetCfg(struct cfg &cfgInstance)
{
    // Create a new instance of cfg with default values
    struct cfg newCfg;

    // Assign the newCfg to the existing cfgInstance
    cfgInstance = newCfg;
}

///////////////////////////////////////
/////////// ENCODE/DECODE /////////////
///////////////////////////////////////

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
uint8_t get_any_to_bool(std::shared_ptr<cfg::io_point_struct> io_point, std::any val, Uri_req &uri, uint8_t *regs8)
{
    uint8_t bool_intval = 0;
    if (val.type() == typeid(std::map<std::string, std::any>))
    {
        auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
        if (clothed_val.find("value") != clothed_val.end())
        {
            val = clothed_val["value"];
        }
    }
    if (val.type() == typeid(bool))
    {
        if (std::any_cast<bool>(val))
            bool_intval = 1;
    }
    else if (val.type() == typeid(int64_t))
    {
        if (std::any_cast<int64_t>(val) > 0)
            bool_intval = 1;
    }
    else if (val.type() == typeid(int32_t))
    {
        if (std::any_cast<int32_t>(val) > 0)
            bool_intval = 1;
    }
    else if (val.type() == typeid(uint32_t))
    {
        if (std::any_cast<uint32_t>(val) > 0)
            bool_intval = 1;
    }
    else if (val.type() == typeid(uint64_t))
    {
        if (std::any_cast<uint64_t>(val) > 0)
            bool_intval = 1;
    }
    else if (val.type() == typeid(std::string))
    {
        if (std::any_cast<std::string>(val) == "true")
            bool_intval = 1;
    }
    else if (val.type() == typeid(double))
    {
        if (std::any_cast<double>(val) > 0)
            bool_intval = 1;
    }
    if (io_point->scale == -1)
        bool_intval ^= 1;

    if (io_point->is_forced)
    {
        bool_intval = static_cast<uint8_t>(io_point->forced_val);
    }
    regs8[0] = bool_intval;
    return bool_intval;
}

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
uint64_t get_any_to_uint64(std::shared_ptr<cfg::io_point_struct> io_point, std::any val, Uri_req &uri, uint16_t *regs16)
{
#ifdef FPS_DEBUG_MODE
    bool debug = false;
#endif
    uint16_t u16val = 0;
    uint32_t u32val = 0;
    uint64_t u64val = 0;
    int64_t i64val = 0;
    double f64val = 0.0;
    bool is_uint = false;
    bool is_int = false;
    if (uri.is_unforce_request)
    {
        io_point->is_forced = false;
    }
    if (uri.is_enable_request)
    {
        io_point->is_enabled = true;
    }
    if (uri.is_disable_request)
    {
        io_point->is_enabled = false;
    }

    // // handle val being in a value field
    if (val.type() == typeid(std::map<std::string, std::any>))
    {
        auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
        if (clothed_val.find("value") != clothed_val.end())
        {
            val = clothed_val["value"];
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " value found " << std::endl;
#endif
        }
    }
    // todo add other exceptions here
    if (io_point->is_bit_field || io_point->is_enum || io_point->packed_register)
    {
        io_point->normal_set = false;
    }
    else
    {
#ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << __func__ << " io_point normal set, io_point size " << io_point->size << std::endl;
#endif
        io_point->normal_set = true;
    }
    if (io_point->normal_set)
    {
#ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << ">>>>" << __func__ << " normal_set " << std::endl;
#endif
        if (val.type() == typeid(bool))
        {
            if (std::any_cast<bool>(val))
                i64val = 1;
        }
        else if ((val.type() == typeid(int32_t)) || (val.type() == typeid(int64_t)))
        {
            is_int = true;
            if (val.type() == typeid(int32_t))
            {
                i64val = static_cast<int64_t>(std::any_cast<int32_t>(val));
            }
            else
            {
                i64val = std::any_cast<int64_t>(val);
            }
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
#endif
            i64val <<= io_point->starting_bit_pos;
            i64val -= io_point->shift;
            f64val = static_cast<double>(i64val);
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
        }
        else if ((val.type() == typeid(uint32_t)) || (val.type() == typeid(uint64_t)))
        {
            is_uint = true;
            if (val.type() == typeid(uint32_t))
            {
                u64val = static_cast<uint64_t>(std::any_cast<uint32_t>(val));
            }
            else
            {
                u64val = std::any_cast<int64_t>(val);
            }
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found uint >>" << u64val << std::endl;
#endif
            u64val <<= io_point->starting_bit_pos;
            u64val -= io_point->shift;
            f64val = static_cast<double>(u64val);
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
        }
        else if (val.type() == typeid(double))
        {
            f64val = std::any_cast<double>(val);
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found double >>" << f64val << std::endl;
#endif
            f64val -= io_point->shift;
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
        }
        else
        {
            FPS_ERROR_LOG("[%s]: value undefined %s", __func__, val.type().name());
        }
        if (io_point->size == 1)
        {
            if (is_uint && !io_point->scale)
            {
                u16val = static_cast<uint16_t>(u64val);
            }
            else if (is_int && !io_point->scale)
            {
                u16val = static_cast<int16_t>(i64val);
            }
            else
            {
                if (!io_point->is_signed)
                {
                    u16val = static_cast<uint16_t>(f64val);
                }
                else
                {
                    u16val = static_cast<uint16_t>(static_cast<int16_t>(f64val));

                    //u16val = static_cast<uint16_t>(f64val);
                }
            }
            u16val ^= static_cast<uint16_t>(io_point->invert_mask);
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " final u64val >>" << u16val << " i64val >> " << i64val << std::endl;
#endif
            if (uri.is_force_request)
            {
                io_point->is_forced = true;
                io_point->forced_val = static_cast<uint64_t>(u16val);
            }
            if (io_point->is_forced)
                u16val = static_cast<uint16_t>(io_point->forced_val);
            regs16[0] = static_cast<uint16_t>(u16val);
            u64val = static_cast<uint64_t>(u16val);
        }
        else if (io_point->size == 2)
        {
            if (is_uint && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    u32val = static_cast<uint32_t>(u64val);
                }
                else
                {
                    const auto f32val = static_cast<float>(u64val);
                    memcpy(&u32val, &f32val, sizeof(u32val));
                }
            }
            else if (is_int && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    u32val = static_cast<uint32_t>(i64val);
                }
                else
                {
                    const auto f32val = static_cast<float>(i64val);
                    memcpy(&u32val, &f32val, sizeof(u32val));
                }
            }
            else
            {
                if (io_point->is_float)
                {
                    const auto f32val = static_cast<float>(f64val);
                    memcpy(&u32val, &f32val, sizeof(u32val));
                }
                else if (!io_point->is_signed)
                {
                    u32val = static_cast<uint32_t>(f64val);
                }
                else
                {
                    u32val = static_cast<uint32_t>(static_cast<int32_t>(f64val));
                }
            }
            u32val ^= static_cast<uint32_t>(io_point->invert_mask);
            if (uri.is_force_request)
            {
                io_point->is_forced = true;
                io_point->forced_val = static_cast<uint64_t>(u32val);
            }
            if (io_point->is_forced)
                u32val = static_cast<uint32_t>(io_point->forced_val);
            if (!io_point->is_byte_swap)
            {
                regs16[0] = static_cast<uint16_t>(u32val >> 16);
                regs16[1] = static_cast<uint16_t>(u32val >> 0);
            }
            else
            {
                regs16[0] = static_cast<uint16_t>(u32val >> 0);
                regs16[1] = static_cast<uint16_t>(u32val >> 16);
            }
            u64val = static_cast<uint64_t>(u32val);
        }
        else if (io_point->size == 4)
        {
            if (is_uint && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    // u64val = static_cast<uint64_t>(u64val);
                }
                else
                {
                    const auto f64val = static_cast<double>(u64val);
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
            }
            else if (is_int && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    u64val = static_cast<uint64_t>(i64val);
                }
                else
                {
                    const auto f64val = static_cast<double>(i64val);
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
            }
            else
            {
                if (io_point->is_float)
                {
                    // const auto f32val = static_cast<float>(f64val);
                    // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
                else if (!io_point->is_signed)
                {
                    u64val = static_cast<uint64_t>(f64val);
                }
                else
                {
                    u64val = static_cast<uint64_t>(static_cast<int64_t>(f64val));
                }
            }
            // uint64_t tval;
            // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
            u64val ^= static_cast<uint64_t>(io_point->invert_mask);
            if (uri.is_force_request)
            {
                io_point->is_forced = true;
                io_point->forced_val = static_cast<uint64_t>(u64val);
            }
            if (io_point->is_forced)
                u64val = io_point->forced_val;
            if (!io_point->is_byte_swap)
            {
                regs16[0] = static_cast<uint16_t>(u64val >> 48);
                regs16[1] = static_cast<uint16_t>(u64val >> 32);
                regs16[2] = static_cast<uint16_t>(u64val >> 16);
                regs16[3] = static_cast<uint16_t>(u64val >> 0);
                // memcpy(&tval, regs16, sizeof(tval));
                // printf( " sent regs16 #1 0x%08lx  \n", tval);
                // printf( " sent u64val   #1 0x%08lx  \n", u64val);
            }
            else
            {
                regs16[0] = static_cast<uint16_t>(u64val >> 0);
                regs16[1] = static_cast<uint16_t>(u64val >> 16);
                regs16[2] = static_cast<uint16_t>(u64val >> 32);
                regs16[3] = static_cast<uint16_t>(u64val >> 48);
            }
            // TODO test this
            if (io_point->word_order > 0)
            {
                regs16[io_point->byte_index[0]] = static_cast<uint16_t>(u64val >> 48);
                regs16[io_point->byte_index[1]] = static_cast<uint16_t>(u64val >> 32);
                regs16[io_point->byte_index[2]] = static_cast<uint16_t>(u64val >> 16);
                regs16[io_point->byte_index[3]] = static_cast<uint16_t>(u64val >> 0);

            }

        }
    }
    return u64val;
}

/**
 * @brief Based on the settings in io_point, store a uint64 value in a set of 16-bit registers.
 * 
 * @param io_point an io_point_struct that contains the settings to base the conversion on
 * @param uval the uint64 value to store in regs16
 * @param regs16 a pointer to the 4x16-bit register to store the uint64 value.
 * 
 * @return io_point->size
*/
int set_reg16_from_uint64(std::shared_ptr<cfg::io_point_struct> io_point, uint64_t &uval, uint16_t *regs16)
{
    if (io_point->size == 1)
    {
        regs16[0] = static_cast<uint16_t>(uval);
    }
    else if (io_point->size == 2)
    {
        if (!io_point->is_byte_swap)
        {
            regs16[0] = static_cast<uint16_t>(uval >> 16);
            regs16[1] = static_cast<uint16_t>(uval >> 0);
        }
        else
        {
            regs16[0] = static_cast<uint16_t>(uval >> 0);
            regs16[1] = static_cast<uint16_t>(uval >> 16);
        }
    }

    else if (io_point->size == 4)
    {
        if (!io_point->is_byte_swap)
        {
            regs16[0] = static_cast<uint16_t>(uval >> 48);
            regs16[1] = static_cast<uint16_t>(uval >> 32);
            regs16[2] = static_cast<uint16_t>(uval >> 16);
            regs16[3] = static_cast<uint16_t>(uval >> 0);
            // memcpy(&tval, regs16, sizeof(tval));
            // printf( " sent regs16 #1 0x%08lx  \n", tval);
            // printf( " sent u64val   #1 0x%08lx  \n", u64val);
        }
        else
        {
            regs16[3] = static_cast<uint16_t>(uval >> 48);
            regs16[2] = static_cast<uint16_t>(uval >> 32);
            regs16[1] = static_cast<uint16_t>(uval >> 16);
            regs16[0] = static_cast<uint16_t>(uval >> 0);
        }
        // TODO test this
        if (io_point->word_order > 0)
        {
            regs16[io_point->byte_index[0]] = static_cast<uint16_t>(uval >> 48);
            regs16[io_point->byte_index[1]] = static_cast<uint16_t>(uval >> 32);
            regs16[io_point->byte_index[2]] = static_cast<uint16_t>(uval >> 16);
            regs16[io_point->byte_index[3]] = static_cast<uint16_t>(uval >> 0);

        }

    }

    return io_point->size;
}

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
uint64_t set_any_to_uint64(struct cfg &myCfg, std::shared_ptr<cfg::io_point_struct> io_point, std::any val)
{
#ifdef FPS_DEBUG_MODE
    bool debug = false;
#endif
    uint16_t u16val = 0;
    uint32_t u32val = 0;
    uint64_t u64val = 0;
    int64_t i64val = 0;
    double f64val = 0.0;
    bool is_uint = false;
    bool is_int = false;

    // // handle val being in a value field
    // if (val.type() == typeid(unsigned long))
    // {
    //     std::cout << __func__ << " we got an unsigned long" << std::endl;
    // }
    // if (val.type() == typeid(uint64_t))
    // {
    //     std::cout << __func__ << " we got an uint64_t" << std::endl;

    // }
    if (val.type() == typeid(std::map<std::string, std::any>))
    {
        //std::cout << __func__ << " looking for a clothed value" << std::endl;
        auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
        if (clothed_val.find("value") != clothed_val.end())
        {
            val = clothed_val["value"];
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " value found " << std::endl;
#endif
        }
    }
    // todo add other exceptions here
    if (io_point->is_bit_field || io_point->is_enum || io_point->packed_register)
    {
        io_point->normal_set = false;
    }
    else
    {
#ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << __func__ << " io_point normal set, io_point size " << io_point->size << std::endl;
#endif
        io_point->normal_set = true;
    }

    if (io_point->normal_set)
    {
#ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << ">>>>" << __func__ << " normal_set " << std::endl;
#endif
        if (val.type() == typeid(bool))
        {
            if (std::any_cast<bool>(val))
                    i64val = 1;
            if (io_point->scale < 0)
            {
                if(i64val>0)
                    i64val = 0;
                else
                    i64val = 1;
            }
            //u64val = static_cast<uint64_t>(i64val);
            is_int = true;
            //std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int value [" << i64val <<"]"<< std::endl;
            if (io_point->scale)
            {
                f64val = static_cast<double>(i64val);
                f64val *= io_point->scale;
            }

        }
        else if ((val.type() == typeid(int32_t)) || (val.type() == typeid(int64_t)))
        {
            is_int = true;
            if (val.type() == typeid(int32_t))
            {
                i64val = static_cast<int64_t>(std::any_cast<int32_t>(val));
            }
            else
            {
                i64val = std::any_cast<int64_t>(val);
            }
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
#endif
            //std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
            i64val <<= io_point->starting_bit_pos;
            i64val -= io_point->shift;
            f64val = static_cast<double>(i64val);
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
            //std::cout << ">>>>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
        }
        else if ((val.type() == typeid(uint32_t)) || (val.type() == typeid(uint64_t)))
        {
            is_uint = true;
            if (val.type() == typeid(uint32_t))
            {
                u64val = static_cast<uint64_t>(std::any_cast<uint32_t>(val));
            }
            else
            {
                u64val = std::any_cast<uint64_t>(val);
            }
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found uint >>" << u64val << std::endl;
#endif
                //std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found uint >>" << u64val << std::endl;
            u64val <<= io_point->starting_bit_pos;
            u64val -= io_point->shift;
            f64val = static_cast<double>(u64val);
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
        }
        else if (val.type() == typeid(double))
        {
            f64val = std::any_cast<double>(val);
#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found double >>" << f64val << std::endl;
#endif
            //std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found double >>" << f64val << std::endl;
            f64val -= io_point->shift;
            if (io_point->scale)
            {
                f64val *= io_point->scale;
            }
        }
        else
        {
            FPS_ERROR_LOG("[%s]: value undefined %s", __func__, val.type().name());
        }
        if (io_point->size == 1)
        {
            if (is_uint && !io_point->scale)
            {
                u16val = static_cast<uint16_t>(u64val);
                //std::cout << ">>>>" << __func__ << " uint size 1 offset  " << io_point->offset << " found u64val value [" << u64val <<"]"<< std::endl;
            }
            else if (is_int && !io_point->scale)
            {
                if (io_point->is_signed)
                {
                        if (i64val > std::numeric_limits<int16_t>::max()) {
                            i64val = std::numeric_limits<int16_t>::max();
                        } else if (i64val < -std::numeric_limits<int16_t>::max()) {
                            i64val = -(std::numeric_limits<int16_t>::max()+1);
                        }

                    u16val = static_cast<int16_t>(i64val);
                    //std::cout << ">>>>" << __func__ << " signed int size 1 offset  " << io_point->offset << " found u16val value [" << u16val <<"]"<< std::endl;

                }
                else
                {
                    if (i64val > std::numeric_limits<uint16_t>::max()) {
                        i64val = std::numeric_limits<uint16_t>::max();
                    } else if (i64val < 0) {
                        i64val = 0;
                    }
                    u16val = static_cast<uint16_t>(i64val);
                    //std::cout << ">>>>" << __func__ << " not signed int size 1 offset  " << io_point->offset << " found u16val value [" << u16val <<"]"<< std::endl;
                }
            }
            else
            {
                if (!io_point->is_signed)
                {
                    if (f64val > std::numeric_limits<uint16_t>::max()) {
                        f64val = std::numeric_limits<uint16_t>::max();
                    } else if (f64val < 0) {
                        f64val = 0;
                    }
                    u16val = static_cast<uint16_t>(f64val);
                    if(0)std::cout << ">>>>" << __func__ << " not signed double  size 1 offset  " << io_point->offset 
                            << " found f64val value [" << f64val 
                            <<"] u16val [" <<u16val << "]"
                            << std::endl;
                }
                else
                {

                    if (f64val > std::numeric_limits<int16_t>::max()) {
                        f64val = std::numeric_limits<int16_t>::max();
                    } else if (f64val < -std::numeric_limits<int16_t>::max()) {
                        f64val = -(std::numeric_limits<int16_t>::max()+1);
                    }
                    u16val = static_cast<uint16_t>(static_cast<int16_t>(f64val));

                    //u16val = static_cast<uint16_t>(f64val);
                }
            }
            u16val ^= static_cast<uint16_t>(io_point->invert_mask);
            // if (uri.is_force_request)
            // {
            //     io_point->is_forced = true;
            //     io_point->forced_val = static_cast<uint64_t>(u16val);
            // }
            // regs16[0] = static_cast<uint16_t>(u16val);
            u64val = static_cast<uint64_t>(u16val);
            //std::cout << ">>>>" << __func__ << " size 1 offset  " << io_point->offset << " found uint value [" << u64val <<"]"<< std::endl;

        }
        else if (io_point->size == 2)
        {
            if (is_uint && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    u32val = static_cast<uint32_t>(u64val);
                }
                else
                {
                    const auto f32val = static_cast<float>(u64val);
                    memcpy(&u32val, &f32val, sizeof(u32val));
                }
            }
            else if (is_int && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    if(io_point->is_signed)
                    {
                        if (i64val > std::numeric_limits<int>::max()) {
                            i64val = std::numeric_limits<int>::max();
                        } else if (i64val < -std::numeric_limits<int>::max()) {
                            i64val = -(std::numeric_limits<int>::max());
                        // } else {
                        //     f32val = static_cast<float>(f64val);
                        }
                    }
                    else
                    {
                        if (i64val < 0)
                            i64val = 0;
                        if (i64val > std::numeric_limits<uint>::max()) {
                            i64val = std::numeric_limits<uint>::max();
                        } 
                    }
                
                    u32val = static_cast<uint32_t>(i64val);
                    //std::cout << ">>>>" << __func__ << " size 2 offset  " << io_point->offset << " found uint value [" << u32val <<"]"<< std::endl;

                }
                else
                {
                    float f32val;
                    if (i64val > std::numeric_limits<float>::max()) {
                        f32val = std::numeric_limits<float>::max();
                    } else if (i64val < -std::numeric_limits<float>::max()) {
                        f32val = -std::numeric_limits<float>::max();
                    } else {
                        f32val = static_cast<float>(i64val);
                    }

                    //float f32val = static_cast<float>(i64val);
                    memcpy(&u32val, &f32val, sizeof(u32val));
                    if(0)std::cout << ">>>> " << __func__ 
                        << " size 2 int found i64 value " << i64val 
                        << " got f32 value " << f32val 
                        << " got u32 value " << u32val 
                        << std::endl;
                }
            }
            else  // deal with a float or a scaled int in f64val
            {

                if(!io_point->is_float)
                {
                    if(!io_point->is_signed && !io_point->is_float)
                    {
                        if (f64val < 0)
                            f64val = 0;
                    }

                    int32_t i32val;

                    if(io_point->is_signed)
                    {
                        if (f64val > std::numeric_limits<int32_t>::max()) {
                            i32val = std::numeric_limits<int32_t>::max();
                        } else if (f64val < -std::numeric_limits<int32_t>::max()) {
                            i32val = -(std::numeric_limits<int32_t>::max());
                        } else {
                            i32val = static_cast<int32_t>(f64val);
                        }

                        u32val = static_cast<int32_t>(i32val);
                    }
                    else
                    {
                        if (f64val > std::numeric_limits<uint32_t>::max()) {
                            u32val = std::numeric_limits<uint32_t>::max();
                        } else if (f64val < 0) {
                            u32val = 0;//-std::numeric_limits<int32_t>::max();
                        } else {
                            u32val = static_cast<uint32_t>(f64val);
                        }
                            //u32val = static_cast<uint32_t>(f32val);
                        //}
                        if(0)std::cout << ">>>>" << __func__ 
                            << " size 2 found f64 value " << f64val 
                            //<< " got f32 value " << f32val 
                            << " got u32 value " << u32val 
                            << std::endl;
                    }
                }

                else //if (io_point->is_float)
                {
                    float f32val;
                    if (f64val > std::numeric_limits<float>::max()) {
                        f32val = std::numeric_limits<float>::max();
                    } else if (f64val < -std::numeric_limits<float>::max()) {
                        f32val = -(std::numeric_limits<float>::max()+1);
                    } else {
                        f32val = static_cast<float>(f64val);
                    }
                    memcpy(&u32val, &f32val, sizeof(u32val));
                    if(0)std::cout << ">>>> " << __func__ 
                        << " size 2 float found f64 value " << f64val 
                        << " got f32 value " << f32val 
                        << " got u32 value " << u32val 
                        << std::endl;

                }
            }
            u32val ^= static_cast<uint32_t>(io_point->invert_mask);
            u64val = static_cast<uint64_t>(u32val);
        }
        else if (io_point->size == 4)
        {
            if (is_uint && !io_point->scale)
            {
                if (!io_point->is_float)
                {
                    // u64val = static_cast<uint64_t>(u64val);
                }
                else
                {
                    const auto f64val = static_cast<double>(u64val);
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
            }
            else if (is_int && !io_point->scale)
            {
                if(!io_point->is_signed && !io_point->is_float)
                {
                    if (i64val < 0)
                        i64val = 0;
                }

                if (!io_point->is_float)
                {
                    u64val = static_cast<uint64_t>(i64val);
                }
                else
                {
                    const auto f64val = static_cast<double>(i64val);
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
            }
            else
            {
                // we are a double in f64val
                if(!io_point->is_signed && !io_point->is_float)
                {
                    if (f64val < 0)
                        f64val = 0;
                }

                if (io_point->is_float)
                {
                    // const auto f32val = static_cast<float>(f64val);
                    // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
                    memcpy(&u64val, &f64val, sizeof(u64val));
                }
                else if (!io_point->is_signed)
                {
                    // not a float so limit to uint64_t
                    if (f64val > std::numeric_limits<uint64_t>::max()) {
                        f64val = std::numeric_limits<uint64_t>::max();
                    } else if (f64val < 0) {//-std::numeric_limits<int32_t>::max()) {
                        f64val = 0;//-std::numeric_limits<int32_t>::max();
                    // } else {
                    //         i32val = static_cast<int32_t>(f64val);
                    }
                    /// here
                    u64val = static_cast<uint64_t>(f64val);
                }
                else
                {
                    int64_t i64val;
                    // not a float so limit to int64_t
                    if (f64val > std::numeric_limits<int64_t>::max()) {
                        i64val = std::numeric_limits<int64_t>::max();
                    } else if (f64val < -std::numeric_limits<int64_t>::max()) {
                        i64val = -(std::numeric_limits<int64_t>::max());
                    } else {
                        i64val = static_cast<int64_t>(f64val);
                    }
                    //u64val = static_cast<uint64_t>(static_cast<int64_t>(f64val));
                    u64val = static_cast<uint64_t>(i64val);
                }
            }
            // uint64_t tval;
            // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
            u64val ^= static_cast<uint64_t>(io_point->invert_mask);
        }
    }
    return u64val;
}

///////////////////////////////////////
//////////// POINT LOOKUP /////////////
///////////////////////////////////////

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
void addIOPointToComponentIOPointMap(Component_IO_point_map &imap, const char *component_uri_prefix, const char *component_id, std::shared_ptr<cfg::io_point_struct> io_point)
{
    if (io_point)
    {
        if (imap.find(component_uri_prefix) == imap.end())
        {
            imap[component_uri_prefix] = std::map<std::string, std::map<std::string, std::shared_ptr<cfg::io_point_struct>>>();
        }
        auto &component_map = imap[component_uri_prefix];
        if (component_map.find(component_id) == component_map.end())
        {
            component_map[component_id] = std::map<std::string, std::shared_ptr<cfg::io_point_struct>>();
        }
        auto &point_map = component_map[component_id];
        if (point_map.find(io_point->id) != point_map.end())
        {
            FPS_WARNING_LOG("Duplicate point id [%s] offset [%d] found. Unexpected behavior may occur.", io_point->id.c_str(), io_point->offset);
        } else if(io_point->is_individual_bits){
            for(std::string bit_string : io_point->bit_str){
                if (point_map.find(bit_string) != point_map.end())
                {
                    FPS_WARNING_LOG("Duplicate id [%s] found for individual bit of point [%s] offset [%d]. Unexpected behavior may occur.", bit_string.c_str(), io_point->id.c_str(), io_point->offset);
                } else {
                    point_map[bit_string] = io_point;
                }
            }
        }
        point_map[io_point->id] = io_point;
    }
    else
    {
        FPS_INFO_LOG("Cannot add nullptr to component IO map.\n");
    }
}

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
void addIOPointToIdMap(MapIdMap &imap, std::shared_ptr<cfg::io_point_struct> io_point)
{
    if(io_point)
    {
        if (imap.find(io_point->device_id) == imap.end())
        {
            imap[io_point->device_id] = std::map<cfg::Register_Types, std::map<int, cfg::io_point_struct*>>();
        }
        auto &idMap = imap[io_point->device_id];
        if (idMap.find(io_point->register_type) == idMap.end())
        {
            idMap[io_point->register_type] = std::map<int, cfg::io_point_struct*>();
        }
        auto &point_map = idMap[io_point->register_type]; 
        point_map[io_point->offset] = io_point.get();
    }
    else
    {
        FPS_INFO_LOG("Cannot add nullptr to reg Id map.\n");
    }
}

/**
 * @brief check if an io_point exists within a configuration
 * 
 * @param io_point a reference to the io_point struct to populate if the point lookup is successful
 * @param myCfg the config to search for the point within
 * @param uri_keys a vector of uri fragments to identify the point (e.g. /some/random/uri would be [some, random, uri])
 * @param key_size the number of uri fragments in uri_keys
 * @param io_point_key the io_point id to look for, if not specified by the uri. Default is "".
*/
bool ioPointExists(
    std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, int uri_key_size, std::string io_point_key = "")
{
    int key_idx = 0;
    if ((uri_key_size > 1) && (uri_keys[0].size() == 0))
        key_idx = 1;
    std::string myvar = io_point_key;
    if (uri_key_size < (key_idx + 3))
    {
        myvar = io_point_key;
    }
    else
    {
        myvar = uri_keys[key_idx + 2];
    }
    try
    {
        // Using at() for safe access. Catch exceptions if key not found
        auto &myComp = myCfg.component_io_point_map.at(uri_keys[key_idx]);
        // std::cout   << "         func : #2 "<< __func__ << "\n";
        auto &register_group = myComp.at(uri_keys[key_idx + 1]);
        // std::cout   << "         func : #3 "<< __func__ << "\n";
        io_point = register_group.at(myvar);
        return true;
    }
    catch (const std::out_of_range &)
    {
        // Key not found, return nullptr
        return false;
    }
    return false;
}

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
std::map<std::string, std::shared_ptr<cfg::io_point_struct>> *cfg::findIOPointMapFromUriFragments(std::vector<std::string> keys)
{
    // Look up outerKey in the outer map
    if (keys.size() < 3)
    {
        //std::cout << "Not enough keys found" << std::endl;
        return nullptr;
    }
    int key_idx = 0;
    if ((int)keys[0].size() == (int)0)
    {
        //std::cout << "skipping key 0" << std::endl;
        key_idx = 1;
    }
    auto outerIt = component_io_point_map.find(keys[key_idx]); // like "components" in /components/sel_3530_slow_rtac
    if (outerIt != component_io_point_map.end())
    {
        // If outerKey is found, look up innerKey in the inner map
        // like "sel_3530_slow_rtac" in /components/sel_3530_slow_rtac
        auto &innerMap = outerIt->second;
        auto innerIt = innerMap.find(keys[key_idx + 1]);
        if (innerIt != innerMap.end())
        {
            // If innerKey is also found, return the inner map (the io_point map for a component)
            return &innerIt->second;
        }
        else
        {
            //std::cout << " inner key [" << keys[key_idx + 1] << "] not  found" << std::endl;
            return nullptr;
        }
    }
    else
    {
        if(0)std::cout << " base key [" << keys[key_idx] << "] not  found" << std::endl;
    }
    // If any key not found, return nullopt
    return nullptr;
}

///////////////////////////////////////
/////// FORMAT TO STRING_STREAM ///////
///////////////////////////////////////

/**
 * @brief Format the details of the server config document to a stringstream.
 * 
 * @param myCfg the client configuration used to populate the server config stringstream
 * @param ss the string stream to format the details of the server config to
*/
void showServerConfig(struct cfg &myCfg, std::stringstream &ss)
{
    ss << "\"system\":{\n";
    showSystem(myCfg, ss);
    ss << "},\n";
    ss << "\"registers\":[";

    bool first_dev = true;
    for (std::pair<const int, std::map<cfg::Register_Types, std::map<int, cfg::io_point_struct *>>> &dev_id_map : myCfg.idMap)
    {
        for (std::pair<const cfg::Register_Types, std::map<int, cfg::io_point_struct *>> &register_group_map : dev_id_map.second)
        {
            if(!first_dev)
                ss << ",";
            ss << "\n";
            first_dev = false;
            showDeviceRegisterGroup(myCfg,  dev_id_map.first, register_group_map.first, register_group_map.second, ss);
            //ss <<"]\n";
        }
    }
    ss << "\n   ]\n";
}

/**
 * @brief Format the details of the server config "system" field to a stringstream.
 * 
 * Includes name, protocol, id, ip_address, port, and device id.
 * 
 * @param myCfg the client configuration used to populate the server config "system" stringstream
 * @param ss the string stream to format the details of the server config "system" to
*/
void showSystem(struct cfg &myCfg, std::stringstream &ss)
{
    ss << "                  \"name\":\"" << myCfg.connection.name<< "\",\n";
    ss << "                  \"protocol\":\"" << myCfg.connection.protocol<< "\",\n";
    ss << "                  \"id\":\"" << myCfg.connection.name<< "\",\n";
    ss << "                  \"ip_address\":\"" << "0.0.0.0"<< "\",\n";
    ss << "                  \"port\":" << myCfg.connection.port<< ",\n";
    ss << "                  \"device_id\":" << myCfg.connection.device_id<< "\n";

}

/**
 * @brief Format a register group, including its device_id and all io_points, to a stringstream.
 * 
 * @param myCfg the client configuration used to populate the server config "registers" stringstream
 * @param dev_id the device_id of the current register group
 * @param reg_group_type the register group type (e.g. holding, coil, input, discrete input)
 * @param io_point_map_lookup std::map<int, cfg::io_point_struct *> containing all points in a register group
 * @param ss the string stream to format the details of the io_point to
*/
void showDeviceRegisterGroup(struct cfg &myCfg, int dev_id, cfg::Register_Types reg_group_type, std::map<int, cfg::io_point_struct *> io_point_map_lookup, std::stringstream &ss)
{
    ss  <<"             {\n";
    ss  << "             \"device_id\":"<< dev_id << ", \n";
    ss  << "             \""<< myCfg.typeToServerStr(reg_group_type)<<"\":[";
    bool first_io_point = true;
    for (std::pair<const int, cfg::io_point_struct *> &offset_io_point :  io_point_map_lookup)
    {
        if(!first_io_point)
            ss << ",";
        ss << "\n";
        first_io_point = false;
        showIOPoint(offset_io_point.second, ss);
    }
    ss <<"\n                 ]\n";
    ss <<"            }";
}

/**
 * @brief Format the details of an io_point to a stringstream.
 * 
 * Includes id, name, offset, size, shift, scale, signed, and float.
 * 
 * @param io_point the io_point to unwrap
 * @param ss the string stream to format the details of the io_point to
*/
void showIOPoint(cfg::io_point_struct *io_point, std::stringstream &ss)
{
    std::string myuri = "/" + io_point->component->component_id + "/" + io_point->component->id;

    ss  << "                     {"
        << "\"id\":\""<< io_point->id << "\",   "
        << "\"name\":\""<< io_point->name<< "\",   "
        << "\"offset\":"<< io_point->offset<< ",   ";
    if(io_point->size > 1)
        ss << "\"size\":"<< io_point->size<< ",   ";
    if(io_point->shift > 0)
        ss << "\"shift\":"<< io_point->shift<< ",   ";
    if(io_point->scale > 0)
        ss << "\"scale\":"<< io_point->scale<< ",   ";
    if(io_point->is_signed)
        ss << "\"signed\":"<< "true"<< ",   ";
    if(io_point->is_float)
        ss << "\"float\":"<< "true"<< ",   ";
    ss << "\"uri\":\""<< myuri<< "\"   "
       << "}";
}

///////////////////////////////////////
///////// PREPARE WORK ITEMS///////////
///////////////////////////////////////

/**
 * @brief Compares the offsets of two io_points. If a->offset is less than b->offset, returns true.
 * If a->offset is greater than or equal to b->offset, returns false.
 * 
 * Also compares register type and device_id (in that order) prior to point offset.
 * 
 * @param a the first io_point
 * @param b the second io_point
*/
bool compareIOPointOffsets(const std::shared_ptr<cfg::io_point_struct> &a, const std::shared_ptr<cfg::io_point_struct> &b)
{
    // First, compare by register_type
    if (a->register_type < b->register_type)
        return true;
    else if (a->register_type > b->register_type)
        return false;
    // then by device_id
    if (a->device_id < b->device_id)
        return true;
    else if (a->device_id > b->device_id)
        return false;

    return a->offset < b->offset;
}

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
void check_work_items(std::vector<std::shared_ptr<IO_Work>> &io_work_vec, std::vector<std::shared_ptr<cfg::io_point_struct>> &io_map_vec, struct cfg &myCfg, const char *work_type, bool include_all_points, bool debug)
{
    // Sort the vector using the custom comparison function
    if (io_map_vec.size() == 0)
    {
        return;
    }
    #ifdef FPS_DEBUG_MODE
    if (debug)
    {
        std::cout
            << __func__ << " Starting io_work .. " << std::endl;
    }
    #endif
    std::string repto("");
    if (io_map_vec.size() > 1)
        std::sort(io_map_vec.begin(), io_map_vec.end(), compareIOPointOffsets);
    bool first = true;
    int offset = 0;
    int first_offset = 0;
    int device_id = 0;
    int num = 0;
    int isize = 0;
    bool local = false;
    auto io_points = io_map_vec;
    int max_item_size = myCfg.max_register_group_size; // adjust for bit
    int max_bit_size = myCfg.max_bit_size;             // adjust for bit
    cfg::Register_Types register_type;

    auto io_point = io_map_vec.at(0);
    auto io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, 1, nullptr, nullptr, strToWorkType(work_type, false));
    if ((io_point->register_type == cfg::Register_Types::Coil) || (io_point->register_type == cfg::Register_Types::Discrete_Input))
        max_item_size = max_bit_size;
    #ifdef FPS_DEBUG_MODE
    if (debug)
    {
        std::cout
            << " After Sort .. " << std::endl;
        for (auto io_point : io_points)
        {
            std::cout
                << "            .. "
                << " Id " << io_point->id
                << " device_id " << io_point->device_id
                << " offset " << io_point->offset
                << " size " << io_point->size
                << std::endl;
        }
    }
    #endif
    for (auto io_point : io_points)
    {
        if (!io_point->is_enabled && !io_point->is_forced && !include_all_points)
            continue;
        if (first)
        {
            first = false;
            offset = io_point->offset;
            first_offset = io_point->offset;
            device_id = io_point->device_id;
            num = io_point->size;
            isize = io_point->size;
            register_type = io_point->register_type;
            io_work->register_type = register_type;
            io_work->component = io_point->component;
            io_work->io_points.emplace_back(io_point);
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " >>>>>>>>>>>> state  #1 offset " << offset
                          << " isize " << isize
                          << " io_point->offset " << io_point->offset
                          << " io_point->size " << io_point->size
                          << std::endl;
            #endif
        }
        else
        {
            #ifdef FPS_DEBUG_MODE
            if (debug)
            {
                if (((offset + isize) != io_point->offset))
                {
                    std::cout << " >>>>>>>>>>>> Break detected  #1 offset " << offset
                              << " isize " << isize
                              << " io_point->offset " << io_point->offset
                              << std::endl;
                }
                if (((offset + io_point->size - first_offset > max_item_size)))
                {
                    std::cout << " >>>>>>>>>>>> Break detected  #2 offset " << offset
                              << " io_point->size " << io_point->size
                              << " max_item_size" << max_item_size
                              << std::endl;
                }
                if ((io_point->device_id != device_id))
                {
                    std::cout << " >>>>>>>>>>>> Break detected  #3 (device_id)"
                              << std::endl;
                }
                if ((io_point->register_type != register_type))
                {
                    if(0)std::cout << " >>>>>>>>>>>> Break detected  #4 (register_type)" << std::endl;
                }
                if (!io_point->is_enabled && io_point->is_forced)
                {
                    if(0)std::cout << " >>>>>>>>>>>> Break detected  #5 !enabled but forced, use local " << offset
                            << " isize " << isize
                            << " io_point->offset " << io_point->offset
                            << std::endl;
                }
            }
            #endif
            
            if (((offset + isize) != io_point->offset) || ((offset + io_point->size - first_offset > max_item_size)) || (io_point->device_id != device_id) || (io_point->register_type != register_type) || (!io_point->is_enabled && io_point->is_forced))
            {
                if ((!io_point->is_enabled && io_point->is_forced && !include_all_points))
                {
                    continue;
                }
                io_work->offset = first_offset;
                io_work->num_registers = num;
                io_work_vec.emplace_back(io_work);
                #ifdef FPS_DEBUG_MODE
                if (debug)
                    std::cout << " >>>>>>>>>>>> At break; offset : " << first_offset << " num : " << num << std::endl;
                #endif
                io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, 1, nullptr, nullptr, strToWorkType(work_type, false));
                offset = io_point->offset;
                first_offset = io_point->offset;
                device_id = io_point->device_id;
                num = io_point->size;
                isize = io_point->size;
                register_type = io_point->register_type;
                io_work->register_type = register_type;
                io_work->component = io_point->component;
                local = !io_point->is_enabled;
                io_work->io_points.emplace_back(io_point);
            }
            else
            {
                offset += isize;
                isize = io_point->size;
                num += io_point->size;
                io_work->io_points.emplace_back(io_point);
            }
        }
    }
    #ifdef FPS_DEBUG_MODE
    if (debug)
        std::cout << " >>>>>>>>>>>>>> After Check; offset : " << first_offset << " num : " << num << std::endl;
    #endif
    io_work->offset = first_offset;
    io_work->num_registers = num;
    io_work->local = local;

    io_work_vec.emplace_back(io_work);

    return;
}

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
void check_item_debounce(bool &enabled, std::shared_ptr<cfg::io_point_struct> io_point, bool debug)
{
    if (io_point->use_debounce)
    {
        double tNow = get_time_double();
        #ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << " io_point id " << io_point->id << " using_debounce" << std::endl;
        #endif
        if (io_point->debounce == 0.0) // && (io_point->debounce > 0.0) && (io_work->tIo > io_point->debounce_time))
        {
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " io_point id" << io_point->id << " debounce is zero " << std::endl;
            #endif
            io_point->use_debounce = false;
        }
        else if (io_point->debounce_time == 0.0)
        {
            io_point->debounce_time = (tNow + io_point->debounce);
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " debounce time set up  :" << io_point->debounce_time << std::endl;
            #endif
        }
        else
        {
            if (tNow > io_point->debounce_time)
            {
                if (debug)
                    std::cout << " debounce time   :" << io_point->debounce_time << " passed: tNow: " << tNow << std::endl;
                io_point->debounce_time += io_point->debounce;
            }
            else
            {
                if (debug)
                    std::cout << " still in debounce time :" << io_point->debounce_time << " tNow: " << tNow << std::endl;
                enabled = false;
            }
        }
    }
}

///////////////////////////////////////
////////// NOT LOOKED AT YET///////////
///////////////////////////////////////

/// @brief parses the body of an incoming fims message
/// this could be raw data or some kind of json object
bool gcom_parse_data(std::any &anyFimsMessageBody, const char *data, size_t length, bool debug)
{
    simdjson::dom::parser parser;
    // Convert u8* data to padded_string
    simdjson::padded_string padded_content(reinterpret_cast<const char *>(data), length);
    // if(debug)
    // {
    //     std::cout << " \"parse_data\": {" << std::endl;
    //     std::cout << "                 \"Data length\": " << length << std::endl;
    //     std::cout << "} " << std::endl;
    // }
    // Parse
    auto result = parser.parse(padded_content);
    if (result.error())
    {
        //std::string cjerr = 
        FPS_ERROR_LOG("input body [%s] ", data);
        FPS_ERROR_LOG("parser error [%s] ", simdjson::error_message(result.error()));

        //std::cout << "parser error ....\n";
        //std::cerr << simdjson::error_message(result.error()) << std::endl;
        return false;
    }
    if (debug)
        std::cout << "parser result OK ....\n";
    // Convert JSON to any
    // anyFimsMessageBody = jsonToMap(result.value());
    anyFimsMessageBody = jsonToAny(result.value());
    // Utility function to print nested maps
    if (debug)
    {
        //std::any &nonConstAnyVal = const_cast<std::any&>(anyFimsMessageBody);
        printAny(anyFimsMessageBody, 0);
    }
    if (anyFimsMessageBody.type() == typeid(std::map<std::string, std::any>))
    {
        if (debug)
        {
            std::cout << "parser result  map ....\n";
            auto mkey = std::any_cast<std::map<std::string, std::any>>(anyFimsMessageBody);
            for (auto key : mkey)
            {
                std::cout << " key " << key.first << "\n";
                // TODO create the IO_Work items
            }
        }
    }
    return true;
}

bool gcom_findCompVar(
    std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const cfg::component_struct *comp, std::string kvar)

{
    // std::string myvar = kvar;
    // return false;
    // auto myvar = uri_keys[key_idx + 2];
    // std::cout   << "         func : #1 "<< __func__ << " myVAR [" << myvar << "] \n";
    for (auto rgroup : comp->register_groups)
    {
        for (auto iop : rgroup->io_point_map)
        {
            if (iop->id == kvar)
            {
                // TODO use io_point
                io_point = iop;
                return true;
            }
        }
    }
    return false;
}

bool add_all_component_points_to_io_vec(std::vector<std::shared_ptr<cfg::io_point_struct>> &io_point_vec, const struct cfg &myCfg,
                                                                         const std::vector<std::string> &uri_keys, bool skip_disabled)
{
    bool debug = false;
    int key_idx = 0;
    if ((uri_keys.size() > 1) && (uri_keys[0].size() == 0))
        key_idx = 1;
    try
    {
        // Using at() for safe access. Catch exceptions if key not found
        auto &myComp = myCfg.component_io_point_map.at(uri_keys[key_idx]);
        // std::cout   << "         func : #2 "<< __func__ << "\n";
        auto &component_points = myComp.at(uri_keys[key_idx + 1]);
        // std::cout   << "         func : #3 "<< __func__ << "\n";
        for (std::pair<std::string, std::shared_ptr<cfg::io_point_struct>> io_point : component_points)
        {
            if (debug)
                std::cout << __func__ << " Item : " << io_point.first << " offset : " << io_point.second->offset << "\n";
            if (skip_disabled)
                if (!io_point.second->is_enabled)
                    continue;
            io_point_vec.emplace_back(io_point.second);
        }
        // map_result = register_group.at(myvar);
        return true;
    }
    catch (const std::out_of_range &)
    {
        // Key not found, return nullptr
        return false;
    }
    return false;
}

// Utility function to print nested maps
void printConstAny(const std::any &value, int indent)
{
    if (value.type() == typeid(std::map<std::string, std::any>))
    {
        std::cout << "\n";
        printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
    }
    else if (value.type() == typeid(int))
    {
        //std::cout << " type is int " << "\n";
        std::cout << std::any_cast<int>(value) << "\n";
    }
    else if (value.type() == typeid(int64_t))
    {
        std::cout << std::any_cast<int64_t>(value) << "\n";
    }
    else if (value.type() == typeid(double))
    {
        std::cout << std::any_cast<double>(value) << "\n";
    }
    else if (value.type() == typeid(std::string))
    {
        std::cout << std::any_cast<std::string>(value) << "\n";
    }
    else if (value.type() == typeid(std::any))
    {
        std::cout << " type is std::any " << "\n";
    }
    else if (value.type() == typeid(bool))
    {
        auto val = std::any_cast<bool>(value);
        if (val)
            std::cout << "true"
                      << "\n";
        else
            std::cout << "false"
                      << "\n";
    }
    else
    {
        std::cout << "type unknown"
                  << "\n";
    }
}
// Utility function to print nested maps
void printAny(std::any &value, int indent)
{
    if (value.type() == typeid(std::map<std::string, std::any>))
    {
        std::cout << "\n";
        printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
    }
    else if (value.type() == typeid(int))
    {
        std::cout << std::any_cast<int>(value) << "\n";
    }
    else if (value.type() == typeid(int64_t))
    {
        std::cout << std::any_cast<int64_t>(value) << "\n";
    }
    else if (value.type() == typeid(double))
    {
        std::cout << std::any_cast<double>(value) << "\n";
    }
    else if (value.type() == typeid(std::string))
    {
        std::cout << std::any_cast<std::string>(value) << "\n";
    }
    else if (value.type() == typeid(bool))
    {
        auto val = std::any_cast<bool>(value);
        if (val)
            std::cout << "true"
                      << "\n";
        else
            std::cout << "false"
                      << "\n";
    }
    else
    {
        std::cout << "type unknown"
                  << "\n";
    }
}

// ... (include the parseMessage function from the previous answer here)
//
// Utility function to print nested maps
void printMap(const std::map<std::string, std::any> &baseMap, int indent = 0)
{
    for (const auto &[key, value] : baseMap)
    {
        for (int i = 0; i < indent; ++i)
        {
            std::cout << "  ";
        }
        std::cout << key << ": ";
        if (value.type() == typeid(std::map<std::string, std::any>))
        {
            std::cout << "\n";
            printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
        }
        else if (value.type() == typeid(int))
        {
            std::cout << std::any_cast<int>(value) << "\n";
        }
        else if (value.type() == typeid(int64_t))
        {
            std::cout << std::any_cast<int64_t>(value) << "\n";
        }
        else if (value.type() == typeid(double))
        {
            std::cout << std::any_cast<double>(value) << "\n";
        }
        else if (value.type() == typeid(std::string))
        {
            std::cout << std::any_cast<std::string>(value) << "\n";
        }
        else if (value.type() == typeid(bool))
        {
            auto val = std::any_cast<bool>(value);
            if (val)
                std::cout << "true"
                          << "\n";
            else
                std::cout << "false"
                          << "\n";
        }
    }
}
