/**
 * Library of time and functions shared by various HybridOS components
 * More functions will be added as appropriate
 *
 * Created on: January 28, 2021
 *      Author: jnshade
 */

#ifndef SITE_CONTROLLER_UTILS_H_
#define SITE_CONTROLLER_UTILS_H_

/* C Standard Library Dependencies */
#include <time.h>
/* Internal Dependencies */
#include <macros.h>
#include <Fims_Object.h>
#include <Logger.h>
#include <fims/libfims.h>
/* External Dependencies */
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/compile.h>

// string functions
template <typename... Args>
std::string string_format(const std::string& format, Args... args);
std::vector<std::string> split(std::string str, std::string delim);
bool has_suffix(const char* str, const char* suffix);
bool trim_suffix_if_exists(std::string& str, const char* suffix);

// Time Functions
bool check_expired_time(timespec current_time, timespec target_time);
long int get_elapsed_time(timespec current_time, timespec target_time);
double get_raw_timestamp(void);
void increment_timespec_ms(timespec& time_struct, int ms_inc);
std::string seconds_to_timer_string(int seconds_left, int start_value);

// Event Functions
// Send the event using this fims instance
void emit_event(const char* source, const char* message, int severity);

// Logic Functions
int get_int_from_bit_position(int integer);
bool get_tolerance(float tolerance_value, float actual_value, int tolerance_percent);
void set_curve_points(Fims_Object* curve_points, std::vector<std::pair<float, float>>& curve);
float get_curve_cmd(float, std::vector<std::pair<float, float>>& curve);
float range_check(float value, float max, float min);
int range_check(int value, int max, int min);
float zero_check(float value);
float less_than_zero_check(float value);
void capture_stdout();
void release_stdout(bool print);
bool near(float a, float b, float tolerance);
bool greater_than_or_near(float a, float b, float tolerance);
bool less_than_or_near(float a, float b, float tolerance);

// FIMS
void free_fims_msg(fims_message* msg);
bool send_buffer_to(const char* uri, fmt::memory_buffer& buff);

// cJSON functions
cJSON* clothe_naked_cJSON(cJSON* object);
cJSON* grab_naked_or_clothed(cJSON& fimsbody, cJSON* object, const char* name);
cJSON* grab_naked_or_clothed_and_check_type(cJSON& fimsbody, cJSON* object, int CJSONType, const char* name);
bool is_naked(cJSON* object);

/**
 * @brief Macro for formatting arguments and appending them to an fmt string buffer
 * @param fmt_buf The string buffer we are adding to.
 * @param fmt_str The string which determines how arguments are formatted.
 * @param ... Variadic arguments to be formatted.
 * @return An output iterator (type OutputIt) for adding more to the buffer, this iterator is generally not used.
 */
#define FORMAT_TO_BUF(fmt_buf, fmt_str, ...) fmt::format_to(std::back_inserter(fmt_buf), FMT_COMPILE(fmt_str), __VA_ARGS__)

#define FORMAT_TO_BUF_C_STRING(fmt_buf, fmt_str, ...)                                                                                                                                                                                                    \
    fmt::format_to(std::back_inserter(fmt_buf), fmt_str, __VA_ARGS__);                                                                                                                                                                                   \
    fmt_buf.push_back('\0');

/**
 * @brief Formats a bool item to JSON and appends it to an fmt string buffer. Uses a trailing comma.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 */
inline void bufJSON_AddBool(fmt::memory_buffer& fmt_buf, const char* const name, const bool value) {
    FORMAT_TO_BUF(fmt_buf, R"("{}":{},)", name, value);
}

/**
 * @brief Alternative bufJSON_AddBool used when we might only want to add the value and only of a particular var.
 *  If var is null, it calls the simpler bufJSON_AddBool. If var doesn't match name, we do nothing.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 * @param var The particular variable we want, or possibly null.
 */
inline void bufJSON_AddBoolCheckVar(fmt::memory_buffer& fmt_buf, const char* const name, const bool value, const char* const var) {
    if (var == NULL)
        bufJSON_AddBool(fmt_buf, name, value);
    else if (strcmp(name, var) == 0)
        FORMAT_TO_BUF(fmt_buf, R"({})", value);
}

/**
 * @brief Formats a number item to JSON and appends it to an fmt string buffer. Uses a trailing comma.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 */
inline void bufJSON_AddNumber(fmt::memory_buffer& fmt_buf, const char* const name, const double value) {
    FORMAT_TO_BUF(fmt_buf, R"("{}":{},)", name, value);
}

/**
 * @brief Alternative bufJSON_AddNumber used when we might only want to add the value and only of a particular var.
 *  If var is null, it calls the simpler bufJSON_AddNumber. If var doesn't match name, we do nothing.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 * @param var The particular variable we want, or possibly null.
 */
inline void bufJSON_AddNumberCheckVar(fmt::memory_buffer& fmt_buf, const char* const name, const double value, const char* const var) {
    if (var == NULL)
        bufJSON_AddNumber(fmt_buf, name, value);
    else if (strcmp(name, var) == 0)
        FORMAT_TO_BUF(fmt_buf, R"({})", value);
}

/**
 * @brief Formats a string item to JSON and appends it to an fmt string buffer. Uses a trailing comma.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 */
inline void bufJSON_AddString(fmt::memory_buffer& fmt_buf, const char* const name, const char* const value) {
    FORMAT_TO_BUF(fmt_buf, R"("{}":"{}",)", name, value);
}

/**
 * @brief Alternative bufJSON_AddString used when we might only want to add the value and only of a particular var.
 *  If var is null, it calls the simpler bufJSON_AddString. If var doesn't match name, we do nothing.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The name of the item we are adding.
 * @param value The value of the item we are adding.
 * @param var The particular variable we want, or possibly null.
 */
inline void bufJSON_AddStringCheckVar(fmt::memory_buffer& fmt_buf, const char* const name, const char* const value, const char* const var) {
    if (var == NULL)
        bufJSON_AddString(fmt_buf, name, value);
    else if (strcmp(name, var) == 0)
        FORMAT_TO_BUF(fmt_buf, R"("{}")", value);
}

/**
 * @brief Adds an identifier to a JSON buffer, used to name a following object or array.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The identifier we are adding.
 */
inline void bufJSON_AddId(fmt::memory_buffer& fmt_buf, const char* const name) {
    FORMAT_TO_BUF(fmt_buf, R"("{}":)", name);
}

/**
 * @brief Adds an identifier to a JSON buffer, used to name a following object or array.
 * @param fmt_buf The string buffer we are adding to.
 * @param name The identifier we are adding.
 */
inline void bufJSON_AddId(fmt::memory_buffer& fmt_buf, const std::string& name) {
    FORMAT_TO_BUF(fmt_buf, R"("{}":)", name.c_str());
}

/**
 * @brief Adds the start of a JSON object to an fmt string buffer: '{'
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_StartObject(fmt::memory_buffer& fmt_buf) {
    fmt_buf.push_back('{');
}

/**
 * @brief Checks if there is a trailing comma at the end of the buffer and removes the comma if found,
 *  Then adds the end of a JSON object to the buffer: "},"
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_EndObject(fmt::memory_buffer& fmt_buf) {
    if (fmt_buf[fmt_buf.size() - 1] == ',')
        fmt_buf.resize(fmt_buf.size() - 1);
    FORMAT_TO_BUF(fmt_buf, "{}", "},");
}

/**
 * @brief Checks if there is a trailing comma at the end of the buffer and removes the comma if found,
 *  Then adds the end of a JSON object to the buffer: "}"
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_EndObjectNoComma(fmt::memory_buffer& fmt_buf) {
    if (fmt_buf[fmt_buf.size() - 1] == ',')
        fmt_buf.resize(fmt_buf.size() - 1);
    FORMAT_TO_BUF(fmt_buf, "{}", "}");
}

/**
 * @brief Adds the start of a JSON array to an fmt string buffer: '['
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_StartArray(fmt::memory_buffer& fmt_buf) {
    fmt_buf.push_back('[');
}

/**
 * @brief Checks if there is a trailing comma at the end of the buffer and removes the comma if found,
 *  Then adds the end of a JSON array to the buffer: "],"
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_EndArray(fmt::memory_buffer& fmt_buf) {
    if (fmt_buf[fmt_buf.size() - 1] == ',')
        fmt_buf.resize(fmt_buf.size() - 1);
    FORMAT_TO_BUF(fmt_buf, "{}", "],");
}

/**
 * @brief Checks if the buffer is empty, then checks if there is a trailing comma at the end of the buffer
 *  and removes the comma if found. This function should be called before sending the buffer contents to
 *  ensure the contents are valid JSON.
 * @param fmt_buf The string buffer we are modifying
 */
inline void bufJSON_RemoveTrailingComma(fmt::memory_buffer& fmt_buf) {
    if (fmt_buf.size() > 0 && fmt_buf[fmt_buf.size() - 1] == ',')
        fmt_buf.resize(fmt_buf.size() - 1);
}

/**
 * @brief Adds a comma to the end of the JSON buffer.
 * @param fmt_buf The string buffer we are adding to.
 */
inline void bufJSON_AddComma(fmt::memory_buffer& fmt_buf) {
    FORMAT_TO_BUF(fmt_buf, "{}", ",");
}

#endif /* SITE_CONTROLLER_UTILS_H_ */
