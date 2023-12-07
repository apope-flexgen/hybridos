// given a file name
// creates a
//      std::map<std::string, std::any>
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
////////// GLOBAL VARIABLES ///////////
///////////////////////////////////////


//constexpr size_t bufferSize = 4096 * 128;                            // Modify according to your needs
std::map<std::string, std::map<int, struct type_map>> types;         // map of type names / offsets to actual map component
std::map<std::string, std::map<std::string, struct type_map>> comps; // map of component names /  ids  to actual map components
std::vector<std::string> subs;
std::map<int, std::map<int, std::vector<std::shared_ptr<cfg::register_group_struct>>>> pubs;

bool gcom_test_bit_str(void);
void test_parse_message(const char *uri, const char *method, const char *body);
void test_merge_message(const char *uri, const char *method, const char *body);
bool gcom_points_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *decode);
bool gcom_point_type_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *ptype, const char *decode);
bool gcom_msg_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg);
bool gcom_config_test_uri(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *uri, const char *id);
bool uri_is_single(std::shared_ptr<cfg::io_point_struct> &io_point, struct cfg &myCfg, struct Uri_req &uri, bool debug);

void clearChan(bool debug);
bool decode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, 
                struct cfg &myCfg, Uri_req &uri, const char *mode, bool debug);
std::string extractCompFromURI(const std::string &uri);
std::string extractCompIdFromURI(const std::string &uri, const std::string &component);

struct type_map *gcom_get_comp(struct cfg &myCfg, std::string component, std::string id, bool debug = false);


//maybe only used in test
std::string getKeyFromURI(const std::string &uri);
std::map<std::string, std::any> parseMessage(const std::string &uri, const std::string method, const std::string &body);
int test_printMap(void);
std::string extractIdFromURI(const std::string &uri);
std::string getKeyFromURI(const std::string &uri);
void mergeSubMaps(std::map<std::string, std::any> &base, const std::map<std::string, std::any> &toMerge);
void mapToBuffer(const std::map<std::string, std::any> &baseMap, spdlog::memory_buf_t &buf, int indent = 0);
void printResultVector(const std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result);
Component_IO_point_map extract_structure(Component_IO_point_map &structure, const std::map<std::string, std::any> &jsonData);
void printComponentIOPointMap(const Component_IO_point_map &items);
void printIOPoint(const std::shared_ptr<cfg::io_point_struct> io_point);
void printPublishGroups(std::vector<std::shared_ptr<PublishGroup>> &publishGroups);
//struct type_map *gcom_get_comp(struct cfg &myCfg, std::string component, std::string id, bool debug = false);
std::string gcom_show_types(struct cfg &myCfg);
std::string gcom_show_FirstLevel(const std::map<std::string, std::any> &m, std::string key);
std::string gcom_show_subs(struct cfg &myCfg, bool debug = false);
//std::string gcom_show_pubs(struct cfg &myCfg, bool debug = false);
















bool compareIOPointOffsets(const std::shared_ptr<cfg::io_point_struct> &a, const std::shared_ptr<cfg::io_point_struct> &b);


// // ///////////////////////////////////////
// // ////////// UTILITY METHODS ////////////
// // ///////////////////////////////////////

// std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj)
// {
//     std::map<std::string, std::any> map;
//     for (auto [key, value] : obj)
//     {
//         map[std::string(key)] = jsonToAny(value);
//     }
//     return map;
// }

// std::any jsonToAny(simdjson::dom::element elem)
// {
//     switch (elem.type())
//     {
//     case simdjson::dom::element_type::INT64:
//     {
//         int64_t val;
//         if (elem.get(val) == simdjson::SUCCESS)
//         {
//             if (val <= INT32_MAX && val >= INT32_MIN)
//             {
//                 return static_cast<int32_t>(val);
//             }
//             else
//             {
//                 return val;
//             }
//         }
//         break;
//     }
//     case simdjson::dom::element_type::BOOL:
//     {
//         bool val;
//         if (elem.get(val) == simdjson::SUCCESS)
//         {
//             return val;
//         }
//         break;
//     }
//     case simdjson::dom::element_type::UINT64:
//     {
//         uint64_t val;
//         if (elem.get(val) == simdjson::SUCCESS)
//         {
//             if (val <= UINT32_MAX)
//             {
//                 return static_cast<uint32_t>(val);
//             }
//             else
//             {
//                 return val;
//             }
//         }
//         break;
//     }
//     case simdjson::dom::element_type::DOUBLE:
//     {
//         double val;
//         if (elem.get(val) == simdjson::SUCCESS)
//         {
//             return val;
//         }
//         break;
//     }
//     case simdjson::dom::element_type::STRING:
//     {
//         std::string_view sv;
//         if (elem.get(sv) == simdjson::SUCCESS)
//         {
//             return std::string(sv);
//         }
//         break;
//     }
//     case simdjson::dom::element_type::ARRAY:
//     {
//         simdjson::dom::array arr = elem;
//         std::vector<std::any> vec;
//         for (simdjson::dom::element child : arr)
//         {
//             vec.push_back(jsonToAny(child));
//         }
//         return vec;
//     }
//     case simdjson::dom::element_type::OBJECT:
//         return jsonToMap(elem);
//     default:
//         return std::any(); // Empty std::any
//     }
//     // If we reach here, something went wrong.
//     FPS_ERROR_LOG("Error processing JSON element.");
//     return std::any(); // Return empty std::any
// }

// std::string check_str_for_error(const std::string_view str, const std::string_view Forbidden_Chars = R"({}\/ "%)", const std::size_t Max_Str_Size = std::numeric_limits<u8>::max());
// // {
// //     if (str.empty())
// //     {
// //         return "string is empty";
// //     }
// //     if (str.find_first_of(Forbidden_Chars) != std::string_view::npos)
// //     {
// //         return fmt::format("string (currently: \"{}\") contains one of the forbidden characters: '{}'", str, fmt::join(Forbidden_Chars, "', '"));
// //     }
// //     if (str.size() > Max_Str_Size)
// //     {
// //         return fmt::format("string (currently: \"{}\", size: {}) has exceeded the maximum character limit of {}", str, str.size(), Max_Str_Size);
// //     }
// //     return "";
// // }

// // int hostname_to_ip(std::string_view hostname, char *ip, int iplen)
// // {
// //     struct hostent *he;
// //     struct in_addr **addr_list;
// //     if (he = gethostbyname(hostname.data()); he == NULL)
// //     {
// //         // get the host info
// //         return 1;
// //     }
// //     addr_list = (struct in_addr **)he->h_addr_list;
// //     for (int i = 0; addr_list[i] != NULL; i++)
// //     {
// //         // Return the first one;
// //         strncpy(ip, inet_ntoa(*addr_list[i]), iplen);
// //         return 0;
// //     }
// //     return 1;
// // }

// bool service_to_port(std::string_view service, int &port);
// // {
// //     struct servent *serv;
// //     /* getservbyname() - opens the etc.services file and returns the */
// //     /* values for the requested service and protocol.                */
// //     serv = getservbyname(service.data(), "tcp");
// //     if (serv == NULL)
// //     {
// //         FPS_INFO_LOG("port cannot be derived from service [%s] for protocol [tcp] (it doesn't exist), going back to port provided in config (or default)", service);
// //         return false;
// //     }
// //     port = ntohs(serv->s_port);
// //     return true;
// // }

// std::string hex_to_str(u16 *raw16)
// {
//     std::stringstream stream;
//     stream << "0x";
//     for (int i = 0; i < 4; i++)
//     {
//         stream << std::hex << raw16[i];
//     }
//     return stream.str();
// }

// std::string hex_to_str(u8 *raw8)
// {
//     std::stringstream stream;
//     stream << "0x" << std::hex << static_cast<u16>(raw8[0]);
//     return stream.str();
// }

// std::string hex_to_str(u64 raw64)
// {
//     std::stringstream stream;
//     stream << "0x" << std::hex << raw64;
//     return stream.str();
// }

// std::string hex_to_str(int raw_int)
// {
//     std::stringstream stream;
//     stream << "0x" << std::hex << raw_int;
//     return stream.str();
// }

// bool ioPointExists(
//     std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, std::string io_point_key = "")
// {
//     int key_idx = 0;
//     if ((uri_keys.size() > 1) && (uri_keys[0].size() == 0))
//         key_idx = 1;
//     std::string myvar = io_point_key;
//     if ((int)uri_keys.size() < (key_idx + 3))
//     {
//         myvar = io_point_key;
//     }
//     else
//     {
//         myvar = uri_keys[key_idx + 2];
//     }
//     // return false;
//     // auto myvar = uri_keys[key_idx + 2];
//     // std::cout   << "         func : #1 "<< __func__ << " myVAR [" << myvar << "] \n";
//     try
//     {
//         // Using at() for safe access. Catch exceptions if key not found
//         auto &myComp = myCfg.component_io_point_map.at(uri_keys[key_idx]);
//         // std::cout   << "         func : #2 "<< __func__ << "\n";
//         auto &register_group = myComp.at(uri_keys[key_idx + 1]);
//         // std::cout   << "         func : #3 "<< __func__ << "\n";
//         io_point = register_group.at(myvar);
//         return true;
//     }
//     catch (const std::out_of_range &)
//     {
//         // Key not found, return nullptr
//         return false;
//     }
//     return false;
// }

// ///////////////////////////////////////
// //////// CONFIG PARSING METHODS////////
// ///////////////////////////////////////

// bool gcom_load_cfg_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, struct cfg &myCfg, bool debug)
// {
//     bool ok = true;
//     ok = gcom_parse_file(jsonMapOfConfig, filename, false);
//     if (!ok)
//     {
//         FPS_ERROR_LOG("Unable to parse config file [%s]. Quitting.", filename);
//         return false;
//     }
//     // extract connection information into myCfg
//     ok = extract_connection(jsonMapOfConfig, "connection", myCfg, false);
//     if (!ok)
//     {
//         FPS_ERROR_LOG("Unable to extract connection information from [%s]. Quitting.", filename);
//         return false;
//     }
//     // pull out the components into myCfg
//     ok = extract_components(jsonMapOfConfig, "components", myCfg, false);
//     if (!ok)
//     {
//         FPS_ERROR_LOG("Unable to extract components from [%s]. Quitting.", filename);
//         return false;
//     }
// #ifdef FPS_DEBUG_MODE
//     if (debug)
//     {
//         printf(" >>>>>> components found \n");
//         for (auto &component : myCfg.component_io_point_map)
//         {
//             printf(" >>>>>>>>>>>>> <%s> component >> <%s> \n", __func__, component.first.c_str());
//         }
//     }
// #endif
//     FPS_INFO_LOG("Connection IP: [%s]. Port: [%d]", myCfg.connection.ip_address, myCfg.connection.port);
//     // now load up the components we have found
//     for (auto &component : myCfg.components)
//     {
// #ifdef FPS_DEBUG_MODE
//         if (debug)
//         {
//             std::cout
//                 << "component id: " << component->id
//                 << " device_id: " << component->device_id
//                 << " freq " << component->frequency
//                 << " offset_time :" << component->offset_time
//                 << std::endl;
//         }
// #endif

//         // add a subscription to this component
//         myCfg.addSub("components", component->id);

//         // this is the key to setting up a pub.
//         // the io_work framework is here
//         // add a pub system for this component
//         myCfg.addPub("components", component->id, component, &myCfg);

//         // we'll do HB and Wd at the component level for now
//         // GCOM allows a more liberal definition of these elements
//         myCfg.addWatchdog("components", component->id, component, &myCfg);
//         myCfg.addHeartbeat("components", component->id, component, &myCfg);

//         // now check register_group sizes and correct if needed
//         for (auto &register_group : component->register_groups)
//         {
//             checkNonZeroIOPointSize(register_group, register_group->io_point_map, false);

//             // TODO add next pointers
//             sortIOPointVectorByOffset(register_group->io_point_map);

//             int first_offset = getFirstOffset(register_group->io_point_map);
//             if (first_offset < 0)
//             {
//                 FPS_ERROR_LOG("Register group [%s] for component [%s] is empty.\n", register_group->register_type_str.c_str(), component->id.c_str());
//                 return false;
//             }

//             int total_size = getTotalNumRegisters(register_group->io_point_map);
//             if (total_size < 0)
//             {
//                 return false;
//             }

//             // maybe report config file error here
//             register_group->starting_offset = first_offset;
//             register_group->number_of_registers = total_size;
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//             {
//                 std::cout << "     register type: " << register_group->register_type_str
//                           << " offset: " << register_group->starting_offset
//                           << " num_regs: " << register_group->number_of_registers
//                           << " first_offset: " << first_offset
//                           << " total_size: " << total_size
//                           << std::endl;
//                 for (auto &mymap : register_group->io_point_map)
//                 {
//                     std::cout << "         map : " << mymap->id
//                               << " offset : " << mymap->offset
//                               << " size : " << mymap->size
//                               << std::endl;
//                 }
//             }
// #endif
//         }
//     }
//     myCfg.client_name = "my_client";
//     return true;
// }

// bool gcom_parse_file(std::map<std::string, std::any> &jsonMapOfConfig, const char *filename, bool debug)
// {
//     simdjson::dom::parser parser;
//     std::string fname(filename);
//     // Open file
//     std::ifstream file; //(fname, std::ios::in | std::ios::binary);
//     file = std::ifstream(fname, std::ios::in | std::ios::binary);
//     if (!file.is_open())
//     {
//         fname += ".json";
//         file = std::ifstream(fname, std::ios::in | std::ios::binary);
//     }
//     if (!file.is_open())
//     {
//         FPS_ERROR_LOG("Failed to open config file [%s].", filename);
//         return false;
//     }
//     // Read content into padded_string
//     std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     simdjson::padded_string padded_content = simdjson::padded_string(content);
//     if (debug)
//     {
//         std::cout << " \"parse_file\": {" << std::endl;
//         std::cout << "                 \"File name\": \"" << filename << "\"," << std::endl;
//         std::cout << "                 \"File length\": " << padded_content.size() << std::endl;
//         std::cout << "} " << std::endl;
//     }
//     // Parse
//     auto result = parser.parse(padded_content);
//     if (result.error())
//     {
//         FPS_ERROR_LOG("Error parsing json config: %s", simdjson::error_message(result.error()));
//         return false;
//     }
//     // Convert JSON to map
//     jsonMapOfConfig = jsonToMap(result.value());
//     return true;
// }

// bool extract_connection(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug = false)
// {
//     bool ok = true;
//     std::string error_message;
//     // connection name
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.name", myCfg.connection.name, std::string("modbus_client"), true, true, false);
//     error_message = check_str_for_error(myCfg.connection.name);
//     if (error_message.length() > 0)
//     {
//         FPS_INFO_LOG("Connection field \"name\" %s. Using default of \"modbus_client\".", error_message);
//         error_message = "";
//     }
//     // port
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.port", myCfg.connection.port, static_cast<int>(502), true, true, false);
//     if (ok && (myCfg.connection.port < 0 || myCfg.connection.port > std::numeric_limits<u16>::max()))
//     {
//         FPS_INFO_LOG("Port must be between 0 and %d. Configured value is [%d]. Using default of 502.", std::numeric_limits<u16>::max(), myCfg.connection.port);
//         myCfg.connection.port = 502;
//     }
//     // ip_address
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.ip_address", myCfg.connection.ip_address, std::string("172.3.0.2"), true, true, false);
//     if (ok && !myCfg.connection.ip_address.empty())
//     {
//         char new_ip[HOST_NAME_MAX + 1];
//         new_ip[HOST_NAME_MAX] = '\0';
//         auto ret = hostname_to_ip(myCfg.connection.ip_address, new_ip, HOST_NAME_MAX);
//         if (ret == 0)
//         {
//             myCfg.connection.ip_address = new_ip;
//         }
//         else
//         {
//             FPS_ERROR_LOG("ip_address \"%s\" isn't valid or can't be found from the local service file", myCfg.connection.ip_address);
//             ok = false;
//         }
//     }
//     // debug
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.debug", myCfg.connection.debug, false, true, true, false);
//     // connection_timeout
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.connection_timeout", myCfg.connection.connection_timeout, 2, true, true, false);
//     if (ok && (myCfg.connection.connection_timeout < 2 || myCfg.connection.connection_timeout > 10))
//     {
//         FPS_INFO_LOG("Connection timeout must be between 2 and 10 seconds. Configured value is [%d] seconds. Using default of 2s.", myCfg.connection.connection_timeout);
//         myCfg.connection.connection_timeout = 2;
//     }
//     // transfer_timeout
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.transfer_timeout", myCfg.connection.transfer_timeout, 500, true, true, false);
//     if (ok && (myCfg.connection.transfer_timeout < 10 || myCfg.connection.transfer_timeout > 2000))
//     {
//         FPS_INFO_LOG("Transfer timeout must be between 10 and 2000 milli seconds. Configured value is [%d] milliseconds. Using default of 500.", myCfg.connection.transfer_timeout);
//         myCfg.connection.transfer_timeout = 500;
//     }

//     // max_num_connections
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.max_num_connections", myCfg.connection.max_num_connections, 1, true, true, false);
//     if (ok && (myCfg.connection.max_num_connections <= 0 || myCfg.connection.max_num_connections > 25))
//     {
//         FPS_INFO_LOG("max_num_connections must be greater than 0. Configured value is [%d]. Using default of 1.", myCfg.connection.max_num_connections);
//         myCfg.connection.max_num_connections = 1;
//     }
//     // data_buffer_size
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.data_buffer_size", myCfg.connection.data_buffer_size, 100000, true, true, false);
//     if (ok && (myCfg.connection.data_buffer_size <= 10000 || myCfg.connection.data_buffer_size > 200000))
//     {
//         FPS_INFO_LOG("data_buffer_size must be between 10000 and 200000. Configured value is [%d]. Using default of 100000.", myCfg.connection.data_buffer_size);
//         myCfg.connection.data_buffer_size = 100000;
//     }

//     // auto_disable
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.auto_disable", myCfg.auto_disable, true, true, true, false);

//     // allow_multi_sets
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.allow_multi_sets", myCfg.allow_multi_sets, false, true, true, false);

//     // force_multi_sets
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.force_multi_sets", myCfg.force_multi_sets, false, true, true, false);

//     // max_register_group_size
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.max_register_group_size", myCfg.max_register_group_size, 125, true, true, false);
//     if (ok && myCfg.max_register_group_size <= 0)
//     {
//         FPS_INFO_LOG("max register size must be greater than 0. Configured value is [%d]. Using default of 125.", myCfg.max_register_group_size);
//         myCfg.max_register_group_size = 125;
//     }

//     // max_bit_size
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.max_bit_size", myCfg.max_bit_size, 125, true, true, false);
//     if (ok && myCfg.max_bit_size <= 0)
//     {
//         FPS_INFO_LOG("max bit size must be greater than 0. Configured value is [%d]. Using default of 125.", myCfg.max_bit_size);
//         myCfg.max_bit_size = 125;
//     }
//     // service
//     std::string service;
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.service", service, std::string(""), false, false, false);
//     if (ok && !service.empty())
//     {
//         ok = service_to_port(service, myCfg.connection.port);
//     }
//     // serial_device
//     bool is_RTU = false;
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.serial_device", myCfg.connection.device_name, std::string(""), false, false, false);
//     if (ok && !myCfg.connection.device_name.empty())
//     {
//         error_message = check_str_for_error(myCfg.connection.name);
//         if (error_message.length() > 0)
//         {
//             FPS_ERROR_LOG("Connection field \"serial_device\" %s.", error_message);
//             error_message = "";
//             ok = false;
//         }
//         else
//         {
//             is_RTU = true;
//         }
//     }
//     myCfg.connection.is_RTU = is_RTU;
//     // baud_rate
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.baud_rate", myCfg.connection.baud_rate, 115200, true, is_RTU, false);
//     if (ok && myCfg.connection.baud_rate < 0)
//     {
//         FPS_INFO_LOG("Baud rate cannot be less than 0. Current value is [%d]. Setting to 115200.", myCfg.connection.baud_rate);
//         myCfg.connection.baud_rate = 115200;
//     }
//     // parity
//     std::string parity("none");
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.parity", parity, parity, true, is_RTU, false);
//     if (ok && !parity.empty())
//     {
//         if (parity == "none")
//         {
//             myCfg.connection.parity = 'N';
//         }
//         else if (parity == "even")
//         {
//             myCfg.connection.parity = 'E';
//         }
//         else if (parity == "odd")
//         {
//             myCfg.connection.parity = 'O';
//         }
//         else
//         {
//             FPS_ERROR_LOG("parity (currently: \"%s\") must be one of \"none\", \"even\", or \"odd\". Setting to default of \"none\".", parity);
//             myCfg.connection.parity = 'N';
//         }
//     }
//     // data_bits
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.data_bits", myCfg.connection.data_bits, 8, true, is_RTU, false);
//     if (ok && (myCfg.connection.data_bits < 5 || myCfg.connection.data_bits > 8))
//     {
//         FPS_INFO_LOG("data_bits (currently: %d) must be between 5 and 8. Using default of 8.", myCfg.connection.data_bits);
//         myCfg.connection.data_bits = 8;
//     }
//     // stop_bits
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.stop_bits", myCfg.connection.stop_bits, 1, true, is_RTU, false);
//     if (ok && (!(myCfg.connection.stop_bits == 1 || myCfg.connection.stop_bits == 2)))
//     {
//         FPS_INFO_LOG("stop_bits (currently: %d) must be either 1 or 2. Using default of 1.", myCfg.connection.stop_bits);
//         myCfg.connection.stop_bits = 1;
//     }

//     ok &= getItemFromMap(jsonMapOfConfig, "connection.stats_pub_uri", myCfg.connection.stats_uri, std::string("/stats/modbus_client"), true, true, false);
//     if (myCfg.connection.stats_uri.length() > 0)
//     {
//         error_message = check_str_for_error(myCfg.connection.stats_uri, R"({}\ "%)");
//         if (error_message.length() > 0)
//         {
//             FPS_INFO_LOG("Connection field \"stats_pub_uri\" %s. Omitting stats pubs.", error_message);
//             myCfg.connection.stats_uri = "";
//             error_message = "";
//         }
//     }

//     ok &= getItemFromMap(jsonMapOfConfig, "connection.stats_pub_frequency", myCfg.connection.stats_frequency_ms, 1000, true, true, false);
//     if (ok && myCfg.connection.stats_uri.length() > 0 && myCfg.connection.stats_frequency_ms <= 0)
//     {
//         FPS_INFO_LOG("Connection field \"stats_pub_frequency\" must be greater than 0. Setting to default of 1000 ms.");
//         myCfg.connection.stats_frequency_ms = 1000;
//     }

//     // inherited stuff:
//     // device_id
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.device_id", myCfg.connection.device_id, 1, true, true, false);
//     // off_by_one
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.off_by_one", myCfg.inherited_fields.off_by_one, false, true, true, false);
//     // byte_swap
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.byte_swap", myCfg.inherited_fields.byte_swap, false, true, true, false);

//     // multi_write_op_code
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.multi_write_op_code", myCfg.inherited_fields.multi_write_op_code, false, true, true, false);

//     // frequency
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.frequency", myCfg.inherited_fields.frequency, 100, true, true, false);
//     // debounce
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.debounce", myCfg.inherited_fields.debounce, 0, true, true, false);

//     // format
//     std::string format_str;
//     ok &= getItemFromMap(jsonMapOfConfig, "connection.format", format_str, std::string(""), true, true, false);

//     if (format_str.compare("naked") == 0)
//     {
//         myCfg.format = Fims_Format::Naked;
//     }
//     else if (format_str.compare("clothed") == 0)
//     {
//         myCfg.format = Fims_Format::Clothed;
//     }
//     else if (format_str.compare("full") == 0)
//     {
//         myCfg.format = Fims_Format::Full;
//     }
//     else
//     {
//         myCfg.format = Fims_Format::Naked;
//     }

//     if (debug)
//     {
//         printf(" >>>>>>>>>>>>> <%s>  device_id %d\n", __func__, myCfg.connection.device_id);
//     }
//     return ok;
// }

// // this is the big unpacker
// //  components
// //     ->  register_groups (extract_register_groups)
// //           -> io_point_map (extract_io_point_map)
// //                ->
// bool extract_components(std::map<std::string, std::any> jsonMapOfConfig, const std::string &query, struct cfg &myCfg, bool debug = false)
// {
//     bool ok = true;
//     std::optional<std::vector<std::any>> compArray = getMapValue<std::vector<std::any>>(jsonMapOfConfig, query);
//     if (compArray.has_value())
//     {
//         if (debug)
//             std::cout << " Found components " << std::endl;
//     }
//     else
//     {
//         FPS_ERROR_LOG("Could not find \"components\" in config file.");
//         return false;
//     }
//     std::vector<std::any> rawCompArray = compArray.value();
//     for (const std::any &rawComp : rawCompArray)
//     {
//         if (debug)
//             std::cout << " Processing component" << std::endl;
//         if (rawComp.type() == typeid(std::map<std::string, std::any>))
//         {
//             std::map<std::string, std::any> jsonComponentMap = std::any_cast<std::map<std::string, std::any>>(rawComp);
//             std::shared_ptr<cfg::component_struct> component = std::make_shared<cfg::component_struct>();
//             std::string componentId;
//             getItemFromMap(jsonComponentMap, "component_id", componentId, std::string("components"), true, true, false);
//             component->component_id = componentId;
//             component->myCfg = &myCfg;

//             // ok &= getItemFromMap(jsonComponentMap, "name",                component->name,           std::string("noName"), true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "serial_device",       component->serial_device,  std::string("none"),   true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "parity",              component->parity,         std::string("none"),   true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "data_bits",           component->data_bits,      8,                     true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "stop_bits",           component->stop_bits,      1,                     true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "baud_rate",           component->baud_rate,      115200,                true, true, debug);

//             // ok &= getItemFromMap(jsonComponentMap, "multi_write_op_code", component->multi_write_op_code, myCfg.inherited_fields.multi_write_op_code,            true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "off_by_one",          component->off_by_one,          false,            true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "id", component->id, std::string("noId"), true, true, false);
//             ok &= getItemFromMap(jsonComponentMap, "device_id", component->device_id, myCfg.connection.device_id, true, true, false);
//             ok &= getItemFromMap(jsonComponentMap, "frequency", component->frequency, myCfg.inherited_fields.frequency, true, true, false);
//             ok &= getItemFromMap(jsonComponentMap, "offset_time", component->offset_time, 0, true, true, false);
//             ok &= getItemFromMap(jsonComponentMap, "byte_swap", component->is_byte_swap, myCfg.inherited_fields.byte_swap, true, true, debug);
//             // ok &= getItemFromMap(jsonComponentMap, "off_by_one",          component->off_by_one,   myCfg.inherited_fields.off_by_one, true, true, debug);

//             ok &= getItemFromMap(jsonComponentMap, "pub_sync",                       myCfg.pub_sync, true, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "heartbeat_enabled",              component->heartbeat_enabled, false, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_enabled",               component->watchdog_enabled, false, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_read_uri",   component->component_heartbeat_read_uri, std::string(""), true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_write_uri",  component->component_heartbeat_write_uri, std::string(""), true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_max_value",  component->component_heartbeat_max_value, 4096, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "modbus_heartbeat_timeout_ms",    component->modbus_heartbeat_timeout_ms, 0, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "component_heartbeat_timeout_ms", component->component_heartbeat_timeout_ms, 0, true, true, debug);

//             ok &= getItemFromMap(jsonComponentMap, "watchdog_uri", component->watchdog_uri, std::string(""), true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_alarm_timeout_ms", component->watchdog_alarm_timeout, 0, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_fault_timeout_ms", component->watchdog_fault_timeout, 0, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_recovery_timeout_ms", component->watchdog_recovery_timeout, 0, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_recovery_time_ms", component->watchdog_time_to_recover, 0, true, true, debug);
//             ok &= getItemFromMap(jsonComponentMap, "watchdog_frequency_ms", component->watchdog_frequency, 1000, true, true, debug);

//             // I think word_swap is actually the same as byte_swap?
//             int word_order = 0;
//             ok &= getItemFromMap(jsonComponentMap, "word_swap", component->is_byte_swap, false, true, true, debug);

//             ok &= getItemFromMap(jsonComponentMap, "word_order", component->word_order, word_order, true, true, debug);

//             std::string format_str;
//             ok &= getItemFromMap(jsonComponentMap, "format", format_str, std::string(""), true, true, false);

//             if (format_str.compare("naked") == 0)
//             {
//                 component->format = Fims_Format::Naked;
//             }
//             else if (format_str.compare("clothed") == 0)
//             {
//                 component->format = Fims_Format::Clothed;
//             }
//             else if (format_str.compare("full") == 0)
//             {
//                 component->format = Fims_Format::Full;
//             }
//             else
//             {
//                 component->format = myCfg.format;
//             }

//             std::map<std::string, std::any>::iterator reg_group_it = jsonComponentMap.find("registers");
//             if (reg_group_it != jsonComponentMap.end())
//             {
//                 if (debug)
//                     std::cout << " Processing  registers" << std::endl;
//                 ok &= extract_register_groups(component->register_groups, reg_group_it->second, component, myCfg, debug);
//             }
//             else
//             {
//                 ok = false;
//             }
//             if (ok)
//                 myCfg.components.push_back(component);
//         }
//     }
//     if (debug)
//     {
//         printf(" >>>>>>>>>>>>> <%s> done \n", __func__);
//         for (auto &myComp : myCfg.component_io_point_map)
//         {
//             printf(" >>>>>>>>>>>>> <%s> component >> <%s> \n", __func__, myComp.first.c_str());
//         }
//     }
//     return ok;
// }

// bool extract_register_groups(std::vector<std::shared_ptr<cfg::register_group_struct>> &register_groups, const std::any &raw_register_groups_json_array, std::shared_ptr<struct cfg::component_struct> component, struct cfg &myCfg, bool debug = false)
// {
//     bool ok = true;
//     // initialize component
//     if (raw_register_groups_json_array.type() == typeid(std::vector<std::any>))
//     {
//         std::vector<std::any> raw_register_groups = std::any_cast<std::vector<std::any>>(raw_register_groups_json_array);
//         for (const std::any &raw_register_group : raw_register_groups)
//         {
//             if (raw_register_group.type() == typeid(std::map<std::string, std::any>))
//             {
//                 std::map<std::string, std::any> jsonRegisterGroupMap = std::any_cast<std::map<std::string, std::any>>(raw_register_group);
//                 std::shared_ptr<cfg::register_group_struct> register_group = std::make_shared<cfg::register_group_struct>();
//                 register_group->multi_write_op_code = false; // component->multi_write_op_code;
//                 register_group->component = component;
//                 register_group->id = component->id;
//                 register_group->component_id = component->component_id;
//                 if (debug)
//                     printf(" >>>>>>>>>>>>> <%s> component %p  initial register_group %p \n", __func__, (void *)component.get(), (void *)register_group.get());
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "type", register_group->register_type_str, std::string("type"), true, true, debug);
//                 // now we get modbus specific
//                 register_group->register_type = myCfg.typeFromStr(register_group->register_type_str);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "starting_offset", register_group->starting_offset, 0, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "number_of_registers", register_group->number_of_registers, 0, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "device_id", register_group->device_id, component->device_id, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "byte_swap", register_group->is_byte_swap, component->is_byte_swap, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "word_swap", register_group->is_byte_swap, component->is_byte_swap, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "word_order", register_group->word_order, component->word_order, true, true, debug);
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "multi_write_op_code", register_group->multi_write_op_code, false, true, true, debug);
//                 std::string format_str;
//                 ok &= getItemFromMap(jsonRegisterGroupMap, "format", format_str, std::string(""), true, true, false);

//                 if (format_str.compare("naked") == 0)
//                 {
//                     register_group->format = Fims_Format::Naked;
//                 }
//                 else if (format_str.compare("clothed") == 0)
//                 {
//                     register_group->format = Fims_Format::Clothed;
//                 }
//                 else if (format_str.compare("full") == 0)
//                 {
//                     register_group->format = Fims_Format::Full;
//                 }
//                 else
//                 {
//                     register_group->format = component->format;
//                 }

//                 // Extract register details (like type, starting_offset, etc.) here...
//                 // Call extract_io_point_map for each register to extract map details...
//                 if (ok)
//                 {
//                     // register_group.io_point_map = extract_io_point_map(&register_group, jsonRegisterGroupMap["map"], myCfg);
//                     //  Add register details + extracted io_point_map to the register_groups vector...
//                     register_groups.push_back(register_group);
//                     // register_groups.emplace_back(register_group);
//                     std::shared_ptr<cfg::register_group_struct> &register_group = register_groups.back();
//                     // register_group->component = component;
//                     extract_io_point_map(register_group, nullptr, jsonRegisterGroupMap["map"], myCfg, debug);
//                     if (debug)
//                         printf(" >>>>>>>>>>>>> <%s> component %p  register_group %p  device_id %d \n", __func__, (void *)component.get(), (void *)&register_group, register_group->device_id);
//                 }
//             }
//         }
//     }
//     return ok;
// }

// bool extract_io_point_map(std::shared_ptr<struct cfg::register_group_struct> register_group, std::shared_ptr<cfg::io_point_struct> packed_io_point, std::any &rawIOPointList, struct cfg &myCfg, bool debug = false)
// {
//     // did we get a vector of io_points
//     if (rawIOPointList.type() == typeid(std::vector<std::any>))
//     {
//         std::vector<std::any> rawIOPoints = std::any_cast<std::vector<std::any>>(rawIOPointList);
//         for (const std::any &rawIOPoint : rawIOPoints)
//         {
//             if (rawIOPoint.type() == typeid(std::map<std::string, std::any>))
//             {
//                 std::map<std::string, std::any> json_io_point = std::any_cast<std::map<std::string, std::any>>(rawIOPoint);
//                 // struct cfg::io_point_struct map;
//                 std::shared_ptr<cfg::io_point_struct> io_point = std::make_shared<cfg::io_point_struct>();
//                 io_point->register_type = register_group->register_type;
//                 io_point->register_group = register_group;
//                 io_point->component = register_group->component;
//                 io_point->multi_write_op_code = register_group->multi_write_op_code;
//                 io_point->size = 1;
//                 // io_point.register_type = register_group->register_type;
//                 // int starting_bit_pos;
//                 // int shift;
//                 // double scale;
//                 // bool normal_set;
//                 // bool is_float;
//                 // "multi_write_op_code": true
//                 double scale = 0.0;
//                 io_point->scale = scale;
//                 io_point->register_type_str = register_group->register_type_str;
//                 std::shared_ptr<cfg::io_point_struct> parent_io_point = io_point;
//                 if (packed_io_point)
//                     parent_io_point = packed_io_point;
//                 getItemFromMap(json_io_point, "id", io_point->id, std::string("Some_id"), true, true, debug);
//                 getItemFromMap(json_io_point, "name", io_point->name, std::string("Some_name"), true, true, debug);
//                 getItemFromMap(json_io_point, "offset", io_point->offset, parent_io_point->offset, true, true, debug);
//                 getItemFromMap(json_io_point, "size", io_point->size, parent_io_point->size, true, true, debug);
//                 getItemFromMap(json_io_point, "multi_write_op_code", io_point->multi_write_op_code, parent_io_point->multi_write_op_code, true, true, debug);
//                 // getItemFromMap(json_io_point, "off_by_one",          io_point->off_by_one, myCfg.inherited_fields.off_by_one, true, true, debug);

//                 // set up default
//                 io_point->number_of_bits = io_point->size * 16;
//                 getItemFromMap(json_io_point, "shift", io_point->shift, parent_io_point->shift, true, true, debug);
//                 getItemFromMap(json_io_point, "starting_bit_pos", io_point->starting_bit_pos, parent_io_point->starting_bit_pos, true, true, debug);
//                 getItemFromMap(json_io_point, "number_of_bits", io_point->number_of_bits, parent_io_point->number_of_bits, true, true, debug);
//                 io_point->bit_mask = (io_point->number_of_bits * io_point->number_of_bits) - 1;
//                 getItemFromMap(json_io_point, "scale", io_point->scale, parent_io_point->scale, true, true, debug);
//                 getItemFromMap(json_io_point, "normal_set", io_point->normal_set, parent_io_point->normal_set, true, true, debug);
//                 getItemFromMap(json_io_point, "signed", io_point->is_signed, parent_io_point->is_signed, true, true, debug);
//                 std::string format_str;
//                 getItemFromMap(json_io_point, "format", format_str, std::string(""), true, true, false);

//                 if (format_str.compare("naked") == 0)
//                 {
//                     io_point->format = Fims_Format::Naked;
//                 }
//                 else if (format_str.compare("clothed") == 0)
//                 {
//                     io_point->format = Fims_Format::Clothed;
//                 }
//                 else if (format_str.compare("full") == 0)
//                 {
//                     io_point->format = Fims_Format::Full;
//                 }
//                 else
//                 {
//                     if (packed_io_point)
//                     {
//                         io_point->format = parent_io_point->format;
//                     }
//                     else
//                     {
//                         io_point->format = register_group->format;
//                     }
//                 }
//                 io_point->invert_mask = 0;
//                 io_point->care_mask = std::numeric_limits<u64>::max();
//                 std::string invert_mask_str("0x0");
//                 std::string default_invert_mask_str("0x0");
//                 io_point->invert_mask = 0;
//                 io_point->invert_mask = parent_io_point->invert_mask;
//                 getItemFromMap(json_io_point, "invert_mask", invert_mask_str, default_invert_mask_str, true, true, debug);
//                 if (invert_mask_str != "0x0")
//                 {
//                     uint64_t invmask = 0;
//                     const char *istr = invert_mask_str.c_str();
//                     if (invert_mask_str.length() > 1 && (istr[0] == '0') && (istr[1] == 'x'))
//                     {
//                         istr += 2;
//                         invmask = strtoull(istr, NULL, 16);
//                     }
//                     else if ((invert_mask_str[0] == '0') && (invert_mask_str[1] == 'b'))
//                     {
//                         istr += 2;
//                         invmask = strtoull(istr, NULL, 2);
//                     }
//                     io_point->invert_mask = invmask;
//                 }
//                 // Extract io_point details (like id, offset, name, etc.) here...
//                 // Add to the io_point_map vector...
//                 io_point->is_float = false;
//                 io_point->is_byte_swap = false;
//                 io_point->is_byte_swap = false;
//                 getItemFromMap(json_io_point, "float", io_point->is_float, parent_io_point->is_float, true, true, debug);
//                 getItemFromMap(json_io_point, "word_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, true, true, debug);
//                 getItemFromMap(json_io_point, "word_order", io_point->word_order, parent_io_point->word_order, true, true, debug);
//                 getItemFromMap(json_io_point, "byte_swap", io_point->is_byte_swap, parent_io_point->is_byte_swap, true, true, debug);

//                 // for the benefit of size 4 regs
//                 // TODO remove word_swap
//                 if (io_point->is_byte_swap)
//                 {
//                     io_point->byte_index[0] = 3;
//                     io_point->byte_index[1] = 2;
//                     io_point->byte_index[2] = 1;
//                     io_point->byte_index[3] = 0;
//                 }
//                 else
//                 {
//                     io_point->byte_index[0] = 0;
//                     io_point->byte_index[1] = 1;
//                     io_point->byte_index[2] = 2;
//                     io_point->byte_index[3] = 3;
//                 }
//                 // word_order is 1234 or 4321 or 1324 etc
//                 if (io_point->word_order > 0)
//                 {
//                     io_point->byte_index[0] = (io_point->word_order / 1000) - 1;
//                     io_point->byte_index[1] = ((io_point->word_order % 1000) / 100) - 1;
//                     io_point->byte_index[2] = ((io_point->word_order % 100) / 10) - 1;
//                     io_point->byte_index[3] = (io_point->word_order % 10) - 1;
//                 }

//                 io_point->is_enum = false;
//                 io_point->is_random_enum = false;
//                 io_point->is_individual_bits = false;
//                 io_point->is_bit_field = false;
//                 getItemFromMap(json_io_point, "enum", io_point->is_enum, parent_io_point->is_enum, true, true, debug);
//                 getItemFromMap(json_io_point, "random_enum", io_point->is_random_enum, parent_io_point->is_random_enum, true, true, debug);
//                 getItemFromMap(json_io_point, "individual_bits", io_point->is_individual_bits, parent_io_point->is_individual_bits, true, true, debug);
//                 getItemFromMap(json_io_point, "bit_field", io_point->is_bit_field, io_point->is_bit_field, true, true, debug);
//                 double dval = 0.0;
//                 io_point->debounce = dval;
//                 io_point->deadband = dval;
//                 io_point->use_bool = false;
//                 getItemFromMap(json_io_point, "debounce", io_point->debounce, parent_io_point->debounce, true, true, debug);
//                 getItemFromMap(json_io_point, "deadband", io_point->deadband, parent_io_point->deadband, true, true, debug);
//                 if (io_point->debounce > 0.0)
//                     io_point->use_debounce = true;
//                 if (io_point->deadband > 0.0)
//                     io_point->use_deadband = true;
//                 getItemFromMap(json_io_point, "use_bool", io_point->use_bool, parent_io_point->use_bool, true, true, debug);
//                 if ((io_point->is_enum) || (io_point->is_random_enum) || (io_point->is_individual_bits) || (io_point->is_bit_field)
//                     /* maybe add more here */)
//                 {
//                     extract_bitstrings(io_point, json_io_point["bit_strings"]);
//                 }
//                 io_point->forced_val = 0;
//                 io_point->raw_val = 0;
//                 io_point->device_id = register_group->device_id;
//                 io_point->packer = packed_io_point;
//                 if (!packed_io_point)
//                 {
//                     register_group->io_point_map.push_back(io_point);
//                     if (debug)
//                         printf(" mapping register id [%s]\n", io_point->id.c_str());
//                     auto &io_point_map = register_group->io_point_map.back();
//                     // auto regshr = mymap->register_group.lock();
//                     register_group->io_point_map_lookup[io_point_map->offset] = io_point_map;
//                 }
//                 else
//                 {
//                     if (debug)
//                         printf(" packing register id [%s]\n", io_point->id.c_str());
//                     packed_io_point->bit_ranges.push_back(io_point);
//                 }
//                 // we only want the root registers in the main dict
//                 // if(!packed_io_point)
//                 // this will also add the packed_io_point items to the io_point so that they can be found with sets or gets
//                 addIOPointToComponentIOPointMap(myCfg.component_io_point_map, register_group->component_id.c_str(), register_group->id.c_str(), io_point);
//                 // addMapId(myCfg.idMap, device_id, io_point.register_type, mymap);
//                 getItemFromMap(json_io_point, "packed_register", io_point->packed_register, false, true, true, debug);
//                 if (io_point->packed_register && !packed_io_point)
//                 {
//                     extract_io_point_map(register_group, io_point, json_io_point["bit_ranges"], myCfg, debug);
//                 }
//                 if (debug)
//                     printf(" >>>>>>>>>>>>> <%s> component [%s] register_group [%s] id [%s] packed_io_point [%d] struct size %d \n", __func__,
//                            register_group->component_id.c_str(), register_group->id.c_str(), io_point->id.c_str(), (int)io_point->packed_register, (int)sizeof(cfg::io_point_struct));
//                 // pull out
//             }
//         }
//     }
//     return true;
// }

// void extract_bitstrings(std::shared_ptr<cfg::io_point_struct> io_point, std::any &rawStringData)
// {
//     // Ensure io_point is not nullptr
//     if (!io_point)
//     {
//         throw std::runtime_error("io_point pointer is null!");
//     }
//     io_point->bits_known = 0;
//     io_point->bits_unknown = 0;
//     if (rawStringData.type() == typeid(std::vector<std::any>))
//     {
//         std::vector<std::any> rawDataList = std::any_cast<std::vector<std::any>>(rawStringData);
//         auto bit_num = 0;
//         for (const std::any &rawData : rawDataList)
//         {
//             // Handle simple string
//             if (rawData.type() == typeid(std::string))
//             {
//                 std::string stringData = std::any_cast<std::string>(rawData);
//                 io_point->bit_str.push_back(stringData);
//                 io_point->bit_str_num.push_back(bit_num);
//             }
//             // Handle object with "value" and "string"
//             else if (rawData.type() == typeid(std::map<std::string, std::any>))
//             {
//                 std::map<std::string, std::any> rawMap = std::any_cast<std::map<std::string, std::any>>(rawData);
//                 if (rawMap.find("string") != rawMap.end() && rawMap["string"].type() == typeid(std::string))
//                 {
//                     io_point->bit_str.push_back(std::any_cast<std::string>(rawMap["string"]));
//                 }
//                 if (rawMap.find("value") != rawMap.end() && rawMap["value"].type() == typeid(int))
//                 {
//                     bit_num = std::any_cast<int>(rawMap["value"]);
//                     io_point->bit_str_num.push_back(bit_num);
//                 }
//                 else
//                 {
//                     io_point->bit_str_num.push_back(bit_num);
//                 }
//             }
//             else if (rawData.type() == typeid(void))
//             {
//                 io_point->bit_str.push_back("");
//                 io_point->bit_str_num.push_back(bit_num);
//                 io_point->bits_unknown |= 1 << bit_num;
//             }
//             else
//             {
//                 std::string err = io_point->id;
//                 err += ": unknown bitstring type :";
//                 err += anyTypeString(rawData);
//                 throw std::runtime_error(err);
//             }
//             io_point->bits_known |= 1 << bit_num;
//             bit_num++;
//         }
//     }
//     return;
// }

// void checkNonZeroIOPointSize(std::shared_ptr<cfg::register_group_struct> register_group, std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map, bool debug = false)
// {
//     for (size_t i = 0; i < io_point_map.size(); ++i)
//     {
//         if (!io_point_map[i]->packed_register)
//         {
//             if (debug)
//                 printf(">>>> %s setting [%d] io_point_map_lookup %d to map %p id %s\n", __func__, (int)i, io_point_map[i]->offset, (void *)io_point_map[i].get(), io_point_map[i]->id.c_str());

//             if (io_point_map[i]->size == 0)
//                 io_point_map[i]->size = 1;
//         }
//     }
// }

// void sortIOPointVectorByOffset(std::vector<std::shared_ptr<cfg::io_point_struct>> &elements)
// {
//     if (elements.size() > 1)
//     {
//         // TODO check then both compares are the same
//         //std::sort(elements.begin(), elements.end(), compareIOPointOffsets);

//         std::sort(elements.begin(), elements.end(),
//               [](const std::shared_ptr<cfg::io_point_struct> a, const std::shared_ptr<cfg::io_point_struct> b) -> bool
//               {
//                   return cfg::io_point_struct::compare(*a, *b); // Dereference the shared_ptr here
//               });
//     }
// }

// int getFirstOffset(const std::vector<std::shared_ptr<cfg::io_point_struct>> elements)
// {
//     if (elements.empty())
//     {
//         return -1;
//     }
//     return elements[0]->offset;
// }

// int getTotalNumRegisters(std::vector<std::shared_ptr<cfg::io_point_struct>> io_point_map)
// {
//     if (io_point_map.empty())
//     {
//         FPS_ERROR_LOG("Error: io_point_map vector is empty.");
//         return 0; // or handle error appropriately
//     }
//     int total_size = io_point_map[0]->size;
//     for (size_t i = 1; i < io_point_map.size(); ++i)
//     {
//         auto unpacked = io_point_map[i];
//         //FPS_INFO_LOG("Checking Register [%s]: offset [%d] size [%d]", unpacked->id.c_str(), unpacked->offset,unpacked->size);
//         unpacked->next.reset();
//         if (!unpacked->packed_register)
//         {
//             total_size += unpacked->size;
//             if (unpacked->offset > io_point_map[i - 1]->offset + io_point_map[i - 1]->size)
//             {
//                 FPS_INFO_LOG("Register size gap at Offset [%d]: [%s]", io_point_map[i - 1]->offset, io_point_map[i - 1]->id.c_str());
//             }
//             else if (unpacked->offset < io_point_map[i - 1]->offset + io_point_map[i - 1]->size)
//             {
//                 FPS_ERROR_LOG("Overlapping registers at Offset [%d]: [%s] (size %d) and Offset [%d]: [%s] (size %d)"
//                         , io_point_map[i - 1]->offset, io_point_map[i - 1]->id.c_str(), io_point_map[i - 1]->size
//                         , unpacked->offset, unpacked->id.c_str(), unpacked->size);
//                 // TODO stop on invalid config...
//                 //return -1;
//             }
//         }
//         io_point_map[i - 1]->next = unpacked; // setting the 'next' pointer to the next shared_ptr
//     }
//     return total_size;
// }

// void resetCfg(struct cfg &cfgInstance)
// {
//     // Create a new instance of cfg with default values
//     struct cfg newCfg;

//     // Assign the newCfg to the existing cfgInstance
//     cfgInstance = newCfg;
// }

// void addIOPointToComponentIOPointMap(Component_IO_point_map &imap, const char *component_uri_prefix, const char *component_id, std::shared_ptr<cfg::io_point_struct> io_point)
// {
//     if (io_point)
//     {
//         if (imap.find(component_uri_prefix) == imap.end())
//         {
//             imap[component_uri_prefix] = std::map<std::string, std::map<std::string, std::shared_ptr<cfg::io_point_struct>>>();
//         }
//         auto &component_map = imap[component_uri_prefix];
//         if (component_map.find(component_id) == component_map.end())
//         {
//             component_map[component_id] = std::map<std::string, std::shared_ptr<cfg::io_point_struct>>();
//         }
//         auto &point_map = component_map[component_id];
//         point_map[io_point->id] = io_point;
//     }
//     else
//     {
//         FPS_INFO_LOG("Cannot add nullptr to component IO map.\n");
//     }
// }

// ///////////////////////////////////////
// /////////// ENCODE/DECODE /////////////
// ///////////////////////////////////////

// uint8_t get_any_to_bool(std::shared_ptr<cfg::io_point_struct> io_point, std::any val, Uri_req &uri, uint8_t *regs8)
// {
//     // if (uri.is_unforce_request)
//     // {
//     //     io_point->is_forced = false;
//     // }
//     // if (uri.is_enable_request)
//     // {
//     //     io_point->is_enabled = true;
//     // }
//     // if (uri.is_disable_request)
//     // {
//     //     io_point->is_enabled = false;
//     // }
//     uint8_t bool_intval = 0;
//     if (val.type() == typeid(std::map<std::string, std::any>))
//     {
//         auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
//         if (clothed_val.find("value") != clothed_val.end())
//         {
//             val = clothed_val["value"];
//         }
//     }
//     if (val.type() == typeid(bool))
//     {
//         if (std::any_cast<bool>(val))
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(int64_t))
//     {
//         if (std::any_cast<int64_t>(val) > 0)
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(int32_t))
//     {
//         if (std::any_cast<int32_t>(val) > 0)
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(uint32_t))
//     {
//         if (std::any_cast<uint32_t>(val) > 0)
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(uint64_t))
//     {
//         if (std::any_cast<uint64_t>(val) > 0)
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(std::string))
//     {
//         if (std::any_cast<std::string>(val) == "true")
//             bool_intval = 1;
//     }
//     else if (val.type() == typeid(double))
//     {
//         if (std::any_cast<double>(val) > 0)
//             bool_intval = 1;
//     }
//     if (io_point->scale == -1)
//         bool_intval ^= 1;
//     // if (uri.is_force_request)
//     // {
//     //     io_point->is_forced = true;
//     //     io_point->forced_val = static_cast<uint64_t>(bool_intval);
//     // }
//     if (io_point->is_forced)
//     {
//         bool_intval = static_cast<uint8_t>(io_point->forced_val);
//     }
//     regs8[0] = bool_intval;
//     return bool_intval;
// }

// // put the correct sized u64 val into the regs return the size
// int set_reg16_from_uint64(struct cfg &myCfg, std::shared_ptr<cfg::io_point_struct> io_point, uint64_t &uval, uint16_t *regs16)
// {
//     if (io_point->size == 1)
//     {
//         regs16[0] = static_cast<uint16_t>(uval);
//     }
//     else if (io_point->size == 2)
//     {
//         if (!io_point->is_byte_swap)
//         {
//             regs16[0] = static_cast<uint16_t>(uval >> 16);
//             regs16[1] = static_cast<uint16_t>(uval >> 0);
//         }
//         else
//         {
//             regs16[0] = static_cast<uint16_t>(uval >> 0);
//             regs16[1] = static_cast<uint16_t>(uval >> 16);
//         }
//     }
//     else if (io_point->size == 4)
//     {
//         if (!io_point->is_byte_swap)
//         {
//             regs16[0] = static_cast<uint16_t>(uval >> 48);
//             regs16[1] = static_cast<uint16_t>(uval >> 32);
//             regs16[2] = static_cast<uint16_t>(uval >> 16);
//             regs16[3] = static_cast<uint16_t>(uval >> 0);
//             // memcpy(&tval, regs16, sizeof(tval));
//             // printf( " sent regs16 #1 0x%08lx  \n", tval);
//             // printf( " sent u64val   #1 0x%08lx  \n", u64val);
//         }
//         else
//         {
//             regs16[3] = static_cast<uint16_t>(uval >> 48);
//             regs16[2] = static_cast<uint16_t>(uval >> 32);
//             regs16[1] = static_cast<uint16_t>(uval >> 16);
//             regs16[0] = static_cast<uint16_t>(uval >> 0);
//         }
//     }

//     return io_point->size;
// }

// /// convert any value to uint64
// uint64_t set_any_to_uint64(struct cfg &myCfg, std::shared_ptr<cfg::io_point_struct> io_point, std::any val)
// {
// #ifdef FPS_DEBUG_MODE
//     bool debug = false;
// #endif
//     uint16_t u16val = 0;
//     uint32_t u32val = 0;
//     uint64_t u64val = 0;
//     int64_t i64val = 0;
//     double f64val = 0.0;
//     bool is_uint = false;
//     bool is_int = false;

//     // // handle val being in a value field
//     if (val.type() == typeid(std::map<std::string, std::any>))
//     {
//         auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
//         if (clothed_val.find("value") != clothed_val.end())
//         {
//             val = clothed_val["value"];
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " value found " << std::endl;
// #endif
//         }
//     }
//     // todo add other exceptions here
//     if (io_point->is_bit_field || io_point->is_enum || io_point->packed_register)
//     {
//         io_point->normal_set = false;
//     }
//     else
//     {
// #ifdef FPS_DEBUG_MODE
//         if (debug)
//             std::cout << __func__ << " io_point normal set, io_point size " << io_point->size << std::endl;
// #endif
//         io_point->normal_set = true;
//     }

//     if (io_point->normal_set)
//     {
// #ifdef FPS_DEBUG_MODE
//         if (debug)
//             std::cout << ">>>>" << __func__ << " normal_set " << std::endl;
// #endif
//         if (val.type() == typeid(bool))
//         {
//             if (std::any_cast<bool>(val))
//                 i64val = 1;
//         }
//         else if ((val.type() == typeid(int32_t)) || (val.type() == typeid(int64_t)))
//         {
//             is_int = true;
//             if (val.type() == typeid(int32_t))
//             {
//                 i64val = static_cast<int64_t>(std::any_cast<int32_t>(val));
//             }
//             else
//             {
//                 i64val = std::any_cast<int64_t>(val);
//             }
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
// #endif
//             i64val <<= io_point->starting_bit_pos;
//             i64val -= io_point->shift;
//             f64val = static_cast<double>(i64val);
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else if ((val.type() == typeid(uint32_t)) || (val.type() == typeid(uint64_t)))
//         {
//             is_uint = true;
//             if (val.type() == typeid(uint32_t))
//             {
//                 u64val = static_cast<uint64_t>(std::any_cast<uint32_t>(val));
//             }
//             else
//             {
//                 u64val = std::any_cast<int64_t>(val);
//             }
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found uint >>" << u64val << std::endl;
// #endif
//             u64val <<= io_point->starting_bit_pos;
//             u64val -= io_point->shift;
//             f64val = static_cast<double>(u64val);
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else if (val.type() == typeid(double))
//         {
//             f64val = std::any_cast<double>(val);
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found double >>" << f64val << std::endl;
// #endif
//             f64val -= io_point->shift;
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else
//         {
//             FPS_ERROR_LOG("[%s]: value undefined %s", __func__, val.type().name());
//         }
//         if (io_point->size == 1)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 u16val = static_cast<uint16_t>(u64val);
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 u16val = static_cast<int16_t>(i64val);
//             }
//             else
//             {
//                 if (!io_point->is_signed)
//                 {
//                     u16val = static_cast<uint16_t>(f64val);
//                 }
//                 else
//                 {
//                     u16val = static_cast<uint16_t>(f64val);
//                 }
//             }
//             u16val ^= static_cast<uint16_t>(io_point->invert_mask);
//             // if (uri.is_force_request)
//             // {
//             //     io_point->is_forced = true;
//             //     io_point->forced_val = static_cast<uint64_t>(u16val);
//             // }
//             // regs16[0] = static_cast<uint16_t>(u16val);
//             u64val = static_cast<uint64_t>(u16val);
//         }
//         else if (io_point->size == 2)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u32val = static_cast<uint32_t>(u64val);
//                 }
//                 else
//                 {
//                     const auto f32val = static_cast<float>(u64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u32val = static_cast<uint32_t>(i64val);
//                 }
//                 else
//                 {
//                     const auto f32val = static_cast<float>(i64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//             }
//             else
//             {
//                 if (io_point->is_float)
//                 {
//                     const auto f32val = static_cast<float>(f64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//                 else if (!io_point->is_signed)
//                 {
//                     u32val = static_cast<uint32_t>(f64val);
//                 }
//                 else
//                 {
//                     u32val = static_cast<uint32_t>(static_cast<int32_t>(f64val));
//                 }
//             }
//             u32val ^= static_cast<uint32_t>(io_point->invert_mask);
//             // if (uri.is_force_request)
//             // {
//             //     io_point->is_forced = true;
//             //     io_point->forced_val = static_cast<uint64_t>(u32val);
//             // }
//             // if (!io_point->is_byte_swap)
//             // {
//             //     regs16[0] = static_cast<uint16_t>(u32val >> 16);
//             //     regs16[1] = static_cast<uint16_t>(u32val >> 0);
//             // }
//             // else
//             // {
//             //     regs16[0] = static_cast<uint16_t>(u32val >> 0);
//             //     regs16[1] = static_cast<uint16_t>(u32val >> 16);
//             // }
//             u64val = static_cast<uint64_t>(u32val);
//         }
//         else if (io_point->size == 4)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     // u64val = static_cast<uint64_t>(u64val);
//                 }
//                 else
//                 {
//                     const auto f64val = static_cast<double>(u64val);
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u64val = static_cast<uint64_t>(i64val);
//                 }
//                 else
//                 {
//                     const auto f64val = static_cast<double>(i64val);
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//             }
//             else
//             {
//                 if (io_point->is_float)
//                 {
//                     // const auto f32val = static_cast<float>(f64val);
//                     // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//                 else if (!io_point->is_signed)
//                 {
//                     u64val = static_cast<uint64_t>(f64val);
//                 }
//                 else
//                 {
//                     u64val = static_cast<uint64_t>(static_cast<int64_t>(f64val));
//                 }
//             }
//             // uint64_t tval;
//             // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
//             u64val ^= static_cast<uint64_t>(io_point->invert_mask);
//             // if (uri.is_force_request)
//             // {
//             //     io_point->is_forced = true;
//             //     io_point->forced_val = static_cast<uint64_t>(u64val);
//             // }
//             //     if (!io_point->is_byte_swap)
//             //     {
//             //         regs16[0] = static_cast<uint16_t>(u64val >> 48);
//             //         regs16[1] = static_cast<uint16_t>(u64val >> 32);
//             //         regs16[2] = static_cast<uint16_t>(u64val >> 16);
//             //         regs16[3] = static_cast<uint16_t>(u64val >> 0);
//             //         // memcpy(&tval, regs16, sizeof(tval));
//             //         // printf( " sent regs16 #1 0x%08lx  \n", tval);
//             //         // printf( " sent u64val   #1 0x%08lx  \n", u64val);
//             //     }
//             //     else
//             //     {
//             //         regs16[0] = static_cast<uint16_t>(u64val >> 0);
//             //         regs16[1] = static_cast<uint16_t>(u64val >> 16);
//             //         regs16[2] = static_cast<uint16_t>(u64val >> 32);
//             //         regs16[3] = static_cast<uint16_t>(u64val >> 48);
//             //     }
//         }
//     }
//     return u64val;
// }

// /// populate regs16 with the correct value of the correct size.
// uint64_t get_any_to_uint64(std::shared_ptr<cfg::io_point_struct> io_point, std::any val, Uri_req &uri, uint16_t *regs16)
// {
// #ifdef FPS_DEBUG_MODE
//     bool debug = false;
// #endif
//     uint16_t u16val = 0;
//     uint32_t u32val = 0;
//     uint64_t u64val = 0;
//     int64_t i64val = 0;
//     double f64val = 0.0;
//     bool is_uint = false;
//     bool is_int = false;
//     if (uri.is_unforce_request)
//     {
//         io_point->is_forced = false;
//     }
//     if (uri.is_enable_request)
//     {
//         io_point->is_enabled = true;
//     }
//     if (uri.is_disable_request)
//     {
//         io_point->is_enabled = false;
//     }

//     // // handle val being in a value field
//     if (val.type() == typeid(std::map<std::string, std::any>))
//     {
//         auto clothed_val = std::any_cast<std::map<std::string, std::any>>(val);
//         if (clothed_val.find("value") != clothed_val.end())
//         {
//             val = clothed_val["value"];
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " value found " << std::endl;
// #endif
//         }
//     }
//     // todo add other exceptions here
//     if (io_point->is_bit_field || io_point->is_enum || io_point->packed_register)
//     {
//         io_point->normal_set = false;
//     }
//     else
//     {
// #ifdef FPS_DEBUG_MODE
//         if (debug)
//             std::cout << __func__ << " io_point normal set, io_point size " << io_point->size << std::endl;
// #endif
//         io_point->normal_set = true;
//     }
//     if (io_point->normal_set)
//     {
// #ifdef FPS_DEBUG_MODE
//         if (debug)
//             std::cout << ">>>>" << __func__ << " normal_set " << std::endl;
// #endif
//         if (val.type() == typeid(bool))
//         {
//             if (std::any_cast<bool>(val))
//                 i64val = 1;
//         }
//         else if ((val.type() == typeid(int32_t)) || (val.type() == typeid(int64_t)))
//         {
//             is_int = true;
//             if (val.type() == typeid(int32_t))
//             {
//                 i64val = static_cast<int64_t>(std::any_cast<int32_t>(val));
//             }
//             else
//             {
//                 i64val = std::any_cast<int64_t>(val);
//             }
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found int >>" << i64val << std::endl;
// #endif
//             i64val <<= io_point->starting_bit_pos;
//             i64val -= io_point->shift;
//             f64val = static_cast<double>(i64val);
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else if ((val.type() == typeid(uint32_t)) || (val.type() == typeid(uint64_t)))
//         {
//             is_uint = true;
//             if (val.type() == typeid(uint32_t))
//             {
//                 u64val = static_cast<uint64_t>(std::any_cast<uint32_t>(val));
//             }
//             else
//             {
//                 u64val = std::any_cast<int64_t>(val);
//             }
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found uint >>" << u64val << std::endl;
// #endif
//             u64val <<= io_point->starting_bit_pos;
//             u64val -= io_point->shift;
//             f64val = static_cast<double>(u64val);
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else if (val.type() == typeid(double))
//         {
//             f64val = std::any_cast<double>(val);
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " found double >>" << f64val << std::endl;
// #endif
//             f64val -= io_point->shift;
//             if (io_point->scale)
//             {
//                 f64val *= io_point->scale;
//             }
//         }
//         else
//         {
//             FPS_ERROR_LOG("[%s]: value undefined %s", __func__, val.type().name());
//         }
//         if (io_point->size == 1)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 u16val = static_cast<uint16_t>(u64val);
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 u16val = static_cast<int16_t>(i64val);
//             }
//             else
//             {
//                 if (!io_point->is_signed)
//                 {
//                     u16val = static_cast<uint16_t>(f64val);
//                 }
//                 else
//                 {
//                     u16val = static_cast<uint16_t>(f64val);
//                 }
//             }
//             u16val ^= static_cast<uint16_t>(io_point->invert_mask);
// #ifdef FPS_DEBUG_MODE
//             if (debug)
//                 std::cout << ">>>>" << __func__ << " offset  " << io_point->offset << " final u64val >>" << u16val << " i64val >> " << i64val << std::endl;
// #endif
//             if (uri.is_force_request)
//             {
//                 io_point->is_forced = true;
//                 io_point->forced_val = static_cast<uint64_t>(u16val);
//             }
//             if (io_point->is_forced)
//                 u16val = static_cast<uint16_t>(io_point->forced_val);
//             regs16[0] = static_cast<uint16_t>(u16val);
//             u64val = static_cast<uint64_t>(u16val);
//         }
//         else if (io_point->size == 2)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u32val = static_cast<uint32_t>(u64val);
//                 }
//                 else
//                 {
//                     const auto f32val = static_cast<float>(u64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u32val = static_cast<uint32_t>(i64val);
//                 }
//                 else
//                 {
//                     const auto f32val = static_cast<float>(i64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//             }
//             else
//             {
//                 if (io_point->is_float)
//                 {
//                     const auto f32val = static_cast<float>(f64val);
//                     memcpy(&u32val, &f32val, sizeof(u32val));
//                 }
//                 else if (!io_point->is_signed)
//                 {
//                     u32val = static_cast<uint32_t>(f64val);
//                 }
//                 else
//                 {
//                     u32val = static_cast<uint32_t>(static_cast<int32_t>(f64val));
//                 }
//             }
//             u32val ^= static_cast<uint32_t>(io_point->invert_mask);
//             if (uri.is_force_request)
//             {
//                 io_point->is_forced = true;
//                 io_point->forced_val = static_cast<uint64_t>(u32val);
//             }
//             if (io_point->is_forced)
//                 u32val = static_cast<uint32_t>(io_point->forced_val);
//             if (!io_point->is_byte_swap)
//             {
//                 regs16[0] = static_cast<uint16_t>(u32val >> 16);
//                 regs16[1] = static_cast<uint16_t>(u32val >> 0);
//             }
//             else
//             {
//                 regs16[0] = static_cast<uint16_t>(u32val >> 0);
//                 regs16[1] = static_cast<uint16_t>(u32val >> 16);
//             }
//             u64val = static_cast<uint64_t>(u32val);
//         }
//         else if (io_point->size == 4)
//         {
//             if (is_uint && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     // u64val = static_cast<uint64_t>(u64val);
//                 }
//                 else
//                 {
//                     const auto f64val = static_cast<double>(u64val);
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//             }
//             else if (is_int && !io_point->scale)
//             {
//                 if (!io_point->is_float)
//                 {
//                     u64val = static_cast<uint64_t>(i64val);
//                 }
//                 else
//                 {
//                     const auto f64val = static_cast<double>(i64val);
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//             }
//             else
//             {
//                 if (io_point->is_float)
//                 {
//                     // const auto f32val = static_cast<float>(f64val);
//                     // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
//                     memcpy(&u64val, &f64val, sizeof(u64val));
//                 }
//                 else if (!io_point->is_signed)
//                 {
//                     u64val = static_cast<uint64_t>(f64val);
//                 }
//                 else
//                 {
//                     u64val = static_cast<uint64_t>(static_cast<int64_t>(f64val));
//                 }
//             }
//             // uint64_t tval;
//             // memcpy(&current_u64_val, &current_float_val, sizeof(current_float_val));
//             u64val ^= static_cast<uint64_t>(io_point->invert_mask);
//             if (uri.is_force_request)
//             {
//                 io_point->is_forced = true;
//                 io_point->forced_val = static_cast<uint64_t>(u64val);
//             }
//             if (io_point->is_forced)
//                 u64val = io_point->forced_val;
//             if (!io_point->is_byte_swap)
//             {
//                 regs16[0] = static_cast<uint16_t>(u64val >> 48);
//                 regs16[1] = static_cast<uint16_t>(u64val >> 32);
//                 regs16[2] = static_cast<uint16_t>(u64val >> 16);
//                 regs16[3] = static_cast<uint16_t>(u64val >> 0);
//                 // memcpy(&tval, regs16, sizeof(tval));
//                 // printf( " sent regs16 #1 0x%08lx  \n", tval);
//                 // printf( " sent u64val   #1 0x%08lx  \n", u64val);
//             }
//             else
//             {
//                 regs16[0] = static_cast<uint16_t>(u64val >> 0);
//                 regs16[1] = static_cast<uint16_t>(u64val >> 16);
//                 regs16[2] = static_cast<uint16_t>(u64val >> 32);
//                 regs16[3] = static_cast<uint16_t>(u64val >> 48);
//             }
//         }
//     }
//     return u64val;
// }

// ///////////////////////////////////////
// ////////// NOT LOOKED AT YET///////////
// ///////////////////////////////////////

// std::vector<std::string> split_string(const std::string &str, char delimiter)
// {
//     std::vector<std::string> tokens;
//     std::string token;
//     std::istringstream tokenStream(str);
//     while (std::getline(tokenStream, token, delimiter))
//     {
//         tokens.push_back(token);
//     }
//     return tokens;
// }

// std::map<std::string, std::shared_ptr<cfg::io_point_struct>> *cfg::findIOPointMapFromUriFragments(std::vector<std::string> keys)
// {
//     // Look up outerKey in the outer map
//     if (keys.size() < 3)
//     {
//         std::cout << "Not enough keys found" << std::endl;
//         return nullptr;
//     }
//     int key_idx = 0;
//     if ((int)keys[0].size() == (int)0)
//     {
//         std::cout << "skipping key 0" << std::endl;
//         key_idx = 1;
//     }
//     auto outerIt = component_io_point_map.find(keys[key_idx]);
//     if (outerIt != component_io_point_map.end())
//     {
//         // If outerKey is found, look up innerKey in the inner map
//         auto &innerMap = outerIt->second;
//         auto innerIt = innerMap.find(keys[key_idx + 1]);
//         if (innerIt != innerMap.end())
//         {
//             // If innerKey is also found, return the inner map
//             return &innerIt->second;
//         }
//         else
//         {
//             std::cout << " inner key [" << keys[key_idx + 1] << "] not  found" << std::endl;
//             return nullptr;
//         }
//     }
//     else
//     {
//         std::cout << " base key [" << keys[key_idx] << "] not  found" << std::endl;
//     }
//     // If any key not found, return nullopt
//     return nullptr;
// }

// // used in extract_bitfields
// bool anyTypeString(const std::any &value)
// {
//     int status;
//     char *demangled = abi::__cxa_demangle(value.type().name(), 0, 0, &status);
//     if (status == 0)
//     {
//         std::cout << "Type of value: " << demangled << std::endl;
//         free(demangled);
//     }
//     else
//     {
//         std::cout << "Type of value (mangled): " << value.type().name() << std::endl;
//     }
//     return true;
// }

// // pub freq, offset , list of io_point_map, each with a start and number, assume that the config has been corrected

// // used by gcom_show_pubs (Which is currently not used)
// template <typename Iterator>
// std::string join(const std::string &separator, Iterator begin, Iterator end)
// {
//     std::string result;
//     if (begin != end)
//     {
//         result += *begin;
//         ++begin;
//     }
//     while (begin != end)
//     {
//         result += separator + *begin;
//         ++begin;
//     }
//     return result;
// }

// // used by check_work_items
// bool compareIOPointOffsets(const std::shared_ptr<cfg::io_point_struct> &a, const std::shared_ptr<cfg::io_point_struct> &b)
// {
//     // First, compare by register_type
//     if (a->register_type < b->register_type)
//         return true;
//     else if (a->register_type > b->register_type)
//         return false;
//     // then by device_id
//     if (a->device_id < b->device_id)
//         return true;
//     else if (a->device_id > b->device_id)
//         return false;
//     // // then, compare by is_enabled
//     // if (a->is_enabled < b->is_enabled)
//     //     return true;
//     // if (a->is_enabled > b->is_enabled)
//     //     return false;
//     // If device_id is equal, compare by offset
//     return a->offset < b->offset;
// }

// // TODO start with a blank result vector
// /// create io_work structures from vectors of io_map points
// void check_work_items(std::vector<std::shared_ptr<IO_Work>> &io_work_vec, std::vector<std::shared_ptr<cfg::io_point_struct>> &io_map_vec, struct cfg &myCfg, const char *oper, bool include_all_points, bool debug)
// {
//     // Sort the vector using the custom comparison function
//     if (io_map_vec.size() == 0)
//     {
//         // std::cout << __func__ << " nothing here" << std::endl;
//         return;
//     }
//     if (debug)
//     {
//         std::cout
//             << __func__ << " Starting io_work .. " << std::endl;
//     }
//     std::string repto("");
//     if (io_map_vec.size() > 1)
//         std::sort(io_map_vec.begin(), io_map_vec.end(), compareIOPointOffsets);
//     bool first = true;
//     int offset = 0;
//     int first_offset = 0;
//     int device_id = 0;
//     int num = 0;
//     int isize = 0;
//     bool local = false;
//     auto io_points = io_map_vec;
//     int max_item_size = myCfg.max_register_group_size; // adjust for bit
//     int max_bit_size = myCfg.max_bit_size;             // adjust for bit
//     cfg::Register_Types register_type;
//     // io_work->io_points.clear();
//     auto io_point = io_map_vec.at(0);
//     auto io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, 1, nullptr, nullptr, strToWorkType(oper, false));
//     if ((io_point->register_type == cfg::Register_Types::Coil) || (io_point->register_type == cfg::Register_Types::Discrete_Input))
//         max_item_size = max_bit_size;
//     if (debug)
//     {
//         std::cout
//             << " After Sort .. " << std::endl;
//         for (auto io_point : io_points)
//         {
//             std::cout
//                 << "            .. "
//                 << " Id " << io_point->id
//                 << " device_id " << io_point->device_id
//                 << " offset " << io_point->offset
//                 << " size " << io_point->size
//                 << std::endl;
//         }
//     }
//     for (auto io_point : io_points)
//     {
//         // std::cout << " >>>>>>>>>>>> state  #1 offset " << offset << " isize " << isize <<" io_point->offset " << io_point->offset << std::endl;
//         if (!io_point->is_enabled && !io_point->is_forced && !include_all_points)
//             continue;
//         if (first)
//         {
//             first = false;
//             offset = io_point->offset;
//             first_offset = io_point->offset;
//             device_id = io_point->device_id;
//             num = io_point->size;
//             isize = io_point->size;
//             register_type = io_point->register_type;
//             io_work->register_type = register_type;
//             io_work->component = io_point->component.lock();
//             io_work->io_points.emplace_back(io_point);
//             if (debug)
//                 std::cout << " >>>>>>>>>>>> state  #1 offset " << offset
//                           << " isize " << isize
//                           << " io_point->offset " << io_point->offset
//                           << " io_point->size " << io_point->size
//                           << std::endl;
//             // offset += io_point->size;
//         }
//         else
//         {
//             // std::cout << " >>>>>>>>>>>> state  #2 offset " << offset << " isize " << isize << " io_point->offset " << io_point->offset << " io_point size " << io_point->size << std::endl;
//             if (debug)
//             {
//                 if (((offset + isize) != io_point->offset)) // || ((offset+io_point->size > max_item_size)) || (io_point->device_id != device_id) || (io_point->register_type != register_type))
//                 {
//                     std::cout << " >>>>>>>>>>>> Break detected  #1 offset " << offset
//                               << " isize " << isize
//                               << " io_point->offset " << io_point->offset
//                               << std::endl;
//                 }
//                 if (((offset + io_point->size - first_offset > max_item_size))) // || (io_point->device_id != device_id) || (io_point->register_type != register_type))
//                 {
//                     std::cout << " >>>>>>>>>>>> Break detected  #2 offset " << offset
//                               << " io_point->size " << io_point->size
//                               << " max_item_size" << max_item_size
//                               << std::endl;
//                 }
//                 if ((io_point->device_id != device_id)) // || (io_point->register_type != register_type))
//                 {
//                     std::cout << " >>>>>>>>>>>> Break detected  #3 (device_id)"
//                               << std::endl;
//                 }
//                 if ((io_point->register_type != register_type))
//                 {
//                     std::cout << " >>>>>>>>>>>> Break detected  #4 (register_type)" << std::endl;
//                 }
//             }
//             if (!io_point->is_enabled && io_point->is_forced)
//             {
//                 std::cout << " >>>>>>>>>>>> Break detected  #5 !enabled but forced, use local " << offset
//                           << " isize " << isize
//                           << " io_point->offset " << io_point->offset
//                           << std::endl;
//             }

//             if (((offset + isize) != io_point->offset) || ((offset + io_point->size - first_offset > max_item_size)) || (io_point->device_id != device_id) || (io_point->register_type != register_type) || (!io_point->is_enabled && io_point->is_forced))
//             {
//                 if ((!io_point->is_enabled && io_point->is_forced && !include_all_points))
//                 {
//                     continue;
//                 }
//                 // std::cout << " >>>>>>>>>>>> Break detected " << std::endl;
//                 io_work->offset = first_offset;
//                 io_work->num_registers = num;
//                 io_work_vec.emplace_back(io_work);
//                 if (debug)
//                     std::cout << " >>>>>>>>>>>> At break; offset : " << first_offset << " num : " << num << std::endl;
//                 io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, 1, nullptr, nullptr, strToWorkType(oper, false));
//                 offset = io_point->offset;
//                 first_offset = io_point->offset;
//                 device_id = io_point->device_id;
//                 num = io_point->size;
//                 isize = io_point->size;
//                 register_type = io_point->register_type;
//                 io_work->register_type = register_type;
//                 io_work->component = io_point->component.lock();
//                 local = !io_point->is_enabled;
//                 io_work->io_points.emplace_back(io_point);
//             }
//             else
//             {
//                 offset += isize;
//                 isize = io_point->size;
//                 num += io_point->size;
//                 io_work->io_points.emplace_back(io_point);
//                 // std::cout << " >>>>>>>>>>>> state  #3 offset " << offset << " isize " << isize << " io_point->offset " << io_point->offset << " io_point size " << io_point->size << std::endl;
//             }
//         }
//     }
//     if (debug)
//         std::cout << " >>>>>>>>>>>>>> After Check; offset : " << first_offset << " num : " << num << std::endl;
//     io_work->offset = first_offset;
//     io_work->num_registers = num;
//     io_work->local = local;

//     io_work_vec.emplace_back(io_work);
//     // first sort the io_points
//     //  if all the io_points follow one another then put io_work on the vec.
//     //  if not truncate io_work, create another one and put that on the vec.
//     return;
// }

// /// @brief check_item_debounce
// //     filters get / set operations on debounce status
// void check_item_debounce(bool &enabled, std::shared_ptr<cfg::io_point_struct> io_point, bool debug)
// {
//     if (io_point->use_debounce) // && (io_point->debounce > 0.0) && (io_work->tIo > io_point->debounce_time))
//     {
//         double tNow = get_time_double();
//         if (debug)
//             std::cout << " io_point id " << io_point->id << " using_debounce" << std::endl;
//         if (io_point->debounce == 0.0) // && (io_point->debounce > 0.0) && (io_work->tIo > io_point->debounce_time))
//         {
//             if (debug)
//                 std::cout << " io_point id" << io_point->id << " debounce is zero " << std::endl;
//             io_point->use_debounce = false;
//         }
//         else if (io_point->debounce_time == 0.0)
//         {
//             io_point->debounce_time = (tNow + io_point->debounce);
//             if (debug)
//                 std::cout << " debounce time set up  :" << io_point->debounce_time << std::endl;
//         }
//         else
//         {
//             if (tNow > io_point->debounce_time)
//             {
//                 if (debug)
//                     std::cout << " debounce time   :" << io_point->debounce_time << " passed: tNow: " << tNow << std::endl;
//                 io_point->debounce_time += io_point->debounce;
//             }
//             else
//             {
//                 if (debug)
//                     std::cout << " still in debounce time :" << io_point->debounce_time << " tNow: " << tNow << std::endl;
//                 enabled = false;
//             }
//         }
//     }
// }

// /// @brief parses the body of an incoming fims message
// /// this could be raw data or some kind of json object
// bool gcom_parse_data(std::any &anyFimsMessageBody, const char *data, size_t length, bool debug)
// {
//     simdjson::dom::parser parser;
//     // Convert u8* data to padded_string
//     simdjson::padded_string padded_content(reinterpret_cast<const char *>(data), length);
//     // if(debug)
//     // {
//     //     std::cout << " \"parse_data\": {" << std::endl;
//     //     std::cout << "                 \"Data length\": " << length << std::endl;
//     //     std::cout << "} " << std::endl;
//     // }
//     // Parse
//     auto result = parser.parse(padded_content);
//     if (result.error())
//     {
//         //std::string cjerr = 
//         FPS_ERROR_LOG("input body [%s] ", data);
//         FPS_ERROR_LOG("parser error [%s] ", simdjson::error_message(result.error()));

//         //std::cout << "parser error ....\n";
//         //std::cerr << simdjson::error_message(result.error()) << std::endl;
//         return false;
//     }
//     if (debug)
//         std::cout << "parser result OK ....\n";
//     // Convert JSON to any
//     // anyFimsMessageBody = jsonToMap(result.value());
//     anyFimsMessageBody = jsonToAny(result.value());
//     // Utility function to print nested maps
//     if (debug)
//         printAny(anyFimsMessageBody, 0);
//     if (anyFimsMessageBody.type() == typeid(std::map<std::string, std::any>))
//     {
//         if (debug)
//         {
//             std::cout << "parser result  map ....\n";
//             auto mkey = std::any_cast<std::map<std::string, std::any>>(anyFimsMessageBody);
//             for (auto key : mkey)
//             {
//                 std::cout << " key " << key.first << "\n";
//                 // TODO create the IO_Work items
//             }
//         }
//     }
//     return true;
// }

// bool gcom_findCompVar(
//     std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const cfg::component_struct *comp, std::string kvar)

// {
//     // std::string myvar = kvar;
//     // return false;
//     // auto myvar = uri_keys[key_idx + 2];
//     // std::cout   << "         func : #1 "<< __func__ << " myVAR [" << myvar << "] \n";
//     for (auto rgroup : comp->register_groups)
//     {
//         for (auto iop : rgroup->io_point_map)
//         {
//             if (iop->id == kvar)
//             {
//                 // TODO use io_point
//                 io_point = iop;
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// bool add_all_component_points_to_io_vec(std::vector<std::shared_ptr<cfg::io_point_struct>> &io_point_vec, const struct cfg &myCfg,
//                                                                          const std::vector<std::string> &uri_keys, bool skip_disabled)
// {
//     bool debug = false;
//     int key_idx = 0;
//     if ((uri_keys.size() > 1) && (uri_keys[0].size() == 0))
//         key_idx = 1;
//     // std::string myvar = kvar;
//     // if ((int)uri_keys.size() < (key_idx + 3))
//     // {
//     //     myvar = kvar;
//     // }
//     // else
//     // {
//     //     myvar = uri_keys[key_idx + 2];
//     // }
//     // return false;
//     // auto myvar = uri_keys[key_idx + 2];
//     // std::cout   << "         func : #1 "<< __func__ << " myVAR [" << myvar << "] \n";
//     try
//     {
//         // Using at() for safe access. Catch exceptions if key not found
//         auto &myComp = myCfg.component_io_point_map.at(uri_keys[key_idx]);
//         // std::cout   << "         func : #2 "<< __func__ << "\n";
//         auto &component_points = myComp.at(uri_keys[key_idx + 1]);
//         // std::cout   << "         func : #3 "<< __func__ << "\n";
//         for (std::pair<std::string, std::shared_ptr<cfg::io_point_struct>> io_point : component_points)
//         {
//             if (debug)
//                 std::cout << __func__ << " Item : " << io_point.first << " offset : " << io_point.second->offset << "\n";
//             if (skip_disabled)
//                 if (!io_point.second->is_enabled)
//                     continue;
//             io_point_vec.emplace_back(io_point.second);
//         }
//         // map_result = register_group.at(myvar);
//         return true;
//     }
//     catch (const std::out_of_range &)
//     {
//         // Key not found, return nullptr
//         return false;
//     }
//     return false;
// }

// // Utility function to print nested maps
// void printAny(std::any &value, int indent)
// {
//     if (value.type() == typeid(std::map<std::string, std::any>))
//     {
//         std::cout << "\n";
//         printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
//     }
//     else if (value.type() == typeid(int))
//     {
//         std::cout << std::any_cast<int>(value) << "\n";
//     }
//     else if (value.type() == typeid(int64_t))
//     {
//         std::cout << std::any_cast<int64_t>(value) << "\n";
//     }
//     else if (value.type() == typeid(double))
//     {
//         std::cout << std::any_cast<double>(value) << "\n";
//     }
//     else if (value.type() == typeid(std::string))
//     {
//         std::cout << std::any_cast<std::string>(value) << "\n";
//     }
//     else if (value.type() == typeid(bool))
//     {
//         auto val = std::any_cast<bool>(value);
//         if (val)
//             std::cout << "true"
//                       << "\n";
//         else
//             std::cout << "false"
//                       << "\n";
//     }
//     else
//     {
//         std::cout << "type unknown"
//                   << "\n";
//     }
// }

// // ... (include the parseMessage function from the previous answer here)
// //
// // Utility function to print nested maps
// void printMap(const std::map<std::string, std::any> &baseMap, int indent = 0)
// {
//     for (const auto &[key, value] : baseMap)
//     {
//         for (int i = 0; i < indent; ++i)
//         {
//             std::cout << "  ";
//         }
//         std::cout << key << ": ";
//         if (value.type() == typeid(std::map<std::string, std::any>))
//         {
//             std::cout << "\n";
//             printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
//         }
//         else if (value.type() == typeid(int))
//         {
//             std::cout << std::any_cast<int>(value) << "\n";
//         }
//         else if (value.type() == typeid(int64_t))
//         {
//             std::cout << std::any_cast<int64_t>(value) << "\n";
//         }
//         else if (value.type() == typeid(double))
//         {
//             std::cout << std::any_cast<double>(value) << "\n";
//         }
//         else if (value.type() == typeid(std::string))
//         {
//             std::cout << std::any_cast<std::string>(value) << "\n";
//         }
//         else if (value.type() == typeid(bool))
//         {
//             auto val = std::any_cast<bool>(value);
//             if (val)
//                 std::cout << "true"
//                           << "\n";
//             else
//                 std::cout << "false"
//                           << "\n";
//         }
//     }
// }




ioChannel<std::shared_ptr<IO_Work>> io_respChan; // Use Channel to send IO-Work to thread


u64 decode_raw(const u16 *raw_registers, cfg::io_point_struct &io_point, std::any &decode_output);

bool extractJsonValue(const simdjson::dom::element &el, std::any &value)
{
    bool ret = true;
    if (el.is_bool())
    {
        value = bool(el);
    }
    else if (el.is_int64())
    {
        value = int64_t(el);
    }
    else if (el.is_string())
    {
        value = std::string(el);
    }
    else if (el.is_double())
    {
        value = double(el);
    }
    else
    {
        ret = false;
    }
    // Extend this for other data types if needed
    return ret;
}

std::string mapToString(const std::map<std::string, std::any> &m)
{
    std::string result = "{";
    for (const auto &[key, value] : m)
    {
        result += "\"" + key + "\": " + anyToString(value) + ", ";
    }
    if (result.length() > 1)
    {
        result = result.substr(0, result.length() - 2); // remove the last ", "
    }
    result += "}";
    return result;
}

int some_test(std::map<std::string, std::any> &m)
{
    // Now m contains the JSON data
    for (auto mx : m)
    {
        std::cout << " Key " << mx.first << std::endl;
        try
        {
            auto mymap = std::any_cast<std::map<std::string, std::any>>(mx.second);
            for (auto my : mymap)
            {
                std::cout << "     Item " << my.first << std::endl;
            }
        }
        catch (const std::bad_any_cast &)
        {
            std::cout << "     Value cannot be cast to map<string, any>" << std::endl;
        }
    }
    // for (auto mx : m) {
    //     std::cout << " Key " << mx.first << std::endl;
    //     for (auto my : mx.second) {
    //         std::cout << "     Item " << my.first << std::endl;
    //     }
    // }
    spdlog::info("Map content: {}", mapToString(m));
    return 0;
}

// we have not dome the output fims message handling yet but this is all
// stuff we may hvae to use.
// Your utility function to print nested maps
size_t computeSize(const std::map<std::string, std::any> &baseMap, int indent = 0)
{
    size_t totalSize = 0;
    for (const auto &[key, value] : baseMap)
    {
        totalSize += key.size() + 2 * indent + 2; // Key size + indentation + ": "
        if (value.type() == typeid(std::map<std::string, std::any>))
        {
            totalSize += 1; // Newline
            totalSize += computeSize(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
        }
        else if (value.type() == typeid(int))
        {
            totalSize += std::to_string(std::any_cast<int>(value)).size() + 1; // Value + newline
        }
        else if (value.type() == typeid(int64_t))
        {
            totalSize += std::to_string(std::any_cast<int64_t>(value)).size() + 1;
        }
        else if (value.type() == typeid(double))
        {
            // This is an approximation; actual size might vary due to floating-point representation
            totalSize += std::to_string(std::any_cast<double>(value)).size() + 1;
        }
        else if (value.type() == typeid(std::string))
        {
            totalSize += std::any_cast<std::string>(value).size() + 1;
        }
        else if (value.type() == typeid(bool))
        {
            totalSize += std::any_cast<bool>(value) ? 5 : 6; // Either "true\n" or "false\n"
        }
    }
    return totalSize;
}

int test_buffer_size()
{
    std::map<std::string, std::any> testMap = {
        {"key1", std::map<std::string, std::any>{
                     {"subkey1", 123},
                     {"subkey2", "value2"},
                     {"subkey3", std::map<std::string, std::any>{
                                     {"subsubkey1", 45.6}}}}},
        {"key2", "value2"},
        {"key3", 789}};
    size_t requiredBufferSize = computeSize(testMap);
    std::cout << "Required buffer size: " << requiredBufferSize << std::endl;
    return 0;
}

// given a range of io_point options and values test the decode operation
bool test_decode_raw()
{
    cfg::io_point_struct io_point;
    u16 raw_registers[4];
    std::any decode_output;
    // first the number 1234 no scale no shift etc
    raw_registers[0] = (u16)1234;
    raw_registers[1] = (u16)0;
    raw_registers[2] = (u16)0;
    raw_registers[3] = (u16)0;
    io_point.scale = 0;
    io_point.shift = 0;
    io_point.starting_bit_pos = 0;
    io_point.is_float = false;
    io_point.is_signed = false;
    io_point.is_byte_swap = false;
    io_point.is_byte_swap = false;
    io_point.invert_mask = 0;
    io_point.care_mask = 0;
    io_point.uses_masks = false;
    io_point.size = 1;

    auto res = decode_raw(raw_registers, io_point, decode_output);
    anyTypeString(decode_output);
    std::cout << "test decode " << std::hex << res << std::dec << " decode_output " << anyToString(decode_output) << std::endl;

    raw_registers[0] = (u16)0x4499;
    raw_registers[1] = (u16)0xc000;
    raw_registers[2] = (u16)3;
    raw_registers[3] = (u16)4;
    io_point.is_float = true;
    io_point.size = 2;

    res = decode_raw(raw_registers, io_point, decode_output);
    anyTypeString(decode_output);
    std::cout << "test decode " << std::hex << res << std::dec << " decode_output " << anyToString(decode_output) << std::endl;

    return true;
}

std::string anyToString(const std::any &value)
{
    // std::cout << "Type of value: " << value.type().name() << std::endl;
    try
    {
        return std::to_string(std::any_cast<int>(value));
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return std::to_string(std::any_cast<double>(value));
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return std::to_string(std::any_cast<long>(value));
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return std::to_string(std::any_cast<unsigned long>(value));
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return std::any_cast<bool>(value) ? "true" : "false";
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return std::any_cast<std::string>(value);
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        return mapToString(std::any_cast<std::map<std::string, std::any>>(value));
    }
    catch (const std::bad_any_cast &)
    {
    }
    try
    {
        std::vector<std::any> vec = std::any_cast<std::vector<std::any>>(value);
        std::string result = "[";
        for (const auto &v : vec)
        {
            result += anyToString(v) + ", ";
        }
        if (result.length() > 1)
        {
            result = result.substr(0, result.length() - 2); // remove the last ", "
        }
        result += "]";
        return result;
    }
    catch (const std::bad_any_cast &e)
    {
        std::cout << "Bad type " << e.what() << std::endl;
    }
    return "unknown";
}

bool test_uri_body(struct cfg &myCfg, const char *uri, const char *method, const char *pname, const char *uname, const char *repto, const char *body)
{
    std::string_view uri_view;
    Uri_req uri_req(uri_view, uri);
    // if (std::string(method) == "set") {
    //     std::cout << myCfg.client_name <<" detected set method, uri :"<< uri << "\n" ;
    // }
    // else{
    //     std::cout << "detected other method\n" ;
    // }
    // create an any object from the data
    std::any gcom_data;
    gcom_parse_data(gcom_data, body, strlen(body), true);
    // std::cout << __func__<< "   decode body [" << body << "]"<< std::endl ;
    // printAny(gcom_data);
    // std::cout << __func__<< "   decode done" << std::endl ;
    // look for a single
    bool debug = true;
    std::shared_ptr<cfg::io_point_struct> var;
    std::vector<std::shared_ptr<IO_Work>> work_vec;
    // have to look for _disable / _enable  / _force /_unforce
    if (uri_is_single(var, myCfg, uri_req, debug))
    {
        if (std::string(method) == "set")
        {
            std::cout << __func__ << "  found Set single [" << uri << "]" << std::endl;
            std::string repto = "";
            // auto ok =
            encode_io_point_struct(work_vec, var, gcom_data, myCfg, uri_req, repto, "set", debug);
            std::cout << __func__ << "  work_vec size  [" << work_vec.size() << "]" << std::endl;
            if (pname)
                var->process_name = std::string(pname);
            if (uname)
                var->username = std::string(uname);

            auto io_work = work_vec.begin();
            if (io_work != work_vec.end())
            {
                (*io_work)->io_repChan = &io_respChan;
                pollWork(*io_work);
                work_vec.erase(io_work);
            }
            clearChan(true);
        }
        else if (std::string(method) == "get")
        {
            std::cout << __func__ << "  found Get single [" << uri << "]" << std::endl;
            // auto ok =
            decode_io_point_struct(work_vec, var, gcom_data, myCfg, uri_req, "get", debug);
            std::cout << __func__ << "  work_vec size  [" << work_vec.size() << "]" << std::endl;
            auto io_work = work_vec.begin();
            if (io_work != work_vec.end())
            {
                (*io_work)->io_repChan = &io_respChan;
                pollWork(*io_work);
                work_vec.erase(io_work);
            }
            clearChan(true);
        }
    }
    else
    {
        if (std::string(method) == "set")
        {
            std::cout << __func__ << "  processing set multi [" << uri << "]" << std::endl;
            if (gcom_data.type() == typeid(std::map<std::string, std::any>))
            {
                std::cout << " found map\n";
                auto baseMap = std::any_cast<std::map<std::string, std::any>>(gcom_data);
                for (const auto &[key, value] : baseMap)
                {
                    auto vok = ioPointExists(var, myCfg, uri_req.uri_vec, uri_req.num_uris, key);
                    if (vok)
                    {
                        std::string repto = "";
                        encode_io_point_struct(work_vec, var, value, myCfg, uri_req, repto, "set", debug);
                        std::cout << " found :" << key << " offset " << var->offset << std::endl;
                        if (pname)
                            var->process_name = std::string(pname);
                        if (uname)
                            var->username = std::string(uname);
                    }
                    else
                    {
                        std::cout << " Not found :" << key << std::endl;
                    }
                }
                // printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
            }
            std::cout << __func__ << "  work_vec size  [" << work_vec.size() << "]" << std::endl;
            // TODO comperss this
            // std::vector<std::shared_ptr<IO_Work>>work_vec;
            for (auto io_work : work_vec)
            {
                (io_work)->io_repChan = &io_respChan;
                pollWork(io_work);
            }
            work_vec.clear();
        }
        else if (std::string(method) == "get")
        {
            std::cout << __func__ << "  Not yet processing get multi [" << uri << "]" << std::endl;
        }
    }
    return true; // You might want to return a status indicating success or failure.
}

/*
/// @brief
            split a string_view object into a vector seperated by the delimiter
*/
std::vector<std::string> any_split(std::string_view str, char delimiter)
{
    std::vector<std::string> result;
    size_t pos = 0;
    size_t next_pos;
    while ((next_pos = str.find(delimiter, pos)) != std::string_view::npos)
    {
        result.emplace_back(str.substr(pos, next_pos - pos));
        pos = next_pos + 1; // Move past the delimiter
    }
    result.emplace_back(str.substr(pos)); // Add the last token
    return result;
}

// map of typenames / offsets to actual map component
// std::map<std::string,std::map<int,struct type_map>>types;
/// @brief extract the different types in a config
void gcom_extract_types(struct cfg &myCfg)
{
    for (auto &rawComp : myCfg.components)
    {
        for (auto &rawReg : rawComp->register_groups)
        {
            for (auto &rawMap : rawReg->io_point_map)
            {
                struct type_map tmap;
                tmap.map = rawMap;
                tmap.register_group = rawReg;
                tmap.component = rawComp;
                types[rawReg->register_type_str][rawMap->offset] = tmap;
            }
        }
    }
}

// Given a device_id (TODO) a type and an offset find the type_map
// we may not need this now
/// @brief Given a device_id (TODO) a type and an offset find the type_map
struct type_map *gcom_get_type(std::string type, int offset, bool debug = false)
{
    if (types.find(type) != types.end())
    {
        if (types[type].find(offset) != types[type].end())
        {
            auto register_type = &types[type][offset];
            if (debug)
            {
                std::cout << "\"gcom_get_type\": {\n"
                          << "                   \"status\": \"success\",\n"
                          << "                   \"type\": \"" << type << "\",\n"
                          << "                   \"offset\": " << offset << ",\n "
                          << "                   \"component_id\": \"" << register_type->component->id << "\",\n "
                          << "                   \"register_type\": \"" << register_type->register_group->register_type_str << "\",\n"
                          << "                   \"map_id\": \"" << register_type->map->id << "\",\n"
                          << "                   \"map_offset\": " << register_type->map->offset << "\n"
                          << "                   }" << std::endl;
            }
            return register_type;
        }
    }
    if (debug)
    {
        std::cout << "\"gcom_get_type\": {\n"
                  << "                       \"status\": \"error\",\n"
                  << "                       \"type\": \"" << type << "\",\n"
                  << "                       \"offset\": " << offset << ",\n"
                  << "                       \"message\": \"type '" << type << "' " << offset << " not found\""
                  << "                       }" << std::endl;
    }
    return nullptr;
}

// std::map<std::string,std::map<std::string,struct type_map>>comps;
//  no longer used
void gcom_extract_comps(struct cfg &myCfg)
{
    for (auto &rawComp : myCfg.components)
    {
        for (auto rawReg : rawComp->register_groups)
        {
            for (auto rawMap : rawReg->io_point_map)
            {
                struct type_map tmap;
                tmap.map = rawMap;
                tmap.register_group = rawReg;
                tmap.component = rawComp;
                comps[rawComp->id][rawMap->id] = tmap;
            }
        }
    }
}

void gcom_extract_subs(struct cfg &myCfg)
{
    for (auto &rawComp : myCfg.components)
    {
        subs.push_back(rawComp->id);
    }
}

bool gcom_config_test_uri(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *uri, const char *id)
{
    bool debug = false;
    extract_components(jsonMapOfConfig, "components", myCfg, false);
    gcom_extract_comps(myCfg);
    std::cout << " seeking uri  :" << uri << " id :" << id << std::endl;
    // auto mytype =
    gcom_get_comp(myCfg, uri, id, debug);
    // std::cout << " the type yields an id :" << mytype->map->id<<std::endl;
    return false;
}

// may well be deprecated
void gcom_extract_pubs(struct cfg &myCfg)
{
    for (auto &rawComp : myCfg.components)
    {
        for (auto &rawReg : rawComp->register_groups)
        {
            if (pubs.find(rawComp->frequency) == pubs.end() ||
                pubs[rawComp->frequency].find(rawComp->offset_time) == pubs[rawComp->frequency].end())
            {
                std::vector<std::shared_ptr<cfg::register_group_struct>> rs;
                rawReg->device_id = rawComp->device_id;
                pubs[rawComp->frequency][rawComp->offset_time] = rs;
            }
            pubs[rawComp->frequency][rawComp->offset_time].push_back(rawReg);
        }
    }
}

// TODO test for max register_group size
int merge_IO_Work_Reg(std::vector<std::shared_ptr<IO_Work>> &work_vector,
                      std::vector<std::shared_ptr<IO_Work>> &discard_vector)
{
    auto it = work_vector.begin();
    while (it != work_vector.end())
    {
        if (it + 1 != work_vector.end() && (*it)->offset + (*it)->size == (*(it + 1))->offset)
        {
            // TODO copy
            for (auto i = 0; i < (*it)->size; i++)
            {
                (*it)->buf16[(*it)->num_registers + i] = (*(it + 1))->buf16[i];
            }
            (*it)->size += (*(it + 1))->size;
            (*it)->num_registers += (*(it + 1))->size;
            discard_vector.emplace_back(*(it + 1));
            it = work_vector.erase(it + 1); // it now points to the element after the erased one
        }
        else
        {
            ++it;
        }
    }
    return 0;
}

int sort_IO_Work(std::vector<std::shared_ptr<IO_Work>> &work_vector)
{
    // Sorting the vector based on IO_Work->offset
    std::sort(work_vector.begin(), work_vector.end(),
              [](const std::shared_ptr<IO_Work> &a, const std::shared_ptr<IO_Work> &b) -> bool
              {
                  return a->offset < b->offset;
              });
    return 0;
}

using IO_point_ptr = std::shared_ptr<cfg::io_point_struct>;
using RetVar = std::variant<std::monostate, std::map<std::string, std::shared_ptr<cfg::io_point_struct>> *, IO_point_ptr>;

RetVar findMapVar(struct cfg &myCfg, std::vector<std::string> keys)
{
    // Look up outerKey in the outer map
    std::cout << "skipping key 0" << std::endl;
    if (keys.size() < 3)
    {
        std::cout << "Not enough keys found" << std::endl;
        return std::monostate{};
    }
    int key_idx = 0;
    if ((int)keys[0].size() == (int)0)
    {
        // std::cout << "skipping key 0" << std::endl;
        key_idx = 1;
    }
    // std::cout << "component_io_point_map" << myCfg.component_io_point_map << std::endl;
    auto outerIt = myCfg.component_io_point_map.find(keys[key_idx]);
    if (outerIt != myCfg.component_io_point_map.end())
    {
        std::cout << "found key 0" << std::endl;
        // If outerKey is found, look up innerKey in the inner map
        auto &innerMap = outerIt->second;
        auto innerIt = innerMap.find(keys[key_idx + 1]);
        if (innerIt != innerMap.end())
        {
            if ((int)keys.size() == key_idx + 2)
            {
                // If innerKey is also found, return the inner map
                std::cout << "found key 2" << std::endl;
                return &innerIt->second;
            }
            else
            {
                auto iMap = innerMap[keys[key_idx + 1]];
                auto mapIt = iMap.find(keys[key_idx + 2]);
                if (mapIt != iMap.end())
                {
                    std::cout << "found key 1" << std::endl;
                    return mapIt->second;
                }
            }
        }
    }
    // If any key not found, return nullopt
    return std::monostate{};
}

// deprecated
std::string getElementTypeString(simdjson::dom::element_type etype)
{
    switch (etype)
    {
    case simdjson::dom::element_type::ARRAY:
        return "ARRAY";
    case simdjson::dom::element_type::OBJECT:
        return "OBJECT";
    case simdjson::dom::element_type::INT64:
        return "INT64";
    case simdjson::dom::element_type::UINT64:
        return "UINT64";
    case simdjson::dom::element_type::DOUBLE:
        return "DOUBLE";
    case simdjson::dom::element_type::STRING:
        return "STRING";
    case simdjson::dom::element_type::BOOL:
        return "BOOL";
    case simdjson::dom::element_type::NULL_VALUE:
        return "NULL_VALUE";
    default:
        return "UNKNOWN";
    }
}

// deprecated
std::string processFieldAsString(const simdjson::dom::element &field_value)
{
    simdjson::dom::element_type etype = field_value.type();
    if (field_value.is_string())
    {
        return std::string(field_value);
    }
    else if (field_value.is_int64())
    {
        int64_t int_val;
        simdjson::error_code ec = field_value.get(int_val);
        if (ec == simdjson::SUCCESS)
        {
            return std::to_string(int_val);
        }
        else
        {
            return "Failed to extract int64 value: " + std::string(simdjson::error_message(ec));
        }
    }
    else if (field_value.is_uint64())
    {
        uint64_t uint_val;
        simdjson::error_code ec = field_value.get(uint_val);
        if (ec == simdjson::SUCCESS)
        {
            return std::to_string(static_cast<unsigned int>(uint_val));
        }
        else
        {
            return "Failed to extract uint64 value: " + std::string(simdjson::error_message(ec));
        }
    }
    else if (field_value.is_bool())
    {
        bool bool_val;
        if (field_value.get(bool_val) == simdjson::SUCCESS)
        {
            return bool_val ? "true" : "false";
        }
    }
    else if (field_value.is_object())
    {
        return "{...}"; // Some generic representation for objects, or you can serialize it if you want
    }
    else
    {
        return "Unknown or unhandled type encountered: " + getElementTypeString(etype);
    }
    return "Unknown Error";
}

// deprecated
std::any processField(const simdjson::dom::element &field_value)
{
    simdjson::dom::element_type etype = field_value.type();
    std::cout << " this is the type encountered: " << getElementTypeString(etype) << std::endl;
    if (field_value.is_string())
    {
        return std::any(std::string(field_value));
    }
    else if (field_value.is_int64())
    {
        int64_t int_val;
        simdjson::error_code ec = field_value.get(int_val);
        if (ec == simdjson::SUCCESS)
        {
            std::cout << "Extract int64 value: " << int_val << std::endl;
            return std::any(int_val);
        }
        else
        {
            std::cerr << "Failed to extract int64 value: " << simdjson::error_message(ec) << std::endl;
        }
    }
    else if (field_value.is_uint64())
    {
        uint64_t uint_val;
        simdjson::error_code ec = field_value.get(uint_val);
        if (ec == simdjson::SUCCESS)
        {
            return std::any(static_cast<unsigned int>(uint_val));
        }
        else
        {
            std::cerr << "Failed to extract uint64 value: " << simdjson::error_message(ec) << std::endl;
        }
    }
    else if (field_value.is_bool())
    {
        bool bool_val;
        if (field_value.get(bool_val) == simdjson::SUCCESS)
        {
            return std::any(bool_val);
        }
    }
    else if (field_value.is_object())
    {
        std::map<std::string, std::any> valueMap;
        for (auto [sub_key, sub_value] : field_value.get_object())
        {
            valueMap[std::string(sub_key)] = processField(sub_value);
        }
        return valueMap;
    }
    else
    {
        // In case of an unknown type or an error, you might want to log it:
        simdjson::dom::element_type etype = field_value.type();
        std::cerr << "Unknown or unhandled type encountered: " << getElementTypeString(etype) << std::endl;
    }
    // // In case of an unknown type or an error, you might want to log it:
    // std::cerr << "Unknown or unhandled type encountered." << std::endl;
    return {}; // returning an empty std::any
}

std::map<std::string, std::any> parseInputMessage(std::map<std::string, std::any> sysMap, const std::string &uri, const std::string &method, const std::string &body)
{
    std::map<std::string, std::any> baseMap;
    // Extract the id and key from the URI
    // /components/comp_sel_2440
    // TODO use the split thing
    std::string component = extractCompFromURI(uri);
    std::string id = extractCompIdFromURI(uri, component);
    std::string key = getKeyFromURI(uri);
    std::cout << __func__ << " found component [" << component << "]" << std::endl;
    std::cout << __func__ << " found id [" << id << "]" << std::endl;
    std::cout << __func__ << " found key [" << key << "]" << std::endl;
    // so the component is our base map
    // currently its always "components"
    // Create the initial baseMap entry for this id if it doesn't exist
    // if (!baseMap[id].has_value()) {
    //     baseMap[id] = std::map<std::string, std::any>();
    // }
    // Parsing JSON using simdjson
    simdjson::dom::parser parser;
    simdjson::dom::element obj = parser.parse(body);
    if (!obj.is_object())
    {
        std::cerr << "Expected a JSON object in the message body." << std::endl;
        return baseMap;
    }
    // this id is comp_sel_2440
    // it has to be one of our comps in sysmap if not forget it
    if (!sysMap[key].has_value())
    {
        std::cerr << "no key [" << key << "] match in sysMap." << std::endl;
        return baseMap;
    }
    // auto sysObj = sysMap[id];
    // the next key
    // if (!baseMap[id].has_value()) {
    //    baseMap[id] = std::map<std::string, std::any>();
    //}
    // If the URI contains a key, process the body as its value
    auto &componentMap = std::any_cast<std::map<std::string, std::any> &>(sysMap[key]);
    for (auto [key, value] : obj.get_object())
    {
        std::cout << " Processing key:" << key << std::endl;
        if (!componentMap[std::string(key)].has_value())
        {
            std::cerr << "no key match in sysMap." << std::endl;
        }
        else
        {
            if (value.is_object())
            {
                std::cout << "found key/obj [" << key << " ] match in sysMap." << std::endl;
                for (auto [nested_key, nested_value] : value.get_object())
                {
                    if (nested_key == "value")
                    { // if you're specifically looking for the "value" key
                        auto valstr = processFieldAsString(nested_value);
                        std::cout << "  found nested key/value [" << nested_key << " ] [" << valstr << "] in sysMap." << std::endl;
                    }
                }
            }
            else
            {
                auto foo = getElementTypeString(value.type());
                std::cout << "found key/value [" << key << " ] [" << value << "] type [" << foo << "] in sysMap." << std::endl;
            }
            // now value can be an obg with "value" i it or some kid of raw valye
        }
    }
    return baseMap;
    for (auto [key, value] : obj.get_object())
    {
        std::cout << " Processing key:" << key << std::endl;
        if (value.is_object())
        {
            std::map<std::string, std::any> subMap;
            for (auto [sub_key, sub_value] : value.get_object())
            {
                subMap[std::string(sub_key)] = processField(sub_value);
            }
            componentMap[std::string(key)] = subMap;
        }
        else
        {
            componentMap[std::string(key)] = processField(value);
        }
    }
    return baseMap;
}

// this may be deprecated
void test_parse_message(const char *uri, const char *method, const char *body)
{
    std::vector<std::tuple<std::string, std::string, std::string>> testCases = {
        {"/components/comp_sel_2440/fuse_monitoring", "set", "false"},
        {"/components/comp_sel_2440/fuse_monitoring", "set", "{\"value\":false}"},
        {"/components/comp_sel_2440", "set", "{\"fuse_monitoring\":false}"},
        {"/components/comp_sel_2440", "set", "{\"fuse_monitoring\": {\"value\":false}}"},
        {"/components/comp_sel_2440/voltage", "set", "1234"},
        {"/components/comp_sel_2440/voltage", "set", "{\"value\":1234}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":1234}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\": {\"value\":1234}}"},
        {"/components/comp_sel_2440/status", "set", "\"running\""},
        {"/components/comp_sel_2440/status", "set", "{\"value\":\"running\"}"},
        {"/components/comp_sel_2440", "set", "{\"status\":\"running\"}"},
        {"/components/comp_sel_2440", "set", "{\"status\": {\"value\":\"running\"}}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":1234, \"fuse_monitoring\":false, \"status\":\"running\" }"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":{\"value\":1234}, \"fuse_monitoring\":{\"value\":false}, \"status\":{\"value\":\"running\"}}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":{\"value\":1234, \"size\":2}, \"fuse_monitoring\":{\"value\":false}, \"status\":{\"value\":\"running\"}}"}};
    for (const auto &testCase : testCases)
    {
        std::string uri, method, body;
        std::tie(uri, method, body) = testCase;
        std::cout << "URI: " << uri << "\n";
        std::cout << "meth: " << method << "\n";
        std::cout << "Body: " << body << "\n";
        std::map<std::string, std::any> resultMap = parseMessage(uri, method, body);
        parseInputMessage(resultMap, uri, method, body);
        printMap(resultMap,0);
        std::cout << "--------------------\n";
    }
    if (uri && body)
    {
        std::map<std::string, std::any> resultMap = parseMessage(uri, method, body);
        printMap(resultMap,0);
        std::cout << "--------------------\n";
    }
    test_printMap();
}

std::map<std::string, std::any> parseMessage(const std::string &uri, const std::string method, const std::string &body)
{
    std::map<std::string, std::any> baseMap;
    // Parsing JSON using simdjson
    simdjson::dom::parser parser;
    simdjson::dom::element obj = parser.parse(body);
    // Extract the id and key from the URI
    std::string id = extractIdFromURI(uri);
    std::string key = getKeyFromURI(uri);
    // Create the initial baseMap entry for this id if it doesn't exist
    // if (!baseMap[id].has_value()) {
    //     baseMap[id] = std::map<std::string, std::any>();
    // }
    if (!obj.is_object())
    {
        std::cerr << "Expected a JSON object in the message body." << std::endl;
        return baseMap;
    }
    if (!baseMap[id].has_value())
    {
        baseMap[id] = std::map<std::string, std::any>();
    }
    // If the URI contains a key, process the body as its value
    auto &componentMap = std::any_cast<std::map<std::string, std::any> &>(baseMap[id]);
    for (auto [key, value] : obj.get_object())
    {
        std::cout << " Processing key:" << key << std::endl;
        if (value.is_object())
        {
            std::map<std::string, std::any> subMap;
            for (auto [sub_key, sub_value] : value.get_object())
            {
                subMap[std::string(sub_key)] = processField(sub_value);
            }
            componentMap[std::string(key)] = subMap;
        }
        else
        {
            componentMap[std::string(key)] = processField(value);
        }
    }
    return baseMap;
}

int test_extract()
{
    std::map<std::string, std::any> parsedMap;
    parsedMap = parseMessage("/components/comp_sel_2440/fuse_monitoring", "set", R"(false)");
    // Add code to display the map, merge with other parsed messages, etc.
    return 0;
}

// this tests the concept of merging maps
// we may not use it since we can create the results vecs from FimsInput
void test_merge_message(const char *uri, const char *method, const char *body)
{
    std::vector<std::tuple<std::string, std::string, std::string>> testCases = {
        {"/components/comp_sel_2440/fuse_monitoring", "set", "false"},
        {"/components/comp_sel_2440/hvac_status", "set", "{\"value\":false}"},
        {"/components/comp_sel_2440", "set", "{\"active_power\":23456}"},
        {"/components/comp_sel_2440", "set", "{\"fuse_monitoring\": {\"value\":false}}"},
        {"/components/comp_sel_2440/voltage", "set", "1234"},
        {"/components/comp_sel_2440/max_voltage", "set", "{\"value\":1234}"},
        {"/components/comp_sel_2440", "set", "{\"avg_voltage\":1234}"},
        {"/components/comp_sel_2440", "set", "{\"min_voltage\": {\"value\":1234}}"},
        {"/components/comp_sel_2440/status", "set", "\"running\""},
        {"/components/comp_sel_2440/old_status", "set", "{\"value\":\"running\"}"},
        {"/components/comp_sel_2440", "set", "{\"new_status\":\"running\"}"},
        {"/components/comp_sel_2440", "set", "{\"any_status\": {\"value\":\"running\"}}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":1234, \"fuse_monitoring\":false, \"status\":\"running\" }"},
        {"/components/comp_sel_2440", "set", "{\"avg_voltage\":{\"value\":1234}, \"old_fuse_monitoring\":{\"value\":false}, \"new_status\":{\"value\":\"running\"}}"},
        {"/components/comp_sel_2440", "set", "{\"voltage\":{\"value\":1234, \"size\":2}, \"fuse_monitoring\":{\"value\":false}, \"status\":{\"value\":\"running\"}}"}};
    std::map<std::string, std::any> baseMap;
    for (const auto &testCase : testCases)
    {
        std::string uri, method, body;
        std::tie(uri, method, body) = testCase;
        std::cout << "URI: " << uri << "\n";
        std::cout << "meth: " << method << "\n";
        std::cout << "Body: " << body << "\n";
        std::map<std::string, std::any> tmpMap = parseMessage(uri, method, body);
        mergeSubMaps(baseMap, tmpMap);
    }
    if (uri && method && body)
    {
        std::map<std::string, std::any> tmpMap = parseMessage(uri, method, body);
        mergeSubMaps(baseMap, tmpMap);
        std::cout << "--------------------\n";
    }
    printMap(baseMap,0);
    spdlog::memory_buf_t buffer;
    mapToBuffer(baseMap, buffer,0);
    spdlog::info("{}", fmt::to_string(buffer));
}

// TODO put Component_IO_point_map inside cfg
void addMapId(MapIdMap &imap, const int device_id, cfg::Register_Types register_type, cfg::io_point_struct *io_point)
{
    if (io_point)
    {
        if (imap.find(device_id) == imap.end())
        {
            imap[device_id] = std::map<cfg::Register_Types, std::map<int, cfg::io_point_struct *>>();
        }
        auto &itype = imap[device_id];
        if (itype.find(register_type) == itype.end())
        {
            itype[register_type] = std::map<int, cfg::io_point_struct *>();
        }
        auto &ioffset = itype[register_type];
        ioffset[io_point->offset] = io_point;
    }
    else
    {
        std::cout << "Item is nullptr!\n";
    }
}

//
// this is for random sets , we are able to bin the data into CompressedResults
// for processing pubs we.ll need to get back to the register structure , if all the mapped items are included in the message then the whole register set can be
// processed.
// this is really for the modbus server.
// the jsonMapOfConfig is the items
bool parseFimsMessage(struct cfg &myCfg, const Component_IO_point_map &items, std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result,const std::string &method, const std::string &uri, const std::string &body)
{
    simdjson::dom::parser parser;
    simdjson::dom::object json_obj;
    auto error = parser.parse(body).get(json_obj);
    if (error)
    {
        std::cout << "parse body failed [" << body << "]" << std::endl;
        return false; // JSON parsing failed
    }
    std::cout << "parse body OK" << std::endl;
    auto uri_tokens = split_string(uri, '/');
    if (uri_tokens.size() < 2)
    {
        std::cout << "uri_tokens fail size :" << uri_tokens.size() << " uri :" << uri << std::endl;
        return false; // Invalid URI
    }
    // auto uri_keys = split(uri_view, "/")
    // find component_io_point_map[uri[0]][uri[1]
    auto IO_point_map = myCfg.findIOPointMapFromUriFragments(uri_tokens);
    return true;
    std::cout << "uri_tokens OK size :" << uri_tokens.size() << std::endl;
    printf(" IO_point_map %p \n", (void *)IO_point_map);
    const auto &first_field = uri_tokens[1];
    if (items.find(first_field) == items.end())
    {
        std::cout << "components field not mapped in config :" << first_field << std::endl;
        return false; // The first field of the URI doesn't match with the map
    }
    // std::cout << "first_field :"<< first_field << " OK" << std::endl;
    const auto &io_point_id = uri_tokens[2];
    // std::cout << "item_id :"<< item_id << " OK" << std::endl;
    // If the URI provides a direct identification to an io_point
    // ie /components/comp_sel_2440/heartbeat 1234
    // or /components/comp_sel_2440/heartbeat '{"value":1234}'
    if (uri_tokens.size() == 4)
    {
        if (json_obj.size() != 1)
        {
            std::cout << "single uri body invalid :" << body << std::endl;
            return false; // Expected a direct value
        }
        for (auto [key, value] : json_obj)
        {
            if (key == io_point_id)
            {
                std::shared_ptr<cfg::io_point_struct> found_io_point = items.at(first_field).begin()->second.at(io_point_id);
                std::any foundValue;
                extractJsonValue(value, foundValue);
                result.emplace_back(found_io_point, foundValue);
                return true; // Successfully matched a specific io_point
            }
        }
        return false; // No match found
    }
    // Check the body for multiple items
    for (auto [key, value] : json_obj)
    {
        std::string key_str = std::string(key);
        // std::cout << " seeking io_point " << key_str << " in config" << std::endl;
        auto it = items.at(first_field).find(io_point_id);
        if (it != items.at(first_field).end())
        {
            auto it = items.at(first_field).at(io_point_id).find(key_str);
            if (it != items.at(first_field).at(io_point_id).end())
            {
                std::shared_ptr<cfg::io_point_struct> found_io_point = items.at(first_field).at(io_point_id).at(key_str);
                // std::shared_ptr<cfg::io_point_struct> foundItem = (it->second.begin())->second;
                std::any foundValue;
                if (value.is_object() && value["value"].error() == simdjson::SUCCESS)
                {
                    extractJsonValue(value["value"], foundValue);
                }
                else
                {
                    extractJsonValue(value, foundValue);
                }
                // std::cout << " Found io_point " << key_str << " in config  " << std::endl;
                result.emplace_back(found_io_point, foundValue);
            }
        }
        else
        {
            std::cout << " Unable to find io_point " << key_str << " in config , ignoring it " << std::endl;
            return false;
        }
    }
    return true; // Parsing and matching process completed successfully (even if no matches are found)
}

// may be deprecated



// may be deprecated
void mapToRawBuffer(const std::map<std::string, std::any> &baseMap, spdlog::memory_buf_t &buf, int indent = 0)
{
    for (const auto &[key, value] : baseMap)
    {
        for (int i = 0; i < indent; ++i)
        {
            // buf.append("  ", "  " + 2);
            // fmt::format_to(buf, "  ");
            fmt::format_to(std::back_inserter(buf), "  ");
        }
        fmt::format_to(std::back_inserter(buf), "{}: ", key);
        if (value.type() == typeid(std::map<std::string, std::any>))
        {
            fmt::format_to(std::back_inserter(buf), "\n");
            mapToRawBuffer(std::any_cast<std::map<std::string, std::any>>(value), buf, indent + 1);
        }
        else if (value.type() == typeid(int))
        {
            fmt::format_to(std::back_inserter(buf), "{}\n", std::any_cast<int>(value));
        }
        else if (value.type() == typeid(int64_t))
        {
            fmt::format_to(std::back_inserter(buf), "{}\n", std::any_cast<int64_t>(value));
        }
        else if (value.type() == typeid(double))
        {
            fmt::format_to(std::back_inserter(buf), "{}\n", std::any_cast<double>(value));
        }
        else if (value.type() == typeid(std::string))
        {
            fmt::format_to(std::back_inserter(buf), "{}\n", std::any_cast<std::string>(value));
        }
        else if (value.type() == typeid(bool))
        {
            fmt::format_to(std::back_inserter(buf), "{}\n", std::any_cast<bool>(value));
        }
    }
}

// deprecated
void mapToBuffer(const std::map<std::string, std::any> &baseMap, spdlog::memory_buf_t &buf, int indent )
{
    fmt::format_to(std::back_inserter(buf), "{{\n");
    bool firstItem = true;
    for (const auto &[key, value] : baseMap)
    {
        if (!firstItem)
        {
            fmt::format_to(std::back_inserter(buf), ",\n");
        }
        firstItem = false;
        for (int i = 0; i < indent + 1; ++i)
        {
            fmt::format_to(std::back_inserter(buf), "  ");
        }
        // simdjson::dom::element_type etype = value.type();
        // std::cout << " this is the type encountered: " << getElementTypeString(etype) << std::endl;
        // std::cout << " value type >>>" <<value.type().name() << std::endl;
        if (value.type() == typeid(std::map<std::string, std::any>))
        {
            fmt::format_to(std::back_inserter(buf), "\"{}\": ", key);
            mapToBuffer(std::any_cast<std::map<std::string, std::any>>(value), buf, indent + 1);
        }
        else
        {
            // buf.append(key.begin(), key.end());
            // fmt::format_to(std::back_inserter(buf), ": ");
            fmt::format_to(std::back_inserter(buf), "\"{}\": ", key);
            if (value.type() == typeid(int))
            {
                fmt::format_to(std::back_inserter(buf), "{}", std::any_cast<int>(value));
            }
            else if (value.type() == typeid(int64_t))
            {
                fmt::format_to(std::back_inserter(buf), "{}", std::any_cast<int64_t>(value));
            }
            else if (value.type() == typeid(double))
            {
                fmt::format_to(std::back_inserter(buf), "{}", std::any_cast<double>(value));
            }
            else if (value.type() == typeid(const char *))
            {
                fmt::format_to(std::back_inserter(buf), "\"{}\"", std::any_cast<const char *>(value));
            }
            else if (value.type() == typeid(std::string))
            {
                const auto &strValue = std::any_cast<std::string>(value);
                // buf.append(strValue.begin(), strValue.end());
                // fmt::format_to(std::back_inserter(buf), "\"{}\"", std::any_cast<std::string>(value));
                std::cout << " found strvalue [" << strValue << "]" << std::endl;
                fmt::format_to(std::back_inserter(buf), "\"{}\"", strValue);
            }
            else if (value.type() == typeid(bool))
            {
                fmt::format_to(std::back_inserter(buf), "{}", std::any_cast<bool>(value) ? "true" : "false");
            }
        }
    }
    fmt::format_to(std::back_inserter(buf), "\n");
    for (int i = 0; i < indent; ++i)
    {
        fmt::format_to(std::back_inserter(buf), "  ");
    }
    fmt::format_to(std::back_inserter(buf), "}}");
}

// may be deprecated
std::any parseValue(const std::string &mystr)
{
    simdjson::dom::parser parser;
    auto result = parser.parse(mystr);
    if (result.error())
    {
        std::cerr << simdjson::error_message(result.error()) << std::endl;
        return std::any(); // Return an empty any object
    }
    simdjson::dom::element obj = result.value();
    // Check if it's a simple type: string, number or boolean
    if (obj.is_string())
    {
        return std::any(std::string(obj));
    }
    else if (obj.is_int64())
    {
        return std::any(int64_t(obj));
    }
    else if (obj.is_double())
    {
        return std::any(double(obj));
    }
    else if (obj.is_bool())
    {
        return std::any(bool(obj));
    }
    // Check if it's an object with a "value" key
    if (obj.is_object())
    {
        simdjson::dom::element value_obj;
        if (!obj["value"].get(value_obj))
        {
            if (value_obj.is_string())
            {
                return std::any(std::string(value_obj));
            }
            else if (value_obj.is_int64())
            {
                return std::any(int64_t(value_obj));
            }
            else if (value_obj.is_double())
            {
                return std::any(double(value_obj));
            }
            else if (value_obj.is_bool())
            {
                return std::any(bool(value_obj));
            }
        }
    }
    return std::any(); // Return an empty any object for unsupported types
}

//
// read a config
// send it a uri as from fims listen
// see if you get a results vector
//
// now this is getting closer to the real thing.
bool gcom_msg_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg)
{
    // pull out the components into myCfg
    extract_components(jsonMapOfConfig, "components", myCfg, false);
    // extract the Component_IO_point_map
    Component_IO_point_map gcom_cfg;
    extract_structure(gcom_cfg, jsonMapOfConfig);
    std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> result;
    parseFimsMessage(myCfg, gcom_cfg, result, "set", "/components/comp_sel_2440", "{\"door_latch\": false, \"fire_relay\":true,\"disconnect_switch\":true}");
    parseFimsMessage(myCfg, gcom_cfg, result, "set", "/components/comp_sel_2440", "{\"heartbeat\":234}");
    auto res = result.size();
    std::cout << " result size :" << res << std::endl;
    std::cout << " result vector  >>>>:" << std::endl;
    printResultVector(result);
    bool ok = true;
    return ok;
}

int test_ParseValue()
{
    std::string str1 = R"({"value": "Hello"})";
    std::string str2 = R"(42)";
    std::string str3 = R"( true)";
    std::string str4 = R"({"value": 23.5})";
    std::any value1 = parseValue(str1);
    std::any value2 = parseValue(str2);
    std::any value3 = parseValue(str3);
    std::any value4 = parseValue(str4);
    // You can use std::any_cast to extract and use the values
    if (value1.has_value())
    {
        std::cout << "Value1: " << std::any_cast<std::string>(value1) << std::endl;
    }
    if (value2.has_value())
    {
        std::cout << "Value2: " << std::any_cast<int64_t>(value2) << std::endl;
    }
    if (value3.has_value())
    {
        std::cout << "Value3: " << std::any_cast<bool>(value3) << std::endl;
    }
    if (value4.has_value())
    {
        std::cout << "Value4: " << std::any_cast<double>(value4) << std::endl;
    }
    return 0;
}

int test_printMap()
{
    // Setup spdlog
    spdlog::set_level(spdlog::level::debug);
    std::map<std::string, std::any> testMap = {
        {"key1", std::map<std::string, std::any>{
                     {"subkey1", 123},
                     {"subkey2", "value2"},
                     {"subkey3", std::map<std::string, std::any>{
                                     {"subsubkey1", 45.6}}}}},
        {"key2", "value2"},
        {"key3", 789}};
    // spdlog::details::fixed_buffer<bufferSize> raw_buffer;
    // int raw_buffer = 4096;
    // spdlog::memory_buf_t buffer(raw_buffer);
    spdlog::memory_buf_t buffer;
    mapToBuffer(testMap, buffer);
    spdlog::info("{}", fmt::to_string(buffer));
    return 0;
}

// Dummy timer create function
void timer_create(const std::string &name, int frequency, int offset, void (*callback)(void *, void *), void *data)
{
    // For now, let's just print the timer setup
    // Cast the void pointer back to PublishGroup*
    PublishGroup *pg = static_cast<PublishGroup *>(data);
    std::cout << "Timer created with name: " << name << ", frequency: " << pg->frequency << ", offset: " << offset << std::endl;
}


// this is run once after start up 
// it will create the different publish_groups
// when connected to a timer, the callback will get a requests list generated and send the requrst to the iothread pub queue.
// as each result gets back from the iothread the main thread will collect all the results into a pub_vector 
// when they are all back we can then pub the results out.
// of course we'll have to decode the results and save the values in the local Itemmap

// bool extract_publish_groups(std::vector<std::shared_ptr<PublishGroup>> &publishGroups, ItemMap& items, const std::map<std::string, std::any>& jsonData) {
//     bool debug = false;
//     // Extract components array
//     auto rawComponents = getMapValue<std::vector<std::any>>(jsonData, "components");
//     if (!rawComponents.has_value()) {
//         std::cout << __func__ << " error: no components in jsonData" << std::endl;
//         return false;
//     }
//     std::cout << __func__ << " >>>> found components in jsonData" << std::endl;

//     for (const std::any& rawComponent : rawComponents.value()) {
//         if (rawComponent.type() == typeid(std::map<std::string, std::any>)) {
//             std::map<std::string, std::any> componentData = std::any_cast<std::map<std::string, std::any>>(rawComponent);
//             std::string componentId;
//             getItemFromMap(componentData, "component_id", componentId, std::string("components"), true, true, false);
//             std::string comp_id; 
//             getItemFromMap(componentData, "id", comp_id, std::string(), true, false, false);
//             int frequency;
//             getItemFromMap(componentData, "frequency", frequency, 1000, true, false, false);
//             int offset_time;
//             getItemFromMap(componentData, "offset_time", offset_time, 0, true, false, false);
//             int comp_device_id;
//             getItemFromMap(componentData, "device_id", comp_device_id, 255, true, false,  debug);


//             std::string groupName = componentId + "_" + comp_id;
//             auto pubGroup = std::make_shared<PublishGroup>(componentId, comp_id, frequency, offset_time); // Frequency and offset are hardcoded to 0 for now.

//             auto rawRegisters = getMapValue<std::vector<std::any>>(componentData, "registers");
//             if (!rawRegisters.has_value()) {
//                 throw std::runtime_error("Missing registers in component data.");
//             }

//             for (const std::any& rawRegister : rawRegisters.value()) {
//                 if (rawRegister.type() == typeid(std::map<std::string, std::any>)) {
//                     std::map<std::string, std::any> registerData = std::any_cast<std::map<std::string, std::any>>(rawRegister);

//                     CompressedItem compressedItem;
//                     int device_id;
//                     getItemFromMap(registerData, "device_id", device_id, comp_device_id, true, true,  debug);
//                     compressedItem.device_id = device_id;
                    

//                     std::string rtype;
//                     if (getItemFromMap(registerData, "type", rtype, std::string(), true, false,  debug)) {
//                         compressedItem.register_type_str = rtype;
//                     }

//                     int start_offset;
//                     if (getItemFromMap(registerData, "starting_offset", start_offset, 0, true, false,  debug)) {
//                         compressedItem.start_offset = start_offset;
//                     }
//                     compressedItem.end_offset = compressedItem.start_offset; // Assuming end_offset is the same for now.

//                     int number_of_registers;
//                     if (getItemFromMap(registerData, "number_of_registers", number_of_registers, 1, true, false,  debug)) {
//                         compressedItem.end_offset = compressedItem.start_offset + number_of_registers - 1;
//                         compressedItem.number_of_registers = number_of_registers;
//                     }
//                     auto rawMaps = getMapValue<std::vector<std::any>>(registerData, "map");
//                     if (!rawMaps.has_value()) {
//                         continue;  // No maps to process in this register
//                     }

//                     for (const std::any& rawMap : rawMaps.value()) {
//                         if (rawMap.type() == typeid(std::map<std::string, std::any>)) {
//                             std::map<std::string, std::any> mapData = std::any_cast<std::map<std::string, std::any>>(rawMap);

//                             std::string id; 
//                             getItemFromMap(mapData, "id", id, std::string("some_id"), true, false, false);
//                             // need to push this into the values vec structure[componentId][id][item->id] = item;
//                             // 

//                             std::cout << " adding comp :["<< componentId << "]  id :[" <<comp_id << "] item : "<< id << std::endl;
//                             auto itemP = findItem(items, componentId, comp_id, id);
//                             if (itemP)
//                             {

//                                 std::cout << " found comp :["<< componentId << "]  id :[" <<comp_id << "] item : "<< id << std::endl;
//                                 std::any foundValue = itemP->value;
//                                 compressedItem.values.emplace_back(itemP, foundValue);
//                             }
//                         }
//                     }

//                     // Add the CompressedItem to the pubGroup
//                     pubGroup->addCompressedItem(compressedItem);
//                 }
//             }
//             publishGroups.push_back(pubGroup);
//         }
//     }

//     return true;
// }


std::string extractCompFromURI(const std::string &uri)
{
    // Finding the start of the component ID
    std::size_t start = uri.find("/");
    start += std::strlen("/"); // moving past "/"
    // Finding the end of the component ID (next slash or end of the string)
    std::size_t end = uri.find("/", start);
    // Extracting the component ID
    if (end != std::string::npos)
    {
        return uri.substr(start, end - start);
    }
    else
    {
        return uri.substr(start); // Till the end of the string
    }
}

std::string extractCompIdFromURI(const std::string &uri, const std::string &component)
{
    // Finding the start of the component ID
    std::size_t start = uri.find(component);
    if (start == std::string::npos)
    {
        return ""; // or throw an error if appropriate
    }
    start += std::strlen("/"); // moving past "/components/"
    // Finding the end of the component ID (next slash or end of the string)
    std::size_t end = uri.find("/", start);
    // Extracting the component ID
    if (end != std::string::npos)
    {
        return uri.substr(start, end - start);
    }
    else
    {
        return uri.substr(start); // Till the end of the string
    }
}

std::string extractIdFromURI(const std::string &uri)
{
    // Finding the start of the component ID
    std::size_t start = uri.find("/components/");
    if (start == std::string::npos)
    {
        return ""; // or throw an error if appropriate
    }
    start += std::strlen("/components/"); // moving past "/components/"
    // Finding the end of the component ID (next slash or end of the string)
    std::size_t end = uri.find("/", start);
    // Extracting the component ID
    if (end != std::string::npos)
    {
        return uri.substr(start, end - start);
    }
    else
    {
        return uri.substr(start); // Till the end of the string
    }
}

std::string getKeyFromURI(const std::string &uri)
{
    if (uri.back() == '/')
    {
        return "";
    }
    auto pos = uri.find_last_of('/');
    if (pos == std::string::npos)
    {
        return uri;
    }
    return uri.substr(pos + 1);
}

void mergeSubMaps(std::map<std::string, std::any> &base, const std::map<std::string, std::any> &toMerge)
{
    for (const auto &[key, value] : toMerge)
    {
        if (base.find(key) != base.end() && base[key].type() == typeid(std::map<std::string, std::any>) && value.type() == typeid(std::map<std::string, std::any>))
        {
            // If the key exists in both maps and both values are maps, merge them recursively
            mergeSubMaps(std::any_cast<std::map<std::string, std::any> &>(base[key]), std::any_cast<const std::map<std::string, std::any> &>(value));
        }
        else
        {
            // Otherwise, simply overwrite (or add) the value from 'toMerge' into 'base'
            base[key] = value;
        }
    }
}


std::shared_ptr<cfg::io_point_struct> findItem(const Component_IO_point_map &items, const std::string &component, const std::string &id, const std::string &name)
{
    std::cout << " Seeking :" << component << "/" << id << "  " << name << std::endl;
    auto componentIt = items.find(component);
    if (componentIt != items.end())
    {
        const auto &idMap = componentIt->second;
        auto idIt = idMap.find(id);
        if (idIt != idMap.end())
        {
            for (const auto &innerIdPair : idIt->second)
            {
                const std::shared_ptr<cfg::io_point_struct> &io_point = innerIdPair.second;
                if (io_point && io_point->id == name)
                {
                    return io_point;
                }
            }
        }
    }
    return nullptr;
}

struct type_map *gcom_get_comp(struct cfg &myCfg, std::string component, std::string id, bool debug)
{
    std::ostringstream oss;
    // if (comps.find(component) != comps.end()) {
    //     if (comps[component].find(id) != comps[component].end()) {
    //         if (debug) {
    //             oss << "\"gcom_get_comp\" :{\n"
    //                 << "                  \"status\": \"success\",\n"
    //                 << "                  \"component\": \"" << component << "\",\n"
    //                 << "                  \"id\": \"" << id << "\"\n"
    //                 << "                  }";
    //             std::cout << oss.str() << std::endl;
    //         }
    //         return &comps[component][id];
    //     }
    // }
    if (debug)
    {
        oss.str(""); // Clear the stringstream for new data
        oss << "\"gcom_get_comp\" :{\n"
            << "                   \"status\": \"error\",\n"
            << "                   \"component\": \"" << component << "\",\n"
            << "                   \"id\": \"" << id << "\",\n"
            << "                   \"message\": \"component '" << component << "' with id '" << id << "' not found\"\n"
            << "                   }";
        std::cout << oss.str() << std::endl;
    }
    return nullptr;
}

// /// @brief show the types found in a cfg
std::string gcom_show_types(struct cfg &myCfg)
{
    std::ostringstream oss;
    oss << "\"Types found\": {\n";
    bool firstTypeItem = true;
    for (const auto &type_item : types)
    {
        if (!firstTypeItem)
        {
            oss << ",\n";
        }
        oss << "      \"" << type_item.first << "\": {\n";
        bool firstTypeReg = true;
        for (const auto &type_reg : type_item.second)
        {
            if (!firstTypeReg)
            {
                oss << ",\n";
            }
            oss << "         \"" << type_reg.first << "\": {";          // If the type_map has any properties, add them here
            oss << "\"map_id\": \"" << type_reg.second.map->id << "\""; // Adding the map_id property
            oss << "}";
            firstTypeReg = false;
        }
        oss << "\n      }";
        firstTypeItem = false;
    }
    oss << "\n}";
    std::cout << oss.str() << "\n";
    return oss.str();
}

bool gcom_config_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg)
{
    gcom_show_FirstLevel(jsonMapOfConfig, "connection");
    if (jsonMapOfConfig.find("connection") != jsonMapOfConfig.end())
    {
        auto &connectionAny = jsonMapOfConfig["connection"];
        if (connectionAny.type() == typeid(std::map<std::string, std::any>))
        {
            std::map<std::string, std::any> connectionMap = std::any_cast<std::map<std::string, std::any>>(connectionAny);
            // std::cout << " Connection ... "<< std::endl;
            gcom_show_FirstLevel(connectionMap, "types");
        }
    }
    bool ok = true;
    bool debug = false;
    getItemFromMap(jsonMapOfConfig, "connection.port", myCfg.connection.port, static_cast<int>(503), true, true, debug);
    // ok =
    getItemFromMap(jsonMapOfConfig, "connection.ip_address", myCfg.connection.ip_address, std::string("172.3.0.2"), true, true, debug);
    // ok =
    getItemFromMap(jsonMapOfConfig, "connection.debug", myCfg.connection.debug, false, true, true, debug);
    // ok =
    getItemFromMap(jsonMapOfConfig, "connection.device_id", myCfg.connection.device_id, 1, true, true, debug);
    extract_components(jsonMapOfConfig, "components", myCfg, false);
    // now create a list of types and offsets
    // used when we get a register type and offset
    gcom_extract_types(myCfg);
    gcom_show_types(myCfg);
    // test this list
    // auto register_type =
    gcom_get_type("Discrete Inputs", 395, debug);
    // now create a uri/id
    // map a type->offset to a structure with component, register, and a map
    gcom_extract_comps(myCfg);
    // auto ctype =
    gcom_get_comp(myCfg, "comp_sel_2440", "fuse_monitoring", debug);
    // now create pub_lists frequency, offset , components  and pub_map
    // get subs
    gcom_extract_subs(myCfg);
    gcom_show_subs(myCfg, debug);
    // get pubs
    gcom_extract_pubs(myCfg);
    //gcom_show_pubs(myCfg, debug);
    //
    Component_IO_point_map structure;
    extract_structure(structure, jsonMapOfConfig);
    // Component_IO_point_map structure;
    // Component_IO_point_map structure = extract_structure(jsonMapOfConfig);
    printComponentIOPointMap(structure);
    std::shared_ptr<cfg::io_point_struct> io_point = findItem(structure, "/components", "comp_sel_2440", "door_latch");
    if (io_point)
    {
        printIOPoint(io_point);
        // std::cout << "Found io_point with ID: " << io_point->id << ", Name: " << io_point->name << std::endl;
    }
    else
    {
        std::cout << "IO_point not found!" << std::endl;
    }
    return ok;
}

void clearChan(bool debug)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    if (debug)
    {
        std::cout << " Clearing respChan" << std::endl;
    }
    while (io_respChan.receive(io_work, delay))
    {
        // runThreadWork(io_thread, io_work, debug);
        if (debug)
        {
            std::cout
                << "  start " << io_work.get()->offset
                << "  number " << io_work.get()->num_registers
                << std::endl;
        }
    }
    if (debug)
    {
        std::cout << " Cleared respChan" << std::endl;
    }
}

// this create the Item map
// from the jsonMapOfConfig
Component_IO_point_map extract_structure(Component_IO_point_map &structure, const std::map<std::string, std::any> &jsonData)
{
    bool debug = false;
    // Component_IO_point_map structure;
    // Extract components array
    auto rawComponents = getMapValue<std::vector<std::any>>(jsonData, "components");
    if (!rawComponents.has_value())
    {
        std::cout << __func__ << " error no  components in jsonData" << std::endl;
        return structure;
        // throw std::runtime_error("Missing components in JSON data.");
    }
    std::cout << __func__ << " >>>> found components in jsonData" << std::endl;
    for (const std::any &rawComponent : rawComponents.value())
    {
        if (rawComponent.type() == typeid(std::map<std::string, std::any>))
        {
            std::map<std::string, std::any> componentData = std::any_cast<std::map<std::string, std::any>>(rawComponent);
            std::string componentId;
            getItemFromMap(componentData, "component_id", componentId, std::string("components"), true, true, false);
            std::cout << __func__ << " >>>> found componentsId :" << componentId << " in jsonData" << std::endl;
            std::string id;
            getItemFromMap(componentData, "id", id, std::string(), true, false, false);
            int device_id;
            getItemFromMap(componentData, "device_id", device_id, 255, true, true, false);
            auto rawRegisters = getMapValue<std::vector<std::any>>(componentData, "registers");
            if (!rawRegisters.has_value())
            {
                throw std::runtime_error("Missing registers in component data.");
            }
            std::cout << __func__ << " >>>> found registers in jsonData" << std::endl;
            for (const std::any &rawRegister : rawRegisters.value())
            {
                if (rawRegister.type() == typeid(std::map<std::string, std::any>))
                {
                    std::map<std::string, std::any> registerData = std::any_cast<std::map<std::string, std::any>>(rawRegister);
                    std::string register_type_str;
                    getItemFromMap(registerData, "type", register_type_str, std::string(), true, false, debug);
                    auto rawMaps = getMapValue<std::vector<std::any>>(registerData, "map");
                    if (!rawMaps.has_value())
                    {
                        continue; // No maps to process in this register
                    }
                    for (const std::any &rawMap : rawMaps.value())
                    {
                        if (rawMap.type() == typeid(std::map<std::string, std::any>))
                        {
                            std::map<std::string, std::any> mapData = std::any_cast<std::map<std::string, std::any>>(rawMap);
                            // this needs to be struct cfg::io_point_struct
                            // std::vector<cfg::io_point_struct> extract_io_point_map(struct cfg::register_group_struct* register_group,  std::any& rawMapData) {
                            // but we may need a different root object here
                            // look at extract components
                            std::shared_ptr<cfg::io_point_struct> io_point = std::make_shared<cfg::io_point_struct>();
                            // io_point->id =
                            getItemFromMap(mapData, "id", io_point->id, std::string("some_id"), true, false, false);
                            // io_point->name =
                            getItemFromMap(mapData, "name", io_point->name, std::string("some_name"), true, false, false);
                            io_point->register_type_str = register_type_str;
                            // io_point->offset =
                            getItemFromMap(mapData, "offset", io_point->offset, 0, true, false, debug);
                            // io_point->size =
                            getItemFromMap(mapData, "size", io_point->size, 1, true, true, false);
                            io_point->device_id = device_id;
                            structure[componentId][id][io_point->id] = io_point;
                        }
                    }
                }
            }
        }
    }
    return structure;
}

///////////////////////////////////////
///////////// PRINT STUFF /////////////
///////////////////////////////////////
void printComponentIOPointMap(const Component_IO_point_map &items)
{
    for (const auto &component : items)
    {
        std::cout << "Component: " << component.first << "\n";
        for (const auto &idPair : component.second)
        {
            std::cout << "  Uri: " << idPair.first << "\n";
            for (const auto &itemPair : idPair.second)
            {
                std::cout << "    Inner ID: " << itemPair.first << "\n";
                auto &io_point = itemPair.second;
                printIOPoint(io_point);
            }
        }
    }
}

void printIOPoint(const std::shared_ptr<cfg::io_point_struct> io_point)
{
    if (io_point)
    {
        std::cout << "Here is the io_point " << std::endl;
        std::cout << "     ID: " << io_point->id << "\n";
        std::cout << "     Name: " << io_point->name << "\n";
        std::cout << "     Type: " << io_point->register_type_str << "\n";
        std::cout << "     Offset: " << io_point->offset << "\n";
        std::cout << "     Size: " << io_point->size << "\n";
        std::cout << "     Device ID: " << io_point->device_id << "\n";
    }
    else
    {
        std::cout << "Item is nullptr!\n";
    }
}

// after getting in a fims message we'll get a vector of items found in the message and values
// if this is a SET message then we'll have to compress the result vector and the encode the values into the compressed buffer.
// a SET message will also set the local values.
// the iothread can then send the compressed buffer to the device and return the result to the main thread.
// the pubs already have compressed resuts ready to send in that case the iothread will return data in the iobuffer and we'll have to decode that into the actual values.
// a GET message will only access the local data values
//
void printResultVector(const std::vector<std::pair<std::shared_ptr<cfg::io_point_struct>, std::any>> &result)
{
    for (const auto &[io_point_ptr, anyValue] : result)
    {
        // Accessing members of the io_point struct via the shared_ptr
        // printIOPoint(io_point_ptr);
        std::cout << "Item Details:\n";
        std::cout << "         ID: " << io_point_ptr->id << "\n";
        std::cout << "         Name: " << io_point_ptr->name << "\n";
        std::cout << "         Type: " << io_point_ptr->register_type_str << "\n";
        std::cout << "         Offset: " << io_point_ptr->offset << "\n";
        std::cout << "         Size: " << io_point_ptr->size << "\n";
        std::cout << "         Device ID: " << io_point_ptr->device_id << "\n";
        // Extracting and printing value from std::any
        if (anyValue.type() == typeid(bool))
        {
            std::cout << "             Value: " << std::boolalpha << std::any_cast<bool>(anyValue) << "\n";
        }
        else if (anyValue.type() == typeid(int64_t))
        {
            std::cout << "             Value: " << std::any_cast<int64_t>(anyValue) << "\n";
        }
        else if (anyValue.type() == typeid(int))
        {
            std::cout << "             Value: " << std::any_cast<int>(anyValue) << "\n";
        }
        else if (anyValue.type() == typeid(std::string))
        {
            std::cout << "             Value: " << std::any_cast<std::string>(anyValue) << "\n";
        }
        else if (anyValue.type() == typeid(double))
        {
            std::cout << "             Value: " << std::any_cast<double>(anyValue) << "\n";
        }
        else
        {
            std::cout << "using unhandled type :" << anyValue.type().name();
        } // Add more types as needed
          // You can add more types if needed
        std::cout << "--------------------\n";
    }
}

/*
/// @brief
    test code to decode a map object
*/

std::string gcom_show_FirstLevel(const std::map<std::string, std::any> &m, std::string key)
{
    std::ostringstream oss;
    oss << "\"" << key << "\" :{\n";
    bool isFirstItem = true;
    for (const auto &[key, value] : m)
    {
        if (!isFirstItem)
        {
            oss << ",\n";
        }
        oss << "   \"" << key << "\": ";
        if (value.type() == typeid(int))
        {
            oss << std::any_cast<int>(value);
        }
        else if (value.type() == typeid(int64_t))
        {
            oss << std::any_cast<int64_t>(value);
        }
        else if (value.type() == typeid(double))
        {
            oss << std::any_cast<double>(value);
        }
        else if (value.type() == typeid(std::string))
        {
            oss << "\"" << std::any_cast<std::string>(value) << "\""; // Quoted because it's a string in JSON
        }
        else
        {
            // Unknown types will not be added to the JSON to maintain valid format
            continue;
        }
        isFirstItem = false;
    }
    oss << "\n}\n";
    std::cout << oss.str();
    return oss.str();
}

// TODO put subs in config

std::string gcom_show_subs(struct cfg &myCfg, bool debug)
{
    std::ostringstream oss;
    oss << "\"subs\": [";
    bool first = true;
    for (const auto &sub : subs)
    {
        if (!first)
            oss << ",\n         \"" << sub << "\"";
        else
            oss << "\n         \"" << sub << "\"";
        first = false;
    }
    oss << "\n        ]\n";
    if (debug)
        std::cout << oss.str() << "\n";
    return oss.str();
}

// TODO put pubs in config
// std::string gcom_show_pubs(struct cfg &myCfg, bool debug)
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

///////////////////////////////////////
//////////// TEST METHODS /////////////
///////////////////////////////////////
// test_uri_body
// extract the strucure find all the register_group maps and see if they work
bool gcom_points_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *decode)
{
    bool debug = false;
    // pull out connection from jsonMapOfConfig
    Logging::Init("gcom_points", (const int)0, (const char **)nullptr);
    // extract_connection(jsonMapOfConfig, "connection", myCfg);
    // // pull out the components into myCfg
    // extract_components(jsonMapOfConfig, "components", myCfg);
    std::cout << std::endl;
    std::cout << "points test ..." << std::endl;
    double runTime = 0.0;
    // int jobs  = 0;
    //    std::string mygo(go);
    //   auto run = (mygo == "run") ;
    int idx = 0;
    auto io_thread = make_IO_Thread(idx, myCfg.connection.ip_address.c_str(), myCfg.connection.port, myCfg.connection.connection_timeout, myCfg);
    std::cout << " connection ip  ..." << myCfg.connection.ip_address << ":" << myCfg.connection.port << "   ->\n";
    SetupModbusForThread(myCfg, io_thread, debug);
    io_thread->jobs = 0;
    std::string repto("");
    for (int runs = 0; runs < 3; ++runs)
    {
        std::cout << " run  : " << runs << std::endl;
        for (const auto &component : myCfg.components)
        { // Assuming components is a std::vector or similar container
            if (debug)
                std::cout << " component : " << component->id
                          << " Time  Now:" << get_time_double()
                          << " Run Time :" << runTime
                          << std::endl;
            for (const auto &register_group : component->register_groups)
            { // Assuming register_groups is a std::vector or similar container inside the component
                if (debug)
                    std::cout << " >>>>  "
                              << std::left << std::setw(20)
                              << register_group->register_type_str << " start :"
                              << register_group->starting_offset
                              << std::endl;
                for (const auto &io_point : register_group->io_point_map)
                { // Assuming io_point_map is a std::vector or similar container inside the register
                    // auto regshr = map->register_group.lock();
                    if (debug)
                        std::cout << ">>>>>>>>\t\t\t\t"
                                  << std::left << std::setw(20)
                                  << io_point->id
                                  //<<  " dev " << regshr->device_id
                                  << "\toffset " << io_point->offset
                                  << "\tsize " << io_point->size
                                  << "\tdevice_id " << io_point->device_id
                                  << std::endl;
                    std::shared_ptr<IO_Work> io_work;
                    {
                        // double tNow = get_time_double();
                        // std::cout   << ">>>>>>>> start time #1 :" << tNow
                        //                 << " delay since start " << tNow-tStart
                        //                 << std::endl;
                        io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->size, io_point->reg16, io_point->reg8, strToWorkType("get", false));
                        // double tEnd = get_time_double();
                        // std::cout   << ">>>>>>>> done time #1 :" << tEnd
                        //     << " elapsed: " << tEnd - tNow
                        //     << " delay since start "  << tEnd - tStart
                        //     << std::endl;
                    }
                    // std::cout << " start runThreadWork  device_id "<< io_point->device_id<< std::endl;
                    // io_work->device_id = 2;
                    // io_work->jobs = 0;
                    bool enabled = true;
                    // double tNow = get_time_double(); // to do use io_work time
                    if (!io_point->is_enabled)
                    {
                        enabled = false;
                    }
                    if (enabled)
                    {
                        bool mydebug = true;
                        check_item_debounce(enabled, io_point, mydebug);
                    }
                    // we'll have to work on decode before we can tackle the "get" deadband.
                    // if(enabled)
                    // {
                    //     if(io_point->use_deadband)
                    //     {
                    //         io_point->last_float_val = 0.0; // TODO(float) io_point->last_value;
                    //         auto dbval = io_point->float_val - io_point->last_float_val;
                    //         if (dbval < 0.0 )
                    //             dbval *= -1.0;
                    //         if (dbval < io_point->deadband)
                    //         {
                    //             enabled = false;
                    //         }
                    //     }
                    // }
                    if (enabled)
                    {
                        io_work->io_points.emplace_back(io_point);
                        // double tNow = get_time_double();
                        // std::cout   << ">>>>>>>> start time #2 :" << tNow
                        //             << " delay since start " << tNow-tStart
                        //             << std::endl;
                        runThreadWork(myCfg, io_thread, io_work, debug);
                        // TODO save last_float_val and val
                        // double tEnd = get_time_double();
                        // std::cout   << ">>>>>>>> done time #2 :" << tEnd
                        //     << "elapsed: " << tEnd - tNow
                        //     << " delay since start "  << tEnd - tStart
                        //     << std::endl;
                        if (io_work->errors < 0)
                        {
                            io_point->is_enabled = false;
                            std::cout << "\n\n>>>>>>>>>>   runThreadWork  failed for [/" << component->component_id << "/" << component->id << ":" << io_point->id
                                      << "] offset :"
                                      << io_point->offset
                                      << " size :"
                                      << io_point->size
                                      << "\n\n"
                                      << std::endl;
                        }
                        else
                        {
                            std::string rundecode;
                            if (decode)
                            {
                                rundecode = decode;
                            }
                            if (rundecode == "decode")
                            {
                                // std::cout << "OK lets decode this stuff" << std::endl;
                                std::stringstream ss;
                                gcom_modbus_decode(io_work, ss, myCfg);
                            }
                            // since this is a get lets decode the incoming data
                        }
                        runTime += (io_work->tDone - io_work->tIo);
                        io_work->io_points.clear();
                    }
                    else
                    {
                        std::cout << "\n\n>>>>>>>>>>   runThreadWork  skipping  [/" << component->component_id << "/" << component->id << ":" << io_point->id
                                  << "] offset :"
                                  << io_point->offset
                                  << " size :"
                                  << io_point->size
                                  << "\n\n"
                                  << std::endl;
                    }
                    // jobs += io_work->jobs;
                    // std::cout << "                  done runThreadWork" << std::endl;
                    // if (run)
                    //     std::cout << " test_iothread deprecated" << std::endl;
                    //      test_iothread(myCfg.connection.ip_address.c_str(), myCfg.connection.port, "poll", map.register_group->device_id, register_group.type.c_str(), map.offset, 0, false);
                }
            }
            std::cout << ">>> done runThreadWork now : " << get_time_double() << " run time : " << runTime
                      << " Jobs : " << io_thread->jobs
                      << std::endl;
        }
        if (runs == 1)
        {
            std::cout << " sleeping for 500 mS to test debounce " << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    CloseModbusForThread(io_thread, debug);
    bool ok = true;
    return ok;
}

bool gcom_point_type_test(std::map<std::string, std::any> jsonMapOfConfig, struct cfg &myCfg, const char *ptype, const char *decode)
{
    bool debug = false;
    // pull out connection from jsonMapOfConfig
    Logging::Init("gcom_point_type", (const int)0, (const char **)nullptr);
    std::cout << std::endl;
    std::cout << "point type test ..." << std::endl;
    double runTime = 0.0;
    int idx = 0;
    auto io_thread = make_IO_Thread(idx, myCfg.connection.ip_address.c_str(), myCfg.connection.port, myCfg.connection.connection_timeout, myCfg);
    std::cout << " connection ip  ..." << myCfg.connection.ip_address << ":" << myCfg.connection.port << "   ->\n";
    SetupModbusForThread(myCfg, io_thread, debug);
    io_thread->jobs = 0;
    for (int runs = 0; runs < 2; ++runs)
    {
        std::cout << " run  : " << runs << std::endl;
        for (const auto &component : myCfg.components)
        { // Assuming components is a std::vector or similar container
            if (debug)
                std::cout << " component : " << component->id
                          << " Time  Now:" << get_time_double()
                          << " Run Time :" << runTime
                          << std::endl;
            for (const auto &register_group : component->register_groups)
            { // Assuming register_groups is a std::vector or similar container inside the component
                if (debug)
                    std::cout << " >>>>  "
                              << std::left << std::setw(20) << register_group->register_type_str
                              << " start :" << register_group->starting_offset
                              << std::endl;
                if (register_group->register_type_str != ptype)
                    continue;
                std::cout << " >>>>  "
                          << std::left << std::setw(20)
                          << register_group->register_type_str
                          << " start :" << register_group->starting_offset
                          << " num :" << register_group->number_of_registers
                          << std::endl;
                // continue;
                // std::shared_ptr<IO_Work> io_work;
                std::vector<std::shared_ptr<IO_Work>> io_work_vec;
                std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec;
                // {
                //     io_work = make_work(register_group->register_type, register_group->device_id
                //                                 , register_group->starting_offset, register_group->number_of_registers, nullptr
                //                                 , nullptr, strToWorkType("get", false));
                // }
                for (const auto &io_point : register_group->io_point_map)
                { // Assuming io_point_map is a std::vector or similar container inside the register
                    // auto regshr = map->register_group.lock();
                    if (debug)
                        std::cout << ">>>>>>>>\t\t\t\t"
                                  << std::left << std::setw(20)
                                  << io_point->id
                                  //<<  " dev " << regshr->device_id
                                  << "\toffset " << io_point->offset
                                  << "\tsize " << io_point->size
                                  << "\tdevice_id " << io_point->device_id
                                  << std::endl;
                    bool enabled = true;
                    // if(io_point->packer)
                    // {
                    //     enabled = false;
                    // }
                    if (!io_point->is_enabled)
                    {
                        enabled = false;
                    }
                    if (enabled)
                    {
                        bool mydebug = true;
                        check_item_debounce(enabled, io_point, mydebug);
                    }
                    if (enabled)
                    {
                        // stack all the
                        // io_work->io_points.emplace_back(io_point);
                        io_map_vec.emplace_back(io_point);
                        // check_work_(io_work_vec, io_work, myCfg, true);
                        // TODO now we have to split them
                        // runThreadWork(io_thread, io_work, debug);
                        // if (io_work->errors < 0)
                        //  {
                        //      io_point->is_enabled = false;
                        //      std::cout << "\n\n>>>>>>>>>>   runThreadWork  failed for [/"<< component->component_id << "/"<< component->id <<":" << io_point->id
                        //          << "] offset :"
                        //          << io_point->offset
                        //          << " size :"
                        //          << io_point->size
                        //          << "\n\n"
                        //          << std::endl;
                        //  }
                        //  else
                        //  {
                        //      std::string rundecode;
                        //      if(decode)
                        //      {
                        //          rundecode = decode;
                        //      }
                        //      if (rundecode == "decode")
                        //      {
                        //          //std::cout << "OK lets decode this stuff" << std::endl;
                        //          std::stringstream ss;
                        //          gcom_modbus_decode(io_work, ss, myCfg);
                        //      }
                        //      // since this is a get lets decode the incoming data
                        // }
                        // runTime += (io_work->tDone - io_work->tIo);
                    }
                    else
                    {
                        std::cout << "\n\n>>>>>>>>>>   runThreadWork  skipping  [/" << component->component_id << "/" << component->id << ":" << io_point->id
                                  << "] offset :"
                                  << io_point->offset
                                  << " size :"
                                  << io_point->size
                                  << "\n\n"
                                  << std::endl;
                    }
                }
                // check_work_items
                check_work_items(io_work_vec, io_map_vec, myCfg, "get", false, true);
                io_map_vec.clear();
                for (auto io_work : io_work_vec)
                {
                    std::cout << ">>>>>>>>>> io_work io_point  start: " << io_work->offset
                              << " num " << io_work->num_registers
                              << " device_id " << io_work->device_id
                              << std::endl;
                    std::string rundecode;
                    if (decode)
                    {
                        rundecode = decode;
                    }
                    if (rundecode == "decode")
                    {
                        std::cout << ">>>>>>>>>> runThreadWork   start offset: " << io_work->offset
                                  << " num :" << io_work->num_registers
                                  << std::endl;
                        runThreadWork(myCfg, io_thread, io_work, debug);
                        std::cout << ">>>>>>>>>> runThreadWork  done errors: " << io_work->errors
                                  << "\n\n"
                                  << std::endl;
                    }
                }
            }
            std::cout << ">>> done runThreadWork now : " << get_time_double() << " run time : " << runTime
                      << " Jobs : " << io_thread->jobs
                      << std::endl;
        }
        if (runs == 1)
        {
            std::cout << " sleeping for 500 mS to test debounce " << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    CloseModbusForThread(io_thread, debug);
    bool ok = true;
    return ok;
}

bool test_findMapVar(std::shared_ptr<cfg::io_point_struct> &io_point, const struct cfg &myCfg, const std::vector<std::string> &uri_keys, std::string kvar = "")
{
    int key_idx = 0;
    if ((uri_keys.size() > 1) && (uri_keys[0].size() == 0))
        key_idx = 1;
    std::string myvar = kvar;
    if ((int)uri_keys.size() < (key_idx + 3))
    {
        myvar = kvar;
    }
    else
    {
        myvar = uri_keys[key_idx + 2];
    }
    // return false;
    // auto myvar = uri_keys[key_idx + 2];
    // std::cout   << "         func : #1 "<< __func__ << " myVAR [" << myvar << "] \n";
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

bool test_findMapMap(std::map<std::string, std::shared_ptr<cfg::io_point_struct>> &map_result, const struct cfg &myCfg, const std::vector<std::string> &uri_keys)
{
    int key_idx = 0;
    if ((uri_keys.size() > 1) && (uri_keys[0].size() == 0))
        key_idx = 1;
    if ((int)uri_keys.size() >= (key_idx + 2))
    {
        std::cout << "         func : #1 " << __func__ << "\n";
        try
        {
            // Using at() for safe access. Catch exceptions if key not found
            const auto &myComp = myCfg.component_io_point_map.at(uri_keys[key_idx]);
            std::cout << "         func : #2 " << __func__ << "\n";
            map_result = myComp.at(uri_keys[key_idx + 1]);
            return true;
        }
        catch (const std::out_of_range &)
        {
            // If you want to log or handle the exception here, you can do it.
            throw; // Then, rethrow the exception.
        }
    }
    // Throw an exception if there aren't enough keys provided
    throw std::out_of_range("Not enough keys provided or key not found");
    return false;
}

bool test_uri(struct cfg &myCfg, const char *uri)
{
    std::cout << "client_name #2 " << myCfg.client_name << "\n";
    for (auto &myComp : myCfg.component_io_point_map)
    {
        for (auto &register_group : myComp.second)
        {
            for (auto &myMap : register_group.second)
            {
                printf(" >>>>>>>>>>>>> <%s> component >> <%s>  register_group <%s> map <%s>\n", __func__, myComp.first.c_str(), register_group.first.c_str(), myMap.first.c_str());
            }
        }
    }
    auto uri_keys = split_string(uri, '/');
    int num_uris = (int)uri_keys.size();
    for (auto key : uri_keys)
    {
        std::cout << " key : [" << key << "] "
                  << " size : " << key.size() << std::endl;
        if (key.front() == '_') num_uris--;
    }
    std::map<std::string, std::shared_ptr<cfg::io_point_struct>> map_result;

    // 3 keys /component/id
    // 4 keys  /component/id/var
    // gcom_config_any
    std::shared_ptr<cfg::io_point_struct> var_result;
    bool result = false;
    // if is single we'll find it here
    if (uri_keys.size() >= 4)
    {
        std::cout << "  looking for var  "
                  << "\n";
        result = ioPointExists(var_result, myCfg, uri_keys, num_uris, "");
        if (result)
        {
            std::cout << " var found " << var_result->id << "\n";
        }
        else
        {
            std::cout << " var NOT found "
                      << "\n";
        }
    }
    else
    { // this is a map
        try
        {
            // auto& rfoo =
            std::cout << "  looking for map  "
                      << "\n";
            result = test_findMapMap(map_result, myCfg, uri_keys);
            std::cout << " >>> map found "
                      << "\n";
        }
        catch (const std::out_of_range &)
        {
            std::cout << " >>> map NOT found "
                      << "\n";
        }
    }
    return (var_result != nullptr);
}

bool decode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg &myCfg, Uri_req &uri, const char *mode, bool debug)
{
    std::shared_ptr<IO_Work> iop;
    iop = make_work(io_point->register_type, io_point->device_id, io_point->offset, 1, io_point->reg16, io_point->reg8, strToWorkType(mode, false));
    work_vec.emplace_back(iop);
    return true;
}

bool uri_is_single(std::shared_ptr<cfg::io_point_struct> &io_point, struct cfg &myCfg, struct Uri_req &uri, bool debug)
{
    bool single = false;
    for (auto key : uri.uri_vec)
    {
        std::cout << " key : [" << key << "] "
                  << " size : " << key.size() << std::endl;
    }
    // 3 keys /component/id
    // 4 keys  /component/id/var
    // gcom_config_any
    // std::shared_ptr<cfg::io_point_struct> var_result;
    //    std::map<std::string, std::shared_ptr<cfg::io_point_struct>> map_result;
    // if is single we'll find it here
    if (uri.num_uris >= 4)
    {
        std::cout << "  looking for var  "
                  << "\n";
        single = ioPointExists(io_point, myCfg, uri.uri_vec,uri.num_uris, "");
        if (single)
        {
            std::cout << " io_point found " << io_point->id << "\n";
        }
        else
        {
            std::cout << " io_point NOT found "
                      << "\n";
        }
    }
    return single;
    // else { // this is a map
    //     try {
    //     //auto& rfoo =
    //     std::cout << "  looking for map  " << "\n";
    //     result = test_findMapMap(map_result, myCfg, uri_keys);
    //     std::cout << " >>> map found " << "\n";
    //     } catch ( const std::out_of_range&) {
    //         std::cout << " >>> map NOT found " << "\n";
    //     }
    // }
    return single;
}

// // Function to convert JSON to map (assuming you have this implemented)
// std::map<std::string, std::any> jsonToMap(const simdjson::dom::element& elem);
/*
/// @brief
       used to test the type of a std::any object
*/
template <typename T>
bool testAnyVal(const std::any &anyVal, const T &defaultValue)
{
    return anyVal.type() == typeid(T);
}

/*
/// @brief
  test code to test the bitstr extracct from a config
*/
bool gcom_test_bit_str()
{
    auto my_map = std::make_shared<cfg::io_point_struct>();
    std::vector<std::any> rawStringData = {
        std::string("Stopped"),
        std::map<std::string, std::any>{{"value", 1}, {"string", std::string("Running")}},
        std::map<std::string, std::any>{{"value", 4}, {"string", std::string("Paused")}},
        std::string("Fault"),
    };
    std::any data = rawStringData;
    extract_bitstrings(my_map, data); // Ensure extract_bitstrings is modified to accept shared_ptr
    std::cout << "Bit Strings " << std::endl;
    for (const auto &s : my_map->bit_str)
    { // Note the use of -> instead of . for accessing members
        std::cout << s << std::endl;
    }
    // std::cout <<"Bit Numbers " << std::endl;
    // for (const auto& s : my_map->bit_num) { // Note the use of -> instead of . for accessing members
    //     std::cout << s << std::endl;
    // }
    std::cout << "Bits Known "
              << "0x"
              << std::setfill('0')
              << std::setw(8)
              << std::hex << my_map->bits_known << std::endl; // Note the use of -> instead of . for accessing members
    std::cout << "Bits Unknown "
              << "0x"
              << std::setfill('0')
              << std::setw(8)
              << std::hex << my_map->bits_unknown << std::endl; // Note the use of -> instead of . for accessing members
    return true;
}

//// TODO complete this.
/// @brief given an io_point , a value and a vector of IO_Work objects , add this io_point to the vector assuyme a "set" operationm
/// @param work_vec  vector of IO_Work items
/// @param io_point  the io_point we are setting
/// @param val   the value we are using
/// @return true if all worked out
bool encode_io_point_struct(std::vector<std::shared_ptr<IO_Work>> &work_vec, std::shared_ptr<cfg::io_point_struct> io_point, std::any val, struct cfg &myCfg, Uri_req &uri, std::string &replyto, const char *mode, bool debug)
{
    debug = false;
    std::shared_ptr<IO_Work> io_work_single;
    // io_work_single->register_type = io_point->register_type;
    // io_work_single->offset = io_point->offset;
    // io_work_single->device_id = io_point->device_id;
    if (!io_point->is_enabled && !uri.is_local_request)
    {
        return false;
    }
    if (io_point->register_type == cfg::Register_Types::Coil)
    {
        if (debug)
        {
            std::cout << ">>>>" << __func__ << " regtype Coil " << std::endl;
        }
        io_work_single = make_work(io_point->register_type, io_point->device_id, io_point->offset, 1, io_point->reg16, io_point->reg8, strToWorkType(mode, false));
        // io_work_single->ip_address = myCfg.connection.ip_address;
        // io_work_single->port = myCfg.connection.port;

        io_work_single->replyto = replyto;
        if (uri.is_unforce_request)
        {
            io_point->is_forced = false;
        }
        auto bval = get_any_to_bool(io_point, val, uri, io_work_single->buf8);
        // io_point->reg8[0] = bval;
        // io_work_single->u8_buff[0] = bval;
        io_work_single->buf8[0] = bval;
        io_work_single->io_points.emplace_back(io_point);
        work_vec.emplace_back(io_work_single);
    }
    else if (io_point->register_type == cfg::Register_Types::Holding)
    {
        if (debug)
        {
            std::cout << ">>>>" << __func__ << " regtype Holding " << std::endl;
        }
        io_work_single = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->size, io_point->reg16, io_point->reg8, strToWorkType(mode, false));
        io_work_single->replyto = replyto;
        if (uri.is_unforce_request)
        {
            io_point->is_forced = false;
        }
        auto uval = get_any_to_uint64(io_point, val, uri, io_work_single->buf16);
        if (uri.is_force_request)
        {
            io_point->is_forced = true;
            io_point->forced_val = uval;
        }
        std::cout << ">>>>" << __func__ << " uval " << uval << std::endl;
        io_work_single->io_points.emplace_back(io_point);
        work_vec.emplace_back(io_work_single);
    }
    // work_vec.emplace_back(io_work_single);
    return true;
}

