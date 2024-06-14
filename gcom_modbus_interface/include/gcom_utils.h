#pragma once

#include <any>
#include <map>
#include <string>
#include <string_view>
#include <sstream>
#include <memory>

#include "shared_utils.h"
#include "gcom_config.h"

/**
 * @brief Print the "override" options to the screen for the user.
 *
 * Override options include anything that can supersede what is in the loaded config document (e.g. IP
 * address, port, debug, etc.)
 */
void gcom_show_overrides();

/**
 * @brief get the filename without the .json extension or any slashes.
 *
 * This is primarily used for the purpose of the logger.
 *
 * @param filename the filename with slashes preceding it and a file extension
 * @return the filename without any preceding slashes or file extensions
 */
std::string removeSlashesAndExtension(std::string filename);

/**
 * @brief Parse command line arguments into the various override structures.
 *
 * Override options include anything that can supersede what is in the loaded config document (e.g. IP
 * address, port, debug, etc.) This function should be called AFTER a call to gcom_load_cfg_file.
 *
 * @param myCfg config structure to load override data into. Should be pre-populated with config file information.
 * @param next_arg the command line argument to start at. Will vary depending on if a config file flag is used.
 * @param argc the number of arguments passed to main()
 * @param argv a vector of each of the arguments passed to main()
 */
bool gcom_load_overrides(struct cfg& myCfg, int next_arg, const int argc, const char* argv[]);

// Function to detect the data type of the value
std::any detectTypeAndCast(const std::string& value);

std::map<std::string, std::any> parseQueryString(const std::string& query);

bool filter_io_point(int argc, const std::shared_ptr<struct cfg::io_point_struct> point, const std::any& crit);

std::any runParseQuery(std::string& query);

/**
 * @brief Check if a string is empty, contains forbidden characters, or is larger than a given size.
 *
 * @param str the string to check
 * @param Forbidden_Chars the characters that cannot be in the string; defaults are curly braces, slashes, spaces,
 * double quotes, and percent signs.
 * @param Max_Str_Size the maximum length of the string; default is 256
 */
// std::string check_str_for_error(const std::string_view str, const std::string_view Forbidden_Chars, const std::size_t
// Max_Str_Size);
std::string check_str_for_error(const std::string_view str, const std::string_view Forbidden_Chars = R"({}\/ "%)",
                                const std::size_t Max_Str_Size = std::numeric_limits<u8>::max());

/**
 * @brief Convert a string to a host IP address using the /etc/hosts file or a valid IP address.
 *
 * @param hostname the string to look for in /etc/hosts or to convert to an IP address
 * @param ip an empty char buffer to store the IP address
 * @param iplen the maximum length of the IP address
 */
int hostname_to_ip(std::string_view hostname, char* ip, int iplen);

/**
 * @brief Convert a service string to a port number.
 *
 * Uses a call to getservbyname() to look into the services database (typically /etc/services)
 * to match a service (e.g. TCP, FTP, HTTP) to a port number.
 *
 * @param service the service to look for in /etc/services
 * @param port a reference to the port number to populate
 */
bool service_to_port(std::string_view service, int& port);

/*
/// @brief
            split a string_view object into a vector seperated by the delimiter
*/
std::vector<std::string> any_split(std::string_view str, char delimiter);

void replace_space_with_underscores(std::string& str);

std::vector<std::string> split_string(const std::string& str, char delimiter);

std::string addQuote(const std::string& si);

std::string workTypeToStr(WorkTypes& work_type);

cfg::Register_Types strToRegType(std::string& register_type_str);

std::string regTypeToStr(cfg::Register_Types& register_type);

WorkTypes strToWorkType(std::string roper, bool debug);

void randomDelay(int minMicroseconds, int maxMicroseconds);
